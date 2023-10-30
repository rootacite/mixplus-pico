//
// Created by ShiYuanLi on 10/26/2023.
//

#ifndef PICO_BLINK_TIMER_HPP
#define PICO_BLINK_TIMER_HPP

#include "pico/stdlib.h"

#define MP_TIMER_IN_MS 0
#define MP_TIMER_IN_US 1

class Timer
{
public:
    virtual void setFreq(int32_t tp, uint tf = MP_TIMER_IN_MS) = 0;
    virtual void setUserData(void* ud) = 0;
    virtual void setEnabled(bool enabled) = 0;
};


class RepeatingTimer : public Timer
{ // inherit from timer
private:
    repeating_timer timer {0};
    uint time_format = MP_TIMER_IN_MS;
    int32_t time_span = 1000;

    repeating_timer_callback_t cb = nullptr;
    void* userData = nullptr;

public:

    explicit RepeatingTimer(int32_t tp, repeating_timer_callback_t b, uint tf = MP_TIMER_IN_MS, void* ud = nullptr)
    {
        this->time_format = tf;
        this->time_span = tp;
        setCallback(b);
        this->userData = ud;
    }

    void setFreq(int32_t tp, uint tf = MP_TIMER_IN_MS) override
    {
        this->time_format = tf;
        this->time_span = tp;
    }

    void setCallback(repeating_timer_callback_t b)
    {
        this->cb = b;
    }

    void setUserData(void* ud) override
    {
        this->userData = ud;
    }

    void setEnabled(bool enabled) override
    {
        if(!this->cb)return;

        if(enabled)
        {
            switch(this->time_format)
            {
                case MP_TIMER_IN_MS:
                    add_repeating_timer_ms(this->time_span, this->cb, this->userData, &this->timer);
                    break;
                case MP_TIMER_IN_US:
                    add_repeating_timer_us(this->time_span, this->cb, this->userData, &this->timer);
                    break;
                default:
                    break;
            }
        }
        else
        {
            cancel_repeating_timer(&this->timer);
        }
    }

};

#endif //PICO_BLINK_TIMER_HPP
