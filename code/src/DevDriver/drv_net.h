#ifndef _DRV_NET_H_
#define _DRV_NET_H_
#include "stm32f10x.h"
#include "fsm.h"
#include "duplicate.h"

#define ON    1
#define OFF   0
#define STM32 ON
#define UART_MAX_TX_LEN 1200
#define UART_MAX_RX_LEN 350

#define FUNCODE_LOCATION	0x12

#define UPGRADE_OK					0xFF
#define UPGRADE_FAIL				0xAA
#define UPGRADE_PACKET_ERROR	    0xBB
#define UPGRADE_FEEDBACK			0x00

#define REQ_FEEDBACK		0x00
#define REQ_MACHINE_CODE	0x01
#define REQ_DTU_CODE		0x02

#define ADMIN_MAX_NUM	7u
#define USER_MAX_NUM	11u

#define FRAME72 OFF
#define FRAME88 OFF
#define FRAME89 ON
#define FRAME90 OFF
#define FRAME91 ON
#define FRAME92 ON
#define FRAME93 ON
#define FRAME94 ON
#define FRAME95 ON
#define FRAME96 ON
#define FRAME97 ON
#define FRAME99 ON
#define FRAME9A ON
#define FRAME9B ON
#define FRAME9D ON
#define FRAME9E ON
#define FRAME9F ON
#define FRAMEA3 ON
#define FRAME98 ON
#define FRAMEEE ON
#define FRAMEF2 ON
#define FRAMEF3 ON
#define FRAMEA2 ON
#define FRAMEF4 OFF
#define FRAMEF5 ON
#define FRAMEA0 ON
#define SRV_DEBUG 1
#ifdef TEST_UPGRADE
#define SOFTVERSION 	0x25
#else
#define SOFTVERSION		0x24
#endif

#define HARDVERSION		0x40
#define MAX_GETF3_TIME  3
#define MAX_GETF2_TIME  1

#define MAX_MONITOR_NUM	255
#define DEBUG 1

#define HYALINE_MODE	0
#define CHANGE_MODE		1

#define OUTDOOR_ERR_MODE		0u
#define PROJ_DBG_MODE			1u
#define BUTTON_MODE				2u
#define REALMONITOR_MODE		3u
#define INDOOR_ERR_MODE			5u
#define USER_HABIT_MODE			8u
#define IDLE_MODE				16u
#define SIGN_EVERY_MODE			255u

//nor_bit :normal bit
//low_bit :low 8 bit
//hig_bit :high 8 bit
//abnor_low8_bit:abnormal low 8 bit
//abnor_hig8_bit:abnormal hig 8 bit
#define NOR_BIT				0u
#define LOW_BIT				1u
#define HIG_BIT				2u
#define ABNOR_LOW8_BIT		3u
#define ABNOR_MID16_BIT		4u
#define ABNOR_MID24_BIT		5u
#define ABNOR_HIG8_BIT		6u
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
                uint8_t  FuncMode; 
            }frame72Rx;
#endif
        
#ifdef FRAME88
            struct{
                uint8_t  FuncMode; 
            }frame88Rx;
#endif

#ifdef FRAME89
            struct{
                uint8_t   FuncMode; 
                uint16_t  CheckCode;
            }frame89Rx;
#endif
        
#ifdef FRAME90
            struct{
                uint8_t   FuncMode; 
                uint16_t  CheckCode;
            }frame90Rx;
#endif

#ifdef FRAME91
            struct{
                uint8_t  FuncMode; 
                uint8_t  Result;
            }frame91Rx;
#endif
        
#ifdef FRAME92
            struct{
                uint8_t  FuncMode;
                uint8_t  StopMode;
            }frame92Rx;
#endif

#ifdef FRAME93
            struct{
                uint8_t  FuncMode;
                uint8_t  Flag;
            }frame93Rx;
#endif
        
#ifdef FRAME94
            struct{
                uint8_t  FuncMode;
                uint8_t  *Param; 
            }frame94Rx;
#endif
        
#ifdef FRAME95
            struct{
                uint8_t  FuncMode;
                uint8_t  *Param; 
            }frame95Rx;
#endif
      
#ifdef FRAME96
            struct{
                uint8_t  FuncMode;
                uint8_t  Res; 
            }frame96Rx;
#endif     
            
#ifdef FRAME97
            struct{
                uint8_t  FuncMode;
                uint8_t   *Param; 
                uint8_t  Update; 
            }frame97Rx;
#endif    
        
