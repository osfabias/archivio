/**
 * @file arch.h
 * @brief The header of the public Archivio API.
 *
 * @author Ilya Buravov
 * @date 21.09.2024
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#pragma once

#include <stdarg.h>
#include <stdint.h>

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
 *
 * @var arch_log_level_t:ARCH_LOG_LEVEL_MAX_ENUM
 * For internal use by the Archivio.
 */
typedef enum arch_log_level {
  ARCH_LOG_LEVEL_TRACE,
  ARCH_LOG_LEVEL_DEBUG,
  ARCH_LOG_LEVEL_INFO,
  ARCH_LOG_LEVEL_WARN,
  ARCH_LOG_LEVEL_ERROR,
  ARCH_LOG_LEVEL_FATAL,

  ARCH_LOG_LEVEL_MAX_ENUM
} arch_log_level;

/**
 * @brief Logger creation info.
 *
 * @var arch_logger_create_info::pathFormat
 * A format that describes the location where's the log file will
 * be created. This path should not include the log file's name.
 * Leave as NULL if you want to put file at the default "./logs/"
 * directory. If the passed path doesn't end with the '/' symbol
 * the behaviour is undefined.
 *
 * @var arch_logger_create_info::fileNameFormat
 * A sequence of characters that describes how log file will be
 * named. Leave as NULL if you don't want to write messages to a file.
 *
 * @var arch_logger_create_info::messageFormats
 * An array of sequences of characters that describes how log messages
 * will be formatted before they displayed in the console. If the passed
 * value or at least one of the elements in an array is a null pointer 
 * the behaviour of the arch_logger_create() function is undefined.
 *
 * @var arch_logger_create_info::fileMessageFormats
 * An array of sequences of characters that describes how log messages
 * will be formatted before they written to a file. If the passed
 * value or at least one of the elements in an array is a null pointer 
 * the behaviour of the arch_logger_create() function is undefined.
 *
 * @var arch_logger_create_info::level
 * The minimum level of log messages to show in the console, besides
 * the log level all messages will be written to a file. If the passed
 * value isn't presented in the arch_log_level_t enum the behaviour of 
 * the arch_logger_create() function is undefined.
 */
typedef struct arch_logger_create_info {
  const char     *path_format;
  const char     *filename_format;
  const char     *msg_formats[ARCH_LOG_LEVEL_MAX_ENUM];
  const char     *file_msg_formats[ARCH_LOG_LEVEL_MAX_ENUM];
  arch_log_level level;
} arch_logger_create_info_t;

/**
 * @brief An Archivio logger.
 */
typedef struct arch_logger* arch_logger;

/**
 * @brief Initializes Archivio.
 *
 * Initializes the Archivio state and starts an Archivio logging routine
 * in the separate thread, that will wait for the entries to process.
 * If in the logging process some error ocqured the thread will exit.
 * You can check if the thread is still alive using the archIsAlive()
 * function. This function should be called before any other Archivio
 * functions are called, otherwise the behaviour of the other functions
 * is undefined.
 *
 * @param max_entry_count The max number of log entries to be stored
 *                        at once.
 *
 * @return Returns 1 on success, otherwise returns 0.
 */
int arch_init(int max_entry_count);

/**
 * @brief Terminates Archivio.
 *
 * Waits for the Archivio logging thread to finish processing queued
 * entries and then terminates the Archivio state. This functions should
 * be called after all Archivio logger instances where destroyed,
 * otherwise the behaviour is undefined.
 */
void arch_terminate(void);

/**
 * @brief Returns whether an Archivio logging routine is alive or not.
 *
 * @return Returns 1 if the Archivio logging thread is alive, otherwise
 *         returns 0.
 */
int arch_is_alive(void);

/**
 * @brief Creates an Archivio logger instance.
 *
 * @param createInfo A pointer to an arch_logger_create_info_t struct.
 *
 * @return Returns a pointer to an Archivio logger instance.
 */
arch_logger arch_logger_create(const arch_logger_create_info_t *info);

/**
 * @brief Destroys logger.
 *
 * If passed pointer to an Archivio logger is a null pointer
 * the behaviour is undefined.
 * 
 * @param logger A pointer to an Archivio logger instance. If the
 *               passed pointer wasn't returned by the arch_logger_create()
 *               function, the behaviour is undefined.
 */
void arch_logger_destroy(arch_logger logger);

/**
 * @brief Logs a message.
 *
 * If logging passed log level is lower than the logger log level,
 * message wouldn't be displayed. If passed logger is a null handle
 * it will cause a segmentation fault.
 *
 * @param logger A pointer to an Archivio logger instance. If the
 *               passed pointer wasn't returned by the arch_logger_create()
 *               function, the behaviour is undefined.
 * @param level  A log level of the message.
 * @param msg    A pointer to a sequence of characters to print.
 * @param ...    Variadic arguments.
 *
 * @return Returns 1 if log entry was successfully written,
 *         otherwise returns 0.
 */
int arch_log(arch_logger logger, arch_log_level level,
             const char *msg, ...);

/**
 * @brief Logs a message.
 *
 * If logging passed log level is lower than the logger log level,
 * message wouldn't be displayed to the console, but still will be
 * written to file (if logger instance has a file). If the passed
 * logger is a null handle the behaviour is undefined.
 *
 * @param logger A pointer to an Archivio logger instance. If the
 *               passed pointer wasn't returned by the arch_logger_create()
 *               function, the behaviour is undefined.
 * @param level  A log level of the message.
 * @param msg    A pointer to a c-string to log.
 * @param valist List of variadic arguments.
 *
 * @return Returns 1 if log entry was successfully written,
 *         otherwise returns 0.
 */
int arch_logvl(arch_logger logger, arch_log_level level,
               const char *msg, va_list valist);

