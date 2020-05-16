//#include "takedata.h"
#include "common.h"
#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "duplicate.h"
#include "fsm.h"
#include "drv_sms.h"
#include "bsp.h"
#include "lib_def.h"
#include "duplicate.h"
#include "ucos_ii.h"

OS_EVENT * MSemExitWait;
#ifdef USING_USER_HABIT
uint16_t  user_pack_time = 0;
#endif
uint8_t take_photo_style = 0;//30分钟或配置时间内,或起始点需要发送数据标志  0,只为拍照存 ；1为拍照传

#ifdef USING_USER_HABIT
void user_data_fill(uint16_t SrcAdd, uint8_t value, indoorinfoType *indoorinfo, uint8_t repl, uint8_t endflag);

user_habit_t user_data = {
	.write_addr = USERHABIT_START_ADDR,
	.read_addr = USERHABIT_START_ADDR,
};
	
#endif


//power_data_t outdoor_power_data = {
//	.write_addr = POWER_BUFF_START_ADDR,
//	.read_addr = POWER_BUFF_START_ADDR,
//};
#ifdef USING_USER_HABIT

//用户习惯
user_habit_param user_habit_data = {
	.src_add = {0x1F00,0x2100,0x2101,0x7300,0x7301},
	.high_low_flag = {0x00,0x00,0x00,0x01,0x02},
	.cnt = 5,
};	
//0x2101:内机运行风速；0x7300、0x7301:内机设定温度
//0x1f00:开关机状态；0x2100:内机运行模式

#endif

control_precision precision_data = {
	.src_add = {
		0x2863,0x2864,0x2865,0x2866,0x2880,0x2881,0x2884,0x2885,0x288C,
		0x288D,0x2898,0x2899,0x28AA,0x28AB,0x5954,0x5955,0x5956,0x5957	
	},
	.high_low_flag = {
		0,0,0,0,1,2,1,2,1,
		2,1,2,1,2,1,2,1,2
	},
	.cnt = 18,
};

indoor_bigdata_control_precision indoor_bigdata_de_precision = {
	.src_add = {
		 {0x3100,0x3101,0x310E,0x310F},
		 {0x3108,0x3109},
		 {0x2000,0x2001,0x2003,0x2005,0x2006,0x2007,0x2008,0x2009}

	},
	.high_low_flag = {
		  {0,1,0,1},
		  {0,1},
		  {3,3,3,3,3,3,3,3}
	},
	.precision = {
	    1,1,1
	},
	.cnt = {4,2,8}	
};