#ifdef FRAME99
            struct{
                uint8_t  FuncMode;
                uint8_t  config_flag;//配置标志:0x00:没有配置信息，0x01:有配置信息(不管内机状态),0x02:有配置参数，内机全关不发
                uint16_t config_time;//配置时间
                uint16_t DataLen;
                uint8_t  *Data;
            }frame99Rx;                        
#endif      

#ifdef FRAME9A
 			struct{
				uint8_t FuncMode;
				uint8_t result;
 			}frame9ARx;
#endif           

#ifdef FRAME9B
 			struct{
				uint8_t FuncMode;
				uint8_t result;
 			}frame9BRx;
#endif

#ifdef FRAME9D
            struct{
                uint8_t  FuncMode;
                uint8_t  Res; 
            }frame9DRx;
#endif

#ifdef FRAME9E
            struct{
                uint8_t  FuncMode;
                uint8_t  Res; 
            }frame9ERx;
#endif

//功率回复帧
#ifdef FRAME9F
            struct{
                uint8_t  FuncMode;
                uint8_t  Res; 
            }frame9FRx;
#endif

#ifdef FRAMEA2

		struct{
			uint8_t  FuncMode;	
			uint16_t DataLen;
			uint8_t  *data;
		}frameA2Rx;

#endif

//A3帧为控制帧，该帧待定
#ifdef FRAMEA3
 			struct{
				uint8_t FuncMode;
				uint16_t datalen;
				uint8_t* data;
 			}frameA3Rx;
#endif 
        
#ifdef FRAMEF3
            struct{
                uint8_t  FuncMode;
                struct tm Time; 
            }frameF3Rx;
#endif

#ifdef FRAMEF2
            struct{
                uint8_t  FuncMode;
                uint8_t result;
            }frameF2Rx;
#endif
        
#ifdef FRAMEF4
            struct{
                uint8_t  FuncMode;
                uint8_t  Resault;
            }frameF4Rx;
#endif

#ifdef FRAMEF5
//此处由于服务器端也会发送F5帧，并且DTU也会主动发送F5帧，
//所以此处分为两个状态：1、在服务器端主动发送F5帧时，会有
//打卡时间和统计时间配置；2、在DTU主动发送打卡时，会回复
//一帧打卡OK
		union{
			struct{
				uint8_t FuncMode;
				uint8_t result;
			}frameF5Rx_sign;
            struct{
                uint8_t  FuncMode;
                uint16_t  config_sign_time;//客户端配置的用户习惯打卡时间
				uint16_t  config_statistic_time;//客户端配置的统计参数打卡时间
            }frameF5Rx;
		}ret;
#endif 

        
#ifdef FRAMEA0
            struct{
                uint8_t  FuncMode;  
                struct tm Time;
                uint16_t StartAddr;
                uint16_t FinishAddr;
            }frameA0Rx;

#endif 
#ifdef FRAME98
	            struct{
	                uint8_t  FuncMode;
					uint8_t  result;
//	                struct tm Time;
//	                uint16_t* conf_table;
	            }frame98Rx;
				
