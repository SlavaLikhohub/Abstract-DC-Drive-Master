#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Struct for storing settings of PID regulator
 */
struct pid_settings
{
    /** Proportional coefficient */
    int32_t P;
    /** Integral coefficient */
    int32_t I;
    /** Diferential coefficient */
    int32_t D;
    /** Coefficient on which will be divided all coefficients. Must not be 0 */
    int32_t div;
    /** Find average in N last measurment before using as feedback signal */
    int32_t aver_N;
};

/**
 * Type of PID regulators
 */
enum pid_types 
{
    /** None, disable */
    NONE = -1,
    /** Speed control */
    SPEED_PID = 0,
    /** Current control */
    CURRENT_PID,
    /** Position control */
    POSITION_PID,
    /** Direct voltage control */
    DIRECT_CONT,
};

/** Fields of pid settings */
enum pid_fields
{
    /** P field */
    PID_FIELD_P = 0,
    /** I field */
    PID_FIELD_I,
    /** D field */
    PID_FIELD_D,
    /** div field */
    PID_FIELD_DIV,
    /** aver_N field */
    PID_FIELD_AVER_N,
};

/** Struct for encoding settings-changing message from CAN */
struct pid_settings_msg 
{
    /** PID Type according to :c:type:`pid_types`. */
    uint8_t pid_type : 4;
    
    /** PID Field according to :c:type:`pid_fields` */
    uint8_t pid_field : 4;
    
    /** Value to write */
    int32_t value;
};

/** Struct for encoding desire-value-changing message from CAN */
struct pid_des_value_msg
{
    /** PID Type according to :c:type:`pid_types`. */
    uint8_t pid_type;
    
    /** Value to write */
    int32_t value;
};

/** Info data to send throught CAN */
enum log_types
{
    /** Position */
    LOG_POSITION = 0,
    /** Feedback speed */
    LOG_SPEED,
    /** Feedback current */
    LOG_CURRENT,
    /** Temperature */
    LOG_TEMPERATURE,
};

#endif // _COMMON_DEFS_H_
