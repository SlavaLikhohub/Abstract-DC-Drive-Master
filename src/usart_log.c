#include "usart_log.h"

#include <abstractLOG.h>

volatile int32_t fd_speed;
volatile int32_t fd_position;
volatile int32_t fd_current;
volatile int32_t temp;
volatile int32_t motor_v;

void log_set_speed(int32_t speed)
{
    fd_speed = speed;
}

void log_set_position(int32_t position)
{
    fd_position = position;
}

void log_set_current(int32_t current)
{
    fd_current = current;
}

void log_set_motor_v(int32_t v)
{
    motor_v = v;
}

void log_set_temperature(int32_t t)
{
    temp = t;
}

int32_t log_get_speed(void)
{
    return fd_speed;
}

int32_t log_get_position(void)
{
    return fd_position;
}

int32_t log_get_current(void)
{
    return fd_current;
}

int32_t log_get_temp(void)
{
    return temp;
}

int32_t log_get_motor_v(void)
{
    return motor_v;
}
