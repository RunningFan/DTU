
#ifndef _FSM_H_
#define _FSM_H_

#include "stm32f10x.h"

typedef uint8_t (*FuncPtr)(void * , void **);
typedef struct
{
    uint8_t projDebug;
    uint8_t onlineTest;
    uint8_t outdoorErr;
    uint8_t indoorErr;
    uint8_t buttonPush;
    uint8_t realMonitor;
    uint8_t fpt_update;
    uint8_t Hyaline;
}varType;



enum states{ s_idle, s_debug, s_button, s_outdoor , s_indoor , s_real , s_test , s_update};   //枚举系统状态

extern  varType var;
extern uint16_t receive_count;
extern uint8_t fail_count;
extern  FuncPtr eventMode[];
extern  uint8_t NS; //定义下一状态
extern  uint8_t losepower;

uint8_t connectToServer(void);
uint8_t disConnectToServer(void);
void    ModeStateMachine(void *argc , void *argv[]);
#endif