outdoor_bigdata_control_precision outdoor_bigdata_de_precision = {
	.src_add = {
		{0x2882,0x2883,0x2888},
		{0x285F,0x2860,0x2861,0x2889,0x2895,0x28A1},
		{0x3150,0x3151,0x3152,0x3153,0x3189,0x318A,0x7854,0x7855},
		{0x318B,0x318C,0x318D,0x318E},
		{0x3191,0x3192,0x3193,0x3194},
		{0x31A8,0x31A9},
		{0x2057,0x205F,0x2060,0x2061,0x2062,0x2063,0x2064,0x2066,0x2069,0x206A,0x2073,0x2074,0x2075,0x20C8,0x20CA},
		{0x2881,0x2885,0x2886,0x2887},	
	},
	.high_low_flag = {
		 {0,1,3},
		 {3,3,3,3,3,3},
		 {0,1,0,1,0,1,0,1},
		 {0,1,0,1},
		 {0,1,0,1},
		 {0,1},
		 {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
		 {3,3,3,3},//还原精度为0.1A
	},
	.precision = {
 		 1,1,1,1,1,1,1,1,
 	 	},
 	 .cnt = {3,6,8,4,4,2,15,4}
};


uint8_t judge_precision_second(uint16_t SrcAdd,uint8_t *m_data, uint8_t location_data, uint8_t *m_data_pri, uint8_t location_data_pri,uint8_t param_type){
    uint8_t value = 0;
    uint16_t low=0 , mid = 0 , high = 0;
	uint16_t rcv_precision = 1;
#if DEBUG	
	if(SrcAdd == 0x3150){
		value = 0;
	}
#endif	
    if(param_type == OUTDOOR_PARAM){
		    for(uint8_t i=0;i<PRECISION_DATA_TYPE_OUTDOOR;i++){

					low = mid = high = 0;//重新清除局部变量
					switch (i)
						{
							case 0:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_V;
								break;
							case 1:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_Hz;
								break;
							case 2:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_100W;
								break;
							case 3:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_B;
								break;
							case 4:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_kPa;
								break;
							case 5:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_10W;
								break;
							case 6:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_C;
								break;
							case 7:
								rcv_precision = GprsParam.bigdata_reduce_outdoor_A;
								break;
							default:
								rcv_precision = 1;
								break;
						}
		            high = (outdoor_bigdata_de_precision.cnt[i] & 0x7FFF) - 1;
		            while(low <= high && high != 0){
		                mid = (low+high) >> 1 ;		
		                if(SrcAdd == outdoor_bigdata_de_precision.src_add[i][mid]){
		//        			if(outdoor_bigdata_de_precision.second_get[i][mid] > 0){
		        	            if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 0){
									value = DEF_ABS((m_data_pri[location_data_pri+1] << 8 | m_data_pri[location_data_pri]) - (m_data[location_data+1] << 8 | m_data[location_data]));
		                             if(value >= outdoor_bigdata_de_precision.precision[i]*rcv_precision){
		                                return 1;
		                             }else{
		                                return 0;
		                             }
		        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 1){
									value = DEF_ABS((m_data_pri[location_data_pri] << 8 | (m_data_pri[(((location_data_pri-1)>=0)?(location_data_pri-1):location_data_pri)])) - (m_data[location_data] << 8 | (m_data[(((location_data-1)>=0)?(location_data-1):location_data)])));
		                             if(value >= outdoor_bigdata_de_precision.precision[i]*rcv_precision){
		                                return 1;
		                             }else{
		                                return 0;
		                             }        	            
		        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 3){
		        	                 value = DEF_ABS(m_data_pri[location_data_pri] - m_data[location_data]);
		        	                 if(value >= outdoor_bigdata_de_precision.precision[i]*rcv_precision){
		        	                     return 1;
		        	                 }else{
		        	                     return 0;
		        	                 }        	            
		        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 4){
		        	                 value = DEF_ABS(m_data_pri[location_data_pri] - m_data[location_data]);
		        	                 if(value >= 10*outdoor_bigdata_de_precision.precision[i]*rcv_precision){
		        	                     return 1;
		        	                 }else{
		        	                     return 0;
		        	                 }        	                
		        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 5){
//									value = DEF_ABS(((m_data_pri[(location_data_pri+1)]*100)/256 + (m_data_pri[location_data_pri]*100)) - ((m_data[(location_data+1)]*100)/256 + (m_data[location_data]*100)));
//		        	                 if(value >= 100*outdoor_bigdata_de_precision.precision[i]*rcv_precision){
		        	                     return 1;
//		        	                 }else{
//		        	                     return 0;
//		        	                 }        	            
		        	                
		        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 6){
		        	                return 1;
		        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 7){
		        	                 value = DEF_ABS(m_data_pri[location_data_pri] - m_data[location_data]);
		        	                 if(value >= 2*outdoor_bigdata_de_precision.precision[i]*rcv_precision){
		        	                     return 1;
		        	                 }else{
		        	                     return 0;
		        	                 }         	                
		        	            }else{
		        	            	return 1;
		        	            }
		//        			}else{
		//        				outdoor_bigdata_de_precision.second_get[i][mid]++;
		//        				return 1;
		//        			}
		                }else if(outdoor_bigdata_de_precision.src_add[i][mid] > SrcAdd){
		                    if(mid == 0){
//		                        return 0;
								break;
		                    }
		                    high =  mid - 1;
		                    if(high==0) high=1;
		                }else{
		                    if(mid < 1) mid = 1;
		                    low  = mid + 1;
		                }
		            }
		        }
    }else{
			for(uint8_t i=0;i<PRECISION_DATA_TYPE_INDOOR;i++){
				switch (i)
					{
						case 0:
							rcv_precision = GprsParam.bigdata_reduce_indoor_B;
							break;
						case 1:
							rcv_precision = GprsParam.bigdata_reduce_indoor_rpm;
							break;
						case 2:
							rcv_precision = GprsParam.bigdata_reduce_indoor_C;
							break;
						default:
							rcv_precision = 1;
							break;
					}			
					high = (indoor_bigdata_de_precision.cnt[i] & 0x7FFF) - 1;
					while(low <= high && high != 0){
						mid = (low+high) >> 1 ; 	
						if(SrcAdd == indoor_bigdata_de_precision.src_add[i][mid]){
	//						if(indoor_bigdata_de_precision.second_get[i][mid] > 0){
								if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 0){
									value = DEF_ABS((m_data_pri[location_data_pri+1] << 8 | m_data_pri[location_data_pri]) - (m_data[location_data+1] << 8 | m_data[location_data]));
									 if(value >= indoor_bigdata_de_precision.precision[i]*rcv_precision){
										return 1;
									 }else{
										return 0;
									 }
								}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 1){
									value = DEF_ABS((m_data_pri[location_data_pri] << 8 | (m_data_pri[(((location_data_pri-1)>=0)?(location_data_pri-1):location_data_pri)])) - (m_data[location_data] << 8 | (m_data[(((location_data-1)>=0)?(location_data-1):location_data)])));
									 if(value >= indoor_bigdata_de_precision.precision[i]*rcv_precision){
										return 1;
									 }else{
										return 0;
									 }						
								}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 3){
									 value = DEF_ABS(m_data_pri[location_data_pri] - m_data[location_data]);
									 if(value >= indoor_bigdata_de_precision.precision[i]*rcv_precision){
										 return 1;
									 }else{
										 return 0;
									 }						
								}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 4){
									 value = DEF_ABS(m_data_pri[location_data_pri] - m_data[location_data]);
									 if(value >= 10*indoor_bigdata_de_precision.precision[i]*rcv_precision){
										 return 1;
									 }else{
										 return 0;
									 }							
								}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 5){
//									value = DEF_ABS(((m_data_pri[(location_data_pri+1)]*100)/256 + (m_data_pri[location_data_pri]*100)) - ((m_data[(location_data+1)]*100)/256 + (m_data[location_data]*100)));
//									 if(value >= 100*indoor_bigdata_de_precision.precision[i]*rcv_precision){
										 return 1;
//									 }else{
//										 return 0;
//									 }						
									
								}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 6){
									return 1;
								}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 7){
									 value = DEF_ABS(m_data_pri[location_data_pri] - m_data[location_data]);
									 if(value >= 2*indoor_bigdata_de_precision.precision[i]*rcv_precision){
										 return 1;
									 }else{
										 return 0;
									 }							
								}else{
									return 1;
								}
	//						}else{
	//							indoor_bigdata_de_precision.second_get[i][mid]++;
	//							return 1;
	//						}
						}else if(indoor_bigdata_de_precision.src_add[i][mid] > SrcAdd){
							if(mid == 0){
//								return 0;
								break;
							}
							high =	mid - 1;
							if(high==0) high=1;
						}else{
							if(mid < 1) mid = 1;
							low  = mid + 1;
						}
					}
				}
	    }	
    return 1;   
}


