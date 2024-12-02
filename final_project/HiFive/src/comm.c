#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "eecs388_lib.h"

void auto_brake(int devid)
{
    // task-1: use lidar readings to control the led colors
    if ('Y' == ser_read(devid) && 'Y' == ser_read(devid)) {
        uint8_t l_dist = ser_read(devid);
        uint8_t h_dist = ser_read(devid);
        uint16_t dist = (h_dist << 8) | l_dist;

        char buff[50];
        sprintf(buff, "distance: %d cm", dist);
        ser_printline(devid, buff);

        printf("debug: distance read is %d cm\n", dist);

        if (dist > 200) {
            gpio_mode(GREEN_LED, OUTPUT);
            gpio_write(GREEN_LED, ON);
            gpio_write(BLUE_LED, OFF);
            gpio_write(RED_LED, OFF);
            printf("debug: green led on - safe distance\n");
        } else if (dist > 100 && dist <= 200) {
            gpio_mode(GREEN_LED, OUTPUT);
            gpio_write(GREEN_LED, ON);
            gpio_write(BLUE_LED, OFF);
            gpio_mode(RED_LED, OUTPUT);
            gpio_write(RED_LED, ON);
            printf("debug: yellow led on - light braking\n");
        } else if (dist > 60 && dist <= 100) {
            gpio_write(GREEN_LED, OFF);
            gpio_write(BLUE_LED, OFF);
            gpio_mode(RED_LED, OUTPUT);
            gpio_write(RED_LED, ON);
            printf("debug: red led on - hard braking\n");
        } else {
            printf("debug: red led flashing - stopping vehicle\n");
            delay(100);
            gpio_mode(RED_LED, OUTPUT);
            gpio_write(RED_LED, ON);
            delay(100);
            gpio_write(RED_LED, OFF);
            delay(100);
        }
    } else {
        printf("error: invalid data from lidar\n");
    }
}

int read_from_pi(int devid)
{
    // task-2: read angle values from the raspberry pi
    char data[100];

    while (1) {
        if (ser_isready(devid)) {
            int angle;
            ser_readline(devid, 100, data);

            // convert string data to integer
            if (sscanf(data, "%d", &angle) == 1) {
                printf("debug: received angle %d from pi\n", angle);
                return angle;
            } else {
                printf("error: invalid data format from pi\n");
            }
        }
    }
    return 0;
}

void steering(int gpio, int pos)
{
    // task-3: control servo motor position
    int time_cycle = ((2400 - 544) / 180) * pos + 544;
    int rem_cycle = 20000 - time_cycle;

    gpio_write(gpio, ON);
    delay_usec(time_cycle);
    gpio_write(gpio, OFF);
    delay_usec(rem_cycle);

    printf("debug: servo moved to position %d\n", pos);
}

int main()
{
    // initialize uart channels
    ser_setup(0); // uart0 for lidar
    ser_setup(1); // uart1 for pi
    int pi_to_hifive = 1; // connection with pi uses uart1
    int lidar_to_hifive = 0; // lidar uses uart0

    printf("using uart %d for pi -> hifive\n", pi_to_hifive);
    printf("using uart %d for lidar -> hifive\n", lidar_to_hifive);

    // initialize gpio pins
    gpio_mode(PIN_19, OUTPUT);
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(BLUE_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);

    printf("setup completed.\n");
    printf("begin the main loop.\n");

    while (1) {
        auto_brake(lidar_to_hifive); // measure distance and control led
        int angle = read_from_pi(pi_to_hifive); // get angle from pi

        printf("debug: current angle is %d\n", angle);

        // control servo based on angle
        int gpio = PIN_19;
        if (angle > 0) {
            steering(gpio, 180); // move servo to 180 for positive angles
        } else {
            steering(gpio, 0); // move servo to 0 for negative angles
        }
    }
    return 0;
}
