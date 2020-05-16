#ifndef _DUPLICATE_H_
#define _DUPLICATE_H_
#include "stm32f10x.h"

#define NULLTYPE 0
#define BITTYPE  1
#define BYTETYPE 8
#define WORDTYPE 16
#define INDOORLOCK_ERR_DATA_LEN    4
#define OUTLOCK_ERR_DATA_LEN       11
#define INDOORUNLOCK_ERR_DATA_LEN  1
#define SEND_DATA_BUFF_SIZE  5

#define G_UPLOAD_BUFFER_LEN		6

#define INDOOR_PARAM    0
#define OUTDOOR_PARAM   1

#define INDOOR_PARAM_EXIST		2
#define OUTDOOR_PARAM_EXIST		3

#define INDOOR_X_ADDR			6
#define OUTDOOR_X_ADDR			2

#define OPEN_BUFFER_TIME		180u

#define NO_SEND				0
#define YES_SEND			1


#define OUTDOOR_BYTE_ADDR		0x18
#define OUTDOOR_BIT_ADDR		0x36

#define INDOOR_BTYE_ADDR		0x08
#define INDOOR_BIT_ADDR			0x16

#define NORMAL_MODE_FILL		0
#define TIMEOUT_MODE_FILL		1
#define	TAKEPHOTO_MODE_FILL		2

//#define SYSTEM_CHANGE	234
#define SYSTEM_CHANGE	248
#define INDOOR_CHANGE	152

#define BIGDATA_INDOOR_NUM      27
#define BIGDATA_SYSTEM_NUM      61
#define UNLOCK_ERR_UPLOAD_NUM      5

#define SYS_PARAM_NUM                    50
#define INDOOR_MCH_NUM                   16

#define OUTDOOR_BASE_IP					8u
//内机台数
#define OUTDOOR_MCH_NUM                  1u
//外机台数
#define MAX_RECEIVE_REALPAGE             280
//最大实时监控参数量

//#define SYS_PARAM_FLAG                  30
#define SYS_PARAM_FLAG                  31
#define INDOOR_PARAM_FLAG                19

//大数据采集参数变化标志
#define SYS_BIG_DATA_FLAG       14
#define INDOOR_BIG_DATA_FLAG    7


#define INDOORLOCK_ERR_NUM   32
#define OUTLOCK_ERR_NUM      82
#define NONSENCE	0
//与该参数无关

#define OUTDOOR_BASE_SET	7
#define INDOOR_BASE_SET		3

#define OUTDOOR_BIT_SET     0x36        
#define INDOOR_BIT_SET      0x16

#define SYS_PARSE_TABLE_SIZE		108
#define INDOOR_PARSE_TABLE_SIZE		48

//#define SYS_TABLE_SIZE		234
#define SYS_TABLE_SIZE		242
#define INDOOR_TABLE_SIZE	152

#define WORD_TYPE_BINARY		0
#define BIT_TYPE_BINARY			3

#define SND_ERR_NUM				8	//二级故障数量


//系统表变化的标志，一位表示8个字节
typedef struct {
  uint8_t System [SYSTEM_CHANGE];                   //外机变化参数存储位置
  uint8_t Indoor [INDOOR_MCH_NUM][INDOOR_CHANGE];   //每台内机变化参数储存位置
}ParamType;

//大数据采样类型：将去重表转义以后，将值填充在大数据类型中
typedef struct {
    uint8_t System[BIGDATA_SYSTEM_NUM];
    uint8_t Indoor[INDOOR_MCH_NUM][BIGDATA_INDOOR_NUM];
}BigDataType;

typedef struct _BigData_Flash{
    uint32_t write_addr;
    uint32_t read_addr;
	uint32_t tmp_addr;
}BigData_Flash;


typedef struct {
	  uint8_t System[SYS_PARAM_FLAG];
	  uint8_t Indoor [INDOOR_MCH_NUM][INDOOR_PARAM_FLAG];
}Param_ChangflagType;//缓存参数变化的标志

