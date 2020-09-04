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
* Copyright (c) 2017-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_LOGGER_INCLUDE_
#define _IOWA_LOGGER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

/**************************************************************
* Logger API
**************************************************************/

/*******************************
 * Defines
 */

#define IOWA_LOG_LEVEL_NONE    0
#define IOWA_LOG_LEVEL_ERROR   1
#define IOWA_LOG_LEVEL_WARNING 2
#define IOWA_LOG_LEVEL_INFO    3
#define IOWA_LOG_LEVEL_TRACE   4

#define IOWA_PART_ALL      0xFF
#define IOWA_PART_BASE     (1<<0)
#define IOWA_PART_COAP     (1<<1)
#define IOWA_PART_COMM     (1<<2)
#define IOWA_PART_LWM2M    (1<<3)
#define IOWA_PART_SECURITY (1<<4)
#define IOWA_PART_SYSTEM   (1<<5)
#define IOWA_PART_DATA     (1<<6)
#define IOWA_PART_OBJECT   (1<<7)

/*******************************
 * Functions
 */

// Writes a log message to the output.
// Parameters:
// - part: log part.
// - level: log level.
// - functionName: name of the function from where the Log has been called.
// - line: line from where the Log has been called.
// - message: string to display.
void iowa_log(uint8_t part,
              uint8_t level,
              const char *functionName,
              unsigned int line,
              const char *message);

// Writes a log message to the output with specifier arguments.
// Parameters:
// - part: log part.
// - level: log level.
// - functionName: name of the function from where the Log has been called.
// - line: line from where the Log has been called.
// - message: log message.
// - ... : Format specifiers which are replaced by the values specified in additional arguments.
void iowa_log_arg(uint8_t part,
                  uint8_t level,
                  const char *functionName,
                  unsigned int line,
                  const char *message, ...);

// Writes a buffer with a log message to the output.
// Parameters:
// - part: log part.
// - level: log level.
// - functionName: name of the function from where the Log has been called.
// - line: line from where the Log has been called.
// - message: string to display.
// - buffer: buffer.
// - bufferLength: buffer size.
void iowa_log_buffer(uint8_t part,
                     uint8_t level,
                     const char *functionName,
                     unsigned int line,
                     const char *message,
                     const uint8_t *buffer,
                     size_t bufferLength);

// Writes a buffer with a log message to the output with specifier arguments.
// Parameters:
// - part: log part.
// - level: log level.
// - functionName: name of the function from where the Log has been called.
// - line: line from where the Log has been called.
// - message: string to display.
// - buffer: buffer.
// - bufferLength: buffer size.
// - ... : Format specifiers which are replaced by the values specified in additional arguments.
void iowa_log_arg_buffer(uint8_t part,
                         uint8_t level,
                         const char *functionName,
                         unsigned int line,
                         const char *message,
                         const uint8_t *buffer,
                         size_t bufferLength,
                         ...);

#ifdef __cplusplus
}
#endif

#endif
