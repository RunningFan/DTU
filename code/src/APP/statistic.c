/*****************************************************************************
Copyright: 2017 GREE ELECTRIC APPLIANCES,INC.
File name: data_statistic.c
Description: 用于统计机组的各类参数，本文件主要包含两个函数，GetData_can函数用于从CAN总线
上取出需要的数据，需要传入参数，传参类型为CanMsgType结构体，包含在handle_can.h头文件中。以
全局变量的方式传出参数，传出结构体stat_buffer，包含时间累积和次数累积。
Author:  walking
Version: V1.1
Date:    2018/1/3
History: 修改历史记录列表，每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/

/*---------------------------------头文件处------------------------------------*/
#include "common.h"
#include "drv_can.h"
#include "drv_timer.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "drv_flash.h"
#include "duplicate.h"
#include "statistic.h"
#include "fsm.h"
#include "drv_rtc.h"
#include  "bsp.h"

#define upload_time 3600
#define f4_first_seg	250
#define Min(a,b)   (a<b)?a:b
#define Max(a,b)   (a>b)?a:b
#define bit_type  1
#define byte_type 2




//INDOOR_MAX:表示系统工程中最多可以接的内机台数
//INDOOR_BASE:表示内机1-6的基点
buffertype stat_buffer;
uint8_t indoor_cnt = 0;
uint8_t program_version;
uint16_t machineID;
uint32_t TIM2_count ;
//err_type err_data;
uint8_t out_mac[7];
uint32_t begin_time,last_time;
uint8_t recieve_flag;
uint8_t F4_CONNECT; 
uint16_t F4_COUNT;
uint8_t is_lost_data_flag;
uint16_t F4_DTU_COUNT;
uint8_t sign_over_flag;
uint8_t sign_over_flag_month;
//uint8_t a_hour_flag = 0;
static uint16_t signal_low,signal_mid,signal_hig;
uint8_t ok_flag;
uint8_t frest_over;



uint16_t time_account(timetype * time) 
{
    if(time->bgTime == 0 )
    {
    	time->bgTime = TIM2_count;
    }

    time->ctTime = TIM2_count;
    return (time->ctTime - time->bgTime);
}

uint32_t time_account32(timetype *time)
{
	if(time->bgTime == 0)
	{
		time->bgTime = TIM2_count;
	}
	time->ctTime = TIM2_count;
	return (time->ctTime - time->bgTime);
}

uint8_t bit_handle(uint8_t addr,uint8_t row_num,uint8_t *data) 
{
    uint8_t result;
    uint8_t num1,num2,temp;
    num1 =  (addr - row_num)/8+1;
    num2 = (addr - row_num)%8;
    temp= 1<<num2;
    result = (*(data+num1)&temp)>>num2;
    return result;
}

uint8_t byte_handle(uint8_t addr,uint8_t row_num,uint8_t *data) 
{
    uint8_t result;
    uint8_t num1;
    num1 =  (addr - row_num);
    result = *(data+num1+1);
    return result;
}
uint32_t begin_time,last_time;

uint8_t recieve_flag;
uint8_t Get_data(uint8_t row_num,uint8_t type,CanMsgType g_Convert) 
{
    uint8_t out=0;
    if((g_Convert.Data[0] <= row_num )&&(type==bit_type)&&(row_num <= (g_Convert.Data[0] + ((g_Convert.Lenth-1)*8)-1))) {
        out=bit_handle(row_num,g_Convert.Data[0],g_Convert.Data);
        recieve_flag = 1;
    } else if((g_Convert.Data[0] <= row_num )&&(type==byte_type)&& (row_num <= (g_Convert.Data[0] + (g_Convert.Lenth-2)))) {
        out=byte_handle(row_num,g_Convert.Data[0],g_Convert.Data);
        recieve_flag = 1;
    }
    return out;
}
void recv_data(uint8_t * g_data,uint8_t t_data) 
{
    if(recieve_flag == 1) {
        * g_data = t_data;
        recieve_flag = 0;
    }
}
void recv_data2(int8_t * g_data,uint8_t  t_data) 
{
    if(recieve_flag == 1) {
        * g_data =t_data;
        recieve_flag = 0;
    }
}



uint8_t reset_flag,veil_flag,veil_flag_tmp,veil_flag_model,veil_flag_switch;
uint8_t reset_over_flag,reset_flag_exsit,veil_flag_lock,up_down_flag,left_right_flag,sleep_flag,go_out_flag,have_date;
uint8_t swap_fan_flag,muse_mode_flag,save_energy_flag,filt_net_flag,dry_state_flag;
uint8_t aid_hot_flag,super_wind_flag,timing_state_flag,child_lock_flag,cmp_state_flag,enter_flag,instant_flag;
uint16_t cold_ability = 0,hot_ability = 0,indoor_speed;
uint8_t oil_hot_flag,oil_cold_flag,oil_flag,frest_flag;
uint8_t sys_mode,cmp_mode,fan_mode,indoor_fan_mode,up_down_fan,left_right_fan;
uint8_t indoor_gwb_sample[6],indoor_amount,muse_mode,filt_net_state;
uint8_t mach_debug_times,sleep_state,static_press_level[6],indoor_tmp_unit[6];
uint32_t write_flash_times;
uint8_t indoor_speed_flag,outdoor_speed_flag;
uint8_t limit_freq,de_freq,weak_magnet;
uint8_t fan_limit_freq,fan_de_freq,fan_weak_magnet;
int8_t IPM_temp,PFC_temp,fan_ipm_temp,fan_pfc_temp;
uint8_t sys_machine,unit_debug_over,save_energy_state,super_wind;
uint8_t veil_save_energy_state,veil_tmp_state,veil_model_state,veil_switch_state;
uint8_t veil_lock_state,off_memeroy_way,swap_fan,dry_state,aid_hot_state,go_out_state;
uint8_t main_indoor,error_flag,indoor_static_press_flag[6];
uint8_t water_full[6],double_control,timing_state,child_lock_state,lamp_state;
uint32_t main_indoor_ip;
uint8_t accurancy_h,accurancy_l,finish_flag,water_full_flag[6];
uint32_t ten_minute_flag[6];
int8_t temp_inside[INDOOR_MAX],tmp_indoor[4],tmp_indoor_hot[4];
int8_t temp_aver,temp_frest_lowest,temp_diff,temp_outside;
uint32_t exe_sum_time,elec_sum;//用32位代表对应的32台内机开机状态
int8_t H_press,L_press,gas_split_temp;
uint8_t machine_frequence3,wind_frequence;
uint16_t bus_voltage,phrase_voltage,machine2_voltage[4],machine3_voltage,fan_bus_volt;
uint8_t pfc_am_int,cmp_am_int,fan_am_int,fan_am_dec;
float fan_exe_am;
uint8_t work_station[INDOOR_MAX];
uint16_t capability[INDOOR_MAX];
uint16_t out_capability[OUTDOOR_MAX];
uint8_t out_soft_version,indoor_soft_version[6],control_soft_version[6];
uint16_t sys_ability,mach_exe_time,mach_sum_time;
uint16_t indoor_off_on[6],rev_remote_sig[6],control_ID[6];
uint16_t set_tmp[4],set_tmp_hot[4];
uint16_t diff_high_low_press,machine_power,cmp_power,outdoor_power,outdoor_power_pri;
uint8_t outdoor_power_change_flag;
uint16_t abs_high_press,abs_low_press,machineID;
int8_t H_module,L_module,xqguoredu,paiqi1;
uint8_t mach_finish_way,guoredu,warm_band1,set_fan[4],set_fan_hot[4];


/*
*	统计开机时，室内机温度平均值
*/
int8_t aver_indoor_temp(void)
{
	uint8_t count=0;
	int32_t temp=0;
	for(uint8_t i=0;i<INDOOR_MAX;i++){
		if(work_station[i]){
			count++;
			temp += temp_inside[i]; 
		}
	}
	return (int8_t)(temp/count);
}

