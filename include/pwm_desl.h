#ifndef PWM_DESL_H
#define PWM_DESL_H

class Pwm_DESL {
public:
    Pwm_DESL();
    
    void stop_bipolar_unipolar_pwm();
    void run_bipolar_pwm();
    void run_unipolar_pwm();
};

#endif