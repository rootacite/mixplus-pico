//
// Created by ShiYuanLi on 10/28/2023.
//

#ifndef PICO_BLINK_SPI_HPP
#define PICO_BLINK_SPI_HPP

#include <cstring>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spi_transmitonly.pio.h"

#include "dma.hpp"
#include "pio.hpp"
#include "gpio.hpp"

struct SPIDefines
{
    spi_inst_t* spi_index;
    uint sck;
    uint txd;
    uint rxd;
    uint nss;
};

class SPI
{
protected: //
    SPIDefines spi_defines { };

    DMA* tx_dma = nullptr;
    DMA* rx_dma = nullptr;

    uint data_bits;
protected:


public: // Interface
    explicit SPI(SPIDefines defines, uint dataBits)
    {
        memcpy(&spi_defines, &defines, sizeof(SPIDefines));
        data_bits = dataBits;
    }
    ~SPI()
    {
        delete tx_dma;
        delete rx_dma;

        spi_deinit(spi_defines.spi_index);

        if(spi_defines.rxd <= 30)
            gpio_deinit(spi_defines.rxd);
        if(spi_defines.txd <= 30)
            gpio_deinit(spi_defines.txd);
        if(spi_defines.sck <= 30)
            gpio_deinit(spi_defines.sck);
    }

    virtual void init(uint rate) = 0;
    virtual void transmit(void* pData, uint cc, bool useDMA, bool block) = 0;
    virtual void receive(void* pData, uint cc, uint16_t r, bool useDMA, bool block) = 0;

    virtual void DMAWait(bool is_tx)
    {
        if(is_tx)
            tx_dma->WaitForFinish();
        else
            rx_dma->WaitForFinish();
    }

    virtual void ClearIRQFlag(bool is_tx)
    {
        if(is_tx)
            tx_dma->clearIrq(0);
        else
            rx_dma->clearIrq(0);
    }
};

