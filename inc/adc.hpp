//
// Created by ShiYuanLi on 10/29/2023.
//

#ifndef PICO_BLINK_ADC_HPP
#define PICO_BLINK_ADC_HPP

#include "hardware/adc.h"
#include "dma.hpp"

class Analog
{
private:
    uint current_channel = 0xFFFF;
    DMA* AD = nullptr;

public:
    explicit Analog(float div)
    {
        adc_init();
        adc_set_clkdiv(div);

        adc_fifo_setup(
                true,    // Write each completed conversion to the sample FIFO
                true,    // Enable DMA data request (DREQ)
                1,       // DREQ (and IRQ) asserted when at least 1 sample present
                false,   // We won't see the ERR bit because of 8 bit reads; disable.
                false     // Shift each sample to 8 bits when pushing to FIFO
        );

        AD = new DMA(&(adc_hw->fifo), nullptr, 0, DMA_SIZE_16);
        AD->setIncrement(true, false);
        AD->setIncrement(false, true);

        AD->setDataRequests(DREQ_ADC);
        AD->FlushConfigure(false);
    }

    void selectInput(uint ch)
    {
        if(current_channel != 0xFFFF)
        {
            gpio_deinit(26 + ch);
        }
        adc_gpio_init(26 + ch);
        adc_select_input(ch);
        current_channel = ch;
    }

    uint getInput() const
    {
        return current_channel;
    }

    uint getSingle() const
    {
        uint r = adc_read();
        adc_fifo_drain();
        return r;
    }

    void getBlock(uint16_t* buffer, uint cc) const
    {
        adc_run(true);
        AD->setDstBuffer(buffer);
        AD->Begin(cc);
        AD->WaitForFinish();
        adc_run(false);
        adc_fifo_drain();
    }
};

#endif //PICO_BLINK_ADC_HPP
