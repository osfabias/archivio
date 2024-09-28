/**
 * @file arch.h
 * @brief The header of the public Archivio functions.
 *
 * @author Ilya Buravov
 * @date 29.09.2024
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#pragma once

#include <stdarg.h>

/**
 * @brief Log level.
 *
 * @var arch_log_level:ARCH_LOG_LEVEL_TRACE
 * Log level used to provide a low level info, that can be useful for
 * developers for debugging or tracking program behaviour.
 *
 * @var arch_log_level:ARCH_LOG_LEVEL_DEBUG
 * Log level used to provide useful information about the program
 * during the debugging process.
 *
 * @var arch_log_level:ARCH_LOG_LEVEL_INFO
 * Log level used to notify a user or a developer that program
 * was executed successfully.
 *
 * @var arch_log_level:ARCH_LOG_LEVEL_WARN
 * Log level used for messages that warns a user or a developer
 * about the potential unwanted or undefined behaviour.
 *
 * @var arch_log_level:ARCH_LOG_LEVEL_ERROR
 * Log level used to notify a user or a developer that the program
 * wasn't executed successfully.
 *
 * @var arch_log_level:ARCH_LOG_LEVEL_FATAL
 * Log level used to notify a user or a developer that the program
 * faced an error, that couldn't be fixed or ignored, so the program
 * have to be terminated.
 */
typedef enum arch_log_level {
  ARCH_LOG_LEVEL_TRACE,
  ARCH_LOG_LEVEL_DEBUG,
  ARCH_LOG_LEVEL_INFO,
  ARCH_LOG_LEVEL_WARN,
  ARCH_LOG_LEVEL_ERROR,
  ARCH_LOG_LEVEL_FATAL,

  ARCH_LOG_LEVEL_MAX_ENUM
} arch_log_level_t;

/**
 * @brief Logger creation info.
 *
 * @var arch_logger_create_info::path_fmt
 * A format that describes the location where's the log file will
 * be created. This path should not include the log file's name.
 *
 * @var arch_logger_create_info::filename_fmt
 * A sequence of characters that describes how log file will be
 * named. Leave as NULL if you don't want to write messages to a file.
 *
 * @var arch_logger_create_info::msg_fmts
 * An array of sequences of characters that describes how log messages
 * will be formatted before they displayed in the console.
 *
 * @var arch_logger_create_info::file_msg_fmts
 * An array of sequences of characters that describes how log messages
 * will be formatted before they written to a file.
 *
 * @var arch_logger_create_info::level
 * The minimum level of log messages to show in the console, besides
 * the log level all messages will be written to a file.
 */
typedef struct arch_logger_create_info {
  const char      *path_fmt;
  const char      *filename_fmt;
  const char      *msg_fmts[ARCH_LOG_LEVEL_MAX_ENUM];
  const char      *file_msg_fmts[ARCH_LOG_LEVEL_MAX_ENUM];
  arch_log_level_t level;
} arch_logger_create_info_t;

/**
 * @brief An Archivio logger.
 */
typedef struct arch_logger* arch_logger_t;

/**
 * @brief Creates a logger.
 *
 * @param info A pointer to the create info.
 *
 * @return Returns a pointer to the logger instance.
 */
arch_logger_t arch_logger_create(const arch_logger_create_info_t *info);

/**
 * @brief Destroys logger.
 *
 * @param logger An Archivio logger handle.
 */
void arch_logger_destroy(arch_logger_t logger);

/**
 * @brief Logs a message.
 *
 * If logging passed log level is lower than the logger log level,
 * message wouldn't be displayed to the console, but still will be
 * written to the file.
 *
 * @param logger An Archivio logger handle.
 * @param level  The log level.
 * @param msg    The message.
 * @param ...    Variadic arguments.
 *
 * @return Returns 1 on successful log, otherwise returns 0.
 */
int arch_log(
  arch_logger_t logger, arch_log_level_t level, const char *msg, ...
);

/**
 * @brief Logs a message.
 *
 * If logging passed log level is lower than the logger log level,
 * message wouldn't be displayed to the console, but still will be
 * written to the file.
 *
 * @param logger An Archivio logger handle.
 * @param level  The log level.
 * @param msg    The message.
 * @param valist List of variadic arguments.
 *
 * @return Returns 1 on successful log, otherwise returns 0.
 */
int arch_logvl(
  arch_logger_t logger, arch_log_level_t level, const char *msg,
  va_list valist
);