class SPIMaster : public SPI
{
public:
    void init(uint rate) override
    {
        spi_init(spi_defines.spi_index, rate);

        if(spi_defines.rxd <= 30)
            gpio_set_function(spi_defines.rxd, GPIO_FUNC_SPI);
        if(spi_defines.sck <= 30)
            gpio_set_function(spi_defines.sck, GPIO_FUNC_SPI);
        if(spi_defines.txd <= 30)
            gpio_set_function(spi_defines.txd, GPIO_FUNC_SPI);
        if(spi_defines.nss <= 30)
            gpio_set_function(spi_defines.nss, GPIO_FUNC_SPI);

        spi_set_format(spi_defines.spi_index, data_bits, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

        tx_dma = new DMA(nullptr, &spi_get_hw(spi_defines.spi_index)->dr, 0, data_bits == 16 ? DMA_SIZE_16 : DMA_SIZE_8);
        tx_dma->setIncrement(true, true);
        tx_dma->setIncrement(false, false);
        tx_dma->setDataRequests(spi_get_dreq(spi_defines.spi_index, true));
        tx_dma->FlushConfigure(false);

        rx_dma = new DMA(&spi_get_hw(spi_defines.spi_index)->dr, nullptr, 0, data_bits == 16 ? DMA_SIZE_16 : DMA_SIZE_8);
        rx_dma->setIncrement(true, false);
        rx_dma->setIncrement(false, true);
        rx_dma->setDataRequests(spi_get_dreq(spi_defines.spi_index, false));
        rx_dma->FlushConfigure(false);

    }

public:
    explicit SPIMaster(SPIDefines defines, uint dataBits) : SPI(defines, dataBits)
    {

    }

    void transmit(void* pData, uint cc, bool useDMA = true, bool block = true) override
    {
        if(useDMA)
        {
            tx_dma->setSrcBuffer(pData);
            tx_dma->Begin(cc);
        }
        else
        {
            if(data_bits == 16)
            {
                spi_write16_blocking(spi_defines.spi_index, (uint16_t*)pData, cc);
            }
            else
            {
                spi_write_blocking(spi_defines.spi_index, (uint8_t*)pData, cc);
            }
        }

        if(block && useDMA)
            DMAWait(true);
    }

    void receive(void* pData, uint cc, uint16_t r, bool useDMA, bool block) override
    {
        if(useDMA)
        {
            rx_dma->setDstBuffer(pData);
            rx_dma->Begin(cc);
        }
        else
        {
            if(data_bits == 16)
            {
                spi_read16_blocking(spi_defines.spi_index, r, (uint16_t*)pData, cc);
            }
            else
            {
                spi_read_blocking(spi_defines.spi_index, r, (uint8_t*)pData, cc);
            }
        }

        if(block && useDMA)
            DMAWait(false);
    }

    void setDMACallBackTX(irq_handler_t dt) const
    {
        tx_dma->ConfigureIrq0(dt);
    }

    void setDMACallBackRX(irq_handler_t dt) const
    {
        rx_dma->ConfigureIrq0(dt);
    }

};

class SPISlave : public SPI
{
public:
    void init(uint rate) override
    {
        spi_init(spi_defines.spi_index, rate);
        spi_set_slave(spi_defines.spi_index, true);

        if(spi_defines.rxd <= 30)
            gpio_set_function(spi_defines.rxd, GPIO_FUNC_SPI);
        if(spi_defines.sck <= 30)
            gpio_set_function(spi_defines.sck, GPIO_FUNC_SPI);
        if(spi_defines.txd <= 30)
            gpio_set_function(spi_defines.txd, GPIO_FUNC_SPI);
        if(spi_defines.nss <= 30)
            gpio_set_function(spi_defines.nss, GPIO_FUNC_SPI);

        spi_set_format(spi_defines.spi_index, data_bits, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

        tx_dma = new DMA(nullptr, &spi_get_hw(spi_defines.spi_index)->dr, 0, data_bits == 16 ? DMA_SIZE_16 : DMA_SIZE_8);
        tx_dma->setIncrement(true, true);
        tx_dma->setIncrement(false, false);
        tx_dma->setDataRequests(spi_get_dreq(spi_defines.spi_index, true));
        tx_dma->FlushConfigure(false);

        rx_dma = new DMA(&spi_get_hw(spi_defines.spi_index)->dr, nullptr, 0, data_bits == 16 ? DMA_SIZE_16 : DMA_SIZE_8);
        rx_dma->setIncrement(true, false);
        rx_dma->setIncrement(false, true);
        rx_dma->setDataRequests(spi_get_dreq(spi_defines.spi_index, false));
        rx_dma->FlushConfigure(false);
    }

public:
    explicit  SPISlave(SPIDefines defines, uint dataBits) : SPI(defines, dataBits)
    {

    }

    void transmit(void* pData, uint cc, bool useDMA = true, bool block = true) override
    {
        if(useDMA)
        {
            tx_dma->setSrcBuffer(pData);
            tx_dma->Begin(cc);
        }
        else
        {
            if(data_bits == 16)
            {
                spi_write16_blocking(spi_defines.spi_index, (uint16_t*)pData, cc);
            }
            else
            {
                spi_write_blocking(spi_defines.spi_index, (uint8_t*)pData, cc);
            }
        }

        if(block && useDMA)
            DMAWait(true);
    }

    void receive(void* pData, uint cc, uint16_t r, bool useDMA, bool block) override
    {
        if(useDMA)
        {
            rx_dma->setDstBuffer(pData);
            rx_dma->Begin(cc);
        }
        else
        {
            if(data_bits == 16)
            {
                spi_read16_blocking(spi_defines.spi_index, r, (uint16_t*)pData, cc);
            }
            else
            {
                spi_read_blocking(spi_defines.spi_index, r, (uint8_t*)pData, cc);
            }
        }

        if(block && useDMA)
            DMAWait(false);
    }

    void setDMACallBackTX(irq_handler_t dt) const
    {
        tx_dma->ConfigureIrq0(dt);
    }

    void setDMACallBackRX(irq_handler_t dt) const
    {
        rx_dma->ConfigureIrq0(dt);
    }
};

class SPITransmitonly : public SPI
{
private:  // Hidden Methods
    void DMAWait(bool is_tx) override { }
    void ClearIRQFlag(bool is_tx) override { }
    void receive(void* pData, uint cc, uint16_t r, bool useDMA, bool block) override {  }
    void init(uint rate) override {  }

private:
    uint sm = 0;
public:
    explicit SPITransmitonly(SPIDefines defines) : SPI(defines, 8)
    {

    }

