#ifndef DOG_SOOTHER_LOG_H_
#define DOG_SOOTHER_LOG_H_

#include <stdio.h>

/* 各 .c 可在 include 前 #define LOG_TAG 覆盖模块名 */
#ifndef LOG_TAG
#define LOG_TAG "dog_soother"
#endif

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_RESET "\x1b[0m"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_ERROR(fmt, ...)                                    \
    printf(COLOR_RED "[ERROR] [%s:%d]: " fmt COLOR_RESET "\n", \
           __FILENAME__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...)                                      \
    printf(COLOR_GREEN "[INFO] [%s:%d]: " fmt COLOR_RESET "\n", \
           __FILENAME__, __LINE__, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...)                                     \
    printf(COLOR_CYAN "[DEBUG] [%s:%d]: " fmt COLOR_RESET "\n", \
           __FILENAME__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...)                                     \
    printf(COLOR_YELLOW "[WARN] [%s:%d]: " fmt COLOR_RESET "\n", \
           __FILENAME__, __LINE__, ##__VA_ARGS__)
#endif