uint16_t   GetData_can(CanMsgType    m_Convert) 
{
    uint16_t com_data16,m_indoor_num = 0,deviation = 0;
	uint32_t com_data32 = 0;
    uint8_t com_data,can2ip,i;
     int8_t icom_data=0;
	static int8_t temp_lower;
	uint8_t n_n = 0;
    if((m_Convert.FunCode == 0xf7) || (m_Convert.FunCode == 0xE5) || (m_Convert.FunCode == 0xE7)) 
	{
          
        if(m_Convert.CAN2IP==0x7F){
            can2ip = 0;
        }else{
            can2ip = m_Convert.CAN2IP;
        }
        if((m_Convert.CAN1IP>=INDOOR_BASE)&&(m_Convert.CAN1IP<(INDOOR_BASE + INDOOR_MCH_NUM))){
          m_indoor_num = findindoornum(&m_Convert);
            if(m_indoor_num & 0x8000){//未查找到该内机
                if(indoor_cnt<INDOOR_MCH_NUM){ //只查找16台内机数量  
                   indoorinfo[indoor_cnt].Can1Can2IP = (can2ip<<8)|m_Convert.CAN1IP;
                   m_indoor_num = indoor_cnt;
                   indoor_cnt ++;  
				}
            }
        }
		
        switch(m_Convert.DataType)
		{
        case 0x10:

            if((m_Convert.CAN1IP-OUTDOOR_BASE_IP)<OUTDOOR_MAX){
                com_data16 = (uint16_t)Get_data(5,byte_type,m_Convert)<<8|Get_data(4,byte_type,m_Convert);//机型ID
                if(recieve_flag) {
					machineID = com_data16;//机型ID高低位
                    outdoorinfo.online_flag |= 0x02; //在线,,标志
                    outdoorinfo.Can1Can2IP = (can2ip<<8)|m_Convert.CAN1IP;
                    recieve_flag = 0;
                }
				com_data = Get_data(7,byte_type,m_Convert);
				if(recieve_flag){
	                for(i=0; i<6; i++){
	                    com_data = Get_data(i+7,byte_type,m_Convert);//外机mac
	                        out_mac[i] = com_data;
	            	}
                     memcpy(outdoorinfo.MAC,&m_Convert.Data[1],6);//满足主控一次6位全部发送
                     outdoorinfo.online_flag |= 0x01;  //获得MAC代表在线,,标志
                     outdoorinfo.Can1Can2IP = (can2ip<<8)|m_Convert.CAN1IP;
					 recieve_flag = 0;
				}
            }else if((m_Convert.CAN1IP>=INDOOR_BASE)&&(m_Convert.CAN1IP<(INDOOR_BASE+INDOOR_MCH_NUM))){//内机设备信息
                com_data16 = (uint16_t)Get_data(17,byte_type,m_Convert)<<8|Get_data(16,byte_type,m_Convert); //内机工程编号
                if(recieve_flag){
                   indoorinfo[m_indoor_num].number = com_data16;
                           //获得工程编号时，同时赋值CANIP
                   recieve_flag = 0;
                }
					com_data = Get_data(7,byte_type,m_Convert);
					if(recieve_flag){
	                    memcpy(indoorinfo[m_indoor_num].MAC,&m_Convert.Data[1],6);
	                    indoorinfo[m_indoor_num].online_flag |= 0x01;  //获得MAC代表在线
	                    recieve_flag = 0;
					}
            }

            if((work_station[m_Convert.CAN1IP-INDOOR_BASE] == 1) && (m_Convert.CAN1IP >= INDOOR_BASE) && \
															(m_Convert.CAN1IP < INDOOR_MAX+INDOOR_BASE))
			{ //内机的开关机状态
                com_data16 = ((uint16_t)Get_data(15,byte_type,m_Convert) << 8) | \
												Get_data(14,byte_type,m_Convert);
				//内机的额定容量
                if(recieve_flag)
				{
                    capability[m_Convert.CAN1IP-INDOOR_BASE] = com_data16;
                    recieve_flag = 0;
                }

            }else if((work_station[m_Convert.CAN1IP-INDOOR_BASE] == 0) && (m_Convert.CAN1IP >= INDOOR_BASE) && \
            													(m_Convert.CAN1IP < INDOOR_MAX+INDOOR_BASE))
            {
                	capability[m_Convert.CAN1IP-INDOOR_BASE] = 0;
			}

            if((m_Convert.CAN1IP-OUTDOOR_BASE_IP)<OUTDOOR_MAX)
			{ 
				//外机的额定容量计算
                com_data16 = ((uint16_t)Get_data(15,byte_type,m_Convert) << 8) | \
													Get_data(14,byte_type,m_Convert);
                if(recieve_flag)
				{
                    out_capability[m_Convert.CAN1IP-OUTDOOR_BASE_IP] =com_data16;
                    recieve_flag = 0;
                }
            }			

			break;

        case 0x11:
            if((m_Convert.CAN1IP-OUTDOOR_BASE_IP)<OUTDOOR_MAX) { 
                com_data = Get_data(8,byte_type,m_Convert);
                if(recieve_flag) {
                    program_version = com_data;
                    recieve_flag = 0;
                }
            }
//            if((work_station[m_Convert.CAN1IP-INDOOR_BASE] == 1) && (m_Convert.CAN1IP >= INDOOR_BASE) && \
//															(m_Convert.CAN1IP < INDOOR_MAX+INDOOR_BASE))
//			{ //内机的开关机状态
//                com_data16 = ((uint16_t)Get_data(3,byte_type,m_Convert) << 8) | \
//												Get_data(2,byte_type,m_Convert);
//				//内机的额定容量
//                if(recieve_flag)
//				{
//                    capability[m_Convert.CAN1IP-INDOOR_BASE] = com_data16;
//                    recieve_flag = 0;
//                }
//
//            }else if((work_station[m_Convert.CAN1IP-INDOOR_BASE] == 0) && (m_Convert.CAN1IP >= INDOOR_BASE) && \
//            													(m_Convert.CAN1IP <= INDOOR_MAX+INDOOR_BASE))
//            {
//                	capability[m_Convert.CAN1IP-INDOOR_BASE] = 0;
//			}
//
//            if((m_Convert.CAN1IP-OUTDOOR_BASE_IP)<OUTDOOR_MAX)
//			{ 
//				//外机的额定容量计算
//                com_data16 = ((uint16_t)Get_data(3,byte_type,m_Convert) << 8) | \
//													Get_data(2,byte_type,m_Convert);
//                if(recieve_flag)
//				{
//                    out_capability[m_Convert.CAN1IP-OUTDOOR_BASE_IP] =com_data16;
//                    recieve_flag = 0;
//                }
//            }
			
            break;
		case 0x14:
			if(m_Convert.CAN1IP-8 < OUTDOOR_MAX){
				com_data16 = (uint16_t)(Get_data(5,byte_type,m_Convert)<<8 | Get_data(4,byte_type,m_Convert));
				if(recieve_flag){
					outdoor_power = com_data16;
					if(outdoor_power > outdoor_power_pri){
						deviation = outdoor_power - outdoor_power_pri;
					}else{
						deviation = outdoor_power_pri - outdoor_power;
					}
					if(deviation >= GprsParam.outdoor_power_accuracy){
						outdoor_power_change_flag = 1;
						outdoor_power_pri = outdoor_power;
					}
					recieve_flag = 0;
				}
			}
			
			if(m_Convert.CAN1IP-8 < OUTDOOR_MAX){
				com_data32 = (uint32_t)(Get_data(19,byte_type,m_Convert) << 24 | Get_data(18,byte_type,m_Convert) << 16 | \
						Get_data(17,byte_type,m_Convert) << 8 | Get_data(16,byte_type,m_Convert));
				if(recieve_flag){
					elec_sum = com_data32;
					recieve_flag = 0;
				}
//				else{
//					elec_sum = 0;
//				}
				
				com_data16 = (uint16_t)Get_data(21,byte_type,m_Convert) << 8 | Get_data(20,byte_type,m_Convert);
				if(recieve_flag)
				{
					machine_power = com_data16;
					recieve_flag = 0;
				}
			}
			break;

		case 0x1c:
			break;
			
        case 0x1d:
            break;
			
        case 0x1e:
			if(m_Convert.CAN1IP == main_indoor_ip){
				com_data = Get_data(0,bit_type,m_Convert);//室内机故障标志位
				recv_data(&error_flag,com_data);
			}


			if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 6))
			{
				com_data = Get_data(3,bit_type,m_Convert);//室内风机故障标志位
				recv_data(&indoor_static_press_flag[m_Convert.CAN1IP-INDOOR_BASE],com_data);

				com_data = Get_data(5,bit_type,m_Convert);//水满保护标志
				recv_data(&water_full[m_Convert.CAN1IP-INDOOR_BASE],com_data);
			}

			if((m_Convert.CAN1IP - INDOOR_BASE >= 0) && (m_Convert.CAN1IP - INDOOR_BASE < INDOOR_MAX)){
		            com_data=Get_data(3,bit_type,m_Convert);//内风机保护标志
		             if(recieve_flag){
		                  ACErrParam.indoor_wind_protect_err[m_indoor_num] = com_data;
		                  recieve_flag = 0;
#if DEBUG
						if(com_data){
							if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
							for(n_n=1;n_n<m_Convert.Lenth;n_n++){
								if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
							}
							if(dbgPrintf)(*dbgPrintf)("\n");
						}
#endif						 
		            }
		            com_data=Get_data(5,bit_type,m_Convert);//水满保护
		            if(recieve_flag){
		                ACErrParam.water_full_protect_err[m_indoor_num]=com_data;
						recieve_flag = 0;
#if DEBUG
					if(com_data){
						if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
						for(n_n=1;n_n<m_Convert.Lenth;n_n++){
							if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
						}
						if(dbgPrintf)(*dbgPrintf)("\n");
					}
#endif	


		            }

					 com_data = Get_data(6,bit_type,m_Convert);//线控器供电异常
					 if(recieve_flag){
					 	ACErrParam.line_control_problem[m_indoor_num] = com_data;
						recieve_flag = 0;
#if DEBUG
						 if(com_data){
							 if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
							 for(n_n=1;n_n<m_Convert.Lenth;n_n++){
								 if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
							 }
							 if(dbgPrintf)(*dbgPrintf)("\n");
						 }
#endif	

					 
					 }			
					
		            com_data=Get_data(7,bit_type,m_Convert);//防冻结保护
		             if(recieve_flag){
		               ACErrParam.freeze_protect_err[m_indoor_num]=com_data;          
					   recieve_flag = 0;
#if DEBUG
					 if(com_data){
						 if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
						 for(n_n=1;n_n<m_Convert.Lenth;n_n++){
							 if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
						 }
						 if(dbgPrintf)(*dbgPrintf)("\n");
					 }
#endif	

		 
		              }

					 com_data = Get_data(24,bit_type,m_Convert);//一控多机内机台数不一致
					 if(recieve_flag){
					 	ACErrParam.one_control_multi_machine[m_indoor_num] = com_data;
						if(com_data > 1){
							recieve_flag = 1;							
						}
						recieve_flag = 0;
#if DEBUG
					 if(com_data){
						 if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
						 for(n_n=1;n_n<m_Convert.Lenth;n_n++){
							 if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
						 }
						 if(dbgPrintf)(*dbgPrintf)("\n");
					 }
#endif	

 
					 }

					 com_data = Get_data(48,bit_type,m_Convert);//水流开关故障
					 if(recieve_flag){
					 	ACErrParam.water_switch_problem[m_Convert.CAN1IP-INDOOR_BASE] = com_data;
						recieve_flag = 0;
#if DEBUG
					 if(com_data){
						 if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
						 for(n_n=1;n_n<m_Convert.Lenth;n_n++){
							 if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
						 }
						 if(dbgPrintf)(*dbgPrintf)("\n");
					 }
#endif	

					 }

					 com_data = Get_data(49,bit_type,m_Convert);//分水阀设置故障
					 if(recieve_flag){
					 	ACErrParam.divide_water_set_problem[m_indoor_num] = com_data;
						recieve_flag = 0;
#if DEBUG
					 if(com_data){
						 if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
						 for(n_n=1;n_n<m_Convert.Lenth;n_n++){
							 if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
						 }
						 if(dbgPrintf)(*dbgPrintf)("\n");
					 }
#endif	

					 }

					 com_data = Get_data(50,bit_type,m_Convert);//扫风部件故障
					 if(recieve_flag){
					 	ACErrParam.swift_wind_error[m_indoor_num] = com_data;
						recieve_flag = 0;
#if DEBUG
					 if(com_data){
						 if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Convert.Lenth,m_Convert.FunCode,m_Convert.CAN2IP,m_Convert.CAN1IP,m_Convert.DataType,m_Convert.Data[0]);
						 for(n_n=1;n_n<m_Convert.Lenth;n_n++){
							 if(dbgPrintf)(*dbgPrintf)("%02X ",m_Convert.Data[n_n]);
						 }
						 if(dbgPrintf)(*dbgPrintf)("\n");
					 }
#endif	

					 
					 }
			}
		
            break;

        case 0x1f:

//			if((m_Convert.CAN1IP - INDOOR_BASE >= 0) &&(m_Convert.CAN1IP - INDOOR_BASE <= INDOOR_MAX)){
//				com_data = Get_data(90,bit_type,m_Convert);
//				if(recieve_flag){
//					ACErrParam.voltage_supply_short[m_indoor_num] = com_data;//电源供电不足
//					recieve_flag = 0;
//				}
//			}
			
		
			if((m_Convert.CAN1IP >= INDOOR_BASE) && (m_Convert.CAN1IP < INDOOR_MAX+ INDOOR_BASE))
			{
				com_data = Get_data(0,bit_type,m_Convert);//内机开关机状态
				recv_data(&work_station[m_Convert.CAN1IP-INDOOR_BASE],com_data);
			}
			
//			if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 6))
//			{
//				com_data = Get_data(0,bit_type,m_Convert);//内机开关机状态
//				recv_data(&indoor_open_state[m_Convert.CAN1IP-INDOOR_BASE],com_data);
//			}
			
			com_data = Get_data(1,bit_type,m_Convert);//取主内机状态
			recv_data(&main_indoor,com_data);
/*
*	取主内机IP地址
*/
			if(main_indoor)
			{
				main_indoor_ip = m_Convert.CAN1IP;
				main_indoor = 0;
			}

			if(m_Convert.CAN1IP == main_indoor_ip)
			{

				com_data = Get_data(5,bit_type,m_Convert);//超强风状态
				recv_data(&super_wind,com_data);

				com_data = Get_data(6,bit_type,m_Convert);//节能功能状态
				recv_data(&save_energy_state,com_data);

				com_data = Get_data(7,bit_type,m_Convert);//屏蔽节能状态
				recv_data(&veil_save_energy_state,com_data);

				com_data = Get_data(8,bit_type,m_Convert);//屏蔽温度状态
				recv_data(&veil_tmp_state,com_data);

				com_data = Get_data(9,bit_type,m_Convert);//屏蔽模式状态
				recv_data(&veil_model_state,com_data);

				com_data = Get_data(10,bit_type,m_Convert);//屏蔽开关状态位
				recv_data(&veil_switch_state,com_data);

				com_data = Get_data(11,bit_type,m_Convert);//屏蔽锁定功能状态
				recv_data(&veil_lock_state,com_data);

				com_data = Get_data(12,bit_type,m_Convert);//辅助电加热标志
				recv_data(&aid_hot_state,com_data);

				com_data = Get_data(27,bit_type,m_Convert);//外出功能使用状态
				recv_data(&go_out_state,com_data);

				com_data = Get_data(30,bit_type,m_Convert);//干燥功能使用状态
				recv_data(&dry_state,com_data);

				com_data = Get_data(32,bit_type,m_Convert);//换气功能使用次数
				recv_data(&swap_fan,com_data);
			
				com_data = Get_data(59,bit_type,m_Convert);//定时功能使用次数
				recv_data(&timing_state,com_data);

				com_data = Get_data(60,bit_type,m_Convert);//童锁状态
				recv_data(&child_lock_state,com_data);
			}

			
            com_data  = Get_data(81,bit_type,m_Convert);//系统压缩机的运行情况
            recv_data(&sys_machine,com_data);