    void init(float rate)
    {
        uint offset = pio_add_program(pio0, &spi_transmitonly_program);
        sm = pio_claim_unused_sm(pio0, true);
        pio_sm_config c = spi_transmitonly_program_get_default_config(offset);

        sm_config_set_out_pins(&c, spi_defines.txd, 1);
        sm_config_set_sideset_pins(&c, spi_defines.sck);
        // Only support MSB-first in this example code (shift to left, auto push/pull, threshold=nbits)
        sm_config_set_out_shift(&c, false, true, 8);
        sm_config_set_in_shift(&c, false, true, 8);
        sm_config_set_clkdiv(&c, rate);

        // MOSI, SCK output are low, MISO is input
        pio_sm_set_pins_with_mask(pio0, sm, 0, (1u << spi_defines.sck) | (1u << spi_defines.txd));
        pio_sm_set_pindirs_with_mask(pio0, sm, (1u << spi_defines.sck) | (1u << spi_defines.txd), (1u << spi_defines.sck) | (1u << spi_defines.txd));
        pio_gpio_init(pio0, spi_defines.txd);
        pio_gpio_init(pio0, spi_defines.sck);


        pio_sm_init(pio0, sm, offset, &c);
        pio_sm_set_enabled(pio0, sm, true);

        tx_dma = new DMA(nullptr, &pio0->txf[sm], 0, DMA_SIZE_8);
        tx_dma->setIncrement(true, true);
        tx_dma->setIncrement(false, false);
        tx_dma->setDataRequests(pio_get_dreq(pio0, sm, true));
        tx_dma->FlushConfigure(false);
    }

    void transmit(void* pData, uint cc, bool useDMA, bool block) override
    {
        auto* pData_uint8 = (uint8_t*)pData;
        if(useDMA)
        {
            tx_dma->setSrcBuffer(pData);
            tx_dma->Begin(cc);
            if(block)
            {
                tx_dma->WaitForFinish();
            }
        }
        else
        {
            for(uint i = 0;i<cc;i++)
            {
                pio_sm_put_blocking(pio0, sm,  pData_uint8[i] << 24 );
            }
        }
    }
};

class SPITransonlySoftware : public SPI
{

private:  // Hidden Methods
    void DMAWait(bool is_tx) override { }
    void ClearIRQFlag(bool is_tx) override { }
    void receive(void* pData, uint cc, uint16_t r, bool useDMA, bool block) override {  }

private:
    GPIO* SCK = nullptr;
    GPIO* SDA = nullptr;

public:
    explicit SPITransonlySoftware(SPIDefines defines) : SPI(defines, 8)
    {

    }

    void init(uint rate) override
    {
        SCK = new GPIO(spi_defines.sck, true);
        SDA = new GPIO(spi_defines.txd, true);

        SCK->put(true);
        SDA->put(false);
    }

    void transmit(void* pData, uint cc, bool useDMA, bool block) override
    {
        auto* pData_uint8 = (uint8_t*)pData;

        for(int i=0;i<cc;i++)
        {
            uint8_t v2 =
                    ((pData_uint8[i] & 0x01) << 7) |
                    ((pData_uint8[i] & 0x02) << 5) |
                    ((pData_uint8[i] & 0x04) << 3) |
                    ((pData_uint8[i] & 0x08) << 1) |
                    ((pData_uint8[i] & 0x10) >> 1) |
                    ((pData_uint8[i] & 0x20) >> 3) |
                    ((pData_uint8[i] & 0x40) >> 5) |
                    ((pData_uint8[i] & 0x80) >> 7);
            for(uint b = 0;b<8;b++)
            {
                SDA->put(v2 & (1 << b));
                SCK->put(false);
                SDA->put(v2 & (1 << b));
                SCK->put(true);
            }
        }
    }
};

#endif //PICO_BLINK_SPI_HPP
