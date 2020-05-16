#ifndef _DUPLICATE_H_
#define _DUPLICATE_H_
#include "stm32f10x.h"

#define NULLTYPE 0
#define BITTYPE  1
#define BYTETYPE 8
#define WORDTYPE 16

typedef struct {
  uint8_t System [150];
  uint8_t Indoor [80][200];
  uint8_t Outdoor[4 ][300];
}ParamType;

typedef struct {
    uint32_t SrcAdd:16;
    uint32_t Value:16;
}SrcDataTyp;

typedef struct PageTpye{
    const SrcDataTyp  *Page;
    const uint32_t    TableSize;
}PageTpye;

typedef struct {
    union{
        uint32_t ID;
        struct
        {
            uint32_t    DataType:7;
            uint32_t    CAN1IP:7;
            uint32_t    CAN2IP:7;
            uint32_t    FunCode:8;
        };
    };
    uint8_t  Lenth;
    uint8_t  Data[8];
}CanMsgType;

typedef struct {
    uint8_t EraserFlag;
    uint16_t RealDataLen;
    uint16_t ErrDataLen;
	uint16_t TakeDataLen;
    uint32_t writeFlashAddr;
    uint32_t readFlashAddr;
    uint32_t saveFlashTime;
    uint32_t readFlashTime;
    uint32_t errPointTime;
    uint32_t curTimeCnt;
    struct{
        uint32_t    Debug:1;
        uint32_t    OutErr:1;
        uint32_t    Healt:1;
        uint32_t    TestSig:1;
        uint32_t    InErr:1;
        uint32_t    IndOff:1;//�ڻ�ȫ�ر�־λ
        uint32_t    FastTest:1;
        uint32_t    DebugOver:1;
        uint32_t    SendFlag:1;//���ͱ�־λ��׼�����й��ϴ���
        uint32_t    ErrIsClr:1;//�����Ƿ������־:0,�������; 1,���ϴ���
        uint32_t    indoor_off_noupflag:1;//0�����ػ����;1��ֻ�������;
    };
    uint8_t real98flag;//ʵʱ���ģʽ:0��Ĭ��û��������Ϣ��1����������Ϣ�������ڻ���3����������Ϣ�������ڻ�
    uint8_t realup98flag;//ʵʱ��ر�־λ��0����͸����أ�1��ǿ��͸�����
    uint8_t trans_mode;//����ģʽ��0����ʮ����ǰ�仯��������ʮ�������գ�1����������ʱ�����մ�
    struct{
		uint8_t per_5_min:1;//ÿ5��������һ��
		uint8_t per_30_min:1;//ÿ30��������һ��
		uint8_t per_config_min:1;//����ʱ�����30����ʱ��ÿ����ʱ������һ��
		uint8_t err_point:1;//���ϵ�����һ��
		uint8_t enter_realmonitor:1;//�ս���ʵʱ���ʱ��͸�����⣩�������մ���
    };
	uint8_t data_type;//0:��ʾ��������;1:��ʾ��������
//	uint8_t take_photo_poll;//������ɱ�־
    uint8_t indoor_lockerr_flag;//�ڻ��������ϱ�־λ
    uint8_t out_lockerr_flag;//����������ϱ�־λ
    uint8_t indoor_not_lockerr_flag;//�ڻ����������ϱ�־λ
    uint8_t indoor_not_lockerr_num;//�ڻ���������־λ����
    uint8_t  RealBuffer[1024];
    uint8_t  ErrBuffer[1024];
	uint8_t  TakeBuffer[1024];
}UpDataType;

typedef struct 
{
    uint16_t DataLen;
    uint8_t  DataBuf[1024];
}BUFF_TYPE;

extern uint8_t   sendCnt ;
extern OS_EVENT  *QSemSend ;
extern void      *SendPtrArr[];
extern BUFF_TYPE SendBuffer[];

extern UpDataType g_Upload;
extern ParamType ACParam;
extern PageTpye  PageData[];
extern uint32_t g_FlashWriteAddr;
extern OS_EVENT *MutexFlash;
extern OS_EVENT *MutexSend1;
extern OS_EVENT *MutexSend3;
extern OS_EVENT *MutexSend4;
extern OS_EVENT *MutexSend2;

extern SrcDataTyp  SysTable[];
extern SrcDataTyp  OdrTable[];
extern SrcDataTyp  IdrTable[];
extern uint16_t  packTime;
extern uint8_t NewErrBuffer[20];
extern uint8_t Errflg;

uint32_t dec2BCD(uint32_t m_DEC);
uint32_t bcd2DEC(uint32_t m_BCD);
void setACTime(struct tm *m_Time);
struct tm* getACTime(CanMsgType *p_CanMsg , struct tm *p_Time);

void writeSIMToCAN(uint8_t simStatus);
void WriteData2Flash(UpDataType *writrBuff);
void ReadData2Flash(UpDataType *readBuff);
void savePowerDownInfo(UpDataType *saveData);
void checkMachStatus(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff);
void convCanData(const CanRxMsg *m_CanFrame , CanMsgType *m_CanMsg);
uint16_t binarySearch(const PageTpye *PageData , const CanMsgType *m_Msg , uint8_t *m_Data );
uint16_t fillErrData(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff);
uint16_t timeOutPack(UpDataType *p_ErrBuff);
void printToUart(const CanMsgType *m_CanMsg);



#endif