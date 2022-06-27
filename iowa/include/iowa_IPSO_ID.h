/**********************************************
*
*  _________ _________ ___________ _________
* |         |         |   |   |   |         |
* |_________|         |   |   |   |    _    |
* |         |    |    |   |   |   |         |
* |         |    |    |           |         |
* |         |    |    |           |    |    |
* |         |         |           |    |    |
* |_________|_________|___________|____|____|
*
* Copyright (c) 2016-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_IPSO_ID_INCLUDE_
#define _IOWA_IPSO_ID_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/****************************
 * IDs of LWM2M Objects defined by the IPSO Alliance
 */
// Only the ones in this enum are supported by iowa_client_IPSO_add_sensor().
typedef enum
{
    // Sensors using Floats
    IOWA_IPSO_ANALOG_INPUT  = (uint16_t)3202,
    IOWA_IPSO_GENERIC       = (uint16_t)3300,
    IOWA_IPSO_ILLUMINANCE   = (uint16_t)3301,
    IOWA_IPSO_TEMPERATURE   = (uint16_t)3303,
    IOWA_IPSO_HUMIDITY      = (uint16_t)3304,
    IOWA_IPSO_BAROMETER     = (uint16_t)3315,
    IOWA_IPSO_VOLTAGE       = (uint16_t)3316,
    IOWA_IPSO_CURRENT       = (uint16_t)3317,
    IOWA_IPSO_FREQUENCY     = (uint16_t)3318,
    IOWA_IPSO_DEPTH         = (uint16_t)3319,
    IOWA_IPSO_PERCENTAGE    = (uint16_t)3320,
    IOWA_IPSO_ALTITUDE      = (uint16_t)3321,
    IOWA_IPSO_LOAD          = (uint16_t)3322,
    IOWA_IPSO_PRESSURE      = (uint16_t)3323,
    IOWA_IPSO_LOUDNESS      = (uint16_t)3324,
    IOWA_IPSO_CONCENTRATION = (uint16_t)3325,
    IOWA_IPSO_ACIDITY       = (uint16_t)3326,
    IOWA_IPSO_CONDUCTIVITY  = (uint16_t)3327,
    IOWA_IPSO_POWER         = (uint16_t)3328,
    IOWA_IPSO_POWER_FACTOR  = (uint16_t)3329,
    IOWA_IPSO_RATE          = (uint16_t)3346,
    IOWA_IPSO_DISTANCE      = (uint16_t)3330,
    IOWA_IPSO_ENERGY        = (uint16_t)3331,
    IOWA_IPSO_DIRECTION     = (uint16_t)3332,

    // Sensors using Booleans
    IOWA_IPSO_DIGITAL_INPUT = (uint16_t)3200,
    IOWA_IPSO_PRESENCE      = (uint16_t)3302,
    IOWA_IPSO_ON_OFF_SWITCH = (uint16_t)3342,
    IOWA_IPSO_PUSH_BUTTON   = (uint16_t)3347
} iowa_IPSO_ID_t;

#define IOWA_LWM2M_DIGITAL_OUTPUT_OBJECT_ID 3201
#define IOWA_LWM2M_LIGHT_CONTROL_OBJECT_ID  3311
#define IOWA_LWM2M_ACCELEROMETER_OBJECT_ID  3313
#define IOWA_LWM2M_MAGNETOMETER_OBJECT_ID   3314
#define IOWA_LWM2M_GYROMETER_OBJECT_ID      3334
#define IOWA_LWM2M_GPS_OBJECT_ID            3336
#define IOWA_LWM2M_DIMMER_OBJECT_ID         3343
#define IOWA_IPSO_LEVEL_CONTROL             3343

// This is a macro to fill an iowa_lwm2m_desc_t with the values of LWM2M Reusable Resources.
#define IOWA_SET_LWM2M_DESC_T_TO_IPSO_RSC(desc, RES) \
{                                                    \
    desc.id = IPSO_RSC_ID_##RES;                     \
    desc.type = IPSO_RSC_TYPE_##RES;                 \
    desc.operations = IPSO_RSC_OP_##RES;             \
}

/****************************
 * IDs of LWM2M Reusable Resources defined by the IPSO Alliance.
 */