#endif
    }ValidDataRx;

		union
		{
			uint8_t  FuncMode;
#ifdef FRAME72
				struct{
					uint8_t  FuncMode;	
				}frame72Tx;
#endif
			
#ifdef FRAME88
				struct{
					uint8_t  FuncMode;	
				}frame88Tx;
#endif
	
#ifdef FRAME89
				struct{
					uint8_t   FuncMode;  
					uint16_t  CheckCode;
					uint16_t  HardVesion;
					uint16_t  SoftVesion;
					uint8_t*  IMEI;
					uint8_t*  BaseStation;
					uint8_t*  ICCID;
				}frame89Tx;
#endif
			
#ifdef FRAME90
				struct{
					uint8_t   FuncMode;  
					uint16_t  CheckCode;
					uint16_t  SoftVesion;
					uint8_t*   IMEI;
					uint8_t*   BaseStation;
					uint8_t*   ICCID;
				}frame90Tx;
#endif
	
#ifdef FRAME91
				struct{
					uint8_t  FuncMode;
					uint8_t  TranMode;
					struct tm Time;
				}frame91Tx;
#endif
			
#ifdef FRAME92
				struct{
					uint8_t  FuncMode;	
					uint8_t  Result;
				}frame92Tx;
	
#endif
	
#ifdef FRAME93
				struct{
					uint8_t  FuncMode;	
					uint8_t  Result;
				}frame93Tx;
	
#endif
			
#ifdef FRAME94
				struct{
					uint8_t   FuncMode; 
					uint16_t  ParamLen;
					uint8_t   *FailParam; 
				}frame94Tx;
	
#endif
			
#ifdef FRAME95
				struct{
					uint8_t   FuncMode;  
					uint16_t  ParamLen;
					uint8_t   *Param; 
				}frame95Tx;
	
#endif
		  
#ifdef FRAME96
				struct{
					uint8_t  FuncMode;
					uint16_t DataLen;
					uint8_t  *Data;
					uint8_t  TranMode;
					uint8_t  MachMode;
					struct tm Time;
				}frame96Tx;
				
#endif   
				
#ifdef FRAME97
				struct{
					uint8_t  FuncMode;	
					uint8_t  update_type;
					uint8_t  Result; 
					uint8_t  *mac;
					uint8_t  program_version[3];
				}frame97Tx;
	
#endif    
			
#ifdef FRAME99
				struct{
					uint8_t  FuncMode;
					uint8_t  Res; 
				}frame99Tx;
				
#endif      
	
#ifdef FRAME9A
				struct{
					uint8_t  FuncMode;
					struct tm time;
					uint8_t machine_mac[6];//机组MAC
					uint16_t pro_num;//为内机数据时：内机工程编号；为外机数据时：0x0000;系统参数时：0xFFFF
					uint8_t can2ip;
					uint8_t can1ip;
					uint16_t m_id;//机型ID
					uint8_t *data_ptr;
					uint16_t data_length;
				}frame9ATx;
				
#endif           
	
#ifdef FRAME9B
				struct{
					uint8_t  FuncMode;
					struct tm Time;
					uint8_t  *DataPtr;
					uint16_t DataLenth; 
				}frame9BTx;
				
#endif

#ifdef FRAME9D
				struct{
					uint8_t  FuncMode;
					uint16_t DataLen;
					uint8_t  *Data;
					struct tm Time;
				}frame9DTx;
#endif

#ifdef FRAME9E
		 struct{
					 uint8_t  FuncMode;
//					 uint16_t DataLen;
					 uint8_t  MAC[6];
					 uint16_t errtype;
					 struct tm Time;
			 }frame9ETx;

#endif

#ifdef FRAME9F
				struct{
					uint8_t  FuncMode;
					uint16_t DataLen;
					struct tm Time;
					uint8_t  *Data;
				}frame9FTx;
#endif


#ifdef FRAMEA2
            struct{
                uint8_t  FuncMode;
                struct tm Time;
            }frameA2Tx;
#endif

	
	//A3帧为控制帧，该帧待定
#ifdef FRAMEA3
				struct{
					uint8_t  FuncMode;
					uint8_t  Res; 
				}frameA3Tx;
#endif 
	
	
#ifdef FRAMEEE
			struct{
				struct{
					uint8_t  FuncMode;
					uint8_t  ErrCode;
				}frameEETx;
			};
#endif  
			
#ifdef FRAMEF3
				struct{
					uint8_t  FuncMode;	
				}frameF3Tx;
	
#endif 

#ifdef FRAMEF2
				struct{
					uint8_t  FuncMode;
					uint32_t exe_time;
				}frameF2Tx;
	
 #endif 
			
#ifdef FRAMEF4
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
	
#endif
	
#ifdef FRAMEF5
				struct{
					uint8_t  FuncMode;	
					struct tm Time;//时间
					uint16_t StaAddr;//开始地址
					uint8_t  Signal;//信号强度
					uint16_t  Times;//打卡次数
					uint8_t  DataType;//数据类型
					uint16_t DataLenth;//数据长度
					uint8_t  *DataPtr;
					uint8_t Can2IP;//CAN2IP
					uint8_t ProNum;//工程类型
					uint8_t result;
				}frameF5Tx;
	//此处由于服务器端也会发送F5帧，并且DTU也会主动发送F5帧，
	//所以此处分为两个状态：1、在服务器端主动发送F5帧时，会有
	//打卡时间和统计时间配置；2、在DTU主动发送打卡时，会回复
	//一帧打卡OK
#endif 
	
			
#ifdef FRAMEA0
				struct{
					uint8_t  FuncMode;	
					struct tm Time;
					uint16_t StartAddr;
					uint16_t FinishAddr;
				}frameA0Tx;
	
#endif 
#ifdef FRAME98
				struct{
					uint8_t  FuncMode;
					struct tm Time;
					uint8_t  *DataPtr;
					uint16_t DataLenth;
				}frame98Tx;
#endif
		}ValidDataTx;

} TcpType;


