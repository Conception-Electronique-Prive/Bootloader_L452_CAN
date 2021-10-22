#pragma once
#include <stdint.h>
#include "main.h"
#include "bootloader.h"

enum eApplicationErrorCodes
{
    ERR_OK = 0,
    ERR_INIT,
    ERR_APP_LARGE,
    ERR_FLASH,
    ERR_VERIFY,
};

extern uint8_t CMD_ID_RESET;

void    print(const char* str);
void    printr(const char* tag, const char* txt);
void    println(const char* tag, const char* txt);
uint8_t Enter_Bootloader(void);
