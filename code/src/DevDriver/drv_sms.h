#ifndef _DRV_SMS_H_
#define _DRV_SMS_H_
#include "stm32f10x.h"
#include "drv_net.h"

typedef struct
{
    uint8_t Auth;
    char phone[20];
    char pwd[10];
    char cmd[10];
    char param[100];
    char text[200];
}TypeSMS;

extern TypeSMS mySMS;
extern uint8_t sms_flg;
//extern uint8_t g_Hyaline;
uint8_t sendSMS(char *phone , char *text);
uint8_t smsCheck(TypeSMS *sms , uint8_t auth , TypeGPRS *para);
uint8_t checkPhoneAuth(char *phone , TypeGPRS *para);
uint8_t parseSMS(char *smsStream , TypeGPRS *para ,TypeSMS *sms);
#endif
