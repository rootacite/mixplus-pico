//
// Created by ShiYuanLi on 10/30/2023.
//

#ifndef PICO_BLINK_SD_HPP
#define PICO_BLINK_SD_HPP

#include "sd_card.h"
#include "ff.h"

#include <string>

class File
{
private:
    FRESULT fr;
    FIL fil;

public:
    bool open(const std::string& path, BYTE mode)
    {
        fr = f_open(&fil, path.c_str(), FA_READ);
        return (fr == FR_OK);
    }

    void close()
    {
        f_close(&fil);
    }

    uint64_t size()
    {
        return f_size(&fil);
    }

    bool read(uint8_t* pData, uint cc, uint* pCc)
    {
        fr = f_read(&fil, pData, cc, pCc);
        return (fr == FR_OK);
    }

    bool write(uint8_t* pData, uint cc, uint* pCc)
    {
        fr = f_write(&fil, pData, cc, pCc);
        return (fr == FR_OK);
    }

    void reset()
    {
        f_rewind(&fil);
    }
};

class FatFs
{
private:
    FRESULT fr;
    FATFS fs;
    FIL fil;
public:
    bool init()
    {
        if (!sd_init_driver()) {
           return false;
        }
        // Mount drive
        fr = f_mount(&fs, "0:", 1);
        if (fr != FR_OK) {
            return false;
        }
        return true;
    }

    bool test(char* buf, uint* br)
    {
        char filename[] = "hello.txt";

        // Open file for reading
        fr = f_open(&fil, filename, FA_READ);
        if (fr != FR_OK) {
            return false;
        }

        // Print every line in file over serial
        f_read(&fil, buf, 100, br);
        buf[*br] = '\0';
        // Close file
        fr = f_close(&fil);
        return true;
    }

};


#endif //PICO_BLINK_SD_HPP