uint8_t bigdata_judge_precision(uint16_t SrcAdd,const CanMsgType *m_Msg, uint8_t location_can, uint8_t *m_data, uint8_t location_data,uint8_t param_type){
    uint8_t value = 0;
    uint16_t low=0 , mid = 0 , high = 0;
	uint16_t rcv_precision = 1;
    if(param_type == OUTDOOR_PARAM){
    for(uint8_t i=0;i<PRECISION_DATA_TYPE_OUTDOOR;i++){
		
			switch (i)
				{
					case 0:
						rcv_precision = GprsParam.bigdata_reduce_outdoor_V;
						break;
					case 1:
						rcv_precision = GprsParam.bigdata_reduce_outdoor_Hz;
						break;
					case 2:
						rcv_precision = GprsParam.bigdata_reduce_outdoor_100W;
						break;
					case 3:
						rcv_precision = GprsParam.bigdata_reduce_indoor_B;
						break;
					case 4:
						rcv_precision = GprsParam.bigdata_reduce_outdoor_kPa;
						break;
					case 5:
						rcv_precision = GprsParam.bigdata_reduce_outdoor_10W;
						break;
					case 6:
						rcv_precision = GprsParam.bigdata_reduce_outdoor_C;
						break;
					case 7:
						rcv_precision = GprsParam.bigdata_reduce_outdoor_A;
						break;
					default:
						rcv_precision = 1;
						break;
				}
            high = (outdoor_bigdata_de_precision.cnt[i] & 0x7FFF) - 1;
            while(low <= high && high != 0){
                mid = (low+high) >> 1 ;		
                if(SrcAdd == outdoor_bigdata_de_precision.src_add[i][mid]){
//        			if(outdoor_bigdata_de_precision.second_get[i][mid] > 0){
        	            if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 0){
							value = DEF_ABS((m_Msg->Data[(((location_can+1)<(m_Msg->Lenth))?(location_can+1):location_can)] << 8 | m_Msg->Data[location_can]) - (m_data[((location_can+1)<(m_Msg->Lenth))?(location_data+1):location_data] << 8 | m_data[location_data]));
                             if(value >= outdoor_bigdata_de_precision.precision[i]*rcv_precision){
                                return 1;
                             }else{
                                return 0;
                             }
        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 1){
							value = DEF_ABS((m_Msg->Data[location_can] << 8 | (m_Msg->Data[((location_can-1)>=0 ? (location_can-1):location_can)])) - (m_data[location_data] << 8 | (m_data[(((location_data-1)>=0)?(location_data-1):location_data)])));
                             if(value >= outdoor_bigdata_de_precision.precision[i]*rcv_precision){
                                return 1;
                             }else{
                                return 0;
                             }        	            
        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 3){
        	                 value = DEF_ABS(m_Msg->Data[location_can] - m_data[location_data]);
        	                 if(value >= outdoor_bigdata_de_precision.precision[i]*rcv_precision){
        	                     return 1;
        	                 }else{
        	                     return 0;
        	                 }        	            
        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 4){
        	                 value = DEF_ABS(m_Msg->Data[location_can] - m_data[location_data]);
        	                 if(value >= 10*outdoor_bigdata_de_precision.precision[i]*rcv_precision){
        	                     return 1;
        	                 }else{
        	                     return 0;
        	                 }        	                
        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 5){
							value = DEF_ABS(((m_Msg->Data[location_can]*100)/256 + ((m_Msg->Data[((location_can+1)<m_Msg->Lenth)?(location_can+1):location_can]*100))) - ((m_data[((location_can+1)<m_Msg->Lenth)?(location_data+1):location_data]*100)/256 + (m_data[location_data]*100)));
        	                 if(value >= 100*outdoor_bigdata_de_precision.precision[i]*rcv_precision){
        	                     return 1;
        	                 }else{
        	                     return 0;
        	                 }        	            
        	                
        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 6){
        	                return 0;
        	            }else if(outdoor_bigdata_de_precision.high_low_flag[i][mid] == 7){
        	                 value = DEF_ABS(m_Msg->Data[location_can] - m_data[location_data]);
        	                 if(value >= 2*outdoor_bigdata_de_precision.precision[i]*rcv_precision){
        	                     return 1;
        	                 }else{
        	                     return 0;
        	                 }         	                
        	            }
//        			}else{
//        				outdoor_bigdata_de_precision.second_get[i][mid]++;
//        				return 1;
//        			}
                }else if(outdoor_bigdata_de_precision.src_add[i][mid] > SrcAdd){
                    if(mid == 0){
                        return 1;
                    }
                    high =  mid - 1;
                    if(high==0) high=1;
                }else{
                    if(mid < 1) mid = 1;
                    low  = mid + 1;
                }
            }
        }
    }else{
		for(uint8_t i=0;i<PRECISION_DATA_TYPE_INDOOR;i++){
			switch (i)
				{
					case 0:
						rcv_precision = GprsParam.bigdata_reduce_indoor_B;
						break;
					case 1:
						rcv_precision = GprsParam.bigdata_reduce_indoor_rpm;
						break;
					case 2:
						rcv_precision = GprsParam.bigdata_reduce_indoor_C;
						break;
					default:
						rcv_precision = 1;
						break;
				}			
				high = (indoor_bigdata_de_precision.cnt[i] & 0x7FFF) - 1;
				while(low <= high && high != 0){
					mid = (low+high) >> 1 ; 	
					if(SrcAdd == indoor_bigdata_de_precision.src_add[i][mid]){
//						if(indoor_bigdata_de_precision.second_get[i][mid] > 0){
							if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 0){
								value = DEF_ABS((m_Msg->Data[(((location_can+1)<(m_Msg->Lenth))?(location_can+1):location_can)] << 8 | m_Msg->Data[location_can]) - (m_data[((location_can+1)<(m_Msg->Lenth))?(location_data+1):location_data] << 8 | m_data[location_data]));
								 if(value >= indoor_bigdata_de_precision.precision[i]*rcv_precision){
									return 1;
								 }else{
									return 0;
								 }
							}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 1){
								value = DEF_ABS((m_Msg->Data[location_can] << 8 | (m_Msg->Data[((location_can-1)>=0 ? (location_can-1):location_can)])) - (m_data[location_data] << 8 | (m_data[(((location_data-1)>=0)?(location_data-1):location_data)])));
								 if(value >= indoor_bigdata_de_precision.precision[i]*rcv_precision){
									return 1;
								 }else{
									return 0;
								 }						
							}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 3){
								 value = DEF_ABS(m_Msg->Data[location_can] - m_data[location_data]);
								 if(value >= indoor_bigdata_de_precision.precision[i]*rcv_precision){
									 return 1;
								 }else{
									 return 0;
								 }						
							}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 4){
								 value = DEF_ABS(m_Msg->Data[location_can] - m_data[location_data]);
								 if(value >= 10*indoor_bigdata_de_precision.precision[i]*rcv_precision){
									 return 1;
								 }else{
									 return 0;
								 }							
							}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 5){
								value = DEF_ABS(((m_Msg->Data[location_can]*100)/256 + ((m_Msg->Data[((location_can+1)<m_Msg->Lenth)?(location_can+1):location_can]*100))) - ((m_data[((location_can+1)<m_Msg->Lenth)?(location_data+1):location_data]*100)/256 + (m_data[location_data]*100)));
								 if(value >= 100*indoor_bigdata_de_precision.precision[i]*rcv_precision){
									 return 1;
								 }else{
									 return 0;
								 }						
								
							}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 6){
								return 0;
							}else if(indoor_bigdata_de_precision.high_low_flag[i][mid] == 7){
								 value = DEF_ABS(m_Msg->Data[location_can] - m_data[location_data]);
								 if(value >= 2*indoor_bigdata_de_precision.precision[i]*rcv_precision){
									 return 1;
								 }else{
									 return 0;
								 }							
							}
