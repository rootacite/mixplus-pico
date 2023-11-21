//
// Created by acite on 23-11-22.
//

#ifndef PICO_DEMO2_SERVO_HPP
#define PICO_DEMO2_SERVO_HPP

#include "hardware/pwm.hpp"
#include "hardware/clocks.h"
#include <vector>

static std::vector<uint> used_slice(0);

class Servo
{
private:
    uint slice = 0;
    PWM* pwm = nullptr;
    uint pin = 0;

public:
    explicit Servo(uint pin)
    {

        slice = pwm_gpio_to_slice_num(pin);
        PWM::initPin(pin);
        this->pin = pin;
        pwm = new PWM({
            .slice = slice,
            .wrap = 65535,
            .clk_div = (clock_get_hz(clk_sys) / 65535) / (float)50
        });
        if(std::find(used_slice.begin(), used_slice.end(), slice) == used_slice.end())
        {
            pwm->Start();
        }

        SetValue(0.5);
    }

    void SetValue(float v) const
    {
        // min : 1639, max : 8191, diff : 6552
        pwm->setLevel(pin, (uint16_t)(1639 + 6552 * v));
    }
};

#endif //PICO_DEMO2_SERVO_HPP
