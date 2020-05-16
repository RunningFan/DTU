#ifndef _DRV_GPRS_H_
#define _DRV_GPRS_H_
#include "stm32f10x.h"
#include "drv_net.h"
#include "drv_usart.h"
typedef struct 
{
    uint8_t Mode;
    uint8_t Status;
    uint8_t ftpStatus;
}ConType;

typedef struct
{
    SENDARR  TxArr;
    SENDSTR  TxStr;
    OS_EVENT *MSem;
    char     Buf[1100];
}ATSendType;


extern ATSendType AT1, AT2 ;
extern ConType ConSta;
extern uint8_t connect_flag ;
void   softReset( void );

uint8_t chr2hex(uint8_t chr);
uint8_t hex2chr(uint8_t hex);
int16_t str2dec(const char *s);
char*   SendAT(ATSendType *AT , char *cmd , char *ack , uint16_t waittime);
void    gprsPowerUp(void);
void    gprsCheckReg(TypeGPRS *Ptr);
void    checkNetStatus(void);
uint8_t getIMEI(TypeGPRS *Ptr);
uint8_t getICCID(TypeGPRS *Ptr);
uint8_t gprsConnect(TypeGPRS *Ptr);
uint8_t QCgprsConnect(TypeGPRS *Ptr);
uint8_t QCgprsDisConnect(void);
uint8_t gprsDisConnect(void);
uint8_t getSignalVal(uint8_t *res);
uint16_t tcpSendData(uint8_t *data , uint16_t dataLen);
uint8_t CalcCheckSum(uint8_t *pkt, int size);

#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/