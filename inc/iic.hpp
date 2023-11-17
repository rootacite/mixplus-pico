//
// Created by ShiYuanLi on 11/1/2023.
//

#ifndef PICO_BLINK_IIC_HPP
#define PICO_BLINK_IIC_HPP

#include "hardware/i2c.h"
#include "dma.hpp"

#include <vector>

class IIC
{
protected:
    uint8_t addr = 0x00;
    i2c_inst_t* iic_dev = nullptr;

public:
    explicit IIC(i2c_inst_t* iic, uint scl, uint sda, uint speed = 100000)
    {
        this->iic_dev = iic;

        i2c_init(iic, speed);

        gpio_set_function(sda, GPIO_FUNC_I2C);
        gpio_set_function(scl, GPIO_FUNC_I2C);
        gpio_pull_up(sda);
        gpio_pull_up(scl);
    }

    virtual void write_to(uint8_t* pData, uint cc) = 0;
    virtual uint read_from(uint8_t* pData, uint cc) = 0;
    // For Master mode, this sets the target addr
    // For Slave mode, this sets self addr
    virtual void selectAddr(uint8_t addr) = 0;
    virtual void setSpeed(uint baud_rate) = 0;
};

class IICMaster : public IIC
{
private:
    bool reserved_addr(uint8_t addr) {
        return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
    }

public:
    explicit IICMaster(i2c_inst_t* iic, uint scl, uint sda, uint speed = 100000) : IIC(iic, scl, sda, speed)
    {

    }

    uint read_from(uint8_t* pData, uint cc) override
    {
        i2c_read_blocking(iic_dev, addr, pData, cc, false);
    }

    void write_to(uint8_t* pData, uint cc) override
    {
        i2c_write_blocking(iic_dev, addr, pData, cc, false);
    }

    void setSpeed(uint baud_rate) override
    {
        i2c_set_baudrate(iic_dev, baud_rate);
    }

    void selectAddr(uint8_t addr) override
    {
        this->addr = addr;
    }

    std::vector<uint8_t> scan()
    {
        std::vector<uint8_t> r;

        for (int addr = 0; addr < (1 << 7); ++addr) {

            int ret;
            uint8_t rxdata;
            if (reserved_addr(addr))
                ret = PICO_ERROR_GENERIC;
            else
                ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

            if(ret >= 0)
            {
                r.push_back(addr);
            }
        }

        return r;
    }
};

class IICSlave : public IIC
{

};

#endif //PICO_BLINK_IIC_HPP
