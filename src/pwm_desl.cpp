#include "pwm_desl.h"
#include <Arduino.h>

Pwm_DESL::Pwm_DESL() {}

extern volatile bool bipolar_pwm_state;  // global
extern volatile bool unipolar_pwm_state ;  // global 

void Pwm_DESL::stop_bipolar_unipolar_pwm() {
    bipolar_pwm_state = false;
    unipolar_pwm_state = false;
    Serial.printf("OFF. \n");
    Serial.println("---");
}

void Pwm_DESL::run_bipolar_pwm() {
    bipolar_pwm_state = true;
    unipolar_pwm_state = false;
    Serial.printf("Bipolar PWM On. \n");
    Serial.println("---");
}

void Pwm_DESL::run_unipolar_pwm() {
    bipolar_pwm_state = false;
    unipolar_pwm_state = true;
    Serial.printf("Unipolar PWM On. \n");
    Serial.println("---");
}
