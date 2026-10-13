/*

    60 V / 45 A DC solid-state relay, OENG1168 Capstone
    Target: NUCLEO-F042K6 (STM32F042K6T6), STM32Cube HAL

    Headless operation, no serial link.

    Operator control: S501 on PF0 (SWITCH), polled and debounced.
        S501 ON     close relay (precharge -> handover -> on)
        S501 OFF    open relay
        S502        MCU reset: the only way to clear a latched fault

    Closing needs a fresh OFF -> ON of S501 while the relay is ready
    (inputs healthy, STATE_OFF). A switch left ON after a reset, or turned
    ON while the inputs were unhealthy, does nothing until it is cycled.

    Heartbeat LED (PB3 / LD3):
        Running     toggles every 100 ms
        Glitch      two quick blinks then a pause, three times, whenever an
                    OV/UV/PGOOD line glitches without tripping (see below)
        Faulted     toggles every N seconds, where N identifies the FIRST
                    fault that latched (see the fault list below)

*/

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>

/* ------------------------------------------------------------------------ */
/* Latched fault flags (bitmask). Any bit set -> relay open, STATE_FAULT.   */
/* Faults only latch while the relay is energised (PRECHARGE/HANDOVER/ON);  */
/* while idle, an unhealthy input simply holds the relay open.              */
/* Cleared only by an MCU reset (S502).                                     */
/* ------------------------------------------------------------------------ */
/* Heartbeat toggle period in fault = (bit index + 1) seconds, shown at right. */
#define FAULT_OVERCURRENT       (1u << 0)   /* 1 s  CCFLT low: TPSI3103 FLT1 trip (~60 A) */
#define FAULT_INPUT_OV          (1u << 1)   /* 2 s  INCMP1 low: input bus > 65.8 V        */
#define FAULT_INPUT_UV          (1u << 2)   /* 3 s  INCMP2 low: input bus < 48.6 V        */
#define FAULT_CC_SUPPLY         (1u << 3)   /* 4 s  CCPGOOD lost while energised          */
#define FAULT_PRE_SUPPLY        (1u << 4)   /* 5 s  PREPGOOD lost while energised         */
#define FAULT_PRECHARGE_TIMEOUT (1u << 5)   /* 6 s  output never reached handover level   */
#define FAULT_OUTPUT_COLLAPSE   (1u << 6)   /* 7 s  OUTCMP2 fell below ~45 V while on     */

/* ------------------------------------------------------------------------ */
/* Warning flags: never trip. Readable with the debugger (app_get_warnings) */
/* and cleared by MCU reset.                                                */
/* ------------------------------------------------------------------------ */
#define WARN_OC_ALARM           (1u << 0)   /* CCALM low: TPSI3103 ALM1 (~50 A)      */

/* Glitch warnings: the line went bad but recovered within CONFIRM_US, so no
   trip. Same bit positions as the matching fault. Each new glitch also
   triggers the double-blink pattern on the heartbeat LED.                  */
#define WARN_GLITCH_INPUT_OV    FAULT_INPUT_OV
#define WARN_GLITCH_INPUT_UV    FAULT_INPUT_UV
#define WARN_GLITCH_CC_SUPPLY   FAULT_CC_SUPPLY
#define WARN_GLITCH_PRE_SUPPLY  FAULT_PRE_SUPPLY

/* ------------------------------------------------------------------------ */
/* Relay states                                                             */
/* ------------------------------------------------------------------------ */
typedef enum {
    STATE_INIT = 0,     /* power-up / switch-off: qualifying inputs, relay open   */
    STATE_OFF,          /* inputs healthy, relay open, waiting for S501 OFF -> ON */
    STATE_PRECHARGE,    /* PREEN high, output capacitance charging                */
    STATE_HANDOVER,     /* CCEN and PREEN both high for a short overlap           */
    STATE_ON,           /* CCEN high, main path conducting                        */
    STATE_FAULT         /* latched fault, relay open, waiting for MCU reset       */
} relay_state_t;

/* ------------------------------------------------------------------------ */
/* API                                                                      */
/* ------------------------------------------------------------------------ */
void          app_init(void);       /* call once, after all MX_..._Init()   */
void          app_loop(void);       /* call on every pass of while(1)       */

relay_state_t app_get_state(void);
uint32_t      app_get_faults(void);
uint32_t      app_get_warnings(void);

#endif /* APP_H */
