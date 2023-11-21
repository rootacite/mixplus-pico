//
// Created by acite on 23-11-19.
//

#ifndef PICO_DEMO2_PWM_HPP
#define PICO_DEMO2_PWM_HPP

#include "hardware/pwm.h"

struct PWMDefines
{
    uint slice;

    uint16_t wrap;
    float clk_div;
};

class PWM
{
private:
    PWMDefines Defines {  };
    pwm_config config {  };

public:
    static void initPin(uint pin)
    {
        gpio_set_function(pin, GPIO_FUNC_PWM);
    }

public:
    explicit PWM(PWMDefines Defines)
    {
        this->Defines = Defines;

        config = pwm_get_default_config();
        pwm_config_set_clkdiv(&config, Defines.clk_div);
        pwm_config_set_wrap(&config, Defines.wrap);
    }

    void Start()
    {
        // Load the configuration into our PWM slice, and set it running.
        pwm_init(Defines.slice, &config, true);
    }

    void setLevel(uint pin, uint16_t level) const
    {
        pwm_set_gpio_level(pin, level);
    }

    void setDiv(float div) const
    {
        pwm_set_clkdiv(Defines.slice, div);
    }
};

#endif //PICO_DEMO2_PWM_HPP
