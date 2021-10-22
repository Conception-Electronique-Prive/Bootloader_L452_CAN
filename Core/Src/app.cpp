#include "app.h"
#include "bootloader.h"
#include "can.h"
#include "usart.h"
#include "utils.hpp"
#include <string.h>
#include <stdio.h>

constexpr uint32_t BOOT_TIMEOUT = 10 * 1000;    // wait up to 10s for master to answer

constexpr uint8_t CMD_ID_BOOT  = 0x12;
uint8_t           CMD_ID_RESET = 0x13;
enum BootType
{
    Pass,
    Flash,
};

/**
 * @brief  Debug over UART2 -> ST-LINK -> USB Virtual Com Port
 * @param  str: string to be written to UART2
 * @retval None
 */
void print(const char* str) {
    HAL_UART_Transmit(&huart1, (uint8_t*)str, (uint16_t)strlen(str), 100);
}

void printr(const char* tag, const char* txt) {
    char msg[80];
    snprintf(msg, 80, "[%-4s]: %-60s\r", tag, txt);
    print(msg);
}

void println(const char* tag, const char* txt) {
    char msg[80];
    snprintf(msg, 80, "[%-4s]: %-60s\r\n", tag, txt);
    print(msg);
}

/**
 * @brief  This function executes the bootloader sequence.
 * @param  None
 * @retval Application error code ::eApplicationErrorCodes
 *
 */
uint8_t Enter_Bootloader(void) {
    std::array<uint8_t, 8> can;

    char     msg[100];
    size_t   size = 0;
    uint8_t  status;
    uint32_t addr;
    uint64_t data;

    // Step 1: Get boot type
    printr("TYPE", "Waiting for master");
    uint32_t now = HAL_GetTick();
    while (true) {
        while (MX_CAN1_Available() == 0) {
            if (HAL_GetTick() - now > BOOT_TIMEOUT) {
                println("TYPE", "No answer from master");
                return ERR_OK;
            }
        }
        addr = MX_CAN1_Recv(can.data()) >> 4;
        if (addr == CMD_ID_BOOT)
            break;
    }
    toType(can, status);
    if (status == BootType::Pass) {
        println("TYPE", "No action required");
        return ERR_OK;
    } else {
        println("TYPE", "Update requested");
    }
    MX_CAN1_Send(&MSG_NEXT, can.data());

    // Step 2: Get new software size
    printr("SIZE", "Waiting for master");
    while (MX_CAN1_Available() == 0)
        ;
    MX_CAN1_Recv(can.data());
    toType(can, size);
    if (Bootloader_CheckSize(size) != BL_OK) {
        println("SIZE", "app is too large");
        return ERR_APP_LARGE;
    }
    println("SIZE", "App size OK");
    MX_CAN1_Send(&MSG_NEXT, can.data());


    /* Step 3: Init Bootloader and Flash */
    Bootloader_Init();


    /* Step 4: Erase Flash */
    printr("ERAZ", "Erasing flash...");
    Bootloader_Erase();
    println("ERAZ", "Flash erased");


    /* Step 5: Programming */
    printr("PROG", "Starting");
    Bootloader_FlashBegin();
    for (size_t i = 0; i < size; i += 8) {
        while (MX_CAN1_Available() == 0)
            ;
        MX_CAN1_Recv(can.data());
        toType(can, data);
        status = Bootloader_FlashNext(data);
        if (status == BL_OK) {
            if (i % 1024 == 0) {
                snprintf(msg, 50, "%2u%% [%6u/%6u]", i * 100 / size, i, size);
                printr("PROG", msg);
            }
            MX_CAN1_Send(&MSG_NEXT, can.data());
        } else {
            snprintf(msg, 50, "Error at: %u byte", (i));
            println("PROG", msg);
            return ERR_FLASH;
        }
    };
    Bootloader_FlashEnd();
    snprintf(msg, 50, "Flashed %d bytes", size);
    println("PROG", msg);


    /* Step 6: Verify Flash Content */
    printr("CHCK", "Checking data");
    addr = APP_ADDRESS;
    for (size_t i = 0; i < size; i += 8) {
        while (MX_CAN1_Available() == 0)
            ;
        MX_CAN1_Recv(can.data());
        toType(can, data);
        uint32_t* pData = (uint32_t*)&data;
        for (uint8_t i = 0; i < 2; ++i) {
            if (*(uint32_t*)addr != *pData) {
                snprintf(msg, 50, "Error at: %u byte", i);
                println("CHCK", msg);
                return ERR_VERIFY;
            }
            addr += 4;
            ++pData;
        }
        MX_CAN1_Send(&MSG_NEXT, can.data());
        if (i % 1024 == 0) {
            snprintf(msg, 50, "%2u%% [%6u/%6u]", i * 100 / size, i, size);
            printr("CHCK", msg);
        }
    };
    println("CHCK", "Passed");
    return ERR_OK;
}