//			com_data = Get_data(89,bit_type,m_Convert);//机组调试完成标志位
//			recv_data(&unit_debug_over,com_data);
            
            com_data  = Get_data(92,bit_type,m_Convert);//回油标志位
            recv_data(&oil_flag,com_data);

            com_data  = Get_data(93,bit_type,m_Convert);//化霜标志位
            recv_data(&frest_flag,com_data);

            if((m_Convert.CAN1IP-8) < OUTDOOR_MAX)
			{
                com_data = Get_data(119,bit_type,m_Convert) ;//压缩机1电加热带
                recv_data(&warm_band1,com_data);
            }
/*
*	复位完成标志
*/
				com_data = Get_data(136,bit_type,m_Convert);//复位完成标志
				recv_data(&reset_flag,com_data);

/*
*	限磁、弱磁、降频状态获取
*/

		   if(((m_Convert.CAN1IP-8) < OUTDOOR_MAX) && (cmp_mode == 6))
		   {
		   		com_data = Get_data(154,bit_type,m_Convert);
				recv_data(&limit_freq,com_data);
			
				com_data = Get_data(155,bit_type,m_Convert);
				recv_data(&de_freq,com_data);
				
				com_data = Get_data(153,bit_type,m_Convert);
				recv_data(&weak_magnet,com_data);
		   }

		   if(fan_mode == 6)
		   {
		   		com_data = Get_data(169,bit_type,m_Convert);
				recv_data(&fan_limit_freq,com_data);
				
				com_data = Get_data(170,bit_type,m_Convert);
				recv_data(&fan_de_freq,com_data);
				
				com_data = Get_data(171,bit_type,m_Convert);
				recv_data(&fan_weak_magnet,com_data);
				
		   }
		   
            break;
        case 0x20:
			if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 4) && \
												((sys_mode == 2) || (sys_mode == 3)))
			{
				icom_data = Get_data(0,byte_type,m_Convert) - 100;//内机1-内机4室内环境温度
				recv_data2(&tmp_indoor[m_Convert.CAN1IP-INDOOR_BASE],icom_data);
			}else if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 4) && \
													(sys_mode == 5))
			{
				icom_data = Get_data(0,byte_type,m_Convert) - 100;
				recv_data2(&tmp_indoor_hot[m_Convert.CAN1IP-INDOOR_BASE],icom_data);
			}
/*
*	采集各运行内机室内环境温度的方法：
*	frest_flag判断采集时间点处于化霜前
*	建立一个INDOOR_MAX大小数组：当每采集到一个内机的温度值时，将之存在对于的CAN1IP-32
*	的位置，因此，可以采集INDOOR_MAX个不同的外机某个时刻的温度。
*	化霜过程中最低点采集方法：
*	假设第一次采集到的温度为最低点，则每次采集到的数据与上一次比较如更小则替换。
*	
*/


		if(((m_Convert.CAN1IP-INDOOR_BASE) >= 0) && (m_Convert.CAN1IP-INDOOR_BASE) < INDOOR_MAX){
				icom_data = Get_data(0,byte_type,m_Convert) - 100;//化霜前,各个室内机环境温度
				recv_data2(&temp_inside[m_Convert.CAN1IP-INDOOR_BASE],icom_data);
		
//				if(!frest_flag && !frest_over){
//					uint16_t temp_ip = m_Convert.CAN1IP;
//					for(uint8_t i=0;i<INDOOR_MCH_NUM;i++){
//						if(indoorinfo[i].Can1Can2IP == temp_ip){
//							indoor_state |= indoorinfo[i].online_flag << (m_Convert.CAN1IP - INDOOR_BASE);
//						}
//					}
//					
////					icom_data = Get_data(0,byte_type,m_Convert) - 100;//化霜前,各个室内机环境温度
////					recv_data2(&temp_inside[m_Convert.CAN1IP-INDOOR_BASE],icom_data);
//				}
				
				if(frest_flag)
				{
					icom_data = Get_data(0,byte_type,m_Convert) - 100;//化霜时，室内环境温度最低值
					recv_data2(&temp_lower,icom_data);
					if(0 == temp_frest_lowest){
						temp_frest_lowest = temp_lower;
					}
					if(temp_lower < temp_frest_lowest){
						temp_frest_lowest = temp_lower;
					}
//					frest_over = 1;
				}
		}
			
            icom_data= Get_data(80,byte_type,m_Convert)-100;//室外环境温度
            recv_data2(&temp_outside,icom_data);
			
            icom_data  =  Get_data(81,byte_type,m_Convert) - 100;//系统高压值
            recv_data2(&H_press,icom_data);
			
            icom_data  =  Get_data(82,byte_type,m_Convert) - 100;//系统低压值
            recv_data2(&L_press,icom_data);

            icom_data = Get_data(87,byte_type,m_Convert) - 100; //压缩机1排气温度
            recv_data2(&paiqi1,icom_data);

			icom_data = Get_data(98,byte_type,m_Convert) - 100;
			recv_data2(&gas_split_temp,icom_data);//气液分离器出管温度
			
            icom_data = Get_data(105,byte_type,m_Convert) - 100;//模块高压值
            recv_data2(&H_module,icom_data);
			
            icom_data  = Get_data(106,byte_type,m_Convert) - 100;//模块低压值
            recv_data2(&L_module,icom_data);

			if((cmp_mode == 6) && ((m_Convert.CAN1IP-8) < OUTDOOR_MAX))
			{
				icom_data = Get_data(115,byte_type,m_Convert)-100;//IPM模块温度
				recv_data2(&IPM_temp,icom_data);
				icom_data = Get_data(116,byte_type,m_Convert)-100;//PFC模块温度
				recv_data2(&PFC_temp,icom_data);
			}

			if(fan_mode == 6)
			{
				icom_data = Get_data(125,byte_type,m_Convert) - 100;//风机PFC模块温度
				recv_data2(&fan_pfc_temp,icom_data);
				
				icom_data = Get_data(124,byte_type,m_Convert) - 100;//风机IPM模块温度
				recv_data2(&fan_ipm_temp,icom_data);
			}

            break;

        case 0x21:
			if(m_Convert.CAN1IP == main_indoor_ip)
			{
				com_data = Get_data(0,byte_type,m_Convert);//内机运行模式
				recv_data(&indoor_fan_mode,com_data);
			
				com_data = Get_data(2,byte_type,m_Convert);//上下扫风状态
				recv_data(&up_down_fan,com_data);

				com_data = Get_data(3,byte_type,m_Convert);//左右扫风状态
				recv_data(&left_right_fan,com_data);

				com_data = Get_data(11,byte_type,m_Convert);//过滤网通知状态
				recv_data(&filt_net_state,com_data);

				com_data = Get_data(13,byte_type,m_Convert);//静音功能使用次数
				recv_data(&muse_mode,com_data);

				com_data = Get_data(15,byte_type,m_Convert);//睡眠功能状态
				recv_data(&sleep_state,com_data);

			}


			if((m_Convert.CAN1IP - INDOOR_BASE) < 6 && (m_Convert.CAN1IP - INDOOR_BASE) >= 0){
				com_data = Get_data(16,byte_type,m_Convert);//内机静压等级
				recv_data(&static_press_level[m_Convert.CAN1IP-INDOOR_BASE],com_data);
			}

			com_data = Get_data(46,byte_type,m_Convert);//一控多机内机台数
			recv_data(&indoor_amount,com_data);
			
//整机运行模式：1：关机模式；2：制冷模式；3：除湿模式；4：送风模式；5：制热模式；
//				6：主体制冷模式；7：主体制热模式；8：热回收模式；
//
//整机运行模式：			----sys_mode	(系统的工作模式)字节数据
//系统压缩机运行状态:    	---sys_machine 位数据(运行标志)
//变频压缩机运行状态:---cmp_mode	(压缩机程序运行状态)
//
            com_data = Get_data(81,byte_type,m_Convert);//整机运行模式
            recv_data(&sys_mode,com_data);
			
			com_data = Get_data(105,byte_type,m_Convert);//变频压缩机程序运行状态
			recv_data(&cmp_mode,com_data);
			
			com_data = Get_data(109,byte_type,m_Convert);//变频风机程序运行状态
			recv_data(&fan_mode,com_data);
			
            break;
		case 0x27:
			break;
			
        case 0x28:
            
            com_data = Get_data(149,byte_type,m_Convert);//变频外风机1的频率
            recv_data(&wind_frequence,com_data);

            if(((m_Convert.CAN1IP-OUTDOOR_BASE_IP) < OUTDOOR_MAX) && (cmp_mode == 6)) 
			{ 
                com_data16  =  ((uint16_t)Get_data(131,byte_type,m_Convert)<<8)|Get_data(130,byte_type,m_Convert);//压缩机母线电压
                if(recieve_flag)
				{
                    bus_voltage = com_data16;
                    recieve_flag = 0;
                }
    
				com_data16 = ((uint16_t)Get_data(136,byte_type,m_Convert))<<1;
				if(recieve_flag)
				{
					phrase_voltage = com_data16;
					recieve_flag = 0;
				}

				com_data = Get_data(133,byte_type,m_Convert);
				recv_data(&pfc_am_int,com_data);//变频压缩机输入电源电流或者PFC输入电流

				com_data = Get_data(129,byte_type,m_Convert);//变频压缩机电流
				recv_data(&cmp_am_int,com_data);
			}
			
			if(fan_mode == 6)
			{
				com_data = Get_data(141,byte_type,m_Convert);
				recv_data(&fan_am_int,com_data);
				
				com_data = Get_data(140,byte_type,m_Convert);
				recv_data(&fan_am_dec,com_data);
				
				com_data16 = ((uint16_t)Get_data(143,byte_type,m_Convert) << 8 | \
										Get_data(142,byte_type,m_Convert));
				if(recieve_flag)
				{
					fan_bus_volt = com_data16;
					recieve_flag = 0;
				}
			}
            break;
			
        case 0x31:
/*
*	内风机（主）转速
*/
			if((m_Convert.CAN1IP == main_indoor_ip) && (indoor_fan_mode > 0) && \
														(indoor_fan_mode < 13))
			{
				com_data16 = (uint16_t)Get_data(7,byte_type,m_Convert)<<8 | \
										Get_data(6,byte_type,m_Convert);
				if(recieve_flag)
				{
					indoor_speed = com_data16;
					recieve_flag = 0;
				}
			}
/*
*	本机绝对高压低压值
*/
			if((m_Convert.CAN1IP - OUTDOOR_BASE_IP) < OUTDOOR_MAX)
			{
				if(cmp_mode != 6){
					cmp_state_flag = 1;
				}else if((cmp_mode == 6) && (cmp_state_flag == 1)){
					com_data16 = (uint16_t)Get_data(146,byte_type,m_Convert)<<8 | \
											Get_data(145,byte_type,m_Convert);
					if(recieve_flag)
					{
						abs_high_press = com_data16;//绝对高压值
						recieve_flag = 0;
						enter_flag = 1;
					}
					
					com_data16 = (uint16_t)Get_data(148,byte_type,m_Convert)<<8 | \
											Get_data(147,byte_type,m_Convert);
					if(recieve_flag)
					{
						abs_low_press = com_data16;//绝对低压值
						recieve_flag = 0;
					}
					if(enter_flag == 1){
						have_date = 1;
						cmp_state_flag = 0;
						enter_flag = 0;
					}
				}

				com_data16 = (uint16_t)Get_data(169,byte_type,m_Convert) << 8 | \
									Get_data(168,byte_type,m_Convert);
				if(recieve_flag)
				{
					cmp_power = com_data16;
					recieve_flag = 0;
				}else{
					cmp_power = 0;
				}
			}

	            com_data16 = (uint16_t )Get_data(85,byte_type,m_Convert) <<8 | \
            							Get_data(84,byte_type,m_Convert);//整机运行能力
	            if(recieve_flag)
				{
	                sys_ability = com_data16;//整机运行能力
	                recieve_flag = 0;
	            }
			
            break;
	case 0x32:
		break;

	case 0x71:
		if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 4) && \
					((sys_mode == 2) || (sys_mode == 3)))
		{
			com_data = Get_data(1,byte_type,m_Convert);
			recv_data(&set_fan[m_Convert.CAN1IP-INDOOR_BASE],com_data);
		}else if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 4) && \
					(sys_mode == 5))
		{
			com_data = Get_data(1,byte_type,m_Convert);
			recv_data(&set_fan_hot[m_Convert.CAN1IP-INDOOR_BASE],com_data);
		}
		break;
	
	case 0x73:
			if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 4) && \
					((sys_mode == 2) || (sys_mode == 3)))
			{
				com_data16 = (uint16_t)Get_data(1,byte_type,m_Convert) << 8 | \
										Get_data(0,byte_type,m_Convert);//开机设定温度
				if(recieve_flag)
				{
					set_tmp[m_Convert.CAN1IP-INDOOR_BASE] = com_data16/10;
					recieve_flag = 0;
				}
			}else if((m_Convert.CAN1IP-INDOOR_BASE >= 0) && (m_Convert.CAN1IP-INDOOR_BASE < 4) && \
					(sys_mode == 5))
			{
				com_data16 = (uint16_t)Get_data(1,byte_type,m_Convert) << 8 | \
										Get_data(0,byte_type,m_Convert);//制热状态下，开机设定温度
				if(recieve_flag)
				{
					set_tmp_hot[m_Convert.CAN1IP-INDOOR_BASE] = com_data16/10;
					recieve_flag = 0;
				}
			}
			break;
	    }
	}
	return m_indoor_num;
}