#define IPSO_RSC_ID_DIGITAL_INPUT_STATE                5500 // Boolean
#define IPSO_RSC_ID_DIGITAL_INPUT_COUNTER              5501 // Integer
#define IPSO_RSC_ID_DIGITAL_INPUT_POLARITY             5502 // Boolean
#define IPSO_RSC_ID_DIGITAL_INPUT_DEBOUNCE_PERIOD      5503 // Integer
#define IPSO_RSC_ID_DIGITAL_INPUT_EDGE_SELECTION       5504 // Integer
#define IPSO_RSC_ID_DIGITAL_INPUT_COUNTER_RESET        5505 // Exec
#define IPSO_RSC_ID_CURRENT_TIME                       5506 // Time
#define IPSO_RSC_ID_FRACTIONNAL_TIME                   5507 // Float
#define IPSO_RSC_ID_MIN_X_VALUE                        5508 // Float
#define IPSO_RSC_ID_MAX_X_VALUE                        5509 // Float
#define IPSO_RSC_ID_MIN_Y_VALUE                        5510 // Float
#define IPSO_RSC_ID_MAX_Y_VALUE                        5511 // Float
#define IPSO_RSC_ID_MIN_Z_VALUE                        5512 // Float
#define IPSO_RSC_ID_MAX_Z_VALUE                        5513 // Float
#define IPSO_RSC_ID_LATITUDE                           5514 // String
#define IPSO_RSC_ID_LONGITUDE                          5515 // String
#define IPSO_RSC_ID_UNCERTAINTY                        5516 // String
#define IPSO_RSC_ID_VELOCITY                           5517 // Opaque
#define IPSO_RSC_ID_TIMESTAMP                          5518 // Time
#define IPSO_RSC_ID_MIN_LIMIT                          5519 // Float
#define IPSO_RSC_ID_MAX_LIMIT                          5520 // Float
#define IPSO_RSC_ID_DURATION                           5521 // Float
#define IPSO_RSC_ID_CLIP                               5522 // Opaque
#define IPSO_RSC_ID_TRIGGER                            5523 // Exec
#define IPSO_RSC_ID_SOUND_DURATION                     5524 // Float
#define IPSO_RSC_ID_MINIMUM_OFF_TIME                   5525 // Float
#define IPSO_RSC_ID_TIMER_MODE                         5526 // Integer
#define IPSO_RSC_ID_TEXT                               5527 // String
#define IPSO_RSC_ID_X_COORDINATE                       5528 // Integer
#define IPSO_RSC_ID_Y_COORDINATE                       5529 // Integer
#define IPSO_RSC_ID_CLEAR_DISPLAY                      5530 // Exec
#define IPSO_RSC_ID_CONTRAST                           5531 // Float
#define IPSO_RSC_ID_INCREASE_INPUT_STATE               5532 // Boolean
#define IPSO_RSC_ID_DECREASE_INPUT_STATE               5533 // Boolean
#define IPSO_RSC_ID_COUNTER                            5534 // Integer
#define IPSO_RSC_ID_CALIBRATION_OFFSET                 5535 // Float
#define IPSO_RSC_ID_CURRENT_POSITION                   5536 // Float
#define IPSO_RSC_ID_TRANSITION_TIME                    5537 // Float
#define IPSO_RSC_ID_REMAINING_TIME                     5538 // Float
#define IPSO_RSC_ID_UP_COUNTER                         5541 // Integer
#define IPSO_RSC_ID_DOWN_COUNTER                       5542 // Integer
#define IPSO_RSC_ID_DIGITAL_STATE                      5543 // Boolean
#define IPSO_RSC_ID_CUMULATIVE_TIME                    5544 // Float
#define IPSO_RSC_ID_MAX_X_COORDINATE                   5545 // Integer
#define IPSO_RSC_ID_MAX_Y_COORDINATE                   5546 // Integer
#define IPSO_RSC_ID_MULTISTATE_INPUT                   5547 // Integer
#define IPSO_RSC_ID_LEVEL                              5548 // Float
#define IPSO_RSC_ID_DIGITAL_OUTPUT_STATE               5550 // Boolean
#define IPSO_RSC_ID_DIGITAL_OUTPUT_POLARITY            5551 // Boolean
#define IPSO_RSC_ID_ANALOG_INPUT_CURRENT_VALUE         5600 // Float
#define IPSO_RSC_ID_MIN_MEASURED_VALUE                 5601 // Float
#define IPSO_RSC_ID_MAX_MEASURED_VALUE                 5602 // Float
#define IPSO_RSC_ID_MIN_RANGE_VALUE                    5603 // Float
#define IPSO_RSC_ID_MAX_RANGE_VALUE                    5604 // Float
#define IPSO_RSC_ID_RESET_MIN_AND_MAX_MEASURED_VALUES  5605 // Exec
#define IPSO_RSC_ID_ANALOG_OUTPUT_CURRENT_VALUE        5650 // Float
#define IPSO_RSC_ID_SENSOR_VALUE                       5700 // Float
#define IPSO_RSC_ID_SENSOR_UNITS                       5701 // String
#define IPSO_RSC_ID_X_VALUE                            5702 // Float
#define IPSO_RSC_ID_Y_VALUE                            5703 // Float
#define IPSO_RSC_ID_Z_VALUE                            5704 // Float
#define IPSO_RSC_ID_COMPASS_DIRECTION                  5705 // Float
#define IPSO_RSC_ID_COLOUR                             5706 // String
#define IPSO_RSC_ID_APPLICATION_TYPE                   5750 // String
#define IPSO_RSC_ID_SENSOR_TYPE                        5751 // String
#define IPSO_RSC_ID_INSTANTANEOUS_ACTIVE_POWER         5800 // Float
#define IPSO_RSC_ID_MIN_MEASURED_ACTIVE_POWER          5801 // Float
#define IPSO_RSC_ID_MAX_MEASURED_ACTIVE_POWER          5802 // Float
#define IPSO_RSC_ID_MIN_RANGE_ACTIVE_POWER             5803 // Float
#define IPSO_RSC_ID_MAX_RANGE_ACTIVE_POWER             5804 // Float
#define IPSO_RSC_ID_CUMULATIVE_ACTIVE_POWER            5805 // Float
#define IPSO_RSC_ID_ACTIVE_POWER_CALIBRATION           5806 // Float
#define IPSO_RSC_ID_INSTANTANEOUS_REACTIVE_POWER       5810 // Float
#define IPSO_RSC_ID_MIN_MEASURED_REACTIVE_POWER        5811 // Float
#define IPSO_RSC_ID_MAX_MEASURED_REACTIVE_POWER        5812 // Float
#define IPSO_RSC_ID_MIN_RANGE_REACTIVE_POWER           5813 // Float
#define IPSO_RSC_ID_MAX_RANGE_REACTIVE_POWER           5814 // Float
#define IPSO_RSC_ID_CUMULATIVE_REACTIVE_POWER          5815 // Float
#define IPSO_RSC_ID_REACTIVE_POWER_CALIBRATION         5816 // Float
#define IPSO_RSC_ID_POWER_FACTOR                       5820 // Float
#define IPSO_RSC_ID_CURRENT_CALIBRATION                5821 // Float
#define IPSO_RSC_ID_RESET_CUMULATIVE_ENERGY            5822 // Exec
#define IPSO_RSC_ID_EVENT_IDENTIFIER                   5823 // String
#define IPSO_RSC_ID_START_TIME                         5824 // Time
#define IPSO_RSC_ID_DURATION_IN_MIN                    5825 // Integer
#define IPSO_RSC_ID_CRITICALITY_LEVEL                  5826 // Integer
#define IPSO_RSC_ID_AVG_LOAD_ADJPCT                    5827 // Integer
#define IPSO_RSC_ID_DUTY_CYCLE                         5828 // Integer
#define IPSO_RSC_ID_ON_OFF                             5850 // Boolean
#define IPSO_RSC_ID_DIMMER                             5851 // Integer
#define IPSO_RSC_ID_ON_TIME                            5852 // Integer
#define IPSO_RSC_ID_MULTI_STATE_OUTPUT                 5853 // String
#define IPSO_RSC_ID_OFF_TIME                           5854 // Integer
#define IPSO_RSC_ID_SETPOINT_VALUE                     5900 // Float
#define IPSO_RSC_ID_BUSY_TO_CLEAR_DELAY                5903 // Integer
#define IPSO_RSC_ID_CLEAR_TO_BUSY_DELAY                5904 // Integer
#define IPSO_RSC_ID_HOST_DEVICE_MANUFACTURER           5905 // String
#define IPSO_RSC_ID_HOST_DEVICE_MODEL_NUMBER           5906 // String
#define IPSO_RSC_ID_HOST_DEVICE_UNIQUE_ID              5907 // String
#define IPSO_RSC_ID_HOST_DEVICE_SOFTWARE_VERSION       5908 // String
#define IPSO_RSC_ID_BITMAP_INPUT                       5910 // Integer
#define IPSO_RSC_ID_BITMAP_INPUT_RESET                 5911 // Exec
#define IPSO_RSC_ID_ELEMENT_DESCRIPTION                5912 // String
#define IPSO_RSC_ID_INTERVAL_PERIOD                    6000 // Integer
#define IPSO_RSC_ID_INTERVAL_START_OFFSET              6001 // Integer
#define IPSO_RSC_ID_INTERVAL_UTC_OFFSET                6002 // String
#define IPSO_RSC_ID_INTERVAL_COLLECTION_START_TIME     6003 // Time
#define IPSO_RSC_ID_OLDEST_RECORDED_INTERVAL           6004 // Time
#define IPSO_RSC_ID_LAST_DELIVERED_INTERVAL            6005 // Time
#define IPSO_RSC_ID_LATEST_RECORDED_INTERVAL           6006 // Time
#define IPSO_RSC_ID_INTERVAL_DLV_MIDNIGHT_ALIGNED      6007 // Boolean
#define IPSO_RSC_ID_INTERVAL_HISTORICAL_READ           6008 // Exec
#define IPSO_RSC_ID_INTERVAL_HISTORICAL_READ_PAYLOAD   6009 // Opaque
#define IPSO_RSC_ID_INTERVAL_CHANGE_CONFIGURATION      6010 // Exec
#define IPSO_RSC_ID_EVENT_TYPE                         6011 // Integer
#define IPSO_RSC_ID_ALARM_REALTIME                     6012 // Boolean
#define IPSO_RSC_ID_ALARM_STATE                        6013 // Boolean
#define IPSO_RSC_ID_ALARM_SET_THRESHOLD                6014 // Float
#define IPSO_RSC_ID_ALARM_SET_OPERATOR                 6015 // Integer
#define IPSO_RSC_ID_ALARM_CLEAR_THRESHOLD              6016 // Float
#define IPSO_RSC_ID_ALARM_CLEAR_OPERATOR               6017 // Integer
#define IPSO_RSC_ID_ALARM_MAXIMUM_EVENT_COUNT          6018 // Integer
#define IPSO_RSC_ID_ALARM_MAXIMUM_EVENT_PERIOD         6019 // Integer
#define IPSO_RSC_ID_LATEST_DELIVERED_EVENT_TIME        6020 // Time
#define IPSO_RSC_ID_LATEST_RECORDED_EVENT_TIME         6021 // Time
#define IPSO_RSC_ID_ALARM_CLEAR                        6022 // Exec
#define IPSO_RSC_ID_ALARM_AUTO_CLEAR                   6023 // Boolean
#define IPSO_RSC_ID_EVENT_CODE                         6024 // Integer
#define IPSO_RSC_ID_LATEST_EVENT_LOG_PAYLOAD           6025 // Opaque
#define IPSO_RSC_ID_START                              6026 // Exec
#define IPSO_RSC_ID_STOP                               6027 // Exec
#define IPSO_RSC_ID_STATUS                             6028 // Integer
#define IPSO_RSC_ID_LATEST_PAYLOAD                     6029 // Opaque
#define IPSO_RSC_ID_PLMN_ID                            6030 // Integer
#define IPSO_RSC_ID_BAND_INDICATOR                     6031 // Integer
#define IPSO_RSC_ID_DL_EARFCN                          6032 // Integer
#define IPSO_RSC_ID_CELL_ID                            6033 // Integer
#define IPSO_RSC_ID_PCI                                6034 // Integer
#define IPSO_RSC_ID_RSRP                               6035 // Integer
#define IPSO_RSC_ID_RSRQ                               6036 // Integer
#define IPSO_RSC_ID_SYS_FRAME_NUMBER                   6037 // Integer
#define IPSO_RSC_ID_SUB_FRAME_NUMBER                   6038 // Integer
#define IPSO_RSC_ID_ALTITUDE                           6039 // Float
#define IPSO_RSC_ID_SAMPLE_FREQUENCY                   6040 // Float

/****************************
 * Type of the LWM2M Reusable Resources defined by the IPSO Alliance.
 */
