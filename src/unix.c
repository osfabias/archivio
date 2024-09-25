/**
 * @file unix.c
 * @brief The implementation file of the OS-specific internal Archivio functions for Unix systems.
 *
 * This file contains struct definitions and function implementations
 * of OS-specific internal Archivio functions for Unix systems.
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#include <dirent.h>
#include <sys/stat.h>

#include "internal.h"

int _is_dir_exists(const char *path) {
  DIR *dir = opendir(path);
  int _dir_is_exists = !dir;
  if (dir) {
    closedir(dir); // ignore failure
  }
  return !_dir_is_exists;
}

int _mkdir(const char *path) {
  return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

