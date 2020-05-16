#ifndef _DRV_NET_H_
#define _DRV_NET_H_
#include "stm32f10x.h"

#define ON    1
#define OFF   0
#define STM32 ON
#define UART_MAX_TX_LEN 1500
#define UART_MAX_RX_LEN 500

#define FRAME72 ON
#define FRAME88 ON
#define FRAME89 ON
#define FRAME90 ON
#define FRAME91 ON
#define FRAME92 ON
#define FRAME93 ON
#define FRAME94 ON
#define FRAME95 ON
#define FRAME96 ON
#define FRAME97 ON
#define FRAMEEE ON
#define FRAMEF3 ON
#define FRAMEF4 ON
#define FRAMEA0 ON
#define SRV_DEBUG 1
typedef struct
{
#if STM32
    uint8_t   TxFrameArr[UART_MAX_TX_LEN];
    uint8_t   RxFrameArr[UART_MAX_RX_LEN];
#else
    uint8_t   *TxFrameArr;
    uint8_t   *RxFrameArr;
#endif
    uint16_t  GuideCode;
    uint8_t   DstAddr[7];
    uint8_t   SrcAddr[7];
    uint16_t  DataLenth;
    uint8_t   *DataPtr;
    uint8_t   CrcValue;

    union
    {
        uint8_t  FuncMode;
#ifdef FRAME72
        struct{
            struct{
                uint8_t  FuncMode;  
            }frame72Tx;

            struct{
                uint8_t  FuncMode; 
            }frame72Rx;
        };
#endif
        
#ifdef FRAME88
        struct{
            struct{
                uint8_t  FuncMode;  
            }frame88Tx;

            struct{
                uint8_t  FuncMode; 
            }frame88Rx;
        };
#endif

#ifdef FRAME89
        struct{
            struct{
                uint8_t   FuncMode;  
                uint16_t  CheckCode;
                uint16_t  HardVesion;
                uint16_t  SoftVesion;
                uint8_t*  IMEI;
                uint8_t*  BaseStation;
                uint8_t*  ICCID;
            }frame89Tx;

            struct{
                uint8_t   FuncMode; 
                uint16_t  CheckCode;
            }frame89Rx;
        };
#endif
        
#ifdef FRAME90
        struct{
            struct{
                uint8_t   FuncMode;  
                uint16_t  CheckCode;
                uint16_t  SoftVesion;
                uint8_t*   IMEI;
                uint8_t*   BaseStation;
                uint8_t*   ICCID;
            }frame90Tx;

            struct{
                uint8_t   FuncMode; 
                uint16_t  CheckCode;
            }frame90Rx;
        };
#endif

#ifdef FRAME91
        struct{
            struct{
                uint8_t  FuncMode;
                uint8_t  TranMode;
                struct tm Time;
            }frame91Tx;

            struct{
                uint8_t  FuncMode; 
                uint8_t  Result;
            }frame91Rx;
        };
#endif
        
#ifdef FRAME92
        struct{
            struct{
                uint8_t  FuncMode;  
                uint8_t  Result;
            }frame92Tx;

            struct{
                uint8_t  FuncMode;
                uint8_t  StopMode;
            }frame92Rx;
        };
#endif

#ifdef FRAME93
        struct{
            struct{
                uint8_t  FuncMode;  
                uint8_t  Result;
            }frame93Tx;

            struct{
                uint8_t  FuncMode;
                uint8_t  Flag;
            }frame93Rx;
        };
#endif
        
#ifdef FRAME94
        struct{
            struct{
                uint8_t   FuncMode; 
                uint16_t  ParamLen;
                uint8_t   *FailParam; 
            }frame94Tx;

            struct{
                uint8_t  FuncMode;
                uint8_t  *Param; 
            }frame94Rx;
        };
#endif
        
#ifdef FRAME95
        struct{
            struct{
                uint8_t   FuncMode;  
                uint16_t  ParamLen;
                uint8_t   *Param; 
            }frame95Tx;

            struct{
                uint8_t  FuncMode;
                uint8_t  *Param; 
            }frame95Rx;
        };
#endif
      
#ifdef FRAME96
        struct{
            struct{
                uint8_t  FuncMode;
                uint16_t DataLen;
                uint8_t  *Data;
                uint8_t  TranMode;
                uint8_t  MachMode;
                struct tm Time;
            }frame96Tx;
            
            struct{
                uint8_t  FuncMode;
                uint8_t  Res; 
            }frame96Rx;
#endif   
            
#ifdef FRAME97
        struct{
            struct{
                uint8_t  FuncMode;  
                uint8_t  Result; 
            }frame97Tx;

            struct{
                uint8_t  FuncMode;
                uint8_t  Update; 
            }frame97Rx;
        };
#endif           

#ifdef FRAMEEE
            struct{
                uint8_t  FuncMode;
                uint8_t  ErrCode;
            }frameEETx;
        };
#endif  
        
#ifdef FRAMEF3
        struct{
            struct{
                uint8_t  FuncMode;  
            }frameF3Tx;

            struct{
                uint8_t  FuncMode;
                struct tm Time; 
            }frameF3Rx;
        };
#endif 
        
#ifdef FRAMEF4
        struct{
            struct{
                uint8_t  FuncMode;  
                struct tm Time;
                uint16_t StaAddr;
                uint8_t  Signal;
                uint16_t  Times;
                uint8_t  DataType;
                uint16_t DataLenth;
                uint8_t  *DataPtr;
            }frameF4Tx;

            struct{
                uint8_t  FuncMode;
                uint8_t  Resault;
            }frameF4Rx;
        };
#endif 
        
#ifdef FRAMEA0
        struct{
            struct{
                uint8_t  FuncMode;  
                struct tm Time;
                uint16_t StartAddr;
                uint16_t FinishAddr;
            }frameA0Rx;

            struct{
                uint8_t  FuncMode;
                struct tm Time;
                uint16_t  StartAddr;
                uint16_t  FinishAddr;
                uint8_t  data[1000];
            }frameA0Tx;
        };
#endif 
    }ValidData;
} TcpType;