typedef struct
{
    uint8_t  EraserFlag  ;
    uint8_t  Pause       ;
    uint8_t  Version     ;
	uint8_t isOpenSample;
	uint8_t isOpenPower;
    uint16_t SerPort     ;
    uint16_t HeartTime   ;
    uint16_t OutErrTime  ;
	uint16_t Out_Err_After_Time;
    uint16_t HealtTime   ;
    uint16_t ButtTime    ;
    uint16_t InerrtTime  ;
	uint16_t In_Err_After_Time;
    uint16_t big_data_sampe_time;//大数据采样时间配置
//    uint16_t big_data_reduce_valve;//大数据采样降精度配置	
//    uint16_t big_data_reduce_valve;//大数据采样降精度配置
    uint16_t bigdata_reduce_outdoor_V;//外机电压精度配置
    uint16_t bigdata_reduce_outdoor_Hz;//外机频率精度配置
    uint16_t bigdata_reduce_outdoor_100W;//外机百瓦精度配置
    uint16_t bigdata_reduce_outdoor_B;//外机步数配置
    uint16_t bigdata_reduce_outdoor_kPa;//外机千帕斯卡配置
    uint16_t bigdata_reduce_outdoor_10W;//外机十精度配置
    uint16_t bigdata_reduce_outdoor_C;//外机温度参数配置
    uint16_t bigdata_reduce_outdoor_A;//外机电流精度配置
    uint16_t bigdata_reduce_indoor_B;//内机步数精度配置
    uint16_t bigdata_reduce_indoor_rpm;//内机转速度精度配置
    uint16_t bigdata_reduce_indoor_C;//内机温度精度配置    
    uint16_t outdoor_power_accuracy;//外机功率采集精度配置
    uint32_t MaitTime    ;
//	uint32_t real_monitor_elapse_time;    
//    uint16_t power_pack_time;//功率数据打包时间配置
    char ApnAccr[20]  ;
    char ApnUsr [10]  ;
    char ApnPwd[10]   ;
    char SerName[20]  ;
    char SmsPwd[10]   ;
    char Admin[ADMIN_MAX_NUM][20];
    char Usron[USER_MAX_NUM][20];
    char IMEI [16] ;
    char ICCID[21] ;
    char CellInfo[15] ;
}TypeGPRS;


typedef struct hour_data{
	uint8_t weather_status;            //天气状况
	uint8_t temperature;               //温度
	uint8_t humidity;                  //相对湿度
	uint8_t air_pressure_low;          //气压低位
	uint8_t air_pressure_high;         //气压高位
	uint8_t wind_speech;               //风力或风速
	uint8_t wind_direction;            //风向
	uint8_t reserve[13];               //预留
}hour_data;

typedef struct day_data{
	uint8_t date_weather_status;       //白天天气状况
	uint8_t night_weather_status;      //白天天气状况
	uint8_t max_temperature;           //最高温度
	uint8_t mini_temperature;          //最低温度
	uint8_t rainfall_low;              //降雨量低位
	uint8_t rainfall_high;             //降雨量高位
	uint8_t rain_probability;         //降雨概率
	uint8_t humidity;                  //相对湿度
	uint8_t air_pressure_low;          //气压低位
	uint8_t air_pressure_high;         //气压高位
	uint8_t wind_speech;               //风力或风速
	uint8_t wind_direction;            //风向
	uint8_t sunrise_hour;              //日出时间——小时
	uint8_t sunrise_min;               //日出时间——分钟
	uint8_t sunset_hour;               //日落时间——小时
	uint8_t sunset_min;                //日落时间——分钟
	uint8_t reserve[14];               //预留
}day_data;

typedef struct A2_data{
    uint8_t receiveCnt;
    uint16_t dataLenth;
    union{
        uint8_t databuf[500];
        struct {
            uint8_t realtime_value_flag;            //实时天气参数有效标志
            struct {
			  	uint8_t weather_status;             //天气状况
				uint8_t temperature;               //温度
				uint8_t sendible_temperature;      //体感温度
				uint8_t humidity;                  //相对湿度
				uint8_t air_pressure_low;          //气压低位
				uint8_t air_pressure_high;         //气压高位
				uint8_t wind_speech;               //风力或风速
				uint8_t wind_direction;            //风向
				uint8_t rainfall_low;              //降雨量低位
				uint8_t rainfall_high;             //降雨量高位
                uint8_t reserve[10];               //预留
            }real_time;
			uint8_t daytime_value_flag;            //未来天参数有效标志
            day_data future_oneday;
            day_data future_twoday;
			uint8_t hourtime_value_flag;      //未来小时参数有效标志
            hour_data future_onehour;
            hour_data future_twohour;
            hour_data future_threehour;
            hour_data future_fourhour;
			hour_data future_fivehour;
			hour_data future_sixhour;
        };
    }valuedata;
}A2_data;