#define IPSO_RSC_TYPE_DIGITAL_INPUT_STATE                IOWA_LWM2M_TYPE_BOOLEAN    // 5500
#define IPSO_RSC_TYPE_DIGITAL_INPUT_COUNTER              IOWA_LWM2M_TYPE_INTEGER    // 5501
#define IPSO_RSC_TYPE_DIGITAL_INPUT_POLARITY             IOWA_LWM2M_TYPE_BOOLEAN    // 5502
#define IPSO_RSC_TYPE_DIGITAL_INPUT_DEBOUNCE_PERIOD      IOWA_LWM2M_TYPE_INTEGER    // 5503
#define IPSO_RSC_TYPE_DIGITAL_INPUT_EDGE_SELECTION       IOWA_LWM2M_TYPE_INTEGER    // 5504
#define IPSO_RSC_TYPE_DIGITAL_INPUT_COUNTER_RESET        IOWA_LWM2M_TYPE_UNDEFINED  // 5505
#define IPSO_RSC_TYPE_CURRENT_TIME                       IOWA_LWM2M_TYPE_TIME       // 5506
#define IPSO_RSC_TYPE_FRACTIONNAL_TIME                   IOWA_LWM2M_TYPE_FLOAT      // 5507
#define IPSO_RSC_TYPE_MIN_X_VALUE                        IOWA_LWM2M_TYPE_FLOAT      // 5508
#define IPSO_RSC_TYPE_MAX_X_VALUE                        IOWA_LWM2M_TYPE_FLOAT      // 5509
#define IPSO_RSC_TYPE_MIN_Y_VALUE                        IOWA_LWM2M_TYPE_FLOAT      // 5510
#define IPSO_RSC_TYPE_MAX_Y_VALUE                        IOWA_LWM2M_TYPE_FLOAT      // 5511
#define IPSO_RSC_TYPE_MIN_Z_VALUE                        IOWA_LWM2M_TYPE_FLOAT      // 5512
#define IPSO_RSC_TYPE_MAX_Z_VALUE                        IOWA_LWM2M_TYPE_FLOAT      // 5513
#define IPSO_RSC_TYPE_LATITUDE                           IOWA_LWM2M_TYPE_STRING     // 5514
#define IPSO_RSC_TYPE_LONGITUDE                          IOWA_LWM2M_TYPE_STRING     // 5515
#define IPSO_RSC_TYPE_UNCERTAINTY                        IOWA_LWM2M_TYPE_STRING     // 5516
#define IPSO_RSC_TYPE_VELOCITY                           IOWA_LWM2M_TYPE_OPAQUE     // 5517
#define IPSO_RSC_TYPE_TIMESTAMP                          IOWA_LWM2M_TYPE_TIME       // 5518
#define IPSO_RSC_TYPE_MIN_LIMIT                          IOWA_LWM2M_TYPE_FLOAT      // 5519
#define IPSO_RSC_TYPE_MAX_LIMIT                          IOWA_LWM2M_TYPE_FLOAT      // 5520
#define IPSO_RSC_TYPE_DURATION                           IOWA_LWM2M_TYPE_FLOAT      // 5521
#define IPSO_RSC_TYPE_CLIP                               IOWA_LWM2M_TYPE_OPAQUE     // 5522
#define IPSO_RSC_TYPE_TRIGGER                            IOWA_LWM2M_TYPE_UNDEFINED  // 5523
#define IPSO_RSC_TYPE_SOUND_DURATION                     IOWA_LWM2M_TYPE_FLOAT      // 5524
#define IPSO_RSC_TYPE_MINIMUM_OFF_TIME                   IOWA_LWM2M_TYPE_FLOAT      // 5525
#define IPSO_RSC_TYPE_TIMER_MODE                         IOWA_LWM2M_TYPE_INTEGER    // 5526
#define IPSO_RSC_TYPE_TEXT                               IOWA_LWM2M_TYPE_STRING     // 5527
#define IPSO_RSC_TYPE_X_COORDINATE                       IOWA_LWM2M_TYPE_INTEGER    // 5528
#define IPSO_RSC_TYPE_Y_COORDINATE                       IOWA_LWM2M_TYPE_INTEGER    // 5529
#define IPSO_RSC_TYPE_CLEAR_DISPLAY                      IOWA_LWM2M_TYPE_UNDEFINED  // 5530
#define IPSO_RSC_TYPE_CONTRAST                           IOWA_LWM2M_TYPE_FLOAT      // 5531
#define IPSO_RSC_TYPE_INCREASE_INPUT_STATE               IOWA_LWM2M_TYPE_BOOLEAN    // 5532
#define IPSO_RSC_TYPE_DECREASE_INPUT_STATE               IOWA_LWM2M_TYPE_BOOLEAN    // 5533
#define IPSO_RSC_TYPE_COUNTER                            IOWA_LWM2M_TYPE_INTEGER    // 5534
#define IPSO_RSC_TYPE_CALIBRATION_OFFSET                 IOWA_LWM2M_TYPE_FLOAT      // 5535
#define IPSO_RSC_TYPE_CURRENT_POSITION                   IOWA_LWM2M_TYPE_FLOAT      // 5536
#define IPSO_RSC_TYPE_TRANSITION_TIME                    IOWA_LWM2M_TYPE_FLOAT      // 5537
#define IPSO_RSC_TYPE_REMAINING_TIME                     IOWA_LWM2M_TYPE_FLOAT      // 5538
#define IPSO_RSC_TYPE_UP_COUNTER                         IOWA_LWM2M_TYPE_INTEGER    // 5541
#define IPSO_RSC_TYPE_DOWN_COUNTER                       IOWA_LWM2M_TYPE_INTEGER    // 5542
#define IPSO_RSC_TYPE_DIGITAL_STATE                      IOWA_LWM2M_TYPE_BOOLEAN    // 5543
#define IPSO_RSC_TYPE_CUMULATIVE_TIME                    IOWA_LWM2M_TYPE_FLOAT      // 5544
#define IPSO_RSC_TYPE_MAX_X_COORDINATE                   IOWA_LWM2M_TYPE_INTEGER    // 5545
#define IPSO_RSC_TYPE_MAX_Y_COORDINATE                   IOWA_LWM2M_TYPE_INTEGER    // 5546
#define IPSO_RSC_TYPE_MULTISTATE_INPUT                   IOWA_LWM2M_TYPE_INTEGER    // 5547
#define IPSO_RSC_TYPE_LEVEL                              IOWA_LWM2M_TYPE_FLOAT      // 5548
#define IPSO_RSC_TYPE_DIGITAL_OUTPUT_STATE               IOWA_LWM2M_TYPE_BOOLEAN    // 5550
#define IPSO_RSC_TYPE_DIGITAL_OUTPUT_POLARITY            IOWA_LWM2M_TYPE_BOOLEAN    // 5551
#define IPSO_RSC_TYPE_ANALOG_INPUT_CURRENT_VALUE         IOWA_LWM2M_TYPE_FLOAT      // 5600
#define IPSO_RSC_TYPE_MIN_MEASURED_VALUE                 IOWA_LWM2M_TYPE_FLOAT      // 5601
#define IPSO_RSC_TYPE_MAX_MEASURED_VALUE                 IOWA_LWM2M_TYPE_FLOAT      // 5602
#define IPSO_RSC_TYPE_MIN_RANGE_VALUE                    IOWA_LWM2M_TYPE_FLOAT      // 5603
#define IPSO_RSC_TYPE_MAX_RANGE_VALUE                    IOWA_LWM2M_TYPE_FLOAT      // 5604
#define IPSO_RSC_TYPE_RESET_MIN_AND_MAX_MEASURED_VALUES  IOWA_LWM2M_TYPE_UNDEFINED  // 5605
#define IPSO_RSC_TYPE_ANALOG_OUTPUT_CURRENT_VALUE        IOWA_LWM2M_TYPE_FLOAT      // 5650
#define IPSO_RSC_TYPE_SENSOR_VALUE                       IOWA_LWM2M_TYPE_FLOAT      // 5700
#define IPSO_RSC_TYPE_SENSOR_UNITS                       IOWA_LWM2M_TYPE_STRING     // 5701
#define IPSO_RSC_TYPE_X_VALUE                            IOWA_LWM2M_TYPE_FLOAT      // 5702
#define IPSO_RSC_TYPE_Y_VALUE                            IOWA_LWM2M_TYPE_FLOAT      // 5703
#define IPSO_RSC_TYPE_Z_VALUE                            IOWA_LWM2M_TYPE_FLOAT      // 5704
#define IPSO_RSC_TYPE_COMPASS_DIRECTION                  IOWA_LWM2M_TYPE_FLOAT      // 5705
#define IPSO_RSC_TYPE_COLOUR                             IOWA_LWM2M_TYPE_STRING     // 5706
#define IPSO_RSC_TYPE_APPLICATION_TYPE                   IOWA_LWM2M_TYPE_STRING     // 5750
#define IPSO_RSC_TYPE_SENSOR_TYPE                        IOWA_LWM2M_TYPE_STRING     // 5751
#define IPSO_RSC_TYPE_INSTANTANEOUS_ACTIVE_POWER         IOWA_LWM2M_TYPE_FLOAT      // 5800
#define IPSO_RSC_TYPE_MIN_MEASURED_ACTIVE_POWER          IOWA_LWM2M_TYPE_FLOAT      // 5801
#define IPSO_RSC_TYPE_MAX_MEASURED_ACTIVE_POWER          IOWA_LWM2M_TYPE_FLOAT      // 5802
#define IPSO_RSC_TYPE_MIN_RANGE_ACTIVE_POWER             IOWA_LWM2M_TYPE_FLOAT      // 5803
#define IPSO_RSC_TYPE_MAX_RANGE_ACTIVE_POWER             IOWA_LWM2M_TYPE_FLOAT      // 5804
#define IPSO_RSC_TYPE_CUMULATIVE_ACTIVE_POWER            IOWA_LWM2M_TYPE_FLOAT      // 5805
#define IPSO_RSC_TYPE_ACTIVE_POWER_CALIBRATION           IOWA_LWM2M_TYPE_FLOAT      // 5806
#define IPSO_RSC_TYPE_INSTANTANEOUS_REACTIVE_POWER       IOWA_LWM2M_TYPE_FLOAT      // 5810
#define IPSO_RSC_TYPE_MIN_MEASURED_REACTIVE_POWER        IOWA_LWM2M_TYPE_FLOAT      // 5811
#define IPSO_RSC_TYPE_MAX_MEASURED_REACTIVE_POWER        IOWA_LWM2M_TYPE_FLOAT      // 5812
#define IPSO_RSC_TYPE_MIN_RANGE_REACTIVE_POWER           IOWA_LWM2M_TYPE_FLOAT      // 5813
#define IPSO_RSC_TYPE_MAX_RANGE_REACTIVE_POWER           IOWA_LWM2M_TYPE_FLOAT      // 5814
#define IPSO_RSC_TYPE_CUMULATIVE_REACTIVE_POWER          IOWA_LWM2M_TYPE_FLOAT      // 5815
#define IPSO_RSC_TYPE_REACTIVE_POWER_CALIBRATION         IOWA_LWM2M_TYPE_FLOAT      // 5816
#define IPSO_RSC_TYPE_POWER_FACTOR                       IOWA_LWM2M_TYPE_FLOAT      // 5820
#define IPSO_RSC_TYPE_CURRENT_CALIBRATION                IOWA_LWM2M_TYPE_FLOAT      // 5821
#define IPSO_RSC_TYPE_RESET_CUMULATIVE_ENERGY            IOWA_LWM2M_TYPE_UNDEFINED  // 5822
#define IPSO_RSC_TYPE_EVENT_IDENTIFIER                   IOWA_LWM2M_TYPE_STRING     // 5823
#define IPSO_RSC_TYPE_START_TIME                         IOWA_LWM2M_TYPE_TIME       // 5824
#define IPSO_RSC_TYPE_DURATION_IN_MIN                    IOWA_LWM2M_TYPE_INTEGER    // 5825
#define IPSO_RSC_TYPE_CRITICALITY_LEVEL                  IOWA_LWM2M_TYPE_INTEGER    // 5826
#define IPSO_RSC_TYPE_AVG_LOAD_ADJPCT                    IOWA_LWM2M_TYPE_INTEGER    // 5827
#define IPSO_RSC_TYPE_DUTY_CYCLE                         IOWA_LWM2M_TYPE_INTEGER    // 5828
#define IPSO_RSC_TYPE_ON_OFF                             IOWA_LWM2M_TYPE_BOOLEAN    // 5850
#define IPSO_RSC_TYPE_DIMMER                             IOWA_LWM2M_TYPE_INTEGER    // 5851
#define IPSO_RSC_TYPE_ON_TIME                            IOWA_LWM2M_TYPE_INTEGER    // 5852
#define IPSO_RSC_TYPE_MULTI_STATE_OUTPUT                 IOWA_LWM2M_TYPE_STRING     // 5853
#define IPSO_RSC_TYPE_OFF_TIME                           IOWA_LWM2M_TYPE_INTEGER    // 5854
#define IPSO_RSC_TYPE_SETPOINT_VALUE                     IOWA_LWM2M_TYPE_FLOAT      // 5900
#define IPSO_RSC_TYPE_BUSY_TO_CLEAR_DELAY                IOWA_LWM2M_TYPE_INTEGER    // 5903
#define IPSO_RSC_TYPE_CLEAR_TO_BUSY_DELAY                IOWA_LWM2M_TYPE_INTEGER    // 5904
#define IPSO_RSC_TYPE_HOST_DEVICE_MANUFACTURER           IOWA_LWM2M_TYPE_STRING     // 5905
#define IPSO_RSC_TYPE_HOST_DEVICE_MODEL_NUMBER           IOWA_LWM2M_TYPE_STRING     // 5906
#define IPSO_RSC_TYPE_HOST_DEVICE_UNIQUE_ID              IOWA_LWM2M_TYPE_STRING     // 5907
#define IPSO_RSC_TYPE_HOST_DEVICE_SOFTWARE_VERSION       IOWA_LWM2M_TYPE_STRING     // 5908
#define IPSO_RSC_TYPE_BITMAP_INPUT                       IOWA_LWM2M_TYPE_INTEGER    // 5910
#define IPSO_RSC_TYPE_BITMAP_INPUT_RESET                 IOWA_LWM2M_TYPE_UNDEFINED  // 5911
#define IPSO_RSC_TYPE_ELEMENT_DESCRIPTION                IOWA_LWM2M_TYPE_STRING     // 5912
#define IPSO_RSC_TYPE_INTERVAL_PERIOD                    IOWA_LWM2M_TYPE_INTEGER    // 6000
#define IPSO_RSC_TYPE_INTERVAL_START_OFFSET              IOWA_LWM2M_TYPE_INTEGER    // 6001
#define IPSO_RSC_TYPE_INTERVAL_UTC_OFFSET                IOWA_LWM2M_TYPE_STRING     // 6002
#define IPSO_RSC_TYPE_INTERVAL_COLLECTION_START_TIME     IOWA_LWM2M_TYPE_TIME       // 6003
#define IPSO_RSC_TYPE_OLDEST_RECORDED_INTERVAL           IOWA_LWM2M_TYPE_TIME       // 6004
#define IPSO_RSC_TYPE_LAST_DELIVERED_INTERVAL            IOWA_LWM2M_TYPE_TIME       // 6005
#define IPSO_RSC_TYPE_LATEST_RECORDED_INTERVAL           IOWA_LWM2M_TYPE_TIME       // 6006
#define IPSO_RSC_TYPE_INTERVAL_DLV_MIDNIGHT_ALIGNED      IOWA_LWM2M_TYPE_BOOLEAN    // 6007
#define IPSO_RSC_TYPE_INTERVAL_HISTORICAL_READ           IOWA_LWM2M_TYPE_UNDEFINED  // 6008
#define IPSO_RSC_TYPE_INTERVAL_HISTORICAL_READ_PAYLOAD   IOWA_LWM2M_TYPE_OPAQUE     // 6009
#define IPSO_RSC_TYPE_INTERVAL_CHANGE_CONFIGURATION      IOWA_LWM2M_TYPE_UNDEFINED  // 6010
#define IPSO_RSC_TYPE_EVENT_TYPE                         IOWA_LWM2M_TYPE_INTEGER    // 6011
#define IPSO_RSC_TYPE_ALARM_REALTIME                     IOWA_LWM2M_TYPE_BOOLEAN    // 6012
#define IPSO_RSC_TYPE_ALARM_STATE                        IOWA_LWM2M_TYPE_BOOLEAN    // 6013
#define IPSO_RSC_TYPE_ALARM_SET_THRESHOLD                IOWA_LWM2M_TYPE_FLOAT      // 6014
#define IPSO_RSC_TYPE_ALARM_SET_OPERATOR                 IOWA_LWM2M_TYPE_INTEGER    // 6015
#define IPSO_RSC_TYPE_ALARM_CLEAR_THRESHOLD              IOWA_LWM2M_TYPE_FLOAT      // 6016
#define IPSO_RSC_TYPE_ALARM_CLEAR_OPERATOR               IOWA_LWM2M_TYPE_INTEGER    // 6017
#define IPSO_RSC_TYPE_ALARM_MAXIMUM_EVENT_COUNT          IOWA_LWM2M_TYPE_INTEGER    // 6018
#define IPSO_RSC_TYPE_ALARM_MAXIMUM_EVENT_PERIOD         IOWA_LWM2M_TYPE_INTEGER    // 6019
#define IPSO_RSC_TYPE_LATEST_DELIVERED_EVENT_TIME        IOWA_LWM2M_TYPE_TIME       // 6020
#define IPSO_RSC_TYPE_LATEST_RECORDED_EVENT_TIME         IOWA_LWM2M_TYPE_TIME       // 6021
#define IPSO_RSC_TYPE_ALARM_CLEAR                        IOWA_LWM2M_TYPE_UNDEFINED  // 6022
#define IPSO_RSC_TYPE_ALARM_AUTO_CLEAR                   IOWA_LWM2M_TYPE_BOOLEAN    // 6023
#define IPSO_RSC_TYPE_EVENT_CODE                         IOWA_LWM2M_TYPE_INTEGER    // 6024
#define IPSO_RSC_TYPE_LATEST_EVENT_LOG_PAYLOAD           IOWA_LWM2M_TYPE_OPAQUE     // 6025
#define IPSO_RSC_TYPE_START                              IOWA_LWM2M_TYPE_UNDEFINED  // 6026
#define IPSO_RSC_TYPE_STOP                               IOWA_LWM2M_TYPE_UNDEFINED  // 6027
#define IPSO_RSC_TYPE_STATUS                             IOWA_LWM2M_TYPE_INTEGER    // 6028
#define IPSO_RSC_TYPE_LATEST_PAYLOAD                     IOWA_LWM2M_TYPE_OPAQUE     // 6029
#define IPSO_RSC_TYPE_PLMN_ID                            IOWA_LWM2M_TYPE_INTEGER    // 6030
#define IPSO_RSC_TYPE_BAND_INDICATOR                     IOWA_LWM2M_TYPE_INTEGER    // 6031
#define IPSO_RSC_TYPE_DL_EARFCN                          IOWA_LWM2M_TYPE_INTEGER    // 6032
#define IPSO_RSC_TYPE_CELL_ID                            IOWA_LWM2M_TYPE_INTEGER    // 6033
#define IPSO_RSC_TYPE_PCI                                IOWA_LWM2M_TYPE_INTEGER    // 6034
#define IPSO_RSC_TYPE_RSRP                               IOWA_LWM2M_TYPE_INTEGER    // 6035
#define IPSO_RSC_TYPE_RSRQ                               IOWA_LWM2M_TYPE_INTEGER    // 6036
#define IPSO_RSC_TYPE_SYS_FRAME_NUMBER                   IOWA_LWM2M_TYPE_INTEGER    // 6037
#define IPSO_RSC_TYPE_SUB_FRAME_NUMBER                   IOWA_LWM2M_TYPE_INTEGER    // 6038
#define IPSO_RSC_TYPE_ALTITUDE                           IOWA_LWM2M_TYPE_FLOAT      // 6039
#define IPSO_RSC_TYPE_SAMPLE_FREQUENCY                   IOWA_LWM2M_TYPE_FLOAT      // 6040