//						}else{
//							indoor_bigdata_de_precision.second_get[i][mid]++;
//							return 1;
//						}
					}else if(indoor_bigdata_de_precision.src_add[i][mid] > SrcAdd){
						if(mid == 0){
							return 1;
						}
						high =	mid - 1;
						if(high==0) high=1;
					}else{
						if(mid < 1) mid = 1;
						low  = mid + 1;
					}
				}
			}
    }

    return 1;   
}


uint8_t judge_precision(uint16_t SrcAdd, uint8_t lnewvalue, uint8_t loldvalue, uint8_t hnewvalue, uint8_t holdvalue){
    uint8_t value = 0;
    uint16_t low=0 , mid = 0 , high = (precision_data.cnt & 0x7FFF) - 1; 
    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;		
        if(SrcAdd == precision_data.src_add[mid]){
			if(precision_data.second_get[mid] > 0){
	            if(precision_data.high_low_flag[mid] == 0){  //为0的是总线上数据扩大10倍处理的
	                 value = DEF_ABS(lnewvalue - loldvalue);
	                 if(value >= 10){
	                     return 1;
	                 }else{
	                     return 0;
	                 }
	            }else if(precision_data.high_low_flag[mid] == 1){   //存在小数整数部分,且小数部分扩大256倍
	                 value = DEF_ABS(((lnewvalue*100)/256 + hnewvalue*100) - ((loldvalue*100)/256 + holdvalue*100));
	                 if(value >= 100){
	                     return 1;
	                 }else{
	                     return 0;
	                 }
	            }else if(precision_data.high_low_flag[mid] == 2){  
	                return 0;
	            }
			}else{
				precision_data.second_get[mid]++;
				return 1;
			}
        }else if(precision_data.src_add[mid] > SrcAdd){
            if(mid == 0){
                return 1;
            }
            high =  mid - 1;
            if(high==0) high=1;
        }else{
            if(mid < 1) mid = 1;
            low  = mid + 1;
        }
    }
    return 1;   
}
 
