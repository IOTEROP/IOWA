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
* Copyright (c) 2017-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_logger.h"

#ifndef IOWA_LOGGER_USER

#define PRV_STR_LEVEL(S) ((S) == IOWA_LOG_LEVEL_ERROR ? "error" :     \
                         ((S) == IOWA_LOG_LEVEL_WARNING ? "warning" : \
                         ((S) == IOWA_LOG_LEVEL_INFO ? "info" :       \
                         ((S) == IOWA_LOG_LEVEL_TRACE ? "trace" :     \
                         "unknown"))))

#define PRV_STR_PART(S) ((S) == IOWA_PART_BASE ? "base" :         \
                        ((S) == IOWA_PART_COAP ? "coap" :         \
                        ((S) == IOWA_PART_COMM ? "comm" :         \
                        ((S) == IOWA_PART_LWM2M ? "lwm2m" :       \
                        ((S) == IOWA_PART_SECURITY ? "security" : \
                        ((S) == IOWA_PART_SYSTEM ? "system" :     \
                        ((S) == IOWA_PART_DATA ? "data" :         \
                        ((S) == IOWA_PART_OBJECT ? "object" :     \
                        "unknown"))))))))

/*************************************************************************************
** Private functions
*************************************************************************************/

static void prv_printf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    iowa_system_trace(format, args);
    va_end(args);
}

static void prv_logBuffer(const uint8_t *buffer,
                          size_t bufferLength)
{
    size_t i;

    prv_printf("%d bytes\r\n", bufferLength);

    for (i = 0; i < bufferLength; i += 16)
    {
        size_t j;

        prv_printf("  ");

        // Print the buffer by byte
        for (j = 0; j < 16 && i + j < bufferLength; j++)
        {
            prv_printf("%02X ", buffer[i + j]);
            if (j % 4 == 3)
            {
                prv_printf(" ");
            }
        }

        // Complete the line with whitespace if there are not 16 bytes
        while (j < 16) // keep the previous value for the variable 'j'
        {
            prv_printf("   ");
            if (j % 4 == 3)
            {
                prv_printf(" ");
            }
            j++;
        }

        prv_printf(" |");

        // Print the buffer with writable character if possible
        for (j = 0; j < 16 && i + j < bufferLength; j++)
        {
            if (isprint(buffer[i + j])
                && !isspace(buffer[i + j]))
            {
                prv_printf("%c", buffer[i + j]);
            }
            else
            {
                prv_printf(".");
            }
        }
        prv_printf("|\r\n");
    }
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

void iowa_log(uint8_t part,
              uint8_t level,
              const char *functionName,
              unsigned int line,
              const char *message)
{
    if ((IOWA_LOG_PART) & part)
    {
        prv_printf("[%s:%s:%s:%d] %s\r\n", PRV_STR_LEVEL(level), PRV_STR_PART(part), functionName, line, message);
    }
}

void iowa_log_arg(uint8_t part,
                  uint8_t level,
                  const char *functionName,
                  unsigned int line,
                  const char *message,
                  ...)
{
    if ((IOWA_LOG_PART) & part)
    {
        va_list args;

        prv_printf("[%s:%s:%s:%d] ", PRV_STR_LEVEL(level), PRV_STR_PART(part), functionName, line);

        va_start(args, message);
        iowa_system_trace(message, args);
        va_end(args);

        prv_printf("\r\n");
    }
}

void iowa_log_buffer(uint8_t part,
                     uint8_t level,
                     const char *functionName,
                     unsigned int line,
                     const char *message,
                     const uint8_t *buffer,
                     size_t bufferLength)
{
    if ((IOWA_LOG_PART) & part)
    {
        prv_printf("[%s:%s:%s:%d] %s ", PRV_STR_LEVEL(level), PRV_STR_PART(part), functionName, line, message);
        prv_logBuffer(buffer, bufferLength);
    }
}

void iowa_log_arg_buffer(uint8_t part,
                         uint8_t level,
                         const char *functionName,
                         unsigned int line,
                         const char *message,
                         const uint8_t *buffer,
                         size_t bufferLength,
                         ...)
{
    if ((IOWA_LOG_PART) & part)
    {
        va_list args;

        prv_printf("[%s:%s:%s:%d] ", PRV_STR_LEVEL(level), PRV_STR_PART(part), functionName, line);

        va_start(args, bufferLength);
        iowa_system_trace(message, args);
        va_end(args);

        prv_printf(" ");
        prv_logBuffer(buffer, bufferLength);
    }
}

#endif
