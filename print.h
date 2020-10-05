/*
 * print.h
 *
 *  Created on: 21 вер. 2020 р.
 *      Author: Tesla
 */

#ifndef SRC_PRINT_H_
#define SRC_PRINT_H_
#include "stdio.h"
//#include "usart.h"
#include <string.h>
#include <stdarg.h>
#include "USB_Interface.h"

inline static void Printf(const char* fmt, ...) {
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
   // HAL_UART_Transmit(&huart1, (uint8_t*)buff, strlen(buff), HAL_MAX_DELAY);
    VCP_write((uint8_t*)buff,  strlen(buff));
    va_end(args);
}


#endif /* SRC_PRINT_H_ */