/****************************
 * Operations allowed on the LWM2M Reusable Resources defined by the IPSO Alliance.
 */
#define IPSO_RSC_OP_DIGITAL_INPUT_STATE                IOWA_OPERATION_READ                          // 5500
#define IPSO_RSC_OP_DIGITAL_INPUT_COUNTER              IOWA_OPERATION_READ                          // 5501
#define IPSO_RSC_OP_DIGITAL_INPUT_POLARITY             (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5502
#define IPSO_RSC_OP_DIGITAL_INPUT_DEBOUNCE_PERIOD      (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5503
#define IPSO_RSC_OP_DIGITAL_INPUT_EDGE_SELECTION       (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5504
#define IPSO_RSC_OP_DIGITAL_INPUT_COUNTER_RESET        IOWA_OPERATION_EXECUTE                       // 5505
#define IPSO_RSC_OP_CURRENT_TIME                       (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5506
#define IPSO_RSC_OP_FRACTIONNAL_TIME                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5507
#define IPSO_RSC_OP_MIN_X_VALUE                        IOWA_OPERATION_READ                          // 5508
#define IPSO_RSC_OP_MAX_X_VALUE                        IOWA_OPERATION_READ                          // 5509
#define IPSO_RSC_OP_MIN_Y_VALUE                        IOWA_OPERATION_READ                          // 5510
#define IPSO_RSC_OP_MAX_Y_VALUE                        IOWA_OPERATION_READ                          // 5511
#define IPSO_RSC_OP_MIN_Z_VALUE                        IOWA_OPERATION_READ                          // 5512
#define IPSO_RSC_OP_MAX_Z_VALUE                        IOWA_OPERATION_READ                          // 5513
#define IPSO_RSC_OP_LATITUDE                           IOWA_OPERATION_READ                          // 5514
#define IPSO_RSC_OP_LONGITUDE                          IOWA_OPERATION_READ                          // 5515
#define IPSO_RSC_OP_UNCERTAINTY                        IOWA_OPERATION_READ                          // 5516
#define IPSO_RSC_OP_VELOCITY                           IOWA_OPERATION_READ                          // 5517
#define IPSO_RSC_OP_TIMESTAMP                          IOWA_OPERATION_READ                          // 5518
#define IPSO_RSC_OP_MIN_LIMIT                          IOWA_OPERATION_READ                          // 5519
#define IPSO_RSC_OP_MAX_LIMIT                          IOWA_OPERATION_READ                          // 5520
#define IPSO_RSC_OP_DURATION                           (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5521
#define IPSO_RSC_OP_CLIP                               (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5522
#define IPSO_RSC_OP_TRIGGER                            IOWA_OPERATION_EXECUTE                       // 5523
#define IPSO_RSC_OP_SOUND_DURATION                     (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5524
#define IPSO_RSC_OP_MINIMUM_OFF_TIME                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5525
#define IPSO_RSC_OP_TIMER_MODE                         (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5526
#define IPSO_RSC_OP_TEXT                               (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5527
#define IPSO_RSC_OP_X_COORDINATE                       (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5528
#define IPSO_RSC_OP_Y_COORDINATE                       (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5529
#define IPSO_RSC_OP_CLEAR_DISPLAY                      IOWA_OPERATION_EXECUTE                       // 5530
#define IPSO_RSC_OP_CONTRAST                           (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5531
#define IPSO_RSC_OP_INCREASE_INPUT_STATE               IOWA_OPERATION_READ                          // 5532
#define IPSO_RSC_OP_DECREASE_INPUT_STATE               IOWA_OPERATION_READ                          // 5533
#define IPSO_RSC_OP_COUNTER                            (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5534
#define IPSO_RSC_OP_CALIBRATION_OFFSET                 (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5535
#define IPSO_RSC_OP_CURRENT_POSITION                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5536
#define IPSO_RSC_OP_TRANSITION_TIME                    (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5537
#define IPSO_RSC_OP_REMAINING_TIME                     IOWA_OPERATION_READ                          // 5538
#define IPSO_RSC_OP_UP_COUNTER                         (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5541
#define IPSO_RSC_OP_DOWN_COUNTER                       (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5542
#define IPSO_RSC_OP_DIGITAL_STATE                      IOWA_OPERATION_READ                          // 5543
#define IPSO_RSC_OP_CUMULATIVE_TIME                    (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5544
#define IPSO_RSC_OP_MAX_X_COORDINATE                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5545
#define IPSO_RSC_OP_MAX_Y_COORDINATE                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5546
#define IPSO_RSC_OP_MULTISTATE_INPUT                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5547
#define IPSO_RSC_OP_LEVEL                              (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5548
#define IPSO_RSC_OP_DIGITAL_OUTPUT_STATE               (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5550
#define IPSO_RSC_OP_DIGITAL_OUTPUT_POLARITY            (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5551
#define IPSO_RSC_OP_ANALOG_INPUT_CURRENT_VALUE         IOWA_OPERATION_READ                          // 5600
#define IPSO_RSC_OP_MIN_MEASURED_VALUE                 IOWA_OPERATION_READ                          // 5601
#define IPSO_RSC_OP_MAX_MEASURED_VALUE                 IOWA_OPERATION_READ                          // 5602
#define IPSO_RSC_OP_MIN_RANGE_VALUE                    IOWA_OPERATION_READ                          // 5603
#define IPSO_RSC_OP_MAX_RANGE_VALUE                    IOWA_OPERATION_READ                          // 5604
#define IPSO_RSC_OP_RESET_MIN_AND_MAX_MEASURED_VALUES  IOWA_OPERATION_EXECUTE                       // 5605
#define IPSO_RSC_OP_ANALOG_OUTPUT_CURRENT_VALUE        (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5650
#define IPSO_RSC_OP_SENSOR_VALUE                       IOWA_OPERATION_READ                          // 5700
#define IPSO_RSC_OP_SENSOR_UNITS                       IOWA_OPERATION_READ                          // 5701
#define IPSO_RSC_OP_X_VALUE                            IOWA_OPERATION_READ                          // 5702
#define IPSO_RSC_OP_Y_VALUE                            IOWA_OPERATION_READ                          // 5703
#define IPSO_RSC_OP_Z_VALUE                            IOWA_OPERATION_READ                          // 5704
#define IPSO_RSC_OP_COMPASS_DIRECTION                  IOWA_OPERATION_READ                          // 5705
#define IPSO_RSC_OP_COLOUR                             (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5706
#define IPSO_RSC_OP_APPLICATION_TYPE                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5750
#define IPSO_RSC_OP_SENSOR_TYPE                        IOWA_OPERATION_READ                          // 5751
#define IPSO_RSC_OP_INSTANTANEOUS_ACTIVE_POWER         IOWA_OPERATION_READ                          // 5800
#define IPSO_RSC_OP_MIN_MEASURED_ACTIVE_POWER          IOWA_OPERATION_READ                          // 5801
#define IPSO_RSC_OP_MAX_MEASURED_ACTIVE_POWER          IOWA_OPERATION_READ                          // 5802
#define IPSO_RSC_OP_MIN_RANGE_ACTIVE_POWER             IOWA_OPERATION_READ                          // 5803
#define IPSO_RSC_OP_MAX_RANGE_ACTIVE_POWER             IOWA_OPERATION_READ                          // 5804
#define IPSO_RSC_OP_CUMULATIVE_ACTIVE_POWER            IOWA_OPERATION_READ                          // 5805
#define IPSO_RSC_OP_ACTIVE_POWER_CALIBRATION           IOWA_OPERATION_WRITE                         // 5806
#define IPSO_RSC_OP_INSTANTANEOUS_REACTIVE_POWER       IOWA_OPERATION_READ                          // 5810
#define IPSO_RSC_OP_MIN_MEASURED_REACTIVE_POWER        IOWA_OPERATION_READ                          // 5811
#define IPSO_RSC_OP_MAX_MEASURED_REACTIVE_POWER        IOWA_OPERATION_READ                          // 5812
#define IPSO_RSC_OP_MIN_RANGE_REACTIVE_POWER           IOWA_OPERATION_READ                          // 5813
#define IPSO_RSC_OP_MAX_RANGE_REACTIVE_POWER           IOWA_OPERATION_READ                          // 5814
#define IPSO_RSC_OP_CUMULATIVE_REACTIVE_POWER          IOWA_OPERATION_READ                          // 5815
#define IPSO_RSC_OP_REACTIVE_POWER_CALIBRATION         IOWA_OPERATION_WRITE                         // 5816
#define IPSO_RSC_OP_POWER_FACTOR                       IOWA_OPERATION_READ                          // 5820
#define IPSO_RSC_OP_CURRENT_CALIBRATION                (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5821
#define IPSO_RSC_OP_RESET_CUMULATIVE_ENERGY            IOWA_OPERATION_EXECUTE                       // 5822
#define IPSO_RSC_OP_EVENT_IDENTIFIER                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5823
#define IPSO_RSC_OP_START_TIME                         (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5824
#define IPSO_RSC_OP_DURATION_IN_MIN                    (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5825
#define IPSO_RSC_OP_CRITICALITY_LEVEL                  (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5826
#define IPSO_RSC_OP_AVG_LOAD_ADJPCT                    (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5827
#define IPSO_RSC_OP_DUTY_CYCLE                         (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5828
#define IPSO_RSC_OP_ON_OFF                             (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5850
#define IPSO_RSC_OP_DIMMER                             (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5851
#define IPSO_RSC_OP_ON_TIME                            (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5852
#define IPSO_RSC_OP_MULTI_STATE_OUTPUT                 (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5853
#define IPSO_RSC_OP_OFF_TIME                           (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5854
#define IPSO_RSC_OP_SETPOINT_VALUE                     (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5900
#define IPSO_RSC_OP_BUSY_TO_CLEAR_DELAY                (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5903
#define IPSO_RSC_OP_CLEAR_TO_BUSY_DELAY                (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5904
#define IPSO_RSC_OP_HOST_DEVICE_MANUFACTURER           IOWA_OPERATION_READ                          // 5905
#define IPSO_RSC_OP_HOST_DEVICE_MODEL_NUMBER           IOWA_OPERATION_READ                          // 5906
#define IPSO_RSC_OP_HOST_DEVICE_UNIQUE_ID              IOWA_OPERATION_READ                          // 5907
#define IPSO_RSC_OP_HOST_DEVICE_SOFTWARE_VERSION       IOWA_OPERATION_READ                          // 5908
#define IPSO_RSC_OP_BITMAP_INPUT                       IOWA_OPERATION_READ                          // 5910
#define IPSO_RSC_OP_BITMAP_INPUT_RESET                 IOWA_OPERATION_EXECUTE                       // 5911
#define IPSO_RSC_OP_ELEMENT_DESCRIPTION                (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 5912
#define IPSO_RSC_OP_INTERVAL_PERIOD                    IOWA_OPERATION_READ                          // 6000
#define IPSO_RSC_OP_INTERVAL_START_OFFSET              IOWA_OPERATION_READ                          // 6001
#define IPSO_RSC_OP_INTERVAL_UTC_OFFSET                IOWA_OPERATION_READ                          // 6002
#define IPSO_RSC_OP_INTERVAL_COLLECTION_START_TIME     IOWA_OPERATION_READ                          // 6003
#define IPSO_RSC_OP_OLDEST_RECORDED_INTERVAL           IOWA_OPERATION_READ                          // 6004
#define IPSO_RSC_OP_LAST_DELIVERED_INTERVAL            (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6005
#define IPSO_RSC_OP_LATEST_RECORDED_INTERVAL           IOWA_OPERATION_READ                          // 6006
#define IPSO_RSC_OP_INTERVAL_DLV_MIDNIGHT_ALIGNED      (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6007
#define IPSO_RSC_OP_INTERVAL_HISTORICAL_READ           IOWA_OPERATION_EXECUTE                       // 6008
#define IPSO_RSC_OP_INTERVAL_HISTORICAL_READ_PAYLOAD   IOWA_OPERATION_READ                          // 6009
#define IPSO_RSC_OP_INTERVAL_CHANGE_CONFIGURATION      IOWA_OPERATION_EXECUTE                       // 6010
#define IPSO_RSC_OP_EVENT_TYPE                         (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6011
#define IPSO_RSC_OP_ALARM_REALTIME                     (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6012
#define IPSO_RSC_OP_ALARM_STATE                        IOWA_OPERATION_READ                          // 6013
#define IPSO_RSC_OP_ALARM_SET_THRESHOLD                (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6014
#define IPSO_RSC_OP_ALARM_SET_OPERATOR                 (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6015
#define IPSO_RSC_OP_ALARM_CLEAR_THRESHOLD              (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6016
#define IPSO_RSC_OP_ALARM_CLEAR_OPERATOR               (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6017
#define IPSO_RSC_OP_ALARM_MAXIMUM_EVENT_COUNT          (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6018
#define IPSO_RSC_OP_ALARM_MAXIMUM_EVENT_PERIOD         (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6019
#define IPSO_RSC_OP_LATEST_DELIVERED_EVENT_TIME        (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6020
#define IPSO_RSC_OP_LATEST_RECORDED_EVENT_TIME         IOWA_OPERATION_READ                          // 6021
#define IPSO_RSC_OP_ALARM_CLEAR                        IOWA_OPERATION_EXECUTE                       // 6022
#define IPSO_RSC_OP_ALARM_AUTO_CLEAR                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6023
#define IPSO_RSC_OP_EVENT_CODE                         IOWA_OPERATION_READ                          // 6024
#define IPSO_RSC_OP_LATEST_EVENT_LOG_PAYLOAD           IOWA_OPERATION_READ                          // 6025
#define IPSO_RSC_OP_START                              IOWA_OPERATION_EXECUTE                       // 6026
#define IPSO_RSC_OP_STOP                               IOWA_OPERATION_EXECUTE                       // 6027
#define IPSO_RSC_OP_STATUS                             IOWA_OPERATION_READ                          // 6028
#define IPSO_RSC_OP_LATEST_PAYLOAD                     IOWA_OPERATION_READ                          // 6029
#define IPSO_RSC_OP_PLMN_ID                            IOWA_OPERATION_READ                          // 6030
#define IPSO_RSC_OP_BAND_INDICATOR                     IOWA_OPERATION_READ                          // 6031
#define IPSO_RSC_OP_DL_EARFCN                          IOWA_OPERATION_READ                          // 6032
#define IPSO_RSC_OP_CELL_ID                            IOWA_OPERATION_READ                          // 6033
#define IPSO_RSC_OP_PCI                                IOWA_OPERATION_READ                          // 6034
#define IPSO_RSC_OP_RSRP                               IOWA_OPERATION_READ                          // 6035
#define IPSO_RSC_OP_RSRQ                               IOWA_OPERATION_READ                          // 6036
#define IPSO_RSC_OP_SYS_FRAME_NUMBER                   IOWA_OPERATION_READ                          // 6037
#define IPSO_RSC_OP_SUB_FRAME_NUMBER                   IOWA_OPERATION_READ                          // 6038
#define IPSO_RSC_OP_ALTITUDE                           IOWA_OPERATION_READ                          // 6039
#define IPSO_RSC_OP_SAMPLE_FREQUENCY                   (IOWA_OPERATION_READ | IOWA_OPERATION_WRITE) // 6040

