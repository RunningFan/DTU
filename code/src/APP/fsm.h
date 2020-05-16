
#ifndef _FSM_H_
#define _FSM_H_

#include "stm32f10x.h"
#define OUTDOOR_ERR		1
#define INDOOR_ERR		0

//#define TEST_UPGRADE			//生产升级包时打开该宏
//#define DEBUG_PROJECT			//调试上报模式测试
//#define DEBUG_ONLINE			//调试在线测试模式
//#define DEBUG_BUTT				//调试按键上报模式
//#define TEST_FRAME				//44服务器上测试,数据帧无需回复
#define F2FRAME_DEBUG			//测试F2帧，无需F2时注释
//#define TEST_PRINT_TEST			//打印测试
//#define DEBUG_ERROR		1		//调试时避免多主线控器错误干扰
//#define DEBUG_FLY		1		//测试关闭模组
//#define SMS_TEST
#define TEST_7005_PORT		//测试7005端口
#define DEBUG_ING 	//测试中

#define KEEP_TCP_LIVE	//启用功能帧和数据帧
//#define FACTORY_SIMULATE_FLASH	//模拟出厂第一次烧写程序，将Flash全部清零

#define take_photo_mode		1u
#define de_duplicate_mode	0u

#define INDOOR_ONLINE_LAST_TIME		120
#define DE_DUPLICATE_PACK_TIME		60
#define REAL_DE_DUPLICATE_PACK_TIME		120
#define USER_HABIT_PACK_TIME		120
//#define POWER_SAMPLE_PACK_TIME		7200
#define current_indoor_num ((current_indoor_number > (INDOOR_MAX-1)) ? 0 : current_indoor_number)
//修改为大于等于
typedef uint8_t (*FuncPtr)(void * , void **);
typedef struct
{
    uint8_t projDebug;
    uint8_t onlineTest;
    uint8_t outdoorErr;
    uint8_t indoorErr;
    uint8_t buttonPush;
    uint8_t realMonitor;
    uint8_t s_update;
    uint8_t Hyaline;
}varType;


typedef struct
{
/*
************Attention: 在此处不能随便增删元素*****************
*/
    uint8_t  upate_receive_flag;  //接收完成标志
    uint8_t  update_pack_type;    //升级包类型
    uint8_t  update_success_flag; //升级成功标志
    uint8_t  updata_hisSOFTVERSION;//软件历史版本号
    uint8_t  recover_have_packet;         //是否有备份程序标志
    uint8_t  begin_recover_time;      //防止一次升级过程成功置位前断电就回滚问题。
    uint16_t upgrade_succ_count;//DTU自身升级成功的次数
	uint16_t upgrade_fail_count;//DTU自身升级失败的次数
    uint32_t update_readaddr;
    uint32_t update_writeaddr;
/*
************Attention: 在此处不能随便增删元素*****************
*/
}updateInfoType;

enum states{ s_idle, s_debug, s_button, s_outdoor , s_indoor , s_real , s_test , s_update};   //枚举系统状态
enum   updatesteptype{m_getdata, m_register, m_erase, m_senddata, m_updatesuccess};                 //枚举升级状态
extern  varType var;
//extern uint16_t receive_count;
extern uint8_t fail_count;
extern  FuncPtr eventMode[];
extern  uint8_t NS; //定义下一状态
extern  uint8_t losepower;
//extern  uint8_t P_connectToServer(void);//新加wang
//extern  uint32_t Real_sendTime; //实时传输计时wang
extern  updateInfoType updateinfo;
extern  enum updatesteptype updatestep;
extern  uint16_t  recieve_CheckSum;
//extern  uint8_t update_downloading;    //add by sj
uint8_t connectToServer(void);
uint8_t disConnectToServer(void);
//extern uint8_t  begin_real;//
extern uint16_t mode_time ;
extern uint8_t wait_update;//连接失败，进入等待状态
//extern uint16_t config_time;
//extern uint8_t config_flag;
extern uint8_t enter_state;//故障点拍照计数,只需要拍照一次
extern uint8_t enter_state_button;//故障点拍照计数,只需要拍照一次
extern uint8_t enter_state_monitor;//实时监控进入点,只需要拍照一次
extern uint8_t takeup98flag;
extern OS_EVENT * MSemQuickPhoto;
extern OS_EVENT * MSemSavePhoto;
extern uint8_t tran_mode;
extern uint8_t bigdata_take_photo_over;
extern uint8_t switch_one_two;//一二小时切换时间
extern uint8_t five_min_flag;//五分钟计时标志
//extern uint8_t takeup98_flag;
extern uint8_t upgrade_buf[];

extern uint8_t LED_SIM;





uint8_t bigDataSample(uint8_t sample_mode);
uint8_t bigData_sample_into_flash(uint16_t bigdata_param,uint8_t id,uint16_t can_ip,uint8_t data_type);
uint8_t SendDataToServer(uint8_t trans_mode,uint8_t mode_state);
uint16_t match_parse(uint8_t id,uint8_t param_type);



void    ModeStateMachine(void *argc , void *argv[]);
#endif