typedef struct _timer_tag
{
	timetype timer;
	uint8_t	tag;
}timer_tag;

timer_tag timer;

timetype timerbf5,timerbf6,timerbf7,timerbf8[6],timer10;
uint8_t oldflag_frest;
uint32_t start,time_s;
uint16_t capa_percent;
uint8_t lim_frq_flag,de_frq_flag,weak_mag_flag;
uint8_t fan_lim_frq_flag,fan_de_frq_flag,fan_weak_mag_flag;

void Reset_Timer(uint8_t timer_n)
{
	memset(&timer.timer,0,sizeof(timer.timer));
	timer.tag=timer_n;
}

void Statistic_CanData()
{
    uint8_t i = 0;
    uint16_t indoor_capability = 0;
    uint16_t outdoor_capability = 0;
	static uint8_t flag = 0;
/*
*	整机得电时间,此处GPRS模块在运行，即能表明机组得电。
*/
//    f4_time++;

//	check_f4time();

//	if(!(f4_time % HOURS_TIME)){
////		a_hour_flag = 1;
//		g_Upload.flag_hours = 1;
//	}else{
////		a_hour_flag = 0;
//		g_Upload.flag_hours = 1;
//		
//	}
//
//	if(!(f4_time % (stat_buffer.config_time*60))){
//		g_Upload.flag_98 = 1;
//	}
//
//	if(!(f4_time % GprsParam.big_data_sampe_time)){
//		g_Upload.flag_sample = 0;
//	}
//
//	if(!(f4_time % TAKE_PHOTO_TIME)){
//		g_Upload.flag_5 = 1;
//	}
//
//	if(!(f4_time % TAKE_PHOTO_DEFAULT_TIME)){
//		g_Upload.flag_30 = 1;
//	}
	
//    stat_buffer.time_buffer[GPRS_GETELEC_TIME]++;

/*
*	机组待机时间统计，机组压缩机关机，即表示整机处于待机状态。
*/

	if(sys_machine == 0)
	{
		stat_buffer.time_buffer[MACH_WAIT_TIME]++;
	}
	
/*
*	室外环境温度累计
*/

	switch (sys_mode){
			case 2:
			case 3:
		        if(temp_outside <= 10){
					if(timer.tag != 0) Reset_Timer(0);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[COLD_L10_TIME1]++;
		        }else if((temp_outside > 10) && (temp_outside <= 13)){
					if(timer.tag != 1 ) Reset_Timer(1);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[COLD_M10_13_TIME2]++;
		        }else if((temp_outside > 13) && (temp_outside <= 16)){
					if(timer.tag != 2) Reset_Timer(2);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[COLD_M13_16_TIME3]++;
		        }else if((temp_outside > 16) && (temp_outside <= 19)){
					if(timer.tag != 3) Reset_Timer(3);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M16_19_TIME4]++;
		        }else if((temp_outside > 19) && (temp_outside <= 22)){
					if(timer.tag != 4) Reset_Timer(4);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M19_22_TIME5]++;
				}else if((temp_outside > 22) && (temp_outside <= 25)){
					if(timer.tag != 5) Reset_Timer(5);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M22_25_TIME6]++;
				}else if((temp_outside > 25) && (temp_outside <= 28)){
					if(timer.tag != 6) Reset_Timer(6);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M25_28_TIME7]++;
				}else if((temp_outside > 28) && (temp_outside <= 31)){
					if(timer.tag != 7) Reset_Timer(7);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M28_31_TIME8]++;
				}else if((temp_outside > 31) && (temp_outside <= 34)){
					if(timer.tag != 8) Reset_Timer(8);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M31_34_TIME9]++;
				}else if((temp_outside > 34) && (temp_outside <= 37)){
					if(timer.tag != 9) Reset_Timer(9);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M34_37_TIME10]++;
				}else if((temp_outside > 37) && (temp_outside <= 40)){
					if(timer.tag != 10) Reset_Timer(10);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M37_40_TIME11]++;
				}else if((temp_outside > 40) && (temp_outside <= 43)){
					if(timer.tag != 11) Reset_Timer(11);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M40_43_TIME12]++;
				}else if((temp_outside>43) && (temp_outside <= 45)){
					if(timer.tag != 12) Reset_Timer(12);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_M43_45_TIME13]++;
				}else if((temp_outside>45)){
					if(timer.tag != 13) Reset_Timer(13);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[COLD_G45_TIME14]++;
				}	
				break;
				
			case 5:
				if(temp_outside <= -15){
					if(timer.tag != 14) Reset_Timer(14);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[HOT_LN15_TIME1]++;
		        }else if((temp_outside > -15) && (temp_outside <= -12)){
					if(timer.tag != 15) Reset_Timer(15);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[HOT_MN15_N12_TIME2]++;
		        }else if((temp_outside > -12) && (temp_outside <= -9)){
					if(timer.tag != 16) Reset_Timer(16);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[HOT_MN12_N9_TIME3]++;
		        }else if((temp_outside > -9) && (temp_outside <= -6)){
					if(timer.tag != 17) Reset_Timer(17);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[HOT_MN9_N6_TIME4]++;
		        }else if((temp_outside > -6) && (temp_outside <= -3)){
					if(timer.tag != 18) Reset_Timer(18);
		            if((time_account(&timer.timer))>=180)  stat_buffer.time_buffer[HOT_MN6_N3_TIME5]++;
		        }else if((temp_outside > -3) && (temp_outside <= 0)){
					if(timer.tag != 19) Reset_Timer(19);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[HOT_MN3_0_TIME6]++;
		        }else if((temp_outside > 0) && (temp_outside <= 3)){
					if(timer.tag != 20) Reset_Timer(20);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[HOT_M0_3_TIME7]++;
		        }else if((temp_outside > 3) && (temp_outside <= 6)){
					if(timer.tag != 21) Reset_Timer(21);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[HOT_M3_6_TIME8]++;
		        }else if((temp_outside > 6) && (temp_outside <= 9)){
					if(timer.tag != 22) Reset_Timer(22);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[HOT_M6_9_TIME9]++;
		        }else if((temp_outside > 9) && (temp_outside <= 12)){
					if(timer.tag != 23) Reset_Timer(23);
					if((time_account(&timer.timer))>=180) stat_buffer.time_buffer[HOT_M9_12_TIME10]++;
		        }else if((temp_outside > 12) && (temp_outside <= 15)){
					if(timer.tag != 24) Reset_Timer(24);
					if((time_account(&timer.timer))>180) stat_buffer.time_buffer[HOT_M12_15_TIME11]++;
		        }else if((temp_outside > 15) && (temp_outside <= 18)){
					if(timer.tag != 25) Reset_Timer(25);
					if((time_account(&timer.timer))>180) stat_buffer.time_buffer[HOT_M15_18_TIME12]++;
		        }else if((temp_outside > 18) && (temp_outside <= 21)){
					if(timer.tag != 26) Reset_Timer(26);
					if((time_account(&timer.timer))>180) stat_buffer.time_buffer[HOT_M18_21_TIME13]++;
		        }else if((temp_outside > 21) && (temp_outside <= 23)){
					if(timer.tag != 27) Reset_Timer(27);
					if((time_account(&timer.timer))>180) stat_buffer.time_buffer[HOT_M21_23_TIME14]++;
		        }else if(temp_outside > 23){
					if(timer.tag != 28) Reset_Timer(28);
					if((time_account(&timer.timer))>180) stat_buffer.time_buffer[HOT_G23_TIME15]++;
		        }
				break;
				
			default:
				break;
		}


/*
*	制热制冷时回油次数统计
*/
	if((sys_mode == 2) || (sys_mode == 3)){
		if(oil_flag != oil_cold_flag){
			oil_cold_flag = oil_flag;
			if(oil_cold_flag == 1){
				stat_buffer.time_buffer[CEXE_RTOIL_COUNT]++;
			}
		}
	}else if(sys_mode == 5){
		if(oil_flag != oil_hot_flag){
			oil_hot_flag = oil_flag;
			if(oil_hot_flag == 1){
				stat_buffer.time_buffer[HEXE_RTOIL_COUNT]++;
			}
		}
	}

/*
*	化霜运行时长次数统计
*/

    if(frest_flag == 1){
	    if(oldflag_frest == 0){
	        start = RTC_GetCounter();
	        oldflag_frest = 1;
	    }
    }else{
        if(oldflag_frest == 1) time_s = RTC_GetCounter() - start;
        oldflag_frest = 0 ;
    }

    if(time_s != 0){
        if(time_s <= 120){
            stat_buffer.time_buffer[DFRST_EXP_L2_COUNT1]++;
            time_s = 0;
        }else if((time_s > 120) && (time_s <= 180)){
            stat_buffer.time_buffer[DFRST_EXP_M2_3_COUNT2]++;
            time_s = 0;
        }else if((time_s > 180) && (time_s <= 240)){
            stat_buffer.time_buffer[DFRST_EXP_M3_4_COUNT3]++;
            time_s = 0;
        }else if((time_s > 240) && (time_s <= 300)){
			stat_buffer.time_buffer[DFRST_EXP_M4_5_COUNT4]++;
			time_s = 0;
        }else if((time_s > 300) && (time_s <= 360)){
			stat_buffer.time_buffer[DFRST_EXP_M5_6_COUNT5]++;
			time_s = 0;
        }else if((time_s > 360) && (time_s <= 420)){
			stat_buffer.time_buffer[DFRST_EXP_M6_7_COUNT6]++;
			time_s = 0;
        }else if((time_s > 420) && (time_s <= 480)){
			stat_buffer.time_buffer[DFRST_EXP_M7_8_COUNT7]++;
			time_s = 0;
        }else if((time_s > 480)){
			stat_buffer.time_buffer[DFRST_EXP_G8_COUNT8]++;
			time_s = 0;
        }
		stat_buffer.time_buffer[DFRST_SUM_COUNT]++;
		time_s = 0;
    }

/*
*	化霜期间，开机室内机温度波动范围次数统计
*	此处的室内机：是指各运行状态的内机的平均环境温度
*	和化霜过程中最低温度的差值。
*/
	
	if(!frest_flag){
		if(flag){
			if(temp_diff < 3){
				stat_buffer.time_buffer[DFRST_OPN_L3_COUNT1]++;
			}else if((temp_diff >= 3) && (temp_diff < 4)){
				stat_buffer.time_buffer[DFRST_OPN_M3_4_COUNT2]++;
			}else if((temp_diff >= 4) && (temp_diff < 5)){
				stat_buffer.time_buffer[DFRST_OPN_M4_5_COUNT3]++;
			}else if((temp_diff >= 5) && (temp_diff < 6)){
				stat_buffer.time_buffer[DFRST_OPN_M5_6_COUNT4]++;
			}else if((temp_diff >= 6) && (temp_diff < 7)){
				stat_buffer.time_buffer[DFRST_OPN_M6_7_COUNT5]++;
			}else if((temp_diff >= 7)){
				stat_buffer.time_buffer[DFRST_OPN_G7_COUNT6]++;
			}
			
			flag = 0;//一次统计标志flag
//			frest_over = 0;
		}
			temp_aver = aver_indoor_temp();
	}else if(frest_flag){
		flag = 1;
		if(temp_aver >= temp_frest_lowest){
			temp_diff = temp_aver - temp_frest_lowest;
		}else if(temp_aver < temp_frest_lowest){
			temp_diff = temp_frest_lowest - temp_aver;
		}
		
	}

