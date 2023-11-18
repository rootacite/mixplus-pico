//
// Created by ShiYuanLi on 10/28/2023.
//

#ifndef PICO_BLINK_ST7735_HPP
#define PICO_BLINK_ST7735_HPP

#include "st7735_defs.h"

#include "spi.hpp"
#include "dma.hpp"
#include "pio.hpp"

#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

struct ST7735Defines
{
    spi_inst_t* spi; // spi1

    uint dc;  //10
    uint cs;  //9
    uint rst; //11

    uint scl;  //14
    uint sda;  //15
    uint vrx;
};

class ST7735
{
private:
    SPIMaster* SPI = nullptr;
    ProgrammableIO* PIO_SPI = nullptr;

    GPIO* pDC = nullptr;
    GPIO* pCS = nullptr;
    GPIO* pRST = nullptr;

    uint8_t _colstart = 2, _rowstart = 1, _tft_type, _rotation = 0, _xstart = 0, _ystart = 0;
    uint8_t tft_width = 160, tft_height = 128;

    uint16_t* frame_buffer = nullptr;
private:
    void swap_byte(uint16_t *x)
    {
        auto *t = (uint8_t*)x;
        uint8_t tx = t[0];
        t[0] = t[1];
        t[1] = tx;
    }

    void write_command(uint8_t cmd)
    {
        pDC->put(false);
        pCS->put(false);
        SPI->transmit(&cmd, 1, false, true);
        pCS->put(true);
    }

    void write_data(uint8_t data)
    {
        pDC->put(true);
        pCS->put(false);
        SPI->transmit(&data, 1, false, true);
        pCS->put(true);
    }

    void write_block(uint16_t* data, uint cc)
    {
        SPI->transmit(data, cc, true, true);
    }

    void write_r_cmd_1()
    {
        write_command(ST7735_SWRESET);
        sleep_ms(150);
        write_command(ST7735_SLPOUT);
        sleep_ms(250);
        sleep_ms(250);
        write_command(ST7735_FRMCTR1);
        write_data(0x01);
        write_data(0x2C);
        write_data(0x2D);
        write_command(ST7735_FRMCTR2);
        write_data(0x01);
        write_data(0x2C);
        write_data(0x2D);
        write_command(ST7735_FRMCTR3);
        write_data(0x01); write_data(0x2C); write_data(0x2D);
        write_data(0x01); write_data(0x2C); write_data(0x2D);
        write_command(ST7735_INVCTR);
        write_data(0x07);
        write_command(ST7735_PWCTR1);
        write_data(0xA2);
        write_data(0x02);
        write_data(0x84);
        write_command(ST7735_PWCTR2);
        write_data(0xC5);
        write_command(ST7735_PWCTR3);
        write_data(0x0A);
        write_data(0x00);
        write_command(ST7735_PWCTR4);
        write_data(0x8A);
        write_data(0x2A);
        write_command(ST7735_PWCTR5);
        write_data(0x8A);
        write_data(0xEE);
        write_command(ST7735_VMCTR1);
        write_data(0x0E);
        write_command(ST7735_INVOFF);
        write_command(ST7735_MADCTL);
        write_data(0xC8);
        write_command(ST7735_COLMOD);
        write_data(0x05);
    }

    void write_r_cmd_2()
    {
        write_command(ST7735_CASET);
        write_data(0x00); write_data(0x00);
        write_data(0x00); write_data(0x7F);
        write_command(ST7735_RASET);
        write_data(0x00); write_data(0x00);
        write_data(0x00); write_data(0x9F);
    }

    void write_r_cmd_3()
    {
        write_command(ST7735_GMCTRP1);
        write_data(0x02); write_data(0x1C); write_data(0x07); write_data(0x12);
        write_data(0x37); write_data(0x32); write_data(0x29); write_data(0x2D);
        write_data(0x29); write_data(0x25); write_data(0x2B); write_data(0x39);
        write_data(0x00); write_data(0x01); write_data(0x03); write_data(0x10);
        write_command(ST7735_GMCTRN1);
        write_data(0x03); write_data(0x1D); write_data(0x07); write_data(0x06);
        write_data(0x2E); write_data(0x2C); write_data(0x29); write_data(0x2D);
        write_data(0x2E); write_data(0x2E); write_data(0x37); write_data(0x3F);
        write_data(0x00); write_data(0x00); write_data(0x02); write_data(0x10);
        write_command(ST7735_NORON);
        sleep_ms(10);
        write_command(ST7735_DISPON);
        sleep_ms(100);
    }

    void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1){
        write_command(ST7735_CASET);
        write_data(0);
        write_data(x0 + _xstart);
        write_data(0);
        write_data(x1 + _xstart);
        write_command(ST7735_RASET);
        write_data(0);
        write_data(y0 + _ystart);
        write_data(0);
        write_data(y1 + _ystart);
        write_command(ST7735_RAMWR); // Write to RAM
    }

    void select()
    {
        write_command(ST7735_RAMWR); // Write to RAM
    }