typedef struct 
{
    uint8_t  EraserFlag  ;
    uint8_t  Pause       ;
    uint8_t  Version     ;
    uint16_t SerPort     ;
    uint16_t HeartTime   ;
    uint16_t OutErrTime  ;
	uint16_t Out_Err_After_Time;
    uint16_t HealtTime   ;
    uint16_t ButtTime    ;
    uint16_t InerrtTime  ;
	uint16_t In_Err_After_Time;
    uint32_t MaitTime    ;
	uint32_t real_monitor_elapse_time;
    char ApnAccr[20]  ;
    char ApnUsr [10]  ;
    char ApnPwd[10]   ;
    char SerName[20]  ;
    char SmsPwd[10]   ;
    char Admin[5][12] ;
    char Usron[10][12];
    char IMEI [16] ;
    char ICCID[21] ;
    char CellInfo[15] ;
}TypeGPRS;

typedef struct
{
    char FtpAdrr [50];
    char Username[20];
    char PassWord[20];
    char FileName[20];
    char FilePath[50];
    uint16_t FtpPort;
    uint16_t CheckSum;
    uint32_t PacketSize;
    uint32_t Rev_PacketSize;
    uint16_t recieve_CheckSum;
}TypeFTP;

extern OS_EVENT *MSemTCP;
extern TcpType  g_TcpType;
extern TypeGPRS  GprsParam;
extern uint8_t  g_CacheData[200];
extern struct tm nextDay;
extern uint8_t to_s_idle;
extern uint8_t isGetTime;

void     softReset( void );
void     tcpDataProcess(TcpType *m_TcpType);

uint16_t handleTxFrame(TcpType *m_frame);
TcpType *handleRxFrame(TcpType *m_frame ,const uint8_t *m_dataarr , uint16_t m_datalenth);
uint16_t setGprsParam(uint8_t *m_Param, uint16_t m_ParamLen , TypeGPRS *m_GprsParam);
uint16_t getGprsParam(uint8_t *m_Param , uint16_t m_ParamLen ,uint8_t *putParam , TypeGPRS *m_GprsParam);
uint8_t  getServerTime(uint8_t curSta);

uint16_t send89Frame(TcpType *m_Frame);
uint16_t sendF3Frame(TcpType *m_Frame);
uint16_t send91Frame(TcpType *m_Frame , uint8_t TranMode);
uint16_t send96Frame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen , uint8_t t_mode ,uint8_t m_mode);
uint16_t sendF4Frame(TcpType *m_Frame , uint16_t start_addr,uint8_t data_type,uint8_t *dat , uint16_t datLen ,uint16_t Times);
void sendEEFrame(TcpType *m_Frame , uint8_t ErrCode);

void getRandTime(struct tm *pTime);
#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/