/****************************
 * Instance of the LWM2M Reusable Resources defined by the IPSO Alliance.
 */
#define IPSO_RSC_FLAGS_DIGITAL_INPUT_STATE                IOWA_RESOURCE_FLAG_NONE // 5500
#define IPSO_RSC_FLAGS_DIGITAL_INPUT_COUNTER              IOWA_RESOURCE_FLAG_NONE // 5501
#define IPSO_RSC_FLAGS_DIGITAL_INPUT_POLARITY             IOWA_RESOURCE_FLAG_NONE // 5502
#define IPSO_RSC_FLAGS_DIGITAL_INPUT_DEBOUNCE_PERIOD      IOWA_RESOURCE_FLAG_NONE // 5503
#define IPSO_RSC_FLAGS_DIGITAL_INPUT_EDGE_SELECTION       IOWA_RESOURCE_FLAG_NONE // 5504
#define IPSO_RSC_FLAGS_DIGITAL_INPUT_COUNTER_RESET        IOWA_RESOURCE_FLAG_NONE // 5505
#define IPSO_RSC_FLAGS_CURRENT_TIME                       IOWA_RESOURCE_FLAG_NONE // 5506
#define IPSO_RSC_FLAGS_FRACTIONNAL_TIME                   IOWA_RESOURCE_FLAG_NONE // 5507
#define IPSO_RSC_FLAGS_MIN_X_VALUE                        IOWA_RESOURCE_FLAG_NONE // 5508
#define IPSO_RSC_FLAGS_MAX_X_VALUE                        IOWA_RESOURCE_FLAG_NONE // 5509
#define IPSO_RSC_FLAGS_MIN_Y_VALUE                        IOWA_RESOURCE_FLAG_NONE // 5510
#define IPSO_RSC_FLAGS_MAX_Y_VALUE                        IOWA_RESOURCE_FLAG_NONE // 5511
#define IPSO_RSC_FLAGS_MIN_Z_VALUE                        IOWA_RESOURCE_FLAG_NONE // 5512
#define IPSO_RSC_FLAGS_MAX_Z_VALUE                        IOWA_RESOURCE_FLAG_NONE // 5513
#define IPSO_RSC_FLAGS_LATITUDE                           IOWA_RESOURCE_FLAG_NONE // 5514
#define IPSO_RSC_FLAGS_LONGITUDE                          IOWA_RESOURCE_FLAG_NONE // 5515
#define IPSO_RSC_FLAGS_UNCERTAINTY                        IOWA_RESOURCE_FLAG_NONE // 5516
#define IPSO_RSC_FLAGS_VELOCITY                           IOWA_RESOURCE_FLAG_NONE // 5517
#define IPSO_RSC_FLAGS_TIMESTAMP                          IOWA_RESOURCE_FLAG_NONE // 5518
#define IPSO_RSC_FLAGS_MIN_LIMIT                          IOWA_RESOURCE_FLAG_NONE // 5519
#define IPSO_RSC_FLAGS_MAX_LIMIT                          IOWA_RESOURCE_FLAG_NONE // 5520
#define IPSO_RSC_FLAGS_DURATION                           IOWA_RESOURCE_FLAG_NONE // 5521
#define IPSO_RSC_FLAGS_CLIP                               IOWA_RESOURCE_FLAG_NONE // 5522
#define IPSO_RSC_FLAGS_TRIGGER                            IOWA_RESOURCE_FLAG_NONE // 5523
#define IPSO_RSC_FLAGS_SOUND_DURATION                     IOWA_RESOURCE_FLAG_NONE // 5524
#define IPSO_RSC_FLAGS_MINIMUM_OFF_TIME                   IOWA_RESOURCE_FLAG_NONE // 5525
#define IPSO_RSC_FLAGS_TIMER_MODE                         IOWA_RESOURCE_FLAG_NONE // 5526
#define IPSO_RSC_FLAGS_TEXT                               IOWA_RESOURCE_FLAG_NONE // 5527
#define IPSO_RSC_FLAGS_X_COORDINATE                       IOWA_RESOURCE_FLAG_NONE // 5528
#define IPSO_RSC_FLAGS_Y_COORDINATE                       IOWA_RESOURCE_FLAG_NONE // 5529
#define IPSO_RSC_FLAGS_CLEAR_DISPLAY                      IOWA_RESOURCE_FLAG_NONE // 5530
#define IPSO_RSC_FLAGS_CONTRAST                           IOWA_RESOURCE_FLAG_NONE // 5531
#define IPSO_RSC_FLAGS_INCREASE_INPUT_STATE               IOWA_RESOURCE_FLAG_NONE // 5532
#define IPSO_RSC_FLAGS_DECREASE_INPUT_STATE               IOWA_RESOURCE_FLAG_NONE // 5533
#define IPSO_RSC_FLAGS_COUNTER                            IOWA_RESOURCE_FLAG_NONE // 5534
#define IPSO_RSC_FLAGS_CALIBRATION_OFFSET                 IOWA_RESOURCE_FLAG_NONE // 5535
#define IPSO_RSC_FLAGS_CURRENT_POSITION                   IOWA_RESOURCE_FLAG_NONE // 5536
#define IPSO_RSC_FLAGS_TRANSITION_TIME                    IOWA_RESOURCE_FLAG_NONE // 5537
#define IPSO_RSC_FLAGS_REMAINING_TIME                     IOWA_RESOURCE_FLAG_NONE // 5538
#define IPSO_RSC_FLAGS_UP_COUNTER                         IOWA_RESOURCE_FLAG_NONE // 5541
#define IPSO_RSC_FLAGS_DOWN_COUNTER                       IOWA_RESOURCE_FLAG_NONE // 5542
#define IPSO_RSC_FLAGS_DIGITAL_STATE                      IOWA_RESOURCE_FLAG_NONE // 5543
#define IPSO_RSC_FLAGS_CUMULATIVE_TIME                    IOWA_RESOURCE_FLAG_NONE // 5544
#define IPSO_RSC_FLAGS_MAX_X_COORDINATE                   IOWA_RESOURCE_FLAG_NONE // 5545
#define IPSO_RSC_FLAGS_MAX_Y_COORDINATE                   IOWA_RESOURCE_FLAG_NONE // 5546
#define IPSO_RSC_FLAGS_MULTISTATE_INPUT                   IOWA_RESOURCE_FLAG_NONE // 5547
#define IPSO_RSC_FLAGS_LEVEL                              IOWA_RESOURCE_FLAG_NONE // 5548
#define IPSO_RSC_FLAGS_DIGITAL_OUTPUT_STATE               IOWA_RESOURCE_FLAG_NONE // 5550
#define IPSO_RSC_FLAGS_DIGITAL_OUTPUT_POLARITY            IOWA_RESOURCE_FLAG_NONE // 5551
#define IPSO_RSC_FLAGS_ANALOG_INPUT_CURRENT_VALUE         IOWA_RESOURCE_FLAG_NONE // 5600
#define IPSO_RSC_FLAGS_MIN_MEASURED_VALUE                 IOWA_RESOURCE_FLAG_NONE // 5601
#define IPSO_RSC_FLAGS_MAX_MEASURED_VALUE                 IOWA_RESOURCE_FLAG_NONE // 5602
#define IPSO_RSC_FLAGS_MIN_RANGE_VALUE                    IOWA_RESOURCE_FLAG_NONE // 5603
#define IPSO_RSC_FLAGS_MAX_RANGE_VALUE                    IOWA_RESOURCE_FLAG_NONE // 5604
#define IPSO_RSC_FLAGS_RESET_MIN_AND_MAX_MEASURED_VALUES  IOWA_RESOURCE_FLAG_NONE // 5605
#define IPSO_RSC_FLAGS_ANALOG_OUTPUT_CURRENT_VALUE        IOWA_RESOURCE_FLAG_NONE // 5650
#define IPSO_RSC_FLAGS_SENSOR_VALUE                       IOWA_RESOURCE_FLAG_NONE // 5700
#define IPSO_RSC_FLAGS_SENSOR_UNITS                       IOWA_RESOURCE_FLAG_NONE // 5701
#define IPSO_RSC_FLAGS_X_VALUE                            IOWA_RESOURCE_FLAG_NONE // 5702
#define IPSO_RSC_FLAGS_Y_VALUE                            IOWA_RESOURCE_FLAG_NONE // 5703
#define IPSO_RSC_FLAGS_Z_VALUE                            IOWA_RESOURCE_FLAG_NONE // 5704
#define IPSO_RSC_FLAGS_COMPASS_DIRECTION                  IOWA_RESOURCE_FLAG_NONE // 5705
#define IPSO_RSC_FLAGS_COLOUR                             IOWA_RESOURCE_FLAG_NONE // 5706
#define IPSO_RSC_FLAGS_APPLICATION_TYPE                   IOWA_RESOURCE_FLAG_NONE // 5750
#define IPSO_RSC_FLAGS_SENSOR_TYPE                        IOWA_RESOURCE_FLAG_NONE // 5751
#define IPSO_RSC_FLAGS_INSTANTANEOUS_ACTIVE_POWER         IOWA_RESOURCE_FLAG_NONE // 5800
#define IPSO_RSC_FLAGS_MIN_MEASURED_ACTIVE_POWER          IOWA_RESOURCE_FLAG_NONE // 5801
#define IPSO_RSC_FLAGS_MAX_MEASURED_ACTIVE_POWER          IOWA_RESOURCE_FLAG_NONE // 5802
#define IPSO_RSC_FLAGS_MIN_RANGE_ACTIVE_POWER             IOWA_RESOURCE_FLAG_NONE // 5803
#define IPSO_RSC_FLAGS_MAX_RANGE_ACTIVE_POWER             IOWA_RESOURCE_FLAG_NONE // 5804
#define IPSO_RSC_FLAGS_CUMULATIVE_ACTIVE_POWER            IOWA_RESOURCE_FLAG_NONE // 5805
#define IPSO_RSC_FLAGS_ACTIVE_POWER_CALIBRATION           IOWA_RESOURCE_FLAG_NONE // 5806
#define IPSO_RSC_FLAGS_INSTANTANEOUS_REACTIVE_POWER       IOWA_RESOURCE_FLAG_NONE // 5810
#define IPSO_RSC_FLAGS_MIN_MEASURED_REACTIVE_POWER        IOWA_RESOURCE_FLAG_NONE // 5811
#define IPSO_RSC_FLAGS_MAX_MEASURED_REACTIVE_POWER        IOWA_RESOURCE_FLAG_NONE // 5812
#define IPSO_RSC_FLAGS_MIN_RANGE_REACTIVE_POWER           IOWA_RESOURCE_FLAG_NONE // 5813
#define IPSO_RSC_FLAGS_MAX_RANGE_REACTIVE_POWER           IOWA_RESOURCE_FLAG_NONE // 5814
#define IPSO_RSC_FLAGS_CUMULATIVE_REACTIVE_POWER          IOWA_RESOURCE_FLAG_NONE // 5815
#define IPSO_RSC_FLAGS_REACTIVE_POWER_CALIBRATION         IOWA_RESOURCE_FLAG_NONE // 5816
#define IPSO_RSC_FLAGS_POWER_FACTOR                       IOWA_RESOURCE_FLAG_NONE // 5820
#define IPSO_RSC_FLAGS_CURRENT_CALIBRATION                IOWA_RESOURCE_FLAG_NONE // 5821
#define IPSO_RSC_FLAGS_RESET_CUMULATIVE_ENERGY            IOWA_RESOURCE_FLAG_NONE // 5822
#define IPSO_RSC_FLAGS_EVENT_IDENTIFIER                   IOWA_RESOURCE_FLAG_NONE // 5823
#define IPSO_RSC_FLAGS_START_TIME                         IOWA_RESOURCE_FLAG_NONE // 5824
#define IPSO_RSC_FLAGS_DURATION_IN_MIN                    IOWA_RESOURCE_FLAG_NONE // 5825
#define IPSO_RSC_FLAGS_CRITICALITY_LEVEL                  IOWA_RESOURCE_FLAG_NONE // 5826
#define IPSO_RSC_FLAGS_AVG_LOAD_ADJPCT                    IOWA_RESOURCE_FLAG_NONE // 5827
#define IPSO_RSC_FLAGS_DUTY_CYCLE                         IOWA_RESOURCE_FLAG_NONE // 5828
#define IPSO_RSC_FLAGS_ON_OFF                             IOWA_RESOURCE_FLAG_NONE // 5850
#define IPSO_RSC_FLAGS_DIMMER                             IOWA_RESOURCE_FLAG_NONE // 5851
#define IPSO_RSC_FLAGS_ON_TIME                            IOWA_RESOURCE_FLAG_NONE // 5852
#define IPSO_RSC_FLAGS_MULTI_STATE_OUTPUT                 IOWA_RESOURCE_FLAG_NONE // 5853
#define IPSO_RSC_FLAGS_OFF_TIME                           IOWA_RESOURCE_FLAG_NONE // 5854
#define IPSO_RSC_FLAGS_SETPOINT_VALUE                     IOWA_RESOURCE_FLAG_NONE // 5900
#define IPSO_RSC_FLAGS_BUSY_TO_CLEAR_DELAY                IOWA_RESOURCE_FLAG_NONE // 5903
#define IPSO_RSC_FLAGS_CLEAR_TO_BUSY_DELAY                IOWA_RESOURCE_FLAG_NONE // 5904
#define IPSO_RSC_FLAGS_HOST_DEVICE_MANUFACTURER           IOWA_RESOURCE_FLAG_NONE // 5905
#define IPSO_RSC_FLAGS_HOST_DEVICE_MODEL_NUMBER           IOWA_RESOURCE_FLAG_NONE // 5906
#define IPSO_RSC_FLAGS_HOST_DEVICE_UNIQUE_ID              IOWA_RESOURCE_FLAG_NONE // 5907
#define IPSO_RSC_FLAGS_HOST_DEVICE_SOFTWARE_VERSION       IOWA_RESOURCE_FLAG_NONE // 5908
#define IPSO_RSC_FLAGS_BITMAP_INPUT                       IOWA_RESOURCE_FLAG_NONE // 5910
#define IPSO_RSC_FLAGS_BITMAP_INPUT_RESET                 IOWA_RESOURCE_FLAG_NONE // 5911
#define IPSO_RSC_FLAGS_ELEMENT_DESCRIPTION                IOWA_RESOURCE_FLAG_NONE // 5912
#define IPSO_RSC_FLAGS_INTERVAL_PERIOD                    IOWA_RESOURCE_FLAG_NONE // 6000
#define IPSO_RSC_FLAGS_INTERVAL_START_OFFSET              IOWA_RESOURCE_FLAG_NONE // 6001
#define IPSO_RSC_FLAGS_INTERVAL_UTC_OFFSET                IOWA_RESOURCE_FLAG_NONE // 6002
#define IPSO_RSC_FLAGS_INTERVAL_COLLECTION_START_TIME     IOWA_RESOURCE_FLAG_NONE // 6003
#define IPSO_RSC_FLAGS_OLDEST_RECORDED_INTERVAL           IOWA_RESOURCE_FLAG_NONE // 6004
#define IPSO_RSC_FLAGS_LAST_DELIVERED_INTERVAL            IOWA_RESOURCE_FLAG_NONE // 6005
#define IPSO_RSC_FLAGS_LATEST_RECORDED_INTERVAL           IOWA_RESOURCE_FLAG_NONE // 6006
#define IPSO_RSC_FLAGS_INTERVAL_DLV_MIDNIGHT_ALIGNED      IOWA_RESOURCE_FLAG_NONE // 6007
#define IPSO_RSC_FLAGS_INTERVAL_HISTORICAL_READ           IOWA_RESOURCE_FLAG_NONE // 6008
#define IPSO_RSC_FLAGS_INTERVAL_HISTORICAL_READ_PAYLOAD   IOWA_RESOURCE_FLAG_NONE // 6009
#define IPSO_RSC_FLAGS_INTERVAL_CHANGE_CONFIGURATION      IOWA_RESOURCE_FLAG_NONE // 6010
#define IPSO_RSC_FLAGS_EVENT_TYPE                         IOWA_RESOURCE_FLAG_NONE // 6011
#define IPSO_RSC_FLAGS_ALARM_REALTIME                     IOWA_RESOURCE_FLAG_NONE // 6012
#define IPSO_RSC_FLAGS_ALARM_STATE                        IOWA_RESOURCE_FLAG_NONE // 6013
#define IPSO_RSC_FLAGS_ALARM_SET_THRESHOLD                IOWA_RESOURCE_FLAG_NONE // 6014
#define IPSO_RSC_FLAGS_ALARM_SET_OPERATOR                 IOWA_RESOURCE_FLAG_NONE // 6015
#define IPSO_RSC_FLAGS_ALARM_CLEAR_THRESHOLD              IOWA_RESOURCE_FLAG_NONE // 6016
#define IPSO_RSC_FLAGS_ALARM_CLEAR_OPERATOR               IOWA_RESOURCE_FLAG_NONE // 6017
#define IPSO_RSC_FLAGS_ALARM_MAXIMUM_EVENT_COUNT          IOWA_RESOURCE_FLAG_NONE // 6018
#define IPSO_RSC_FLAGS_ALARM_MAXIMUM_EVENT_PERIOD         IOWA_RESOURCE_FLAG_NONE // 6019
#define IPSO_RSC_FLAGS_LATEST_DELIVERED_EVENT_TIME        IOWA_RESOURCE_FLAG_NONE // 6020
#define IPSO_RSC_FLAGS_LATEST_RECORDED_EVENT_TIME         IOWA_RESOURCE_FLAG_NONE // 6021
#define IPSO_RSC_FLAGS_ALARM_CLEAR                        IOWA_RESOURCE_FLAG_NONE // 6022
#define IPSO_RSC_FLAGS_ALARM_AUTO_CLEAR                   IOWA_RESOURCE_FLAG_NONE // 6023
#define IPSO_RSC_FLAGS_EVENT_CODE                         IOWA_RESOURCE_FLAG_NONE // 6024
#define IPSO_RSC_FLAGS_LATEST_EVENT_LOG_PAYLOAD           IOWA_RESOURCE_FLAG_NONE // 6025
#define IPSO_RSC_FLAGS_START                              IOWA_RESOURCE_FLAG_NONE // 6026
#define IPSO_RSC_FLAGS_STOP                               IOWA_RESOURCE_FLAG_NONE // 6027
#define IPSO_RSC_FLAGS_STATUS                             IOWA_RESOURCE_FLAG_NONE // 6028
#define IPSO_RSC_FLAGS_LATEST_PAYLOAD                     IOWA_RESOURCE_FLAG_NONE // 6029
#define IPSO_RSC_FLAGS_PLMN_ID                            IOWA_RESOURCE_FLAG_NONE // 6030
#define IPSO_RSC_FLAGS_BAND_INDICATOR                     IOWA_RESOURCE_FLAG_NONE // 6031
#define IPSO_RSC_FLAGS_DL_EARFCN                          IOWA_RESOURCE_FLAG_NONE // 6032
#define IPSO_RSC_FLAGS_CELL_ID                            IOWA_RESOURCE_FLAG_NONE // 6033
#define IPSO_RSC_FLAGS_PCI                                IOWA_RESOURCE_FLAG_NONE // 6034
#define IPSO_RSC_FLAGS_RSRP                               IOWA_RESOURCE_FLAG_NONE // 6035
#define IPSO_RSC_FLAGS_RSRQ                               IOWA_RESOURCE_FLAG_NONE // 6036
#define IPSO_RSC_FLAGS_SYS_FRAME_NUMBER                   IOWA_RESOURCE_FLAG_NONE // 6037
#define IPSO_RSC_FLAGS_SUB_FRAME_NUMBER                   IOWA_RESOURCE_FLAG_NONE // 6038
#define IPSO_RSC_FLAGS_ALTITUDE                           IOWA_RESOURCE_FLAG_NONE // 6039
#define IPSO_RSC_FLAGS_SAMPLE_FREQUENCY                   IOWA_RESOURCE_FLAG_NONE // 6040


#ifdef __cplusplus
}
#endif

#endif
