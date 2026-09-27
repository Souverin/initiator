#ifndef FSM_H
#define FSM_H

#include "types.h"
#include "config.h"
#include "VoltageMonitor.h"
#include "ButtonHandler.h"
#include "DummySensor.h"
#include "IndicatorModule.h"
#include "PWMTrigger.h"

class StateMachine {
public:
    StateMachine();

    void initHardware();
    void update(); 

    bool commandStart();
    bool commandStop();
    bool commandFire();

    void triggerFailSafe(FailSafeReason reason);

    SystemState getCurrentState() const { return currentState; }
    InitState getInitState() const { return initState; }
    FailSafeReason getFailSafeReason() const { return failReason; }
    unsigned long getArmingRemainingTimeMs() const;
    float getSystemVoltage() const { return voltageMonitor.getVoltage(); }

    // Методи тестування / Fault Injection
    void injectSensorFault() { 
        dummySensor.injectError(true); 
        update(); 
    }
    
    void injectVoltageFault(float v) { 
        voltageMonitor.simulateVoltage(v); 
        update(); // Примусова перевірка умов переходу у FAIL_SAFE в тому ж такті
    }

    void resetFaults() {
        voltageMonitor.resetSimulation();
        dummySensor.injectError(false);
    }

private:
    SystemState currentState;
    InitState initState;
    FailSafeReason failReason;

    unsigned long armingStartMs;
    unsigned long firingStartMs;

    VoltageMonitor voltageMonitor;
    ButtonHandler physicalButton;
    DummySensor dummySensor;
    IndicatorModule indicator;
    PWMTrigger pwmTrigger;

    void handleInit();
    void handleSafe();
    void handleArming();
    void handleArmed();
    void handleFiring();
    void handleFailSafe();

    void checkContinuousSafety();

    bool canTransitionTo(SystemState newState) const;
    void transitionTo(SystemState newState);
};

#endif // FSM_H