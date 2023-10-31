//
// Created by ShiYuanLi on 10/28/2023.
//

#ifndef PICO_BLINK_GPIO_HPP
#define PICO_BLINK_GPIO_HPP

#include "pico/stdlib.h"

#define MX_PULLS_NONE 0x00
#define MX_PULLS_UP   0x01
#define MX_PULLS_DOWN 0x02

class GPIO
{
private:
    uint pin;

public:
    static void setIRQCallback(gpio_irq_callback_t callback)
    {
        if(!callback)
        {
            irq_set_enabled(IO_IRQ_BANK0, false);
            return;
        }

        gpio_set_irq_callback(callback);
        irq_set_enabled(IO_IRQ_BANK0, true);
    }

public:
    explicit GPIO(uint pin, bool out, uint8_t pulls = MX_PULLS_NONE)
    {
        this->pin = pin;

        gpio_init(pin);
        gpio_set_dir(pin, out);

        gpio_set_pulls(
                pin,
                pulls & MX_PULLS_UP,
                pulls & MX_PULLS_DOWN
                );
    }

    void put(bool c) const
    {
        gpio_put(pin, c);
    }

    bool get() const
    {
        return gpio_get(pin);
    }

    void toggle() const
    {
        gpio_put(pin, !get());
    }

    void setEventEnable(uint32_t events, bool enabled) const
    {
        gpio_set_irq_enabled(pin, events, enabled);
    }
};

#endif //PICO_BLINK_GPIO_HPP