typedef struct {
    uint8_t System[SYS_BIG_DATA_FLAG];
    uint8_t Indoor[INDOOR_MCH_NUM][INDOOR_BIG_DATA_FLAG];
}BigData_ChangeflagType;//缓存大数据采样相关的参数是否变化标志

#define WATER_FULL_PROTECT_ERR			1
#define INDOOR_WIND_PROTECT_ERR			0
#define FREEZE_PROTECT_ERR				3
#define LINE_CONTROL_PROBLEM			2
#define SWIFT_WIND_ERROR				7
#define ONE_CONTROL_MULTI_MACHINE		4
#define WATER_SWITCH_PROBLEM			5
#define DIVIDE_WATER_SET_PROBLEM		6



typedef struct{
    uint8_t indoor_lock_err[INDOOR_MCH_NUM][INDOORLOCK_ERR_DATA_LEN];//内机锁机故障
    uint8_t outdoor_lock_err[OUTLOCK_ERR_DATA_LEN];//外机锁机故障
    
    uint8_t water_full_protect_err[INDOOR_MCH_NUM];//水满保护故障---- 1E 05
    uint8_t water_full_protect_err_num[INDOOR_MCH_NUM];//水满保护故障计数
    
    uint8_t indoor_wind_protect_err[INDOOR_MCH_NUM];//内风机保护故障-----1E 03
    uint8_t indoor_wind_protect_err_num[INDOOR_MCH_NUM];//内风机保护故障计数
    
    uint8_t freeze_protect_err[INDOOR_MCH_NUM];//防冻结保护故障-----1E 07
    uint8_t freeze_protect_err_num[INDOOR_MCH_NUM];//防冻结保护故障计数

	uint8_t line_control_problem[INDOOR_MCH_NUM];//线控器供电异常---1E 06
	uint8_t line_control_problem_num[INDOOR_MCH_NUM];//计数
	
//	uint8_t voltage_supply_short[INDOOR_MCH_NUM];//电源供电不足---1F 5A
//	uint8_t voltage_supply_short_num[INDOOR_MCH_NUM];//计数
	
	uint8_t swift_wind_error[INDOOR_MCH_NUM];//扫风部件故障 ----1E 32
	uint8_t swift_wind_error_num[INDOOR_MCH_NUM];//计数

	uint8_t one_control_multi_machine[INDOOR_MCH_NUM];//一机多控内机台数不一致 ---1E 18
	uint8_t one_control_multi_machine_num[INDOOR_MCH_NUM];//计数

	uint8_t water_switch_problem[INDOOR_MCH_NUM];//水流开关故障-----1E 30
	uint8_t water_switch_problem_num[INDOOR_MCH_NUM];//计数

	uint8_t divide_water_set_problem[INDOOR_MCH_NUM];//分水阀设置故障----1E 31
	uint8_t divide_water_set_problem_num[INDOOR_MCH_NUM];//计数
}ErrParamType;

typedef struct {
    uint32_t SrcAdd:16;
    uint32_t Value:16;
}SrcDataTyp;

typedef struct {
	uint16_t SrcAdd;
	uint8_t ParsePlace;
}ParseTableType;

typedef struct{
    uint16_t SrcAdd[MAX_RECEIVE_REALPAGE];
    uint16_t  cnt;
    uint8_t  hl_flag[MAX_RECEIVE_REALPAGE];//高低位标志
}Real_PageTpye;

typedef struct{
    uint16_t SrcAdd[MAX_RECEIVE_REALPAGE];
    uint16_t  cnt;
}R_Real_PageTpye;


typedef struct PageTpye{
    const SrcDataTyp  *Page;
    const uint32_t    TableSize;
}PageTpye;

typedef struct _ParseType{
	const ParseTableType *parse;
	const uint8_t	ParseSize;
}ParseType;