typedef struct
{
    char FilePath[200];
    uint8_t  update_downloading;
	uint8_t update_after_reconnect;//升级后是否重连标志
    uint8_t  try_time;
    uint16_t CheckSum;
    uint16_t recieve_CheckSum;//接收到升级包的校验码
    uint32_t PacketSize;
    uint32_t Rev_PacketSize;//接收到的升级包的大小
}TypeUpdate;

extern OS_EVENT *MSemTCP;


extern TcpType  g_TcpType;
extern TypeGPRS  GprsParam;
//extern uint8_t  g_CacheData[200];
extern uint8_t to_s_idle;
extern uint8_t isGetTime;
//extern uint8_t isOpenSample;

extern TypeUpdate UpdateParam;
extern uint16_t send_data_len;//新加wang

extern A2_data a2data_to_GMV;
//extern uint8_t sendbuff_flag;


extern uint8_t Getf3Time;
extern uint8_t Getf2Time;
extern uint8_t Getf2Heart;

void     softReset( void );
void     tcpDataProcess(TcpType *m_TcpType);

uint16_t handleTxFrame(TcpType *m_frame);
uint16_t handleTxFrame_Sem(TcpType *m_frame);

TcpType *handleRxFrame(TcpType *m_frame ,const uint8_t *m_dataarr , uint16_t m_datalenth);
uint16_t setGprsParam(uint8_t *m_Param, uint16_t m_ParamLen , TypeGPRS *m_GprsParam);
uint16_t getGprsParam(uint8_t *m_Param , uint16_t m_ParamLen ,uint8_t *putParam , TypeGPRS *m_GprsParam);
uint8_t  getServerTime(uint8_t curSta);
//uint8_t heart_beat(uint8_t curSta);
uint8_t heart_beat(uint8_t trans_mode,uint8_t curSta);




uint16_t send89Frame(TcpType *m_Frame);
uint16_t sendF3Frame(TcpType *m_Frame);
uint16_t sendF2Frame(TcpType *m_Frame);

uint16_t send91Frame(TcpType *m_Frame , uint8_t TranMode);
uint16_t send96Frame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen , uint8_t t_mode ,uint8_t m_mode);
uint16_t send9DFrame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen);
uint16_t send9FFrame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen);
uint16_t send9EFrame(TcpType *m_Frame , uint8_t *mac , send9equeue* ptr_9e);




uint16_t sendF4Frame(TcpType *m_Frame , uint16_t start_addr,uint8_t data_type,uint8_t *dat , uint16_t datLen ,uint16_t Times);
uint16_t sendF5Frame(TcpType *m_Frame , uint16_t start_addr,uint8_t data_type,uint8_t *dat , uint16_t datLen ,uint16_t Times,uint8_t can2ip);
uint16_t send97Frame(TcpType *m_Frame, uint8_t update_type, uint8_t result);
uint16_t send98Frame(TcpType *m_Frame , uint8_t *m_data, uint16_t datLen, uint8_t send_flag);
uint16_t send9AFrame(TcpType *m_Frame , uint8_t *m_data, uint16_t datLen, uint8_t send_flag, uint8_t datatype);
uint16_t send9BFrame(TcpType *m_Frame , uint8_t *m_data, uint16_t datLen, uint8_t send_flag, uint8_t datatype);


void sendEEFrame(TcpType *m_Frame , uint8_t ErrCode);
uint8_t setupdatParam(uint8_t *m_Param , uint16_t m_ParamLen, TypeUpdate *mUpdateParam,updateInfoType *mupdateinfo);
void getRandTime(struct tm *pTime);
uint8_t getupdate(uint8_t curSta); 
void sendreal_data(void);

void writeWeatherToCAN(A2_data *WeatherData);
uint8_t askWeatherData(uint8_t curSta);
uint16_t sendA2Frame(TcpType *m_Frame);
void check_packet_crc(updateInfoType *updateinfo1,TypeUpdate *UpdateParam1,uint8_t * buf_1024_size);

#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/






/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/




