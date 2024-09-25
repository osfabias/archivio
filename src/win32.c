/**
 * @file win32.c
 * @brief The implementation file of the OS-specific internal Archivio functions for Windows OS.
 *
 * This file contains struct definitions and function implementations
 * of OS-specific internal Archivio functions for Windows OS.
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#include <windows.h>

#include "internal.h"

int _is_dir_exists(const char *path) {
    const DWORD dwAttrib = GetFileAttributes(path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES &&
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void _mkdir(const char *path) {
  CreateDirectory(path, 0);
}

