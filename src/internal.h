/**
 * @file internal.h
 * @brief The header of the internal Archivio functions.
 *
 * This file contains type and function definitions of the internal
 * OS-specific Archivio functions.
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#pragma once
#include <stdarg.h>  // variadic arguments
#include <stdint.h> // bool

int _is_dir_exists(const char *path);
int _mkdir(const char *path);

