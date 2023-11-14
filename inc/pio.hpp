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
typedef void (*additional_init_t)(PIO, uint, uint);

struct PinConfig
{
    uint32_t     out_pin_count = 0;
    uint32_t     in_pin_enable = 0;
    uint32_t     set_pin_count = 0;
    uint32_t   side_pin_enable = 0;

    uint32_t          out_base = 0;
    uint32_t           in_base = 0;
    uint32_t          set_base = 0;
    uint32_t         side_base = 0;

    uint32_t          pin_mask = 0;
    uint32_t          dir_mask = 0;
};

struct IORConfigs
{
    bool         osr_dir = false;
    bool    osr_auto_pull = true;
    uint   osr_bit_for_pull = 32;

    bool         isr_dir = false;
    bool    isr_auto_push = true;
    uint   isr_bit_for_push = 32;
};

class ProgrammableIO
{
private:
    PIO pio;
    uint sm;
    uint offset;
    pio_program_t *pro = nullptr;

    PinConfig pins{ 0 };

    pio_sm_config c { 0 };

    DMA* pioDMA = nullptr;
public:
    explicit ProgrammableIO(
            PIO pio, const pio_program_t* pro, PinConfig Pins,
            get_default_config_type default_config,
            additional_init_t additional_init = nullptr,
            uint fifo_join = MX_PIO_FIFOJOIN_NONE, float clockDiv = 128,
            IORConfigs IOR = { false, true, 32, false, true, 32 },
            int init_status = -1 // -1: dot't set init status
                    )
    {
        this->pio = pio;
        this->pro = (pio_program_t*)pro;
        this->pins = Pins;

        this->offset = pio_add_program(pio, pro);
        this->sm = pio_claim_unused_sm(pio, true);

        c = default_config(offset);

        // 1. Set out/in/set/side pin maps

        if(Pins.in_pin_enable)
            sm_config_set_in_pins(&c, Pins.in_base);
        if(Pins.out_pin_count > 0)
            sm_config_set_out_pins(&c, Pins.out_base, Pins.out_pin_count);
        if(Pins.set_pin_count > 0)
            sm_config_set_set_pins(&c, Pins.set_base, Pins.set_pin_count);
        if(Pins.side_pin_enable)
            sm_config_set_sideset_pins(&c, Pins.side_base);

        // 2. Pio gpio init

        for(uint32_t i = 0; i < 32 ; i++)
        {
            uint32_t p = 1u << i;
            if(Pins.pin_mask & p)
            {
                pio_gpio_init(pio, i);
            }
        }

        // 3. (Optional) Set pins with mask

        if(init_status != -1)
            pio_sm_set_pins_with_mask(pio, sm, init_status, Pins.pin_mask);

        // 4. Set pin dir with mask

        pio_sm_set_pindirs_with_mask(pio, sm, Pins.dir_mask, Pins.pin_mask);

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

        if(additional_init)additional_init(pio, sm, offset);
    }

    ~ProgrammableIO()
    {
        pio_sm_unclaim(pio, sm);
        pio_remove_program(pio, pro, offset);

        for(uint32_t i = 0; i < 32 ; i++)
        {
            uint32_t p = 1u << i;
            if(pins.pin_mask & p)
            {
                gpio_deinit(i);
            }
        }
    }

    void doInit()
    {
        pio_sm_init(pio ,sm, offset, &c);
        setEnabled(true);
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
