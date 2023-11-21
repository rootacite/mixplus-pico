//
// Created by acite on 23-11-21.
//

#ifndef PICO_DEMO2_GY273_HPP
#define PICO_DEMO2_GY273_HPP
#include "hardware/iic.hpp"

class GY273
{
private: // Private Consts
    const uint8_t addr = 0x1E;
    uint8_t commands[3] = {
            0x70,
            0x20,
            0x01
    };

    const float Xoffset = -0.152110, Yoffset = 0.235505;
    const float Kx = 2.85, Ky = 3.020996;

private:  // Private Method
    IICMaster* Master = nullptr;

public: // Interface
    explicit GY273(IICMaster* iic)
    {
        Master = iic;

        Master->selectAddr(0x1E);
        Master->i2c_write_reg(0x00, &commands[0], 1);  //Config CRA
        Master->i2c_write_reg(0x01, &commands[1], 1);  //Config CRB
    }

    int SingleRead(uint8_t* buf)
    {
        Master->i2c_write_reg(0x02, &commands[2], 1);  //Config Mod CR
        sleep_ms(10);
        return Master->i2c_read_reg(0x03, buf, 6);
    }

    void ReadGaValue(float *GaX, float *GaY)
    {
        uint8_t data[6];
        SingleRead(data);
        int16_t dxra,dyra;
        dxra = (data[0] << 8) | data[1];
        *GaX = (float)dxra /1090;
        dyra = (data[4] << 8) | data[5];
        *GaY = (float)dyra /1090 ;
    }

    float ReadAngle()
    {
        float rawGaX, rawGaY, GaX, GaY;
        ReadGaValue(&rawGaX, &rawGaY);
        GaX = (rawGaX - Xoffset) * Kx;
        GaY = (rawGaY - Yoffset) * Ky;
        //cout << GaX << ", " << GaY << endl;

        float Angle = 0;
        if((GaX > 0)&&(GaY > 0)) Angle = atan(GaY / GaX) * 57;
        else if((GaX > 0)&&(GaY < 0)) Angle = 360 + atan(GaY / GaX) * 57;
        else if((GaX == 0)&&(GaY > 0)) Angle = 90;
        else if((GaX == 0)&&(GaY < 0)) Angle = 270;
        else if(GaX < 0) Angle = 180 + atan(GaY / GaX) * 57;

        return Angle;
    }
};

#endif //PICO_DEMO2_GY273_HPP
