#ifndef UPDATE_M
#define UPDATE_M

#include "stm32f10x.h"
#include "duplicate.h"
#include "fsm.h"
//extern M_updateInfoType;

#define OUTDOOR_DEV			0x77	//外机设备类型
#define GPRS_DEV			0x7F	//GPRS设备类型
#define GENERAL_INDOOR_DEV	0x01	//通用内机设备类型

#ifdef DEBUG_ELF_VERSION
#define UPDATE_TYPE_FUNC					0x80	//升级相关类型功能码
#define UPDATE_TYPE_FUNC_GPRS				0xC0	//升级相关类型功能码
#else
#define UPDATE_TYPE_FUNC					0x60	//升级相关类型功能码
#define UPDATE_TYPE_FUNC_GPRS				0xC0	//升级相关类型功能码
#endif
#define REQ_GET_FUNC				0xC1	//请求接收功能码
#define PUBLISH_STT_FUNC			0xC2	//状态数据功能码
#define REPLY_ACK_FUNC				0xC4	//应答数据
#define REPLY_CODE_FUNC				0xC8	//代码数据功能码

#define NORMAL_UPDATE			0x00	//正常状态

#define READY_UPDATE			0x00	//准备升级
#define NEEDY_UPDATE			0x01	//需要升级
#define NONE_UPDATE				0x02	//无需升级
#define RCV_CODE_OVER			0x03	//接受完成
#define BEING_UPDATE			0x04	//正在升级
#define SUCC_UPDATE				0x05	//升级成功
#define FAIL_UPDATE				0x06	//升级失败
#define PUBLISH_STT				0x07	//公布状态
#define URGENT_UPDATE			0x08	//紧急升级
#define MAKE_SURE_UPDATE		0x09	//确认升级
#define ENTRY_BOOTLOADER_UPDATE	0x0A	//进入bootloader状态

#define MACHINE_NUMBERS			0x04	//机组数量		

#define GRF_FIRST_INDEX					0x01	//get request flag(GRF)发送一级索引所有数据
#define	GRF_RE_LAST_INDEX				0x02	//重复发送最后一帧校验数据
#define GRF_SECOND_INDEX				0x03	//发送二级索引数据
#define GRF_BEGIN_INDEX					0x04	//启动发送时序

#define SRD_254Byte						0x00	//send request data(SRD)发送请求的254B数据
#define SRD_8Byte						0x01	//发送单帧数据

#define WAIT_REGISTER_TIME						20	//GPRS公布状态次数
#define WAIT_REGISTER_TIME_URGENT				60	//GPRS公布紧急状态次数,防止中途换机，重新初始化模块信息
#define WAIT_IDLE_TIME					20	//等待20s
#define WAIT_UPDATE_TIME				2	//失败情况下，对机组升级重试次数
#define WAIT_ACK_TIME					4	//重复应答次数

#define WAIT_FIRST_ACK_NO					0	//一级索引响应OK
#define WAIT_SECOND_ACK_NO					0	//一级索引响应OK
#define WAIT_FIRST_ACK_OK					1	//一级索引响应OK
#define WAIT_SECOND_ACK_OK					1	//一级索引响应OK

#define ERRNO_NORMAL_ERR				0x00	//正常
#define ERRNO_SUPERBLOCK_ERR			0x01	//超级块错误
#define ERRNO_BACKCODE_ERR				0x02	//备份程序错误
#define ERRNO_RESERVE_ERR				0x04	//预留
#define ERRNO_EX_FLH_R_ERR				0x08	//外部Flash读取错误
#define ERRNO_EX_FLH_W_ERR				0x10	//外部FLash写入错误
#define ERRNO_IN_FLH_E_ERR				0x20	//内部Flash擦除错误
#define ERRNO_IN_FLH_W_ERR				0x40	//内部Flash写入错误
#define ERRNO_CODE_CRC_ERR				0x80	//程序校验错误


#define OK			0x01	//OK状态标志位
#define NO			0x00	//OK状态标志位

