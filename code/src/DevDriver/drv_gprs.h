#ifndef _DRV_GPRS_H_
#define _DRV_GPRS_H_
#include "stm32f10x.h"
#include "drv_net.h"
#include "drv_usart.h"

#define REGISTER_SUCC		1
#define REGISTER_FAIL		0
#define TIME_OUT_LIMIT		255
#define SENDAT_BUFF_SIZE	400
#define ASSERT_WAIT_TIME(timeout)		(((uint32_t)timeout < TIME_OUT_LIMIT)?1:0)
#define GET_WAIT_TIME(timeout)			(((uint32_t)timeout & 0x0FF00000)>>20)
#define GET_BUFF_ADDR(timeout)			((char*)(((uint32_t)timeout & 0xF00FFFFF)))
#define SET_WAIT_TIME(timeout,addr)		(((uint32_t)(addr) | (uint8_t)timeout << 20))

#ifdef KEEP_TCP_LIVE
#define TCP_DATA_FRAME			0x01
#define TCP_CONTROL_FRAME		0x02
#endif
typedef struct 
{
    uint8_t Mode;
    uint8_t Status;
    uint8_t updateStatus;
//	uint8_t hayline_stat;
}ConType;

typedef struct
{
    SENDARR  TxArr;
    SENDSTR  TxStr;
    OS_EVENT *MSem;
    char     Buf[MAX_UARTTYPE_LEN];
}ATSendType;

extern ATSendType AT1 , AT2;
extern ConType ConSta;
extern uint8_t connect_flag ;
void   softReset( void );
extern uint8_t get_signal_val;
extern uint32_t base_station;
extern uint8_t resetnum;

uint8_t chr2hex(uint8_t chr);
uint8_t hex2chr(uint8_t hex);
int16_t str2dec(const char *s);
uint16_t str2hex(const char *s);
char*   SendAT(ATSendType *AT , char *cmd , char *ack , uint32_t waittime);
char* SendAT_Mutex(ATSendType *AT , char *cmd , char *ack , uint32_t waittime);

//char* Acess_AT_Buff(ATSendType *AT,char* buff_strstr,uint16_t timeout);
//void* Release_AT_Buff(ATSendType *AT);


void    gprsPowerUp(void);
void    gprsCheckReg(TypeGPRS *Ptr);
void    checkNetStatus(void);

uint8_t GetBaseStation(TypeGPRS *ptr);//add by Running

uint8_t getIMEI(TypeGPRS *Ptr);
uint8_t getICCID(TypeGPRS *Ptr);
uint8_t gprsConnect(TypeGPRS *Ptr);
uint8_t QCgprsConnect(TypeGPRS *Ptr);
uint8_t QCgprsDisConnect(void);
uint8_t gprsDisConnect(void);
uint8_t getSignalVal(uint8_t *res);
uint16_t tcpSendData(uint8_t *data , uint16_t dataLen);
uint8_t CalcCheckSum(uint8_t *pkt, int size);
uint8_t ftpconnect(TypeUpdate *UpdateParam);
int32_t str2dec_32(const char *s);
void restart_sim800c(void);
void close_sim800c(void);

uint8_t HttpconnectToServer(void);
#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