typedef struct {
	uint8_t  MAC[6];                  //内机MAC
    uint8_t  online_flag;              //在线标志
    uint16_t Can1Can2IP;
    uint16_t number;                    //内机编号
}indoorinfoType;

typedef struct {
	uint8_t  MAC[6];                  //外机机MAC
    uint8_t  online_flag;             //在线标志
    uint16_t Can1Can2IP;
}outdoorinfoType;

//#pragma bitfields=disjoint_types
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
//#pragma bitfields=default

typedef struct {
    uint8_t EraserFlag;
    uint16_t RealDataLen;
    uint16_t ErrDataLen;
	uint16_t TakeDataLen;
	uint16_t BigDataLen;
	uint16_t PowerDataLen;
//	uint16_t SendPowerLen;
	uint16_t SendBigDataLen;
    uint32_t writeFlashAddr;
    uint32_t readFlashAddr;
    uint32_t saveFlashTime;//继续向Flash写入参数的时间
    uint32_t readFlashTime;
    uint32_t errPointTime;
	uint32_t err_address;
    uint32_t curTimeCnt;
	uint32_t last_err_time;
	uint32_t last_err_addr;
	uint32_t Sample_9D_Count;
//	uint32_t outdoor_power_write_addr;
//	uint32_t outdoor_power_read_addr;
    struct{
        uint32_t    Debug:1;
        uint32_t    OutErr:1;
        uint32_t    Healt:1;
        uint32_t    TestSig:1;
        uint32_t    InErr:1;
        uint32_t    IndOff:1;//内机全关标志位
        uint32_t    FastTest:1;
        uint32_t    DebugOver:1;
        uint32_t    SendFlag:1;//故障数据是否传输完成标志: 0 传输完成，1正在传输
        uint32_t    ErrIsClr:1;//故障是否清除标志:0,故障清除; 1,故障存在
        uint32_t    indoor_off_noupflag:1;//0，开关机监控;1，只开机监控;
        uint32_t	alrd_find_start:1;//是否查找到故障点30分钟前的起始位置
        uint32_t	write_flash_cross:1;//写过Flash至少一轮
        uint32_t	read_empty_over_sector:1;//读Flash超过64k的空块存在
        uint32_t	can_cnnct_ok:1;//CAN总线是否有数据
        uint32_t	erase_runparam_flash:1;//擦除Flash恢复出厂参数配置
        uint32_t	erase_error_real:1;//擦除错误缓存Flash和实时监控表
    };
	struct{
		uint32_t flag_5:1;
		uint32_t flag_30:1;
		uint32_t flag_sample:1;
		uint32_t flag_98:1;
		uint32_t flag_hours:1;
		uint32_t flag_no_fault:1;
		uint32_t flag_flash_revert:1;
	};
    uint8_t real98flag;//实时监控模式:0、默认没有配置信息，1、有配置信息，不管内机，3、有配置信息，考虑内机
    uint8_t realup98flag;//实时监控标志位：0、不透传监控，1、强制透传监控
    uint8_t trans_mode;//拍照模式：0：三十分钟前变化传，第三十分钟拍照；1：按照配置时间拍照传
    struct{
		uint8_t per_5_min:1;//每5分钟拍照一次
		uint8_t per_30_min:1;//每30分钟拍照一次
		uint8_t per_config_min:1;//配置时间大于30分钟时，每配置时间拍照一次
		uint8_t enter_a_state:1;//第一次进入错误、实时监控、调试、按键时时，9A帧采集完成以后才有96帧
		uint8_t taking_photo_flag:1;//正在拍照标志: 1,正在拍照的过程，0，拍照结束
    };
	uint8_t data_type;//0:表示正常数据;1:表示拍照数据
    uint8_t indoor_lockerr_flag;//内机锁机故障标志位
    uint8_t out_lockerr_flag;//外机锁机故障标志位
    uint8_t indoor_not_lockerr_flag;//内机非锁机故障标志位
    uint8_t  RealBuffer[1024];
    uint8_t  ErrBuffer[1024];
	uint8_t  TakeBuffer[1024];
	uint8_t BigDataSaveBuffer[1024];
	uint8_t BigDataSendBuff[1024];
	uint8_t PowerDataBuffer[1024];
//	uint8_t PowerDataSendBuffer[1024];
}UpDataType;