#ifdef USING_USER_HABIT
//用户习惯的判断及存储
void user_habit(const SrcDataTyp  *Page, uint8_t *m_Data, const CanMsgType *m_Msg, indoorinfoType * indoorinfo, uint8_t bit_location){
    uint8_t i = 0,value = 0,get_high_bit=0; 
    for(i = 0;i < user_habit_data.cnt;i++){
        if(Page->SrcAdd == user_habit_data.src_add[i]){
			//室内温度
            user_data_fill(INDOOR_TMP_ADDR, m_Data[36], indoorinfo, REPEAT_YES, END_FLAG_NO);//上传改变的值加室内温度和室外温度
            value = m_Data[Page->Value >> 4];
			
            if(user_habit_data.high_low_flag[i] == 0x00){ 
                user_data_fill(Page->SrcAdd, value, indoorinfo, REPEAT_NO, END_FLAG_NO);//m_Data[Page->Value >> 4]
            }else if(user_habit_data.high_low_flag[i] == 0x01){
            	if(m_Msg->Lenth >= 0x02){
	                user_data_fill(Page->SrcAdd, value,indoorinfo, REPEAT_NO, END_FLAG_NO);
	                value = m_Data[(Page->Value >> 4)+1];
	                user_data_fill(Page->SrcAdd + 1, value, indoorinfo, REPEAT_NO, END_FLAG_NO);
					get_high_bit = 1;
            	}else{
	                user_data_fill(Page->SrcAdd, value,indoorinfo, REPEAT_NO, END_FLAG_NO);
	                value = m_Data[(Page->Value >> 4)+1];
	                user_data_fill(Page->SrcAdd + 1, value, indoorinfo, REPEAT_NO, END_FLAG_NO);
            	}
            }else if(user_habit_data.high_low_flag[i] == 0x02 && !get_high_bit){
                user_data_fill(Page->SrcAdd, value, indoorinfo, REPEAT_NO, END_FLAG_NO);
                value = m_Data[(Page->Value >> 4)-1];
                user_data_fill(Page->SrcAdd - 1, value, indoorinfo, REPEAT_NO, END_FLAG_NO);
            }
        }
    }
}

