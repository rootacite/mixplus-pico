//
// Created by ShiYuanLi on 10/27/2023.
//

#ifndef PICO_BLINK_DMA_HPP
#define PICO_BLINK_DMA_HPP

#include "pico/stdlib.h"
#include "hardware/dma.h"

class DMA
{
private:
    int dma_channel = 0;
    dma_channel_config c {0};

    volatile void* src = nullptr;
    volatile void* dst = nullptr;

    uint count = 0;
public:

    explicit DMA(const volatile void* s = nullptr, volatile void* d = nullptr, uint cc = 0, dma_channel_transfer_size sz = DMA_SIZE_8)
    {
        dma_channel = dma_claim_unused_channel(true);
        c = dma_channel_get_default_config(dma_channel);

        src = (volatile void*)s;
        dst = d;
        count = cc;

        setDataSize(sz);
    }

    ~DMA()
    {
        dma_channel_unclaim(dma_channel);
    }

    uint getChannel() const
    {
        return dma_channel;
    }

    void WaitForFinish() const
    {
        dma_channel_wait_for_finish_blocking(dma_channel);
    }
    void setDataSize(
            dma_channel_transfer_size sz // Transfer Size
            )
    {
        channel_config_set_transfer_data_size(&c, sz);
    }

    void setIncrement(
            bool ReadWrite, // True for read, False For write
            bool Increment
            )
    {
        if(ReadWrite)
            channel_config_set_read_increment(&c, Increment);
        else
            channel_config_set_write_increment(&c, Increment);
    }

    void setRing(
            bool ReadWrite,  // True for read, False For write
            uint Ring        //The Ring Size if 1 << (Ring) Bits
            )
    {
        channel_config_set_ring(&c, !ReadWrite, Ring);
    }

    void setRoute(volatile void *s, volatile void* d, uint cc, bool preset = true, bool trig = false, bool setCount = true, bool setSrc = true, bool setDst = true)
    {
        src = s;
        dst = d;
        count = cc;

        if(preset)return;

        if((!s) || (!d) || count == 0)
            trig = false;

        if(setCount)
        {
            FlushConfigure(trig);
            return;
        }

        if(setSrc)
        {
            dma_channel_set_read_addr(dma_channel, src, false);
        }

        if(setDst)
        {
            dma_channel_set_write_addr(dma_channel, dst, false);
        }

        if(trig)
        {
            Begin();
        }
    }

    void setDataRequests(uint Dreq)
    {
        channel_config_set_dreq(&c, Dreq);
    }

    void setSrcBuffer(volatile void *s)
    {
        src = s;
        dma_channel_set_read_addr(dma_channel, src, false);
    }

    void setDstBuffer(volatile void *d)
    {
        dst = d;
        dma_channel_set_write_addr(dma_channel, dst, false);
    }

    void setChainTo(DMA* obj)
    {
        channel_config_set_chain_to(&c, obj->dma_channel);
    }

    void FlushConfigure(bool Trig = false)
    {
        this->WaitForFinish();
        dma_channel_configure(dma_channel, &c, dst, src, count, Trig);
    }

    void resetIncrement()
    {
        dma_channel_set_read_addr(dma_channel, src, false);
        dma_channel_set_write_addr(dma_channel, dst, false);
    }

    void Begin() const
    {
        dma_start_channel_mask(1u << dma_channel);
    }

    void Begin(uint cc) const
    {
        dma_channel_set_trans_count(dma_channel, cc, true);
    }

    void ConfigureIrq0(irq_handler_t handler) const
    {
        if(!handler)
        {
            dma_channel_set_irq0_enabled(dma_channel, false);
            irq_set_enabled(DMA_IRQ_0, false);
            return;
        }

        dma_channel_set_irq0_enabled(dma_channel, true);
        irq_set_exclusive_handler(DMA_IRQ_0, handler);
        irq_set_enabled(DMA_IRQ_0, true);
    }

    void ConfigureIrq1(irq_handler_t handler) const
    {
        if(!handler)
        {
            dma_channel_set_irq1_enabled(dma_channel, false);
            irq_set_enabled(DMA_IRQ_1, false);
            return;
        }

        dma_channel_set_irq1_enabled(dma_channel, true);
        irq_set_exclusive_handler(DMA_IRQ_1, handler);
        irq_set_enabled(DMA_IRQ_1, true);
    }

    void clearIrq(uint vec) const
    {
        switch(vec)
        {
            case 0:
                dma_hw->ints0 = 1u << dma_channel;
                break;
            case 1:
                dma_hw->ints1 = 1u << dma_channel;
                break;
            default:
                break;
        }
    }

    void Abort() const
    {
        dma_channel_abort(dma_channel);
    }

    bool isBusy() const
    {
        return dma_channel_is_busy(dma_channel);
    }
};

#endif //PICO_BLINK_DMA_HPP