typedef struct 
{
	uint8_t data_type;
    uint16_t DataLen;
    uint8_t  DataBuf[1026];
}BUFF_TYPE;

typedef struct {
        uint8_t mac_low;
        uint8_t mac_high;
        uint8_t ready_flag;
}mac_type;

typedef struct {
    mac_type mac[100];
    uint8_t data_len;
    uint16_t first_index;
    uint16_t second_index;
}update_mac_type;

#define QSEND9ELEN    5                       
typedef struct {
    uint16_t  data_addr;
    uint16_t  can2_can1_ip;
	time_t error_time_tag;
}send9equeue;
extern send9equeue Send9Edata[];  //9E帧数据队列
extern OS_EVENT *QSend9E;         //9E帧数据队列信号量
extern void      *Send9EdSendPtrArr[QSEND9ELEN];
extern uint8_t  send9e_cnt;



extern uint8_t   sendCnt ;
extern OS_EVENT  *QSemSend ;
extern void      *SendPtrArr[];
extern BUFF_TYPE SendBuffer[];

extern UpDataType g_Upload;
extern ParamType ACParam;
extern PageTpye  PageData[];
extern ParseType parseData[];

extern PageTpye  PageErrData[];
//extern PageTpye PageSndErr[];


extern uint32_t g_FlashWriteAddr;
extern OS_EVENT *MutexFlash;
//extern OS_EVENT *MutexAT1Send;
//extern OS_EVENT *MutexAT2Send;
extern OS_EVENT *SemAT1Send;
extern OS_EVENT *SemAT2Send;
extern OS_EVENT *SemHandleTxFrame;

//extern OS_EVENT *SemAT1Buff;
//extern OS_EVENT *SemAT2Buff;

//extern OS_EVENT *MutexSend1;
//extern OS_EVENT *MutexSend3;
extern OS_EVENT *MutexSend4;
extern OS_EVENT *MutexSend2;
extern OS_EVENT *MutexSendData;
extern BigDataType BigDataParm;
extern BigDataType BigDataParm_pri;

extern BigData_ChangeflagType BigDataParam_ChangeFlag;
extern BigData_ChangeflagType BigDataParam_ExistFlag;//大数据参数表存在标志

extern BigData_Flash BigDataSampleOneHours;
extern BigData_Flash BigDataSampleTwoHours;

extern uint8_t open_up_take_photo;//开机拍照



//extern uint16_t  power_packtime;


extern uint8_t one_two_flag;
extern uint8_t try_time;

extern uint8_t is_spill_flag;
//extern uint8_t sample_ok_flag;
extern uint8_t one_hours_flag;

extern uint8_t real_ret;
//extern uint8_t flash_revert_flag;


extern ParseTableType SysParseTable[];
extern ParseTableType IdrParseTable[];
extern SrcDataTyp  SysTable[];
extern SrcDataTyp  OdrTable[];
extern SrcDataTyp  IdrTable[];
extern SrcDataTyp  outErrTable[]; 
extern SrcDataTyp  indoorErrTable[];
//extern SrcDataTyp SecondErrTable[];

extern uint16_t  packTime;
extern uint16_t  real_packTime;
extern outdoorinfoType outdoorinfo;
extern ErrParamType ACErrParam;
extern const uint8_t TypeTable[];
extern indoorinfoType indoorinfo[INDOOR_MCH_NUM];
extern Param_ChangflagType ACParam_Changflag;
extern Param_ChangflagType ACParam_Existflag;