#endif

void take_photo(uint16_t SrcAdd, uint8_t value, outdoorinfoType *outdoorinfo, indoorinfoType *indoorinfo, uint8_t paramtype,uint8_t over_flag){
		uint8_t err;
		uint16_t re_data = 0xFFFF;
		if(g_Upload.TakeDataLen < 2){
			g_Upload.TakeDataLen = 2;
		}
		if((g_Upload.TakeDataLen >= 2)&&(g_Upload.TakeDataLen < 6)){			
			memcpy(&g_Upload.TakeBuffer[2] , &unixTimeStamp , 4);
			g_Upload.TakeDataLen += 4;
			if(paramtype == OUTDOOR_PARAM){ //外机
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t)(outdoorinfo->Can1Can2IP&0xFF);//Can1 
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = ((uint8_t) (outdoorinfo->Can1Can2IP>>8)&0xFF);//Can2
				memcpy(&g_Upload.TakeBuffer[g_Upload.TakeDataLen], outdoorinfo->MAC, 6);//mac
				g_Upload.TakeDataLen += 6;				  
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = 0xFF;//工程编号1 	
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = 0xFF;//工程编号2
				memcpy(&g_Upload.TakeBuffer[g_Upload.TakeDataLen],&machineID,2);//机型ID
				g_Upload.TakeDataLen += 2;
			}else if(paramtype == INDOOR_PARAM){//内机
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t) indoorinfo->Can1Can2IP & 0xFF;//Can1 
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = ((uint8_t) (indoorinfo->Can1Can2IP>>8) & 0xFF);//Can2
				memcpy(&g_Upload.TakeBuffer[g_Upload.TakeDataLen], indoorinfo->MAC, 6);
				g_Upload.TakeDataLen += 6;				
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t)indoorinfo->number&0xFF; //工程编号
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t)(indoorinfo->number>>8 & 0xFF);//工程编号
				re_data = GetDataAddrr((uint16_t)0x1004,&PageData[1]);						
				if(re_data != 0xFFFF){
					re_data = ((re_data&0xFFF)>>4);
					memcpy(&g_Upload.TakeBuffer[g_Upload.TakeDataLen], &ACParam.Indoor[(uint8_t)outdoorinfo][re_data], 2);
					g_Upload.TakeDataLen += 2;
				}else{
					g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = 0;
					g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = 0;
				}

				
			}
		}	 
	
		if(!over_flag && ((g_Upload.TakeDataLen >= 18) && (g_Upload.TakeDataLen < 950))){
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t)(SrcAdd & 0xFF);//行
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t)(SrcAdd >> 8)&0xFF;	//列
//				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t)(SrcAdd & 0xFF);//行
//				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = (uint8_t)(SrcAdd >> 8)&0xFF;					 
				g_Upload.TakeBuffer[g_Upload.TakeDataLen++] = value;
		}
		if(((g_Upload.TakeDataLen >= 950) && (g_Upload.TakeDataLen <= 1024)) || over_flag){
			OSMutexPend(MutexSendData , 0 , &err);
			memcpy(&g_Upload.TakeBuffer[0] , &g_Upload.TakeDataLen , 2);				
			if((g_Upload.per_30_min || g_Upload.per_config_min) && take_photo_style){
//						OSMutexPend(MutexSendData , 0 , &err);
				memcpy(SendBuffer[sendCnt].DataBuf , g_Upload.TakeBuffer , g_Upload.TakeDataLen);
				SendBuffer[sendCnt].data_type = 1;
//				SendBuffer[sendCnt].DataLen = g_Upload.TakeDataLen | 0x8000;  //强制标记接收完成
//				OSMutexPost(MutexSendData);
				err = OSQPost(QSemSend,(void *)&SendBuffer[sendCnt]);
				if(OS_ERR_NONE == err){
					SendBuffer[sendCnt].DataLen = g_Upload.TakeDataLen | 0x8000;  //强制标记接收完成
					sendCnt++;
				}
				if(sendCnt >= SEND_DATA_BUFF_SIZE){
					sendCnt = 0;
				}
			}
			
			g_Upload.data_type = 1;
			WriteData2Flash(&g_Upload,TAKEPHOTO_MODE_FILL);
			g_Upload.data_type = 0;
			g_Upload.TakeDataLen  = 0;
			OSMutexPost(MutexSendData);
		}else if(g_Upload.TakeDataLen >= 1024){
			g_Upload.TakeDataLen  = 0;
		}

}


