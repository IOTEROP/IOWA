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
* Copyright (c) 2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_LOGGER_INCLUDE_
#define _IOWA_PRV_LOGGER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_config.h"
#include "iowa_logger.h"
#include "iowa_platform.h"

#include <ctype.h>

/**************************************************************
 * Defines
 **************************************************************/

#ifndef IOWA_LOG_LEVEL
#define IOWA_LOG_LEVEL IOWA_LOG_LEVEL_NONE
#endif

#ifndef IOWA_LOG_PART
#define IOWA_LOG_PART IOWA_PART_ALL
#endif

#if (IOWA_LOG_LEVEL >= IOWA_LOG_LEVEL_ERROR)
#define IOWA_LOG_ERROR(PART, STR)                           iowa_log(PART, IOWA_LOG_LEVEL_ERROR, __func__, __LINE__, STR)
#define IOWA_LOG_ARG_ERROR(PART, STR, ...)                  iowa_log_arg(PART, IOWA_LOG_LEVEL_ERROR, __func__, __LINE__, STR, __VA_ARGS__)
#define IOWA_LOG_BUFFER_ERROR(PART, STR, BUF, LEN)          iowa_log_buffer(PART, IOWA_LOG_LEVEL_ERROR, __func__, __LINE__, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_ERROR(PART, STR, BUF, LEN, ...) iowa_log_arg_buffer(PART, IOWA_LOG_LEVEL_ERROR, __func__, __LINE__, STR, BUF, LEN, __VA_ARGS__)
#else
#define IOWA_LOG_ERROR(PART, STR)
#define IOWA_LOG_ARG_ERROR(PART, STR, ...)
#define IOWA_LOG_BUFFER_ERROR(PART, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_ERROR(PART, STR, BUF, LEN, ...)
#endif

#if (IOWA_LOG_LEVEL >= IOWA_LOG_LEVEL_WARNING)
#define IOWA_LOG_WARNING(PART, STR)                           iowa_log(PART, IOWA_LOG_LEVEL_WARNING, __func__, __LINE__, STR)
#define IOWA_LOG_ARG_WARNING(PART, STR, ...)                  iowa_log_arg(PART, IOWA_LOG_LEVEL_WARNING, __func__, __LINE__, STR, __VA_ARGS__)
#define IOWA_LOG_BUFFER_WARNING(PART, STR, BUF, LEN)          iowa_log_buffer(PART, IOWA_LOG_LEVEL_WARNING, __func__, __LINE__, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_WARNING(PART, STR, BUF, LEN, ...) iowa_log_arg_buffer(PART, IOWA_LOG_LEVEL_WARNING, __func__, __LINE__, STR, BUF, LEN, __VA_ARGS__)
#else
#define IOWA_LOG_WARNING(PART, STR)
#define IOWA_LOG_ARG_WARNING(PART, STR, ...)
#define IOWA_LOG_BUFFER_WARNING(PART, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_WARNING(PART, STR, BUF, LEN, ...)
#endif

#if (IOWA_LOG_LEVEL >= IOWA_LOG_LEVEL_INFO)
#define IOWA_LOG_INFO(PART, STR)                           iowa_log(PART, IOWA_LOG_LEVEL_INFO, __func__, __LINE__, STR)
#define IOWA_LOG_ARG_INFO(PART, STR, ...)                  iowa_log_arg(PART, IOWA_LOG_LEVEL_INFO, __func__, __LINE__, STR, __VA_ARGS__)
#define IOWA_LOG_BUFFER_INFO(PART, STR, BUF, LEN)          iowa_log_buffer(PART, IOWA_LOG_LEVEL_INFO, __func__, __LINE__, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_INFO(PART, STR, BUF, LEN, ...) iowa_log_arg_buffer(PART, IOWA_LOG_LEVEL_INFO, __func__, __LINE__, STR, BUF, LEN, __VA_ARGS__)
#else
#define IOWA_LOG_INFO(PART, STR)
#define IOWA_LOG_ARG_INFO(PART, STR, ...)
#define IOWA_LOG_BUFFER_INFO(PART, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_INFO(PART, STR, BUF, LEN, ...)
#endif

#if (IOWA_LOG_LEVEL >= IOWA_LOG_LEVEL_TRACE)
#define IOWA_LOG_TRACE(PART, STR)                           iowa_log(PART, IOWA_LOG_LEVEL_TRACE, __func__, __LINE__, STR)
#define IOWA_LOG_ARG_TRACE(PART, STR, ...)                  iowa_log_arg(PART, IOWA_LOG_LEVEL_TRACE, __func__, __LINE__, STR, __VA_ARGS__)
#define IOWA_LOG_BUFFER_TRACE(PART, STR, BUF, LEN)          iowa_log_buffer(PART, IOWA_LOG_LEVEL_TRACE, __func__, __LINE__, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_TRACE(PART, STR, BUF, LEN, ...) iowa_log_arg_buffer(PART, IOWA_LOG_LEVEL_TRACE, __func__, __LINE__, STR, BUF, LEN, __VA_ARGS__)
#else
#define IOWA_LOG_TRACE(PART, STR)
#define IOWA_LOG_ARG_TRACE(PART, STR, ...)
#define IOWA_LOG_BUFFER_TRACE(PART, STR, BUF, LEN)
#define IOWA_LOG_ARG_BUFFER_TRACE(PART, STR, BUF, LEN, ...)
#endif

#define IOWA_LOG_ERROR_MALLOC(size)  IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Allocation of %u bytes failed.", (size))
#define IOWA_LOG_ERROR_GETTIME(time) IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Bad returned time: %d.", (time))

#ifdef __cplusplus
}
#endif

#endif // _IOWA_PRV_LOGGER_INCLUDE_
