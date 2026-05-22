/**
 * @file main.c
 * @brief dog_soother 进程入口，转调 app_main_run
 */
#include "app_main.h"

int main(int argc, char **argv) /* 退出码来自 app_main_run */
{
    return app_main_run(argc, argv);
}