void take_a_photo(const PageTpye *PageData ,uint8_t *m_Data, uint8_t *d_Changflag, outdoorinfoType *outdoorinfo, indoorinfoType *indoorinfo,uint8_t param_type){
	uint8_t  value = 0;
	uint16_t j = 0;
	for(j=0; j<PageData->TableSize; j++){
		if(d_Changflag[j/8]&(1<<(j%8))){	//判断是否收到过这个参数				  
			value = m_Data[PageData->Page[j].Value >> 4];
			if(j != (PageData->TableSize - 1)){
				take_photo(PageData->Page[j].SrcAdd, value, outdoorinfo, indoorinfo, param_type,RECV_STATE);	 //非最后一个参数收到过0x00
			}
		}					
	}
	take_photo(PageData->Page[j].SrcAdd, value, outdoorinfo, indoorinfo, param_type,RECV_OVER);  //非最后一个参数收到过0x00
}

void Quick_Photo(){
	uint8_t err = 0,res = 0;
//	OSTaskChangePrio(APP_TASK_GPRS_PRIO,15);
//	sendbuff_flag = 0;
	if(take_photo_style){
//		OSQFlush(QSemSend);
		res = (uint8_t)((uint32_t)OSMboxPend( MSemQuickPhoto , 4000, &err ) - 0);
	}
	take_a_photo(&PageData[0] , ACParam.System, ACParam_Existflag.System, &outdoorinfo, NULL, OUTDOOR_PARAM);//数据类型还未嵌入
	
	if(take_photo_style){
		res = (uint8_t)((uint32_t)OSMboxPend( MSemSavePhoto , 1000, &err ) - 0);
	}
	
	for(int i=0;i<INDOOR_MCH_NUM;i++){
		if(indoorinfo[i].online_flag){
			if(dbgPrintf)(*dbgPrintf)("Now is num:%d indoor take-photo data...\n",i);
			take_a_photo(&PageData[1] , ACParam.Indoor[i], ACParam_Existflag.Indoor[i],(void *)i, &indoorinfo[i], INDOOR_PARAM);
			if(take_photo_style){
				res = (uint8_t)((uint32_t)OSMboxPend( MSemSavePhoto , 1000, &err ) - 0);
			}
		}
	}
	
	g_Upload.per_30_min = 0;
	g_Upload.per_config_min = 0;
//	g_Upload.flag_30 = 0;
//	g_Upload.flag_5 = 0;
	take_photo_style = 0;
    OSMboxPostOpt(MSemExitWait,(void *)0xAA,OS_POST_OPT_BROADCAST);
//	if(var.buttonPush || var.projDebug || var.realMonitor){
//		sendbuff_flag = 1;
//	}
//	OSTaskChangePrio(15,APP_TASK_GPRS_PRIO);
}