/*
*	风机频率统计
*/
  
    if(((sys_mode == 2) || (sys_mode == 3) || (sys_mode == 5)) && \
							(oil_flag != 1) && (frest_flag != 1)){
        if(wind_frequence <= 10){
			stat_buffer.time_buffer[NEXE_FANFRQ_L10_TIME1]++;
        }else if((wind_frequence > 10) && (wind_frequence <= 12)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M10_12_TIME2]++;
        }else if((wind_frequence > 12) && (wind_frequence <= 14)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M12_14_TIME3]++;
        }else if((wind_frequence > 14) && (wind_frequence <= 16)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M14_16_TIME4]++;
        }else if((wind_frequence > 16) && (wind_frequence <= 18)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M16_18_TIME5]++;
        }else if((wind_frequence > 18) && (wind_frequence <= 20)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M18_20_TIME6]++;
        }else if((wind_frequence > 20) && (wind_frequence <= 22)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M20_22_TIME7]++;
        }else if((wind_frequence > 22) && (wind_frequence <= 24)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M22_24_TIME8]++;
        }else if((wind_frequence > 24) && (wind_frequence <= 26)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M24_26_TIME9]++;
        }else if((wind_frequence > 26) && (wind_frequence <= 28)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M26_28_TIME10]++;
        }else if((wind_frequence > 28) && (wind_frequence <= 30)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M28_30_TIME11]++;
        }else if((wind_frequence > 30) && (wind_frequence <= 32)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M30_32_TIME12]++;
        }else if((wind_frequence > 32) && (wind_frequence <= 34)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M32_34_TIME13]++;
        }else if((wind_frequence > 34) && (wind_frequence <= 36)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M34_36_TIME14]++;
        }else if((wind_frequence > 36) && (wind_frequence <= 38)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M36_38_TIME15]++;
        }else if((wind_frequence > 38) && (wind_frequence <= 40)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M38_40_TIME16]++;
        }else if((wind_frequence > 40) && (wind_frequence <= 42)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M40_42_TIME17]++;
        }else if((wind_frequence > 42) && (wind_frequence <= 44)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M42_44_TIME18]++;
        }else if((wind_frequence > 44) && (wind_frequence <= 46)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M44_46_TIME19]++;
        }else if((wind_frequence > 46) && (wind_frequence <= 48)){
			stat_buffer.time_buffer[NEXE_FANFRQ_M46_48_TIME20]++;
        }else if((wind_frequence > 48)){
			stat_buffer.time_buffer[NEXE_FANFRQ_G48_TIME21]++;
        }
    }
							
/*
*	开机负荷率统计（内机容量/外机容量）
*/

        for(i = 0; i < INDOOR_MAX; i++){
            indoor_capability += capability[i];
        }
        for(i = 0; i < OUTDOOR_MAX; i++){
            outdoor_capability += out_capability[i];
        }
        capa_percent = (uint16_t)(((float)indoor_capability/(float)outdoor_capability)*100);
		
    if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag != 1)&&(frest_flag != 1)&& !g_Upload.IndOff){
        if((time_account(&timer10)) >= 300){
            if(capa_percent <= 10){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_L10_TIME1]++;
            }else if((capa_percent > 10) && (capa_percent <= 10)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M10_15_TIME2]++;
            }else if((capa_percent > 15) && (capa_percent <= 20)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M15_20_TIME3]++;
            }else if((capa_percent > 20) && (capa_percent <= 25)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M20_25_TIME4]++;
            }else if((capa_percent > 25) && (capa_percent <= 30)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M25_30_TIME5]++;
            }else if((capa_percent > 30) && (capa_percent <= 35)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M30_35_TIME6]++;
            }else if((capa_percent > 35) && (capa_percent <= 40)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M35_40_TIME7]++;
            }else if((capa_percent > 40) && (capa_percent <= 45)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M40_45_TIME8]++;
            }else if((capa_percent > 45) && (capa_percent <= 50)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M45_50_TIME9]++;
            }else if((capa_percent > 50) && (capa_percent <= 55)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M50_55_TIME10]++;
            }else if((capa_percent > 55) && (capa_percent <= 60)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M55_60_TIME11]++;
            }else if((capa_percent > 60) && (capa_percent <= 65)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M60_65_TIME12]++;
            }else if((capa_percent > 65) && (capa_percent <= 70)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M65_70_TIME13]++;
            }else if((capa_percent > 70) && (capa_percent <= 75)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M70_75_TIME14]++;
        	}else if((capa_percent > 75) && (capa_percent <= 80)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M75_80_TIME15]++;
        	}else if((capa_percent > 80) && (capa_percent <= 85)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M80_85_TIME16]++;
        	}else if((capa_percent > 85) && (capa_percent <= 90)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M85_90_TIME17]++;
        	}else if((capa_percent > 90) && (capa_percent <= 95)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M90_95_TIME18]++;
        	}else if((capa_percent > 95) && (capa_percent <= 100)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M95_100_TIME19]++;
        	}else if((capa_percent > 100) && (capa_percent <= 105)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M100_105_TIME20]++;
        	}else if((capa_percent > 105) && (capa_percent <= 110)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M105_110_TIME21]++;
        	}else if((capa_percent > 110) && (capa_percent <= 115)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M110_115_TIME22]++;
        	}else if((capa_percent > 115) && (capa_percent <= 120)){
				stat_buffer.time_buffer[NEXE_LDRT_OPEN_M115_120_TIME23]++;
        	}
    }
}else memset(&timer10,0,sizeof(timer10));

/*
*	正常运行时，排气过热度时间统计
*	排气过热度=压缩机排气温度-模块高压
*/
    if(((sys_mode == 2) || (sys_mode == 3) || (sys_mode == 5)) && (oil_flag != 1) && (frest_flag != 1) && sys_machine){
        if((time_account(&timerbf7))>=300){
                guoredu = paiqi1 - H_press;
                if(guoredu <= 5)  stat_buffer.time_buffer[NEXE_CMP1PQGRD_L5_TIME1]++;
                else if((guoredu > 5) && (guoredu <= 10))  	stat_buffer.time_buffer[NEXE_CMP1PQGRD_M5_10_TIME2]++;
                else if((guoredu > 10) && (guoredu <= 25))  stat_buffer.time_buffer[NEXE_CMP1PQGRD_M10_25_TIME3]++;
                else if((guoredu > 25) && (guoredu <= 30))   stat_buffer.time_buffer[NEXE_CMP1PQGRD_M25_30_TIME4]++;
				else if((guoredu > 30) && (guoredu <= 35))	 stat_buffer.time_buffer[NEXE_CMP1PQGRD_M30_35_TIME5]++;
				else if((guoredu > 35) && (guoredu <= 40))	 stat_buffer.time_buffer[NEXE_CMP1PQGRD_M35_40_TIME6]++;
				else if((guoredu > 40))						stat_buffer.time_buffer[NEXE_CMP1PQGRD_G40_TIME7]++;
/*
*	排气温度
*/
				if((paiqi1 > 100) && (paiqi1 <= 115) && sys_machine) stat_buffer.time_buffer[NEXE_CMP1PQWD_M100_115_TIME0]++;

/*				
*	正常运行时，吸气过热度时间统计
*	吸气过热度= 压缩机分离器出管温度-模块高压
*/
				xqguoredu = gas_split_temp - L_press;
				if(xqguoredu <= -3){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_LN3_TIME1]++;
				}else if((xqguoredu > -3) && (xqguoredu <= -2 )){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_MN3_N2_TIME2]++;
				}else if((xqguoredu > -2) && (xqguoredu <= -1)){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_MN2_N1_TIME3]++;
				}else if((xqguoredu > -1) && (xqguoredu <= 0)){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_MN1_0_TIME4]++;
				}else if((xqguoredu > 0) && (xqguoredu <= 1)){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_M0_1_TIME5]++;
				}else if((xqguoredu > 1) && (xqguoredu <= 2)){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_M1_2_TIME6]++;
				}else if((xqguoredu > 2) && (xqguoredu <= 3)){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_M2_3_TIME7]++;
				}else if((xqguoredu > 3)){
					stat_buffer.time_buffer[NEXE_CMP1XQGRD_G3_TIME8]++;
				}
            }
		}else memset(&timerbf7,0,sizeof(timerbf7));
		
/*
*	机组电量累计统计
*/
	if((sys_mode == 2) || (sys_mode == 3) || (sys_mode == 5) || (sys_mode == 1)){
		stat_buffer.time_buffer[SUM_ELEC_CHRG] = elec_sum;
/*
*	压缩机功率累计
*/
		stat_buffer.time_buffer[SUM_CMP_POWER] += cmp_power*10;

/*
*	压缩机加热带累计时间统计
*/
		if(warm_band1){
			stat_buffer.time_buffer[CMP1_ELECH_OPEN_TIME]++;
		}
	}
/*
*	内风机（主）处于运行状态下，电机转速运行
*/
	if((indoor_fan_mode > 0) && (indoor_fan_mode < 13)){
		if(indoor_speed <= 200){
			stat_buffer.time_buffer[DC_INMT_RPM_L200_TIME1]++;
		}else if((indoor_speed > 200) && (indoor_speed <= 400)){
			stat_buffer.time_buffer[DC_INMT_RPM_M200_400_TIME2]++;
		}else if((indoor_speed > 400) && (indoor_speed <= 600)){
			stat_buffer.time_buffer[DC_INMT_RPM_M400_600_TIME3]++;
		}else if((indoor_speed > 600) && (indoor_speed <= 800)){
			stat_buffer.time_buffer[DC_INMT_RPM_M600_800_TIME4]++;
		}else if((indoor_speed > 800) && (indoor_speed <= 1000)){
			stat_buffer.time_buffer[DC_INMT_RPM_M800_1000_TIME5]++;
		}else if((indoor_speed > 1000) && (indoor_speed <= 1200)){
			stat_buffer.time_buffer[DC_INMT_RPM_M1000_1200_TIME6]++;
		}else if((indoor_speed > 1200) && (indoor_speed <= 1500)){
			stat_buffer.time_buffer[DC_INMT_RPM_M1200_1500_TIME7]++;
		}else if((indoor_speed > 1500)){
			stat_buffer.time_buffer[DC_INMT_RPM_G1500_TIME8]++;
		}
	}

