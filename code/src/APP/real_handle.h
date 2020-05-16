#ifndef _REAL_HANDLE_H_
#define _REAL_HANDLE_H_
#include "stm32f10x.h"

uint16_t checkDataAddrr(uint16_t addr, const Real_PageTpye *real_PageData);

extern const Real_PageTpye Real_Page;
extern BUFF_TYPE RealBuffer;
extern BUFF_TYPE RealBuffer_time_L30;
#endif