extern uint8_t onlineintimer[INDOOR_MCH_NUM];
extern Real_PageTpye  Receive_Real_Page;
extern update_mac_type update_mac;
extern uint8_t update_sendflag;
#ifdef USING_USER_HABIT
extern uint16_t user_pack_time;
#endif

//extern uint8_t power_switch;


uint32_t dec2BCD(uint32_t m_DEC);
uint32_t bcd2DEC(uint32_t m_BCD);
void setACTime(struct tm *m_Time);
struct tm* getACTime(CanMsgType *p_CanMsg , struct tm *p_Time);

void writeSIMToCAN(uint8_t simStatus);
void WriteData2Flash(UpDataType *writrBuff,uint8_t data_mode);

uint8_t ReadData2Flash(UpDataType *readBuff,uint8_t data_type);
void savePowerDownInfo(UpDataType *saveData);
uint16_t timeOutPackUser();
void checkMachStatus(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff);
void convCanData(const CanRxMsg *m_CanFrame , CanMsgType *m_CanMsg);
uint16_t binarySearch(const PageTpye *PageData , const CanMsgType *m_Msg,uint8_t *m_Data,uint8_t param_type);
uint8_t bigDataBinarySearch(const SrcDataTyp *DataAddr ,uint8_t *m_Data,uint8_t *bigdata_change_flag,uint8_t param_type);
uint16_t fillErrData(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff);
uint16_t timeOutPack(UpDataType *p_ErrBuff);
void printToUart(const CanMsgType *m_CanMsg);
uint16_t GetDataAddrr(uint16_t addr, PageTpye *PageData);
uint8_t GetRealPagehlflag(const Real_PageTpye *src_real_page, Real_PageTpye *dst_real_page);

uint16_t CRC16_CheckSum(uint8_t *CRC_Buff, uint16_t Buff_Len, uint16_t crc_code);
uint16_t CRC16_Check(uint8_t *puchMsg, uint16_t usDataLen);

uint16_t binarySearchData(const PageTpye *PageData , const CanMsgType *m_Msg , uint8_t* m_Data);
void check_Err(UpDataType* err_buffer);
uint16_t GetrealData(PageTpye *PageData, uint16_t *addr, uint8_t *m_Data);
uint16_t findindoornum(const CanMsgType *m_CanMsg);
uint8_t XOR_Check(uint8_t* data, uint16_t len);

uint8_t read_bigdata_data(BigData_Flash *readBigData,uint32_t start_addr);
//uint8_t bigData_Sample_Data(const ParseType *parse_type,uint8_t *bigdata_param,uint8_t *bigdata_param_pri,uint8_t *bigdata_change_flag,uint16_t param_type)//uint8_t bigData_Sample_Data(const ParseType *parse_type,uint8_t *bigdata_param,uint8_t *bigdata_param_pri,uint8_t *bigdata_change_flag,uint16_t param_type);
uint8_t bigData_Sample_Data(const ParseType *parse_type,uint8_t *bigdata_param,uint8_t *bigdata_param_pri,uint8_t *bigdata_change_flag,uint8_t *bigdata_exist_flag,uint16_t param_type);

uint8_t send_bigdata_frame(uint8_t trans_mode,uint8_t mode_state);
uint8_t power_sample_into_flash(uint16_t power_param,uint8_t send_flag);
uint8_t sample_power_data_frame(uint16_t power_param);
void bubble_sort(Real_PageTpye *rcv_monitor_page);

uint8_t check_9E(uint8_t trans_mode,uint8_t mode_state);



uint8_t send_outdoor_power_data_frame(uint8_t trans_mode,uint8_t mode_state);
#ifdef USING_USER_HABIT
uint8_t send_user_habit_data_frame(uint8_t trans_mode,uint8_t mode_state);
#endif
uint16_t fillRealData(CanMsgType *p_CanMsg , BUFF_TYPE *real_handler);
uint16_t timeOutRealSend(BUFF_TYPE *Real_Buff_Data);

#endif