/*
*	内电机停机次数统计，内电机（主）转速范围区间
*/

	if(((localTime.tm_hour >= 0x20) && (localTime.tm_hour <= 0x23)) | \
		(localTime.tm_hour >= 0x00) && (localTime.tm_hour <= 0x08))
	{
		if(indoor_speed > 100){
			indoor_speed_flag = 1;
		}else if((indoor_speed <= 100) && (indoor_speed_flag == 1)){
			stat_buffer.time_buffer[INMT_SHTDWN_N_M20_8_COUNT1]++;
			indoor_speed_flag = 0;
		}
		
		if(wind_frequence*10 > 100){
			outdoor_speed_flag = 1;
		}else if((wind_frequence*10 <= 100) && (outdoor_speed_flag == 1)){
			stat_buffer.time_buffer[OUTMT_SHTDWN_N_M20_8_COUNT1]++;
			outdoor_speed_flag = 0;
		}
	}else if((localTime.tm_hour > 0x08) && (localTime.tm_hour < 0x20)){
		if(indoor_speed > 100){
			indoor_speed_flag = 1;
		}else if((indoor_speed <= 100) && (indoor_speed_flag == 1)){
			stat_buffer.time_buffer[INMT_SHTDWN_D_M8_20_COUNT2]++;
			indoor_speed_flag = 0;
		}
		if(wind_frequence*10 > 100){
			outdoor_speed_flag = 1;
		}else if((wind_frequence*10 <= 100) && (outdoor_speed_flag == 1)){
			stat_buffer.time_buffer[OUTMT_SHTDWN_D_M8_20_COUNT2]++;
			outdoor_speed_flag = 0;
		}
	}

	if(cmp_mode == 6){
/*
*	压机处于运行状态，压机输入电流范围统计
*/
		if(pfc_am_int < 16){
			stat_buffer.time_buffer[IN_ELEC_L16_TIME1]++;
		}else if((pfc_am_int >= 16) && (pfc_am_int < 25)){
			stat_buffer.time_buffer[IN_ELEC_M16_25_TIME2]++;
		}else if((pfc_am_int >= 25) && (pfc_am_int < 32)){
			stat_buffer.time_buffer[IN_ELEC_M25_32_TIME3]++;
		}else if((pfc_am_int >= 32) && (pfc_am_int < 40)){
			stat_buffer.time_buffer[IN_ELEC_M32_40_TIME4]++;
		}else if((pfc_am_int >= 40)){
			stat_buffer.time_buffer[IN_ELEC_G40_TIME5]++;
		}
		
		exe_sum_time = stat_buffer.time_buffer[IN_ELEC_L16_TIME1] + \
					   	stat_buffer.time_buffer[IN_ELEC_M16_25_TIME2] + \
						stat_buffer.time_buffer[IN_ELEC_M25_32_TIME3] + \
						stat_buffer.time_buffer[IN_ELEC_M32_40_TIME4] + \
						stat_buffer.time_buffer[IN_ELEC_G40_TIME5];
					
		stat_buffer.time_buffer[IN_ELEC_L16_RATIO] = (stat_buffer.time_buffer[IN_ELEC_L16_TIME1]*100)/exe_sum_time;
		stat_buffer.time_buffer[IN_ELEC_M16_25_RATIO] = (stat_buffer.time_buffer[IN_ELEC_M16_25_TIME2]*100)/exe_sum_time;
		stat_buffer.time_buffer[IN_ELEC_M25_32_RATIO] = (stat_buffer.time_buffer[IN_ELEC_M25_32_TIME3]*100)/exe_sum_time;
		stat_buffer.time_buffer[IN_ELEC_M32_40_RATIO] = (stat_buffer.time_buffer[IN_ELEC_M32_40_TIME4]*100)/exe_sum_time;
//		stat_buffer.time_buffer[IN_ELEC_G40_RATIO] = (stat_buffer.time_buffer[IN_ELEC_G40_TIME5])/exe_sum_time;
		
/*
*		IPM模块温度统计和PFC模块温度统计						
*/

			if(IPM_temp <= 60){
				stat_buffer.time_buffer[OPEN_EXE_IPM_TMP_L60_TIME1]++;
			}else if((IPM_temp > 60) && (IPM_temp <= 70)){
				stat_buffer.time_buffer[OPEN_EXE_IPM_TMP_M60_70_TIME2]++;
			}else if((IPM_temp > 70) && (IPM_temp <= 80)){
				stat_buffer.time_buffer[OPEN_EXE_IPM_TMP_M70_80_TIME3]++;
			}else if((IPM_temp > 80) && (IPM_temp <= 90)){
				stat_buffer.time_buffer[OPEN_EXE_IPM_TMP_M80_90_TIME4]++;
			}else if((IPM_temp > 90) && (IPM_temp <= 100)){
				stat_buffer.time_buffer[OPEN_EXE_IPM_TMP_M90_100_TIME5]++;
			}else if((IPM_temp > 100)){
				stat_buffer.time_buffer[OPEN_EXE_IPM_TMP_G100_TIME6]++;
			}		
			
			if(PFC_temp <= 60){
				stat_buffer.time_buffer[OPEN_EXE_PFC_TMP_L60_TIME1]++;
			}else if((PFC_temp > 60) && (PFC_temp <= 70)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_TMP_M60_70_TIME2]++;
			}else if((PFC_temp > 70) && (PFC_temp <= 80)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_TMP_M70_80_TIME3]++;
			}else if((PFC_temp > 80) && (PFC_temp <= 90)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_TMP_M80_90_TIME4]++;
			}else if((PFC_temp > 90) && (PFC_temp <= 100)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_TMP_M90_100_TIME5]++;
			}else if((PFC_temp > 100)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_TMP_G100_TIME6]++;
			}
			
	/*
	*	压缩机母线电压统计
	*/
			if(bus_voltage < 200){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_L200_TIME1]++;
			}else if((bus_voltage >= 200) && (bus_voltage < 230)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M200_230_TIME2]++;
			}else if((bus_voltage >= 230) && (bus_voltage < 260)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M230_260_TIME3]++;
			}else if((bus_voltage >= 260) && (bus_voltage < 290)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M260_290_TIME4]++;
			}else if((bus_voltage >= 290) && (bus_voltage < 320)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M290_320_TIME5]++;
			}else if((bus_voltage >= 320) && (bus_voltage < 350)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M320_350_TIME6]++;
			}else if((bus_voltage >= 350) && (bus_voltage < 360)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M350_360_TIME7]++;
			}else if((bus_voltage >= 360) && (bus_voltage < 370)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M360_370_TIME8]++;
			}else if((bus_voltage >= 370) && (bus_voltage < 460)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M370_460_TIME9]++;
			}else if((bus_voltage >= 460) && (bus_voltage < 490)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M460_490_TIME10]++;
			}else if((bus_voltage >= 490) && (bus_voltage < 520)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M490_520_TIME11]++;
			}else if((bus_voltage >= 520) && (bus_voltage < 550)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M520_550_TIME12]++;
			}else if((bus_voltage >= 550) && (bus_voltage < 580)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_M550_580_TIME13]++;
			}else if((bus_voltage >= 580)){
				stat_buffer.time_buffer[OPEN_EXE_BUSVLT_G580_TIME14]++;
			}

	/*
	*	输入电源电压统计（针对单相机组）
	*/

			if(phrase_voltage < 165){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_L165_TIME1]++;
			}else if((phrase_voltage >= 165) && (phrase_voltage < 185)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_M165_185_TIME2]++;
			}else if((phrase_voltage >= 185) && (phrase_voltage < 205)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_M185_205_TIME3]++;
			}else if((phrase_voltage >= 205) && (phrase_voltage < 215)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_M205_215_TIME4]++;
			}else if((phrase_voltage >= 215) && (phrase_voltage < 225)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_M215_225_TIME5]++;
			}else if((phrase_voltage >= 225) && (phrase_voltage < 235)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_M225_235_TIME6]++;
			}else if((phrase_voltage >= 235) && (phrase_voltage < 255)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_M235_255_TIME7]++;
			}else if((phrase_voltage >= 255) && (phrase_voltage < 265)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_M255_265_TIME8]++;
			}else if((phrase_voltage >= 265)){
				stat_buffer.time_buffer[OPEN_EXE_INVLT_G265_TIME9]++;
			}
	/*
	*	PFC运行电流统计
	*/
			if(pfc_am_int < 5){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_L5_TIME1]++;
			}else if((pfc_am_int >= 5) && (pfc_am_int < 9)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M5_9_TIME2]++;
			}else if((pfc_am_int >= 9) && (pfc_am_int < 12)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M9_12_TIME3]++;
			}else if((pfc_am_int >= 12) && (pfc_am_int < 15)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M12_15_TIME4]++;
			}else if((pfc_am_int >= 15) && (pfc_am_int < 18)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M15_18_TIME5]++;
			}else if((pfc_am_int >= 18) && (pfc_am_int < 21)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M18_21_TIME6]++;
			}else if((pfc_am_int >= 21) && (pfc_am_int < 24)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M21_24_TIME7]++;
			}else if((pfc_am_int >= 24) && (pfc_am_int < 27)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M24_27_TIME8]++;
			}else if((pfc_am_int >= 27) && (pfc_am_int < 30)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M27_30_TIME9]++;
			}else if((pfc_am_int >= 30) && (pfc_am_int < 33)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_M30_33_TIME10]++;
			}else if((pfc_am_int >= 33)){
				stat_buffer.time_buffer[OPEN_EXE_PFC_ELEC_G33_TIME11]++;
			}

	/*
	*	变频压缩机运行电流统计
	*/
			if(cmp_am_int < 5){
				stat_buffer.time_buffer[OPEN_EXE_CMP_WRKELEC_L5_TIME1]++;
			}else if((cmp_am_int >= 5) && (cmp_am_int < 8)){
				stat_buffer.time_buffer[OPEN_EXE_CMP_WRKELEC_M5_8_TIME2]++;
			}else if((cmp_am_int >= 8) && (cmp_am_int < 11)){
				stat_buffer.time_buffer[OPEN_EXE_CMP_WRKELEC_M8_11_TIME3]++;
			}else if((cmp_am_int >= 11) && (cmp_am_int < 14)){
				stat_buffer.time_buffer[OPEN_EXE_CMP_WRKELEC_M11_14_TIME4]++;
			}else if((cmp_am_int >= 14) && (cmp_am_int < 17)){
				stat_buffer.time_buffer[OPEN_EXE_CMP_WRKELEC_M14_17_TIME5]++;
			}else if((cmp_am_int >= 17) && (cmp_am_int < 20)){
				stat_buffer.time_buffer[OPEN_EXE_CMP_WRKELEC_M17_20_TIME6]++;
			}else if((cmp_am_int >= 20)){
				stat_buffer.time_buffer[OPEN_EXE_CMP_WRKELEC_G20_TIME7]++;
			}

	/*
	*	压缩机启动压差统计
	*/

			if((instant_flag == 1) && (have_date == 1)){
				if(abs_high_press > abs_low_press){
					diff_high_low_press = abs_high_press - abs_low_press;
				}else{
					diff_high_low_press = abs_low_press - abs_high_press;
				}
				
				if(diff_high_low_press < 300){
					stat_buffer.time_buffer[WRK1_EXE_CMP_PRESS_DIFF_L03_COUNT1]++;
				}else if((diff_high_low_press >= 300) && (diff_high_low_press < 600)){
					stat_buffer.time_buffer[WRK1_EXE_CMP_PRESS_DIFF_M03_06_COUNT2]++;
				}else if((diff_high_low_press >= 600) && (diff_high_low_press < 900)){
					stat_buffer.time_buffer[WRK1_EXE_CMP_PRESS_DIFF_M06_09_COUNT3]++;
				}else if((diff_high_low_press >= 900) && (diff_high_low_press < 1200)){
					stat_buffer.time_buffer[WRK1_EXE_CMP_PRESS_DIFF_M09_12_COUNT4]++;
				}else if((diff_high_low_press >= 1200) && (diff_high_low_press < 1500)){
					stat_buffer.time_buffer[WRK1_EXE_CMP_PRESS_DIFF_M12_15_COUNT5]++;
				}else if((diff_high_low_press >= 1500)){
					stat_buffer.time_buffer[WRK1_EXE_CMP_PRESS_DIFF_G15_COUNT6]++;
				}
				instant_flag = 0;
				have_date = 0;
			}

	/*
	*	驱动板限频统计
	*	驱动板降频统计
	*	驱动板弱磁统计
	*/
			if(limit_freq){
				stat_buffer.time_buffer[CMP_DRVBRD_LMTFRQ_TIME]++;
			}
			
			if(lim_frq_flag != limit_freq){
				lim_frq_flag = limit_freq;
				if(lim_frq_flag){
					stat_buffer.time_buffer[CMP_DRVBRD_LMTFRQ_COUNT]++;
				}
			}
			
			if(de_freq){
				stat_buffer.time_buffer[CMP_DRVBRD_RDFRQ_TIME]++;
			}
			
			if(de_frq_flag != de_freq){
				de_frq_flag = de_freq;
				if(de_frq_flag){
					stat_buffer.time_buffer[CMP_DRVBRD_RDFRQ_COUNT]++;
				}
			}
			
			if(weak_magnet){
				stat_buffer.time_buffer[CMP_DRVBRD_WKMGNT_TIME]++;
			}
			
			if(weak_mag_flag != weak_magnet){
				weak_mag_flag = weak_magnet;
				if(weak_mag_flag){
					stat_buffer.time_buffer[CMP_DRVBRD_WKMGNT_COUNT]++;
				}
			}
	}else{
		instant_flag = 1;
	}

	if(fan_mode == 6)
	{
		fan_exe_am = ((float)fan_am_dec )/256.0;
		fan_exe_am += (float)fan_am_int;
		if(fan_exe_am < 0.1){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_L01_TIME1]++;
		}else if((fan_exe_am >= 0.1) && (fan_exe_am < 0.2)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M01_02_TIME2]++;
		}else if((fan_exe_am >= 0.2) && (fan_exe_am < 0.3)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M02_03_TIME3]++;
		}else if((fan_exe_am >= 0.3) && (fan_exe_am < 0.4)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M03_04_TIME4]++;
		}else if((fan_exe_am >= 0.4) && (fan_exe_am < 0.7)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M04_07_TIME5]++;
		}else if((fan_exe_am >= 0.7) && (fan_exe_am < 1.1)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M07_11_TIME6]++;
		}else if((fan_exe_am >= 1.1) && (fan_exe_am < 2)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M11_2_TIME7]++;
		}else if((fan_exe_am >= 2) && (fan_exe_am < 3)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M2_3_TIME8]++;
		}else if((fan_exe_am >= 3) && (fan_exe_am < 4)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M3_4_TIME9]++;
		}else if((fan_exe_am >= 4) && (fan_exe_am < 5)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M4_5_TIME10]++;
		}else if((fan_exe_am >= 5) && (fan_exe_am < 6)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_M5_6_TIME11]++;
		}else if((fan_exe_am >= 6)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_ELEC_G6_TIME12]++;
		}

		if(fan_bus_volt < 200){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_L200_TIME1]++;
		}else if((fan_bus_volt >= 200) && (fan_bus_volt < 220)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M200_220_TIME2]++;
		}else if((fan_bus_volt >= 220) && (fan_bus_volt < 240)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M220_240_TIME3]++;
		}else if((fan_bus_volt >= 240) && (fan_bus_volt < 260)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M240_260_TIME4]++;
		}else if((fan_bus_volt >= 260) && (fan_bus_volt < 280)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M260_280_TIME5]++;
		}else if((fan_bus_volt >= 280) && (fan_bus_volt < 300)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M280_300_TIME6]++;
		}else if((fan_bus_volt >= 300) && (fan_bus_volt < 320)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M300_320_TIME7]++;
		}else if((fan_bus_volt >= 320) && (fan_bus_volt < 340)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M320_340_TIME8]++;
		}else if((fan_bus_volt >= 340) && (fan_bus_volt < 360)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M340_360_TIME9]++;
		}else if((fan_bus_volt >= 360) && (fan_bus_volt < 370)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_M360_370_TIME10]++;
		}else if((fan_bus_volt >= 370)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_BUSVLT_G370_TIME11]++;
		}

		if(fan_pfc_temp <= 50){
			stat_buffer.time_buffer[OPEN_EXE_FAN_PFC_TMP_L50_TIME1]++;
		}else if((fan_pfc_temp > 50) && (fan_pfc_temp <= 60)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_PFC_TMP_M50_60_TIME2]++;
		}else if((fan_pfc_temp > 60) && (fan_pfc_temp <= 70)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_PFC_TMP_M60_70_TIME3]++;
		}else if((fan_pfc_temp > 70) && (fan_pfc_temp <= 80)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_PFC_TMP_M70_80_TIME4]++;
		}else if((fan_pfc_temp > 80) && (fan_pfc_temp <= 90)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_PFC_TMP_M80_90_TIME5]++;
		}else if((fan_pfc_temp > 90) && (fan_pfc_temp <= 100)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_PFC_TMP_M90_100_TIME6]++;
		}else if((fan_pfc_temp > 100)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_PFC_TMP_G100_TIME7]++;
		}

		if(fan_ipm_temp <= 50){
			stat_buffer.time_buffer[OPEN_EXE_FAN_IPM_TMP_L50_TIME1]++;
		}else if((fan_ipm_temp > 50) && (fan_ipm_temp <= 60)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_IPM_TMP_M50_60_TIME2]++;
		}else if((fan_ipm_temp > 60) && (fan_ipm_temp <= 70)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_IPM_TMP_M60_70_TIME3]++;
		}else if((fan_ipm_temp > 70) && (fan_ipm_temp <= 80)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_IPM_TMP_M70_80_TIME4]++;
		}else if((fan_ipm_temp > 80) && (fan_ipm_temp <= 90)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_IPM_TMP_M80_90_TIME5]++;
		}else if((fan_ipm_temp > 90) && (fan_ipm_temp <= 100)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_IPM_TMP_M90_100_TIME6]++;
		}else if((fan_ipm_temp > 100)){
			stat_buffer.time_buffer[OPEN_EXE_FAN_IPM_TMP_G100_TIME7]++;
		}