public:
    explicit ST7735(ST7735Defines Defs)
    {
        pDC = new GPIO(Defs.dc, true);
        pCS = new GPIO(Defs.cs, true);
        pRST = new GPIO(Defs.rst, true);


        SPI = new SPIMaster(
                {spi1, Defs.scl, Defs.sda, Defs.vrx, 0xFF }, 8
                );
        SPI->init(60000000);

        pDC->put(false);
        pCS->put(false);
        pRST->put(false);

        frame_buffer = new uint16_t[tft_width * tft_height];
    }

    ~ST7735()
    {
        delete pDC;
        delete pCS;
        delete pRST;

        delete SPI;
        delete[] frame_buffer;
    }

    void initBlackTab()
    {
        reset();          // Reset Model
        pDC->put(false);

        write_r_cmd_1();  // Write cmd list 1
        write_r_cmd_2();  // Write cmd list 2
        write_r_cmd_3();  // Write cmd list 3

        write_command(ST7735_MADCTL);
        write_data(0xC0);
        //side commands
    }

    void reset()
    {
        pRST->put(true);
        sleep_ms(10);
        pRST->put(false);
        sleep_ms(10);
        pRST->put(true);
        sleep_ms(10);
    }

    void setRotation(uint8_t m) {
        // m can be 0-3
        uint8_t madctl = 0;
        _rotation = m % 4;

        switch (_rotation) {
            case 0:
                madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB;
                _xstart = _colstart;
                _ystart = _rowstart;
                break;
            case 1:
                madctl = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
                _ystart = _colstart;
                _xstart = _rowstart;
                break;
            case 2:
                madctl = ST7735_MADCTL_RGB;
                _xstart = _colstart;
                _ystart = _rowstart;
                break;
            case 3:
                madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
                _ystart = _colstart;
                _xstart = _rowstart;
                break;
        }
        write_command(ST7735_MADCTL);
        write_data(madctl);
    }

    void fillRectangle(uint16_t color, uint8_t w, uint8_t h, uint8_t x = 0, uint8_t y = 0)
    {
        for (uint16_t ly = y ; ly - y < h ; ly ++)
            for (uint16_t lx = x ; lx - x < w ; lx ++)
            {
                frame_buffer[ly * tft_width + lx] = color;
                swap_byte(&frame_buffer[ly * tft_width + lx]);
            }
    }

    void show()
    {
        setWindow(0, 0, tft_width - 1, tft_height - 1);

        pDC->put(true);
        pCS->put(false);

        write_block(frame_buffer, tft_width * tft_height * 2);
        sleep_ms(2);

        pCS->put(true);
    }

    void clear(uint16_t color)
    {
        fillRectangle(color, tft_width, tft_height);
    }

    void drawFastHLine(uint8_t x, uint8_t y, uint8_t w, uint16_t color)
    {
        swap_byte(&color);
        for(uint lx=x; lx - x < w;lx++)
        {
            frame_buffer[y * tft_width + lx] = color;
        }
    }

    void drawFastVLine(uint8_t x, uint8_t y, uint8_t h, uint16_t color)
    {
        swap_byte(&color);
        for(uint ly=y; ly - y < h;ly++)
        {
            frame_buffer[ly * tft_width + x] = color;
        }
    }

    void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
        int16_t i;
        for (i = x; i < x + w; i++) {
            drawFastVLine(i, y, h, color);
        }
    }

    void drawPixel(uint8_t x, uint8_t y, uint16_t color){
        swap_byte(&color);
        frame_buffer[y * tft_width + x] = color;
    }

    void drawChar(uint8_t x, uint8_t y, uint8_t c, uint16_t color, uint16_t bg,  uint8_t size){
        int8_t i, j;
        if((x >= tft_width) || (y >= tft_height))
            return;
        if(size < 1) size = 1;
        if((c < ' ') || (c > '~'))
            c = '?';
        for(i=0; i<5; i++ ) {
            uint8_t line;
            line = Font[(c - LCD_ASCII_OFFSET)*5 + i];
            for(j=0; j<7; j++, line >>= 1) {
                if(line & 0x01) {
                    if(size == 1) drawPixel(x+i, y+j, color);
                    else          fillRect(x+(i*size), y+(j*size), size, size, color);
                }
                else if(bg != color) {
                    if(size == 1) drawPixel(x+i, y+j, bg);
                    else          fillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }
    }

// Draw text character array to screen
    void drawText(uint8_t x, uint8_t y, const char *_text, uint16_t color, uint16_t bg, uint8_t size) {
        uint8_t cursor_x, cursor_y;
        uint16_t textsize, i;
        cursor_x = x, cursor_y = y;
        textsize = strlen(_text);
        for(i = 0; i < textsize; i++){
            if((cursor_x + size * 5) > tft_width) {
                cursor_x = 0;
                cursor_y = cursor_y + size * 7 + 3 ;
                if(cursor_y > tft_height) cursor_y = tft_height;
                if(_text[i] == LCD_ASCII_OFFSET) {
                    continue;
                }
            }
            drawChar(cursor_x, cursor_y, _text[i], color, bg, size);
            cursor_x = cursor_x + size * 6;
            if(cursor_x > tft_width) {
                cursor_x = tft_width;
            }
        }
    }

    void* getBuffer()
    {
        return frame_buffer;
    }
};

#endif //PICO_BLINK_ST7735_HPP
