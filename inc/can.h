#ifndef _CAN_H_
#define _CAN_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "common_defs.h"

enum can_ids 
{
    CAN_SETUP_COMMANDS_ID = (uint32_t) 200,
    CAN_SET_DES_VALUE_COMMANDS_ID =    100,
    CAN_LOG_REQUEST_COMMANDS_ID =      900,
};

bool can_bus_init(void);
void set_pid_coef(enum pid_types type, enum pid_fields field, int32_t value);
void save_pid_to_flash(enum pid_types type);
void set_desired_value(enum pid_types type, int32_t value);
void request_log(uint8_t log_type);
void can1_send(uint32_t id, size_t N, uint8_t* data);
#endif //_CAN_H_
