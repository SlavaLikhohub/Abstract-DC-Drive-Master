#ifndef _USART_LOG_H
#define _USART_LOG_H

#include <stdint.h>

void log_set_speed(int32_t speed);

void log_set_position(int32_t position);

void log_set_current(int32_t current);

void log_set_motor_v(int32_t v);

void log_set_temperature(int32_t t);

#endif // _USART_LOG_H