/*
*	风机驱动板限频统计
*	风机驱动板降频统计
*	风机驱动板弱磁统计
*/
		if(fan_limit_freq){
			stat_buffer.time_buffer[FAN_DRVBRD_LMTFRQ_TIME]++;
		}
		if(fan_lim_frq_flag != fan_limit_freq){
			fan_lim_frq_flag = fan_limit_freq;
			if(fan_lim_frq_flag){
				stat_buffer.time_buffer[FAN_DRVBRD_LMTFRQ_COUNT]++;
			}
		}
		if(fan_de_freq){
			stat_buffer.time_buffer[FAN_DRVBRD_RDFRQ_TIME]++;
		}
		if(fan_de_frq_flag != fan_de_freq){
			fan_de_frq_flag = fan_de_freq;
			if(fan_de_frq_flag){
				stat_buffer.time_buffer[FAN_DRVBRD_RDFRQ_COUNT]++;
			}
		}
		if(fan_weak_magnet){
			stat_buffer.time_buffer[FAN_DRVBRD_WKMGNT_TIME]++;
		}
		if(fan_weak_mag_flag != fan_weak_magnet){
			fan_weak_mag_flag = fan_weak_magnet;
			if(fan_weak_mag_flag){
				stat_buffer.time_buffer[FAN_DRVBRD_WKMGNT_COUNT]++;
			}
		}
	}

/*
*	CAN通信质量
*/


	if(sign_over_flag)
	{

		tcp_rev_total = 0;
		lost_pack = 0;
		sign_over_flag = 0;
		signal_low = 0;
		signal_mid = 0;
		signal_hig = 0;
	}

	if(sign_over_flag_month){
		CanErrCount = 0;
		total_count = 0;
		sign_over_flag_month = 0;
	}
	
	if(total_count != 0){
		stat_buffer.time_buffer[CAN_CMM_QLT] = ((total_count*10000)/(total_count+CanErrCount));
	}else{
		stat_buffer.time_buffer[CAN_CMM_QLT] = 10000;
	}

/*
*	室内机参数：
*/

/*
*	高压传感器和低压传感器的准确性：
*	高压传感器和低压传感器都准确：					        0
*	高压传感器准确，低压传感器不准确：						1
*	高压传感器不准确，低压传感器准确：					    2
*	高压传感器不准确，低压传感器不准确：         				3
*/

	if(temp_outside > H_press) accurancy_h = temp_outside - H_press;
	else accurancy_h = H_press - temp_outside;
	if(temp_outside > L_press) accurancy_l = temp_outside - L_press;
	else accurancy_l = L_press - temp_outside;

	if((!sys_machine) && (finish_flag == 0)){
		if(time_account(&timerbf6) >= 1800){
			if((accurancy_l > 10) ||  (accurancy_h < 10))
				stat_buffer.time_buffer[MACUNIT_H_L_SENSOR_ACCURACY]++;
//			stat_buffer.time_buffer[MACUNIT_H_L_SENSOR_ACCURACY] = 1;
//			else if((accurancy_h > 10) && (accurancy_l < 10))
//				stat_buffer.time_buffer[MACUNIT_H_L_SENSOR_ACCURACY] = 2;
//			else if((accurancy_h > 10) && (accurancy_l > 10))
//				stat_buffer.time_buffer[MACUNIT_H_L_SENSOR_ACCURACY] = 3;
			finish_flag = 1;
		}
	}else{
		finish_flag = 0;
		memset(&timerbf6,0,sizeof(timerbf6));
	}


	if(reset_flag){
		if(reset_over_flag){
			stat_buffer.time_buffer[MACUNIT_RESET_COUNT]++;
			reset_over_flag = 0;
//			reset_flag_exsit = 0;
		}
		reset_flag_exsit = 1;
	}else{
		if(reset_flag_exsit){
			if(time_account(&timerbf5) < 120){
				reset_over_flag = 1;
			}else{
				memset(&timerbf5,0,sizeof(timerbf5));
				reset_over_flag = 0;
//				reset_flag_exsit = 0;
			}
		}
	}
	

	if(g_Upload.DebugOver){
		if(!error_flag && !g_Upload.flag_no_fault){
//			no_fault_exe_time = time_account32(&timerbf4);
			stat_buffer.time_buffer[MACUINT_NOFAULT_WORK_TIME]++;
		}else if(error_flag == 1){
				g_Upload.flag_no_fault = 1;
//				savePowerDownInfo(&g_Upload);				
//			stat_buffer.time_buffer[MACUINT_NOFAULT_WORK_TIME] = no_fault_exe_time;
//            memset(&timerbf4,0,sizeof(timerbf4));
		}
	}

		

	if(veil_save_energy_state == 0) veil_flag = 1;
	else if((veil_save_energy_state == 1) && (veil_flag == 1)){
		stat_buffer.time_buffer[MASK_SAVEENERGY_FUNC_COUNT]++;
		veil_flag = 0;
	}

	if(veil_tmp_state == 0) veil_flag_tmp = 1;
	else if((veil_tmp_state == 1) && (veil_flag_tmp == 1)){
		stat_buffer.time_buffer[MASK_TMP_FUNC_COUNT]++;
		veil_flag_tmp = 0;
	}

	if(veil_model_state == 0) veil_flag_model = 1;
	else if((veil_model_state == 1) && (veil_flag_model == 1)){
		stat_buffer.time_buffer[MASK_MODE_FUNC_COUNT]++;
		veil_flag_model = 0;
	}

	if(veil_switch_state == 0) veil_flag_switch = 1;
	else if((veil_switch_state == 1) && (veil_flag_switch == 1)){
		stat_buffer.time_buffer[MASK_SWITCH_FUNC_COUNT]++;
		veil_flag_switch = 0;
	}

	if(veil_lock_state == 0) veil_flag_lock = 1;
	else if((veil_lock_state == 1) && (veil_flag_lock == 1)){
		stat_buffer.time_buffer[MASK_LOCK_FUNC_COUNT]++;
		veil_flag_lock = 0;
	}

	if(go_out_state == 0) go_out_flag = 1;
	else if((go_out_state) && (go_out_flag == 1)){
		stat_buffer.time_buffer[GO_OUT_FUNC_COUNT]++;
		go_out_flag = 0;
	}

	if(up_down_fan < 2) up_down_flag = 1;
	else if((up_down_fan >= 2) && (up_down_flag == 1)){
		stat_buffer.time_buffer[UP_DOWN_WIND_FUNC_COUNT]++;
		up_down_flag = 0;
	}

	if(left_right_fan < 2) left_right_flag = 1;
	else if((left_right_fan >= 2) && (left_right_flag == 1)){
		stat_buffer.time_buffer[LEFT_RIGHT_WIND_FUNC_COUNT]++;
		left_right_flag = 0;
	}

	if(sleep_state < 2) sleep_flag = 1;
	else if((sleep_state >= 2) && (sleep_flag == 1)){
		stat_buffer.time_buffer[SLEEP_FUNC_COUNT]++;
		sleep_flag = 0;
	}

	if(!swap_fan) swap_fan_flag = 1;
	else if((swap_fan) && (swap_fan_flag)){
		stat_buffer.time_buffer[CHANGE_AIR_FUNC_COUNT]++;
		swap_fan_flag = 0;
	}

	if(muse_mode < 2) muse_mode_flag = 1;
	else if((muse_mode >= 2) && (muse_mode_flag == 1)){
		stat_buffer.time_buffer[MUTE_FUNC_COUNT]++;
		muse_mode_flag = 0;
	}

	if(save_energy_state == 0) save_energy_flag = 1;
	else if((save_energy_state == 1) && (save_energy_flag == 1)){
		stat_buffer.time_buffer[SAVEENERGY_FUNC_COUNT]++;
		save_energy_flag = 0;
	}

	if(filt_net_state < 10) filt_net_flag = 1;
	else if((filt_net_state >= 10) && (filt_net_flag)){
		stat_buffer.time_buffer[FILT_NET_REMIND_FUNC_COUNT]++;
		filt_net_flag = 0;
	}

	if(dry_state == 0) dry_state_flag = 1;
	else if((dry_state) && (dry_state_flag == 1)){
		stat_buffer.time_buffer[DRY_FUNC_COUNT]++;
		dry_state_flag = 0;
	}

	if(aid_hot_state == 0) aid_hot_flag = 1;
	else if((aid_hot_state) && (aid_hot_flag == 1)){
		stat_buffer.time_buffer[AID_HOT_FUNC_COUNT]++;
		aid_hot_flag = 0;
	}

	if(super_wind == 0) super_wind_flag = 1;
	else if((super_wind) && (super_wind_flag == 1)){
		stat_buffer.time_buffer[SUPER_WIND_LEVEL_FUNC_COUNT]++;
		super_wind_flag = 0;
	}

	if(timing_state == 0) timing_state_flag = 1;
	else if((timing_state) && (timing_state_flag == 1)){
		stat_buffer.time_buffer[SET_TIME_FUNC_COUNT]++;
		timing_state_flag = 0;
	}

	if(child_lock_state == 0) child_lock_flag = 1;
	else if((child_lock_state == 1) && (child_lock_flag == 1)){
		stat_buffer.time_buffer[CHILD_LOCK_FUNC_COUNT]++;
		child_lock_flag = 0;
	}

	for(uint8_t indoor_num=0;indoor_num<6;indoor_num++){

	
		if(indoor_static_press_flag[indoor_num]){	
			stat_buffer.time_buffer[INMT1_FAN_FAULT_PRESS_LEVEL+indoor_num*3] = static_press_level[indoor_num];
		}
		
		if((work_station[indoor_num])){
//			if(!ten_minute_flag[indoor_num]){
			if(ten_minute_flag[indoor_num]++ < 600){
//				if(){
					if((water_full[indoor_num]) && (!water_full_flag[indoor_num])){
						stat_buffer.time_buffer[INMT1_OPEN_M10_FLL_WTR_COUNT+indoor_num*3]++;
						water_full_flag[indoor_num] = 1;
					}else if(!water_full[indoor_num]){
						water_full_flag[indoor_num] = 0;
					}
//				}
//				else{
////		            memset(&timerbf3[indoor_num],0,sizeof(timerbf3[indoor_num]));
//					ten_minute_flag[indoor_num] = 1;
//				}
			}

				if(water_full[indoor_num]){
					stat_buffer.time_buffer[INMT1_FLL_WTR_RECOVER_TIME+indoor_num*3]++;					
//					reverse_time[indoor_num] = time_account(&timerbf2[indoor_num]);
				}
//				else if(!water_full[indoor_num]){
//					stat_buffer.time_buffer[INMT1_FLL_WTR_RECOVER_TIME+indoor_num*3]=reverse_time[indoor_num];
//					memset(&timerbf3[indoor_num],0,sizeof(timerbf3[indoor_num]));
//				}
		}else{
			ten_minute_flag[indoor_num] = 0;
		}
	}	

	stat_buffer.DTU_GET_ELEC_TIME++;
	if(tcp_rev_total != 0){
		stat_buffer.dtu_param_buff[DTU_DATA_LOST_RATIO] = lost_pack*100/(tcp_rev_total+lost_pack);
	}else{
		stat_buffer.dtu_param_buff[DTU_DATA_LOST_RATIO] = 0;
	}
	stat_buffer.dtu_param_buff[DTU_UPGRADE_SUCC_COUNT] = updateinfo.upgrade_succ_count;
	stat_buffer.dtu_param_buff[DTU_UPGRADE_FAIL_COUNT] = updateinfo.upgrade_fail_count;
