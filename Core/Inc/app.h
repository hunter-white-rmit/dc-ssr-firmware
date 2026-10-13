/*

    60 V / 45 A DC solid-state relay, OENG1168 Capstone
    Target: NUCLEO-F042K6 (STM32F042K6T6), STM32Cube HAL

    Operator control: S501 on PF0 (SWITCH), polled and debounced.
        S501 ON     close relay (precharge -> handover -> on)
        S501 OFF    open relay
        Reset btn   clears latched faults (MCU reset)

    After any reset S501 must be seen OFF before the relay will close, so a
    switch left ON does not re-close the relay straight after a fault reset.

    Heartbeat LED (PB3): blinks while running, held off when faulted.

*/

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>

/* ------------------------------------------------------------------------ */
/* Latched fault flags (bitmask). Any bit set -> relay open, STATE_FAULT.   */
/* Faults only latch while the relay is energised (PRECHARGE/HANDOVER/ON);  */
/* while idle, an unhealthy input simply holds the relay open.              */
/* Cleared only by an MCU reset.                                            */
/* ------------------------------------------------------------------------ */
#define FAULT_OVERCURRENT       (1u << 0)   /* CCFLT low: TPSI3103 FLT1 trip (~60 A) */
#define FAULT_INPUT_OV          (1u << 1)   /* INCMP1 low: input bus > 65.8 V        */
#define FAULT_INPUT_UV          (1u << 2)   /* INCMP2 low: input bus < 48.6 V        */
#define FAULT_CC_SUPPLY         (1u << 3)   /* CCPGOOD lost while energised          */
#define FAULT_PRE_SUPPLY        (1u << 4)   /* PREPGOOD lost while energised         */
#define FAULT_PRECHARGE_TIMEOUT (1u << 5)   /* output never reached handover level   */
#define FAULT_OUTPUT_COLLAPSE   (1u << 6)   /* OUTCMP2 fell below ~45 V while on     */

// Warning flags: never trips. Visible via debugger, cleared by MCU reset
#define WARN_OC_ALARM           (1u << 0)   /* CCALM low: TPSI3103 ALM1 (~50 A)      */

/* ------------------------------------------------------------------------ */
/* Relay states                                                             */
/* ------------------------------------------------------------------------ */
typedef enum {
    STATE_INIT = 0,     /* power-up / switch-off: qualifying inputs, relay open   */
    STATE_OFF,          /* inputs healthy, relay open, waiting for S501 ON        */
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