uint8_t Find96Take(uint16_t SrcAdd, const Real_PageTpye *real_PageData){
	uint16_t low=0 , mid = 0 , high= ((real_PageData->cnt) & 0xFFFF) - 1; 
	while(low <= high && high != 0){
		mid = (low+high) >> 1 ;
		if(SrcAdd == real_PageData->SrcAdd[mid]){
			return 1;
		}else if(real_PageData->SrcAdd[mid] > SrcAdd){
            if(!mid){
                break;
            }
			high =	mid - 1;
			if(high==0) high=1;
		}else{
			if(mid < 1) mid = 1;
			low  = mid + 1;
		}
	}
	return 0;
}

#ifdef USING_USER_HABIT

void user_data_fill(uint16_t SrcAdd, uint8_t value, indoorinfoType *indoorinfo, uint8_t repl, uint8_t endflag)
{
    uint8_t err;
	static uint32_t unix_time = 0;
    if(RealBuffer.DataLen < 2){
        RealBuffer.DataLen += 2;
    }

    if((RealBuffer.DataLen >= 2)&&(RealBuffer.DataLen < 6)){
        user_pack_time = 0x8000;
		unix_time = getUnixTimeStamp();
	    memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen] , &unixTimeStamp , 4);
	    RealBuffer.DataLen += 4;               
    }
	
    if((RealBuffer.DataLen >= 6)&&(RealBuffer.DataLen < 800) || repl){

			RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(unixTimeStamp - unix_time);
	        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen], indoorinfo->MAC, 6);
	        RealBuffer.DataLen += 6;
            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(SrcAdd >> 8)&0xFF;
            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(SrcAdd & 0xFF);                   
            RealBuffer.DataBuf[RealBuffer.DataLen++] = value;
    }
	
    if((RealBuffer.DataLen >= 800) && (RealBuffer.DataLen <= 1024) || endflag){
        // 室外温度
//        user_data_fill(OUTDOOR_TMP_ADDR, ACParam.System[106],indoorinfo, REPEAT_NO, END_FLAG_NO);//上传改变的值加室内温度和室外温度
        RealBuffer.DataBuf[RealBuffer.DataLen++] = 0x20;
		RealBuffer.DataBuf[RealBuffer.DataLen++] = 0x5F;
		RealBuffer.DataBuf[RealBuffer.DataLen++] = ACParam.System[106];
	
        memcpy(&RealBuffer.DataBuf[0] , &RealBuffer.DataLen , 2);    
        if(!(user_data.write_addr % SUB_SECTOR_SIZE)){
	        if(user_data.write_addr >= USERHABIT_END_ADDR){
	            user_data.write_addr = USERHABIT_START_ADDR;
	        }
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(user_data.write_addr);         //擦除一个扇区，大小4KB
            OSMutexPost(MutexFlash);
        }
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_WriteBuffer(&RealBuffer.DataBuf[0] , user_data.write_addr, RealBuffer.DataLen);        
        OSMutexPost(MutexFlash);
        
        user_data.write_addr += CAN_WRITE_SIZE;
        
        if(user_data.write_addr >= USERHABIT_END_ADDR){
            user_data.write_addr = USERHABIT_START_ADDR;
        }       
        
        RealBuffer.DataLen  = 0x0000;
    }else if(RealBuffer.DataLen >= 1024){
        RealBuffer.DataLen  = 0;
    }    
}


uint8_t read_user_data(){
    uint8_t err;
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_ReadBuffer((uint8_t*)&g_Upload.ErrDataLen, user_data.read_addr , 2);
    if(g_Upload.ErrDataLen <= 1024){
        sFLASH_ReadBuffer(g_Upload.ErrBuffer, user_data.read_addr,g_Upload.ErrDataLen);
    }else{
        g_Upload.ErrDataLen  = 0;
		if(user_data.read_addr >= user_data.write_addr){
		    OSMutexPost(MutexFlash);
			return 1u;
		}else{
		    user_data.read_addr += CAN_WRITE_SIZE;
		    if(user_data.read_addr >= USERHABIT_END_ADDR){
		        user_data.read_addr  = USERHABIT_START_ADDR;
		    }
		    OSMutexPost(MutexFlash);
			return 3u;
		}
    }
    OSMutexPost(MutexFlash);

//	if(user_data.read_addr == user_data.write_addr){
//		return 2u;
//	}
    
    user_data.read_addr += CAN_WRITE_SIZE;
    if(user_data.read_addr >= USERHABIT_END_ADDR){
        user_data.read_addr  = USERHABIT_START_ADDR;
    }else{
		if(user_data.read_addr >= user_data.write_addr){
			return 2u;
		}    	
    }
	
    return 0u;
}


#endif



