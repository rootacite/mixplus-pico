//
// Created by ShiYuanLi on 10/28/2023.
//

#ifndef PICO_BLINK_PIO_HPP
#define PICO_BLINK_PIO_HPP

// This page contains many BUGs
// PIO is hard for encapsulation
// forgive me :)

#include "dma.hpp"

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "pico/types.h"

#define MX_PIO_IN        0
#define MX_PIO_OUT       1
#define MX_PIO_SET       2
#define MX_PIO_SIDESET   4

#define MX_PIO_FIFOJOIN_NONE   0
#define MX_PIO_FIFOJOIN_TX     1
#define MX_PIO_FIFOJOIN_RX     2

typedef pio_sm_config (*get_default_config_type)(uint);

struct PinConfig
{
    uint pin_name;
    bool io; // true for input
    uint pin_type;
};

struct IORConfigs
{
    bool osr_dir;
    bool osr_auto_pull;
    uint osr_bit_for_pull;

    bool isr_dir;
    bool isr_auto_push;
    uint isr_bit_for_push;
};

class ProgrammableIO
{
private:
    PIO pio;
    uint sm;
    uint offset;
    pio_program_t *pro = nullptr;

    PinConfig* pins = nullptr;
    uint countOfPins;

    pio_sm_config c {};

    DMA* pioDMA = nullptr;
public:
    explicit ProgrammableIO(
            PIO pio, const pio_program_t* pro, PinConfig* Pins,
            uint countOfPins, get_default_config_type default_config,
            uint fifo_join = MX_PIO_FIFOJOIN_NONE, float clockDiv = 128,
            IORConfigs IOR = { false, true, 32, false, true, 32 }
                    )
    {
        this->pio = pio;
        this->pro = (pio_program_t*)pro;
        this->pins = new PinConfig[countOfPins];
        this->countOfPins = countOfPins;

        this->offset = pio_add_program(pio, pro);
        this->sm = pio_claim_unused_sm(pio, true);

        c = default_config(offset);

        for(int i = 0 ; i < countOfPins ; i++)
        {
            switch(Pins[i].pin_type)
            {
                case MX_PIO_IN:
                    sm_config_set_in_pins(&c, Pins[i].pin_name);
                    break;
                case MX_PIO_OUT:
                    sm_config_set_out_pins(&c, Pins[i].pin_name, 1);
                    break;
                case MX_PIO_SET:
                    sm_config_set_set_pins(&c, Pins[i].pin_name, 1);
                    break;
                case MX_PIO_SIDESET:
                    sm_config_set_sideset_pins(&c, Pins[i].pin_name);
                    break;
                default:
                    break;
            }

            pio_sm_set_pins(pio, sm, Pins[i].pin_name);
            pio_sm_set_consecutive_pindirs(pio, sm, Pins[i].pin_name, 1, !Pins[i].io);
            pio_gpio_init(pio, Pins[i].pin_name);

            this->pins[i] = Pins[i];
        }

        switch(fifo_join)
        {
            case MX_PIO_FIFOJOIN_NONE:
                break;
            case MX_PIO_FIFOJOIN_TX:
                sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
                break;
            case MX_PIO_FIFOJOIN_RX:
                sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
                break;
            default:
                break;
        }

        sm_config_set_clkdiv(&c, clockDiv);
        if(fifo_join != MX_PIO_FIFOJOIN_RX)
            sm_config_set_out_shift(&c, IOR.osr_dir, IOR.osr_auto_pull, IOR.osr_bit_for_pull);
        if(fifo_join != MX_PIO_FIFOJOIN_TX)
            sm_config_set_in_shift(&c, IOR.isr_dir, IOR.isr_auto_push, IOR.isr_bit_for_push);
    }

    ~ProgrammableIO()
    {
        pio_sm_unclaim(pio, sm);
        pio_remove_program(pio, pro, offset);

        for(int i = 0 ; i < countOfPins ; i++)
        {
            gpio_deinit(pins[i].pin_name);
        }
    }

    void doInit()
    {
        pio_sm_init(pio ,sm, offset, &c);
    }

    void setEnabled(bool e)
    {
        pio_sm_set_enabled(pio, sm, e);
    }

    void put(uint32_t data)
    {
        pio_sm_put_blocking(pio, sm, data);
    }

    void initDMA(dma_channel_transfer_size manual_size)
    {
        pioDMA = new DMA(nullptr, &pio->txf[sm], 0, manual_size);

        pioDMA->setIncrement(true, true);
        pioDMA->setIncrement(false, false);
        pioDMA->setDataRequests(pio_get_dreq(pio, sm, true));
        pioDMA->FlushConfigure();
    }

    void putDMA(void* buf, uint cc)
    {
        pioDMA->setSrcBuffer(buf);
        pioDMA->Begin(cc);
        pioDMA->WaitForFinish();
    }
};

#endif //PICO_BLINK_PIO_HPP
