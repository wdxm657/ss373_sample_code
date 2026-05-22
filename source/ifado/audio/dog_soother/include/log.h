#ifndef DOG_SOOTHER_LOG_H_
#define DOG_SOOTHER_LOG_H_

#include <stdio.h>

/* 各 .c 可在 include 前 #define LOG_TAG 覆盖模块名 */
#ifndef LOG_TAG
#define LOG_TAG "dog_soother"
#endif

#define LOG_ERROR(fmt, ...) fprintf(stderr, "[%s] E: " fmt, LOG_TAG, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  fprintf(stdout, "[%s] I: " fmt, LOG_TAG, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) fprintf(stdout, "[%s] D: " fmt, LOG_TAG, ##__VA_ARGS__)

#endif
