/**
 * @file arch.h
 * @brief The header of the Archivio API
 *
 * Copyright (c) 2023-2024 Osfabias
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <stdint.h>

/**
 * @brief Log level.
 */
typedef enum ArchLogLevel {
  ARCH_LOG_LEVEL_TRACE,
  ARCH_LOG_LEVEL_DEBUG,
  ARCH_LOG_LEVEL_INFO,
  ARCH_LOG_LEVEL_WARN,
  ARCH_LOG_LEVEL_ERROR,
  ARCH_LOG_LEVEL_FATAL,

  ARCH_LOG_LEVEL_MAX_ENUM
} ArchLogLevel;

/**
 * @brief Logger creation info.
 *
 * @var ArchLoggerCreateInfo::name
 * A name of the logger, that can be displayed
 * later using "%l". Leave as NULL if you don't
 * want the logger to have a name.
 *
 * @var ArchLoggerCreateInfo::pathFormat
 * A format that describes the location where's the
 * log file will be created. This path should not
 * include the log file's name. Leave as NULL if you
 * want to put file at the default "./logs/" directory.
 * If the passed path doesnt end with the '/' symbol
 * the archLoggerCreate() function will return null
 * pointer.
 *
 * @var ArchLoggerCreateInfo::fileNameFormat
 * A format that describes how log file will be
 * named. Leave as NULL if you don't want to write
 * messages to a file.
 *
 * @car ArchLoggerCreateInfo::messageFormat
 * A format that describes how log messages
 * will be formatted. If the passed value will
 * be a null pointer the archLoggerCreate function
 * will return a null pointer.
 *
 * @var ArchLoggerCreateInfo::level
 * The minimum level of log messages to show to
 * the console, besides the log level all mesages
 * will be written to a file. If the passed value isn't
 * presented in ArchLogLevel enum the archLoggerCreate()
 * will return a null pointer.
 */
typedef struct ArchLoggerCreateInfo {
  const char   *name;
  const char   *pathFormat;
  const char   *fileNameFormat;
  const char   *messageFormat;
  ArchLogLevel  level;
} ArchLoggerCreateInfo;

/**
 * @brief Extendeed logger creation info.
 *
 * @var ArchLoggerCreateInfoEx::name
 * A name of the logger, that can be displayed
 * later using "%l". Leave as NULL if you don't
 * want the logger to have a name.
 *
 * @var ArchLoggerCreateInfoEx::pathFormat
 * A format that describes the location where's the
 * log file will be created. This path should not
 * include the log file's name. Leave as NULL if you
 * want to put file at the default "./logs/" directory.
 * If the passed path doesnt end with the '/' symbol
 * the archLoggerCreateEx() function will return null
 * pointer.
 *
 * @var ArchLoggerCreateInfoEx::fileNameFormat
 * A format that describes how log file will be
 * named. Leave as NULL if you don't want to write
 * messages to a file.
 *
 * @car ArchLoggerCreateInfoEx::messageFormats
 * An array of formats for each log level that
 * describe how log messages will be formatted.
 * If the passed value is a null pointer the
 * archLoggerCreateEx() function will return a
 * null pointer.
 *
 * @var ArchLoggerCreateInfoEx::level
 * The minimum level of log messages to show to the
 * console, besides the log level all mesages will be
 * written to a file. If the passed value isn't presented
 * in the ArchLogLevel enum the archLoggerCreateEx()
 * function will return a null pointer.
 */
typedef struct ArchLoggerCreateInfoEx {
  const char   *name;
  const char   *pathFormat;
  const char   *fileNameFormat;
  const char   *messageFormats[ARCH_LOG_LEVEL_MAX_ENUM];
  ArchLogLevel  level;
} ArchLoggerCreateInfoEx;

/**
 * @brief An Archivio logger handle.
 */
typedef struct ArchLogger ArchLogger;

/**
 * @brief Creates a logger instance.
 * @param createInfo A pointer to an ArchLoggerCreateInfo struct.
 * @return Returns a pointer to an Archivio logger
 *         instance if the creation was successfull,
 *         otherwise returns 0.
 */
ArchLogger* archLoggerCreate(const ArchLoggerCreateInfo *info);

/**
 * @brief Creates a logger instance.
 *
 * Use this function to create a logger if you want
 * more control of message formats and styles.
 *
 * @param createInfo A pointer to an ArchLoggerCreateInfoEx struct.
 * @return Returns a pointer to an Archivio logger
 *         instance if the creation was successfull,
 *         otherwise returns 0.
 */
ArchLogger* archLoggerCreateEx(const ArchLoggerCreateInfoEx *info);

/**
 * @brief Destroy logger.
 * @param logger A pointer to an existing Archivio logger instance.
 */
void archLoggerDestroy(ArchLogger *logger);

/**
 * @brief Logs a message.
 *
 * If logging level is lower than current set logging level,
 * message wouldn't be displayed.
 *
 * @param logger A pointer to a logger instance.
 * @param level A logging level.
 * @param msg A message to print.
 * @param ... VA arguments.
 */
void archLog(ArchLogger *logger, ArchLogLevel level,
             const char *message, ...);