//	stat_buffer.dtu_param_buff[DTU_SIGNAL_INTENSITY_VALUE] = get_signal_val;
//	stat_buffer.dtu_param_buff[DTU_ENTER_FLY_MODE] = get_signal_val;

	if(ok_flag == OK_IS_3S){
		if((get_signal_val > 0) && (get_signal_val <= 10)){
			signal_low++;
		}else if((get_signal_val > 10) && (get_signal_val <= 20)){
			signal_mid++;
		}else if((get_signal_val > 21) && (get_signal_val <= 31)){
			signal_hig++;
		}
		stat_buffer.dtu_param_buff[DTU_SIGNAL_RATE_LOW] = (uint32_t)(((float)signal_low / (signal_low + signal_mid + signal_hig))*10000);
		stat_buffer.dtu_param_buff[DTU_SIGNAL_RATE_MID] = (uint32_t)(((float)signal_mid / (signal_low + signal_mid + signal_hig))*10000);
		stat_buffer.dtu_param_buff[DTU_SIGNAL_RATE_HIG] = 10000 - stat_buffer.dtu_param_buff[DTU_SIGNAL_RATE_LOW] - \
																stat_buffer.dtu_param_buff[DTU_SIGNAL_RATE_MID];
		ok_flag = OK_NO_3S;
	}
	
//	stat_buffer.dtu_param_buff[DTU_BASE_STATION_LOCATION] = base_station;
//	stat_buffer.dtu_param_buff[DTU_CAN_NET_ERROR_RATIO] = 10000 - stat_buffer.time_buffer[CAN_CMM_QLT];
//	stat_buffer.dtu_param_buff[DTU_CAN_NET_DATA_SUM] = total_count + CanErrCount;
}

uint8_t sign_everyday(uint8_t trans_mode,uint8_t curSta)
{
    uint8_t err = 0 , res = 0 ;
    uint8_t flash_flag = 0;
    static uint16_t prevHour = 0;
    static uint16_t nextHour = 0;
	static uint16_t prev_day = 0;
	static uint16_t next_day = 0;
#ifdef TEST_TEST
	nextHour = (f4_time)/(2*stat_buffer.usr_hbt_time);
	next_day = (f4_time)/(2*stat_buffer.stat_data_time);

#else
	nextHour = (f4_time)/(3600*stat_buffer.usr_hbt_time);
	next_day = (f4_time)/(3600*stat_buffer.stat_data_time);

#endif
	if(g_Upload.flag_hours){
//		a_hour_flag = 0;
		g_Upload.flag_hours = 0;

		stat_buffer.F4_COUNT = F4_COUNT;
		stat_buffer.F4_DTU_COUNT = F4_DTU_COUNT;
		flash_flag = 0xBB;
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
        sFLASH_WriteBuffer(&flash_flag ,STATISTIC_START_ADDR,1);
        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,STATISTIC_START_ADDR+1,2);
        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,STATISTIC_START_ADDR+4,sizeof(stat_buffer));
        sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,STATISTIC_START_ADDR+3,1);

        sFLASH_EraseSubSector(BACKUP_STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
        sFLASH_WriteBuffer(&flash_flag ,BACKUP_STATISTIC_START_ADDR,1);
        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,BACKUP_STATISTIC_START_ADDR+1,2);
        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,BACKUP_STATISTIC_START_ADDR+4,sizeof(stat_buffer));
        sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,BACKUP_STATISTIC_START_ADDR+3,1);		
        OSMutexPost(MutexFlash);
	}
	
//一天一次的打卡和一个月一次的打卡

    if((nextHour - prevHour) || (next_day - prev_day)){
        flash_flag = 0xBB;
		stat_buffer.F4_COUNT = F4_COUNT;
		stat_buffer.F4_DTU_COUNT = F4_DTU_COUNT;
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
        sFLASH_WriteBuffer(&flash_flag ,STATISTIC_START_ADDR,1);
        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,STATISTIC_START_ADDR+1,2);
        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,STATISTIC_START_ADDR+3,sizeof(stat_buffer));
        sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,STATISTIC_START_ADDR+3,1);
		
        sFLASH_EraseSubSector(BACKUP_STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
        sFLASH_WriteBuffer(&flash_flag ,BACKUP_STATISTIC_START_ADDR,1);
        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,BACKUP_STATISTIC_START_ADDR+1,2);
        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,BACKUP_STATISTIC_START_ADDR+4,sizeof(stat_buffer));
        sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,BACKUP_STATISTIC_START_ADDR+3,1);
        OSMutexPost(MutexFlash);
        if(!ConSta.Status){
			SendDataToServer(0xFF,curSta);
//            if(!connectToServer()){
//                while(!disConnectToServer());
//				
//                return curSta; 
//            }
//        if(!send89Frame(&g_TcpType)){
//            while(!disConnectToServer());
//            return curSta;
//        }
//        
//        if(!sendF3Frame(&g_TcpType)){
//            while(!disConnectToServer());
//            return curSta;
//        }
//#if 1
//        if(!send91Frame(&g_TcpType , 0xFF)){
//            while(!disConnectToServer());
//            return curSta;
//        }        
//#endif    
        F4_CONNECT = 1;

        }

        if(ConSta.Status){
            getLocalTime(&(&g_TcpType)->ValidDataTx.frameF5Tx.Time);
		if(nextHour - prevHour){
			if(F4_COUNT == 65535){
				F4_COUNT = 0;
			}else{
				F4_COUNT++;
			}
			prevHour = nextHour;			
//			sign_over_flag = 1;//以每天为单位,用于统计参数更新作为基准
			res = sendF5Frame(&g_TcpType,0,0x02,(uint8_t *)&stat_buffer.dtu_param_buff[0],sizeof(stat_buffer.dtu_param_buff),stat_buffer.F4_COUNT,0); 
			sign_over_flag = 1;//以每天为单位,用于统计参数更新作为基准
			if(!res){
				return curSta;
			}else{
				sendF5Frame(&g_TcpType,0,0x00, NULL, 0,stat_buffer.F4_COUNT+1,0);
			}
		}

		if(next_day - prev_day){
			if(F4_DTU_COUNT == 65535){
				F4_DTU_COUNT = 0;
			}else{
				F4_DTU_COUNT++;
			}
//			sign_over_flag_month = 1;
			prev_day = next_day;
			if(g_Upload.flag_no_fault){
				g_Upload.flag_no_fault = 0;
			}
			res = sendF5Frame(&g_TcpType,0,0x01,(uint8_t *)&stat_buffer.time_buffer[0],sizeof(stat_buffer.time_buffer),stat_buffer.F4_DTU_COUNT,0); 
			sign_over_flag_month = 1;
			if(!res){
				return curSta;
			}else{
				sendF5Frame(&g_TcpType,0,0x00, NULL, 0,stat_buffer.F4_DTU_COUNT+1,0);
			}
		}
//			if(res){
//				prevHour = nextHour;
//				prev_day = next_day;
//				return curSta;
//			}       
        }
        return curSta;
    }else{
        return curSta;
    }
}


void check_f4time(){
	if(!(f4_time % HOURS_TIME)){
//		a_hour_flag = 1;
		g_Upload.flag_hours = 1;
	}

	if(!(f4_time % (stat_buffer.config_time*60)) && var.realMonitor && g_Upload.trans_mode){
			g_Upload.flag_98 = 1;
	}

	if(!(f4_time % GprsParam.big_data_sampe_time)){
		g_Upload.flag_sample = 1;
	}

	if(!(f4_time % TAKE_PHOTO_TIME)){
		g_Upload.flag_5 = 1;
	}

	if(!(f4_time % TAKE_PHOTO_DEFAULT_TIME)){
		g_Upload.flag_30 = 1;
	}	
}