//#define RESET		0x00	//清零

#define REBACK_UPDATE_SUCC		0x01	//反馈升级成功
#define REABCK_UPDATE_FAIL		0x02	//反馈升级失败
#define REBACK_UPDATE_OVER		0x03	//反馈升级程序接收完成
#define REABCK_UPDATE_BAR		0x04	//反馈升级进度


#define CALL_DEVICE_TOTAL		20		//点名频率
#define ACK_DEVICE_TOTAL		20		//点名频率


typedef struct{
    uint16_t m_mac;
    uint16_t first_index;          //一级索引地址
    uint16_t second_index;         //二级索引地址
    uint8_t  m_Type;               //设备类型
    uint8_t  senddata_flag;       //当flag为1时发送一级索引的所有数据，为2是重复发送，3发送二级索引。
    uint8_t  first_index_respone; //接收一级索引完成标志
    uint8_t second_index_respone;//接收二级索引完成标志
    uint8_t have_needupdate;       //是否有需要升级的模组
    uint8_t errno;
	uint8_t update_ok;//反馈升级OK
	uint8_t respond_ok;//进入bootload升级响应OK，等待所有在线机组都响应OK后才进入Bootloader升级状态
}machineinfoType;

typedef struct{
    uint8_t m_software_version;        //程序版本号
    uint32_t paketsize;                //程序大小
    machineinfoType m_info[MACHINE_NUMBERS]; //机组的升级信息
    uint8_t m_mac_num;                  //在线的机组台数
    uint8_t m_mac_num_tmp;                  //在线的机组台数
    uint8_t m_inter_mac_num;                  //在线的内机台数，预留
    uint32_t m_update_writeaddr;//升级包末尾位置
	uint8_t m_op_number;//当前正在升级的外机
	uint8_t urg_up;//紧急升级标志
	uint8_t send_bootloader_op;//是否发送进入bootlaader标志
}M_updateInfoType;

/*
m_register:公布状态。每秒发一次，知道收到回复
m_senddata:发送数据
*/
#define UPDATE_STATE_NUM	12	//更新过程中的状态数量
enum  updatesteptype{m_getdata, m_register, m_erase, m_senddata, m_updatesuccess,Idle,m_monitor,m_failure,m_debugelf,m_codeok,m_ackok};//枚举升级状态

extern BUFF_TYPE  sendcan_buffer;
extern  enum updatesteptype m_updatestep;
extern  M_updateInfoType m_updateInfo;
void send_firstindexdata(M_updateInfoType* mupdateInfo, uint8_t num, uint8_t single_or_no);
extern uint32_t respond_can_update(uint8_t respond);
extern void save_update_machine_flash(M_updateInfoType* m_updateInfo1,uint8_t err);
extern uint8_t online_unit_count;
extern uint8_t current_outdoor_num;


void send_update_register(M_updateInfoType* m_updateInfo);
uint8_t check_update(CanMsgType* m_Data,M_updateInfoType* m_updateInfo1);
void send_update_finish(void);
uint8_t check_online_machine_respond(M_updateInfoType* m_updateInfo2);
uint8_t* read_can_update_boot(uint8_t* can_buf,uint32_t flash_address,uint16_t len);
void updateinfo_init(uint8_t* value1,uint32_t* value2);
void init_time_clock();
void send_entry_bootloader(M_updateInfoType* m_updateInfo,uint8_t op_mac);
void init_monitor_stt(uint8_t* value1,uint32_t* value2);
void send_ack_ok(M_updateInfoType* m_updateInfo,uint8_t op_mac);
uint8_t reback_update_stt(UpDataType* stt,uint8_t opt);




extern char* Update_State[];
uint16_t send_updateto_can(uint16_t first_addr, uint16_t second_addr, uint8_t num);
uint32_t read_can_update(BUFF_TYPE *m_sendcan_buffer, uint32_t flash_addr, uint16_t len);
#endif


