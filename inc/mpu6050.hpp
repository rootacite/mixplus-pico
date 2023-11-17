//
// Created by acite on 23-11-15.
//

#ifndef MPU6050_HPP
#define MPU6050_HPP

#include "haw/MPU6050.h"
#include <stdio.h>
#include <iostream>

struct MPU6050Defines
{
    i2c_inst_t* iic = i2c0;

    uint32_t sck = 17;
    uint32_t sda = 16;

    uint8_t addr = MPU6050_ADDRESS_A0_GND;
};

class MPU6050
{
private:
    MPU6050Defines defines { };
    mpu6050_t mpu6050 { };

public:
    explicit MPU6050(MPU6050Defines Df)
    {
        defines = Df;
        std::cout << "enter produce" << std::endl;
        gpio_init(Df.sda);
        gpio_init(Df.sck);
        gpio_set_function(Df.sda, GPIO_FUNC_I2C);
        gpio_set_function(Df.sck, GPIO_FUNC_I2C);
        gpio_pull_up(Df.sda);
        gpio_pull_up(Df.sck);

        std::cout << "gpio leave" << std::endl;

        mpu6050 = mpu6050_init(Df.iic, Df.addr);

        std::cout << "mpu6050_init leave" << std::endl;

        if (mpu6050_begin(&mpu6050))
        {
            std::cout << "mpu6050_begin success" << std::endl;
            // Set scale of gyroscope
            mpu6050_set_scale(&mpu6050, MPU6050_SCALE_2000DPS);
            // Set range of accelerometer
            mpu6050_set_range(&mpu6050, MPU6050_RANGE_16G);

            // Enable temperature, gyroscope and accelerometer readings
            mpu6050_set_temperature_measuring(&mpu6050, true);
            mpu6050_set_gyroscope_measuring(&mpu6050, true);
            mpu6050_set_accelerometer_measuring(&mpu6050, true);

            // Enable free fall, motion and zero motion interrupt flags
            mpu6050_set_int_free_fall(&mpu6050, false);
            mpu6050_set_int_motion(&mpu6050, false);
            mpu6050_set_int_zero_motion(&mpu6050, false);

            // Set motion detection threshold and duration
            mpu6050_set_motion_detection_threshold(&mpu6050, 2);
            mpu6050_set_motion_detection_duration(&mpu6050, 5);

            // Set zero motion detection threshold and duration
            mpu6050_set_zero_motion_detection_threshold(&mpu6050, 4);
            mpu6050_set_zero_motion_detection_duration(&mpu6050, 2);
        }
        else
        {
            while (1)
            {
                // Endless loop
                printf("Init failed");
                sleep_ms(500);
            }
        }
    }

    void Test()
    {
        mpu6050_event(&mpu6050);

        // Pointers to float vectors with all the results
        mpu6050_vectorf_t *accel = mpu6050_get_accelerometer(&mpu6050);
        mpu6050_vectorf_t *gyro = mpu6050_get_gyroscope(&mpu6050);

        // Activity struct holding all interrupt flags
        mpu6050_activity_t *activities = mpu6050_read_activities(&mpu6050);

        // Rough temperatures as float -- Keep in mind, this is not a temperature sensor!!!
        float tempC = mpu6050_get_temperature_c(&mpu6050);
        float tempF = mpu6050_get_temperature_f(&mpu6050);

        // Print all the measurements
        printf("Accelerometer: %f, %f, %f\n", accel->x, accel->y, accel->z, gyro->x, gyro->y, gyro->z, tempC, tempF);


    }

    void GetAccel(mpu6050_vectorf_t* Accel)
    {
        mpu6050_event(&mpu6050);

        // Pointers to float vectors with all the results
        mpu6050_vectorf_t *accel = mpu6050_get_accelerometer(&mpu6050);

        memcpy(Accel, accel, sizeof(mpu6050_vectorf_t));
    }
};

#endif
