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
#include "fsm.h"
#include "Duplicate.h"
#include "real_handle.h"

Real_PageTpye   Receive_Real_Page =
{
	.SrcAdd = {
	  0x1004, 0x1005, 0x1006, 0x100E, 0x100F, 0x1010, 0x1011, 0x1012, 0x1013, 0x101D,
	  0x101E, 0x1108, 0x1109, 0x111D, 0x1132, 0x1136, 0x113A, 0x113B, 0x113C, 0x113D,
	  0x113E, 0x120D, 0x120E, 0x120F, 0x1210, 0x1211, 0x1212, 0x1213, 0x1214, 0x1215, 
	  0x1216, 0x1217, 0x1218, 0x1219, 0x121A, 0x121B, 0x1351, 0x1C50, 0x1C51, 0x1C58,
	  0x1C5B, 0x1D6A, 0x1D71, 0x1D75, 0x1D88, 0x1D89, 0x1D8A, 0x1D8B, 0x1D8C, 0x1D8D, 
	  0x1D90, 0x1D93, 0x1E03, 0x1E04, 0x1E05, 0x1E06, 0x1E07, 0x1E09, 0x1E0C, 0x1E0E,
	  0x1E0F, 0x1E13, 0x1E15, 0x1E16, 0x1E17, 0x1E18, 0x1E1E, 0x1E1F, 0x1E20, 0x1E21, 
	  0x1E23, 0x1E24, 0x1E2F, 0x1E32, 0x1E35, 0x1E51, 0x1E53, 0x1E55, 0x1E56, 0x1E59,
	  0x1E5A, 0x1E5B, 0x1E5D, 0x1E60, 0x1E63, 0x1E64, 0x1E65, 0x1E68, 0x1E70, 0x1E71, 
	  0x1E72, 0x1E74, 0x1E75, 0x1E79, 0x1E7A, 0x1E7B, 0x1E80, 0x1E89, 0x1E8A, 0x1E90,
	  0x1E91, 0x1E92, 0x1E98, 0x1E99, 0x1E9A, 0x1E9B, 0x1EA5, 0x1EAB, 0x1EB8, 0x1EB9, 
	  0x1EBA, 0x1EBB, 0x1EBC, 0x1EBD, 0x1EBE, 0x1EBF, 0x1EC0, 0x1EC2, 0x1EC3, 0x1EC5,
	  0x1EC7, 0x1ECA, 0x1ECB, 0x1EDE, 0x1F00, 0x1F01, 0x1F02, 0x1F04, 0x1F0C, 0x1F0D, 
	  0x1F16, 0x1F18, 0x1F1B, 0x1F1C, 0x1F37, 0x1F38, 0x1F51, 0x1F54, 0x1F58, 0x1F59,
	  0x1F5C, 0x1F5D, 0x1F68, 0x1F78, 0x1F79, 0x1F7A, 0x1F88, 0x1F8A, 0x2000, 0x2001, 
	  0x2003, 0x2005, 0x2006, 0x2007, 0x2009, 0x200B, 0x200C, 0x2057, 0x205F, 0x2061,
	  0x2062, 0x2063, 0x2064, 0x2069, 0x206A, 0x2073, 0x207C, 0x207D, 0x2085, 0x2086, 
	  0x2100, 0x2101, 0x2105, 0x2106, 0x2107, 0x2109, 0x210A, 0x210C, 0x2110, 0x212E,
	  0x212F, 0x2152, 0x2154, 0x2155, 0x2156, 0x2157, 0x215B, 0x2161, 0x2761, 0x2765, 
	  0x2766, 0x2767, 0x2768, 0x2850, 0x2851, 0x2854, 0x285F, 0x2860, 0x2861, 0x2863,
	  0x2882, 0x2883, 0x2889, 0x288C, 0x288D, 0x288E, 0x288F, 0x2895, 0x2898, 0x2899, 
	  0x28A1, 0x3100, 0x3106, 0x3107, 0x3108, 0x3109, 0x3110, 0x3111, 0x3116, 0x3117,
	  0x3118, 0x3119, 0x3150, 0x3151, 0x3152, 0x3153, 0x3154, 0x3155, 0x3156, 0x3157, 
	  0x315A, 0x315B, 0x3185, 0x3186, 0x3187, 0x3189, 0x318A, 0x318B, 0x318C, 0x318D,
	  0x318E, 0x3200, 0x3201, 0x3202, 0x3203, 0x3208, 0x3209, 0x320A, 0x320B, 0x320C, 
	  0x320D, 0x3250, 0x3251, 0x325C, 0x325D, 0x325E, 0x325F, 0x5355, 0x5356, 0x5357,
	  0x5358, 0x535A, 0x535B, 0x535E, 0x5362, 0x539C, 0x53AC, 0x5456, 0x5457, 0x5458, 
	  0x5459, 0x7100, 0x7300, 0x730C, 0x730D
	  },
	.hl_flag = { 
	  0x01,   0x02,   0x00,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   
	  0x02,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x01,   0x02,   0x01,   0x02,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x03,
	  0x04,   0x05,   0x06,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x01,   0x02,   0x00,   0x01,   0x02,   0x01,   0x02,   0x00,   0x01,   0x02,
	  0x00,   0x00,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,
	  0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,
	  0x01,   0x02,   0x01,   0x02,   0x00,   0x01,   0x02,   0x01,   0x02,   0x01,
	  0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,
	  0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x02,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	  0x00,   0x00,   0x00,   0x01,   0x02,
	  },
	.cnt = {275}
};
BUFF_TYPE RealBuffer;
BUFF_TYPE RealBuffer_time_L30;//拍照上报配置时间小于30

const Real_PageTpye Real_Page = 
{
  .SrcAdd = {
  	0x1004, 0x1005, 0x1006, 0x100E, 0x100F, 0x1010, 0x1011, 0x1012, 0x1013, 0x101D,
	0x101E, 0x1108, 0x1109, 0x111D, 0x1132, 0x1136, 0x113A, 0x113B, 0x113C, 0x113D, 
	0x113E, 0x120D, 0x120E, 0x120F, 0x1210, 0x1211, 0x1212, 0x1213, 0x1214, 0x1215, 
	0x1216, 0x1217, 0x1218, 0x1219, 0x121A, 0x121B, 0x1351, 0x1C50, 0x1C51, 0x1C58, 
	0x1C5B, 0x1D6A, 0x1D71, 0x1D75, 0x1D88, 0x1D89, 0x1D8A, 0x1D8B, 0x1D8C, 0x1D8D, 
	0x1D90, 0x1D93, 0x1E03, 0x1E04, 0x1E05, 0x1E06, 0x1E07, 0x1E09, 0x1E0C, 0x1E0E, 
	0x1E0F, 0x1E13, 0x1E15, 0x1E16, 0x1E17, 0x1E18, 0x1E1E, 0x1E1F, 0x1E20, 0x1E21, 
	0x1E23, 0x1E24, 0x1E2F, 0x1E32, 0x1E35, 0x1E51, 0x1E53, 0x1E55, 0x1E56, 0x1E59, 
	0x1E5A, 0x1E5B, 0x1E5D, 0x1E60, 0x1E63, 0x1E64, 0x1E65, 0x1E68, 0x1E70, 0x1E71, 
	0x1E72, 0x1E74, 0x1E75, 0x1E79, 0x1E7A, 0x1E7B, 0x1E80, 0x1E89, 0x1E8A, 0x1E90, 
	0x1E91, 0x1E92, 0x1E98, 0x1E99, 0x1E9A, 0x1E9B, 0x1EA5, 0x1EAB, 0x1EB8, 0x1EB9, 
	0x1EBA, 0x1EBB, 0x1EBC, 0x1EBD, 0x1EBE, 0x1EBF, 0x1EC0, 0x1EC2, 0x1EC3, 0x1EC5, 
	0x1EC7, 0x1ECA, 0x1ECB, 0x1EDE, 0x1F00, 0x1F01, 0x1F02, 0x1F04, 0x1F0C, 0x1F0D, 
	0x1F16, 0x1F18, 0x1F1B, 0x1F1C, 0x1F37, 0x1F38, 0x1F51, 0x1F54, 0x1F58, 0x1F59, 
	0x1F5C, 0x1F5D, 0x1F68, 0x1F78, 0x1F79, 0x1F7A, 0x1F88, 0x1F8A, 0x2000, 0x2001, 
	0x2003, 0x2005, 0x2006, 0x2007, 0x2009, 0x200B, 0x200C, 0x2057, 0x205F, 0x2061, 
	0x2062, 0x2063, 0x2064, 0x2069, 0x206A, 0x2073, 0x207C, 0x207D, 0x2085, 0x2086, 
	0x2100, 0x2101, 0x2105, 0x2106, 0x2107, 0x2109, 0x210A, 0x210C, 0x2110, 0x212E, 
	0x212F, 0x2152, 0x2154, 0x2155, 0x2156, 0x2157, 0x215B, 0x2161, 0x2761, 0x2765, 
	0x2766, 0x2767, 0x2768, 0x2850, 0x2851, 0x2854, 0x285F, 0x2860, 0x2861, 0x2863, 
	0x2882, 0x2883, 0x2889, 0x288C, 0x288D, 0x288E, 0x288F, 0x2895, 0x2898, 0x2899, 
	0x28A1, 0x3100, 0x3106, 0x3107, 0x3108, 0x3109, 0x3110, 0x3111, 0x3116, 0x3117,
	0x3118, 0x3119, 0x3150, 0x3151, 0x3152, 0x3153, 0x3154, 0x3155, 0x3156, 0x3157, 
	0x315A, 0x315B, 0x3185, 0x3186, 0x3187, 0x3189, 0x318A, 0x318B, 0x318C, 0x318D, 
	0x318E, 0x3200, 0x3201, 0x3202, 0x3203, 0x3208, 0x3209, 0x320A, 0x320B, 0x320C, 
	0x320D, 0x3250, 0x3251, 0x325C, 0x325D, 0x325E, 0x325F, 0x5355, 0x5356, 0x5357, 
	0x5358, 0x535A, 0x535B, 0x535E, 0x5362, 0x539C, 0x53AC, 0x5456, 0x5457, 0x5458, 
	0x5459, 0x7100, 0x7300, 0x730C, 0x730D,
    },
  .hl_flag = { 
	0x01,   0x02,   0x00,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   
	0x02,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x01,   0x02,   0x01,   0x02,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x03,
	0x04,   0x05,   0x06,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x01,   0x02,   0x00,   0x01,   0x02,   0x01,   0x02,   0x00,   0x01,   0x02,
	0x00,   0x00,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,
	0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,
	0x01,   0x02,   0x01,   0x02,   0x00,   0x01,   0x02,   0x01,   0x02,   0x01,
	0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x01,
	0x02,   0x01,   0x02,   0x01,   0x02,   0x01,   0x02,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x02,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
	0x00,   0x00,   0x00,   0x01,   0x02,
    },
  .cnt = {275}
};

uint16_t checkDataAddrr(uint16_t addr,  const Real_PageTpye *real_PageData)
{
    uint16_t low=0 , mid = 0 , high= ((real_PageData->cnt) & 0x7FFF) - 1; 
    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;
        if(addr == real_PageData->SrcAdd[mid]){
            return real_PageData->hl_flag[mid];
        }else if(real_PageData->SrcAdd[mid] > addr){
          if(!mid){
                break;
            }
            high =  mid - 1;
            if(high==0)high=1;                                               
        }else{
            if(mid < 1)mid = 1;
            low  = mid + 1;
        }
    }
    return 0xFFFF;                                                           //返回值最高位置1，表示没有找到值
}

//uint8_t Find96Take(uint16_t SrcAdd, const Real_PageTpye *real_PageData){
//	uint16_t low=0 , mid = 0 , high= ((real_PageData->cnt) & 0xFF) - 1; 
//	while(low <= high && high != 0){
//		mid = (low+high) >> 1 ;
//		if(SrcAdd == real_PageData->SrcAdd[mid]){
//			return 1;
//		}else if(real_PageData->SrcAdd[mid] > SrcAdd){
//			high =	mid - 1;
//			if(high==0) high=1;
//		}else{
//			if(mid < 1) mid = 1;
//			low  = mid + 1;
//		}
//	}
//	return 0;
//}


void sendreal_data(void)
{
    uint16_t i,j,re_data,re_data1;
    for(i=0; i<INDOOR_MCH_NUM; i++){//内机数据
        if(indoorinfo[i].online_flag && (var.realMonitor)){
            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen], indoorinfo[i].MAC, 6); //MAC码
            RealBuffer.DataLen += 6;
            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t) indoorinfo[i].number&0xFF;//工程编号
            RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;
            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t) indoorinfo[i].Can1Can2IP&0xFF;//Can1
            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t) (indoorinfo[i].Can1Can2IP>>8)&0xFF;//Can2
            
            re_data = GetDataAddrr((uint16_t)0x1004,&PageData[1]);//机型ID
            
            if(re_data != 0xFFFF){
                re_data = ((re_data&0xFFF)>>4);
                memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen], &ACParam.Indoor[i][re_data], 2);
                RealBuffer.DataLen += 2;
            }else{
                RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;
                RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;
            }
            
            for(j=0; j<Receive_Real_Page.cnt; j++){
                  re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j], ACParam.Indoor[i]);//
                if(!(re_data&0x8000)){                                                    //有有效的返回
                    if(!Receive_Real_Page.hl_flag[j]){
                        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                        RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                        RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF);                                            //将值放到数组中
                    }else if(Receive_Real_Page.hl_flag[j] == 1){       //找到低位变化，同时将高位放到数据
                        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                        RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                        RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        
                        re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j+1], ACParam.Indoor[i]);
                        if(!(re_data&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+1],sizeof(Receive_Real_Page.SrcAdd[j+1]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+1]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        }
                        j++;
                    }else if(Receive_Real_Page.hl_flag[j] == 2){     //找到高位变化，同时将低位放到数据
                      re_data1 = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j-1], ACParam.Indoor[i]);
                        if(!(re_data1&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j-1],sizeof(Receive_Real_Page.SrcAdd[j-1]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j-1]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data1 & 0xFF); 
                        }
                      
                        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                        RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                        RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                    }else if(Receive_Real_Page.hl_flag[j] == 3){
                        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                        RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                        RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        
                        re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j+1], ACParam.Indoor[i]);
                        if(!(re_data&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+1],sizeof(Receive_Real_Page.SrcAdd[j+1]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+1]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        }
                        re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j+2], ACParam.Indoor[i]);
                        if(!(re_data&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+2],sizeof(Receive_Real_Page.SrcAdd[j+2]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+2]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        }
                        re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j+3], ACParam.Indoor[i]);
                        if(!(re_data&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+3],sizeof(Receive_Real_Page.SrcAdd[j+3]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+3]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        }
						
                        j += 3;
                    }
                }              
            }
            send98Frame(&g_TcpType ,RealBuffer.DataBuf, RealBuffer.DataLen,0);
            RealBuffer.DataLen = 0;
        }
    }
	
    if((outdoorinfo.online_flag==0x03)&&(var.realMonitor == 1)){
      
        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen], outdoorinfo.MAC, 6); //MAC码
        RealBuffer.DataLen += 6;
        
        RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;//工程编号
        RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;
        RealBuffer.DataBuf[RealBuffer.DataLen++] = 8;//Can1
        RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;//Can2
        
         re_data = GetDataAddrr((uint16_t)0x1004,&PageData[0]);//
        if(re_data != 0xFFFF){
            re_data = ((re_data&0xFFF)>>4);
            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen], &ACParam.System, 2); //
            RealBuffer.DataLen += 2;
        }else{
            RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;
            RealBuffer.DataBuf[RealBuffer.DataLen++] = 0;
        }
        
        for(j=0; j<Receive_Real_Page.cnt; j++){                                                               //主控外机参数，需要要传系统参数
                re_data = GetrealData(&PageData[0], &Receive_Real_Page.SrcAdd[j], ACParam.System);
            if(!(re_data&0x8000)){ //有有效的返回
                if(!Receive_Real_Page.hl_flag[j]){
                    memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                    RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                    RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF);                                            //将值放到数组中
                }else if(Receive_Real_Page.hl_flag[j] == 1){       //找到低位变化，同时将高位放到数据
                    memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                    RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                    RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF);
                    
                    re_data = GetrealData(&PageData[0], &Receive_Real_Page.SrcAdd[j+1], ACParam.System);
                    if(!(re_data&0x8000)){
                        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+1],sizeof(Receive_Real_Page.SrcAdd[j+1]));            //将地址发到数组中
                        RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+1]);
                        RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF);
                    }
                    j++;
                }else if(Receive_Real_Page.hl_flag[j] == 2){     //找到高位变化，同时将低位放到数据
                    re_data1 = GetrealData(&PageData[0], &Receive_Real_Page.SrcAdd[j-1], ACParam.System);
                    if(!(re_data1&0x8000)){
                        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j-1],sizeof(Receive_Real_Page.SrcAdd[j-1]));            //将地址发到数组中
                        RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j-1]);
                        RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data1 & 0xFF); 
                    }
                  
                    memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                    RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                    RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                }else if(Receive_Real_Page.hl_flag[j] == 3){
                        memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j],sizeof(Receive_Real_Page.SrcAdd[j]));            //将地址发到数组中
                        RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j]);
                        RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        
                        re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j+1], ACParam.Indoor[i]);
                        if(!(re_data&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+1],sizeof(Receive_Real_Page.SrcAdd[j+1]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+1]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        }
                        re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j+2], ACParam.Indoor[i]);
                        if(!(re_data&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+2],sizeof(Receive_Real_Page.SrcAdd[j+2]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+2]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        }
                        re_data = GetrealData(&PageData[1], &Receive_Real_Page.SrcAdd[j+3], ACParam.Indoor[i]);
                        if(!(re_data&0x8000)){
                            memcpy(&RealBuffer.DataBuf[RealBuffer.DataLen],&Receive_Real_Page.SrcAdd[j+3],sizeof(Receive_Real_Page.SrcAdd[j+3]));            //将地址发到数组中
                            RealBuffer.DataLen += sizeof(Receive_Real_Page.SrcAdd[j+3]);
                            RealBuffer.DataBuf[RealBuffer.DataLen++] = (uint8_t)(re_data & 0xFF); 
                        }
						
                        j += 3;					
					
                }
            }
        }
        send98Frame(&g_TcpType ,RealBuffer.DataBuf, RealBuffer.DataLen,0);
        RealBuffer.DataLen = 0;
    }
    send98Frame(&g_TcpType ,RealBuffer.DataBuf, RealBuffer.DataLen,1);   //buffer中没有机型ID  x20171101
}




uint16_t GetrealData(PageTpye *PageData, uint16_t *addr, uint8_t *m_Data)
{
    uint16_t low=0 , mid = 0 , high= ((PageData->TableSize) & 0x7FFF) - 1; 
    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;
        if(*addr == PageData->Page[mid].SrcAdd){
                if(TypeTable[(PageData->Page[mid].SrcAdd&0x7FFF) >> 8] != 1){         //if(PageData->Page[mid].Value&0x8000){//   byte 数据  x20170929
                    return  m_Data[(PageData->Page[mid].Value&0x7FFF) >> 4]&0x00FF; //返回一个byte数据
                }else{                                                              //bit类型数据
                    return  (m_Data[(PageData->Page[mid].Value&0x7FFF) >> 4]>>(1<<PageData->Page[mid].Value & 0x0F))&0x0001; //返回bit 数据
                } 
        }else if(PageData->Page[mid].SrcAdd > *addr){
          if(!mid){
                break;
            }
            high =  mid - 1;
            if(high==0)high=1;
        }else{
            if(mid < 1)mid = 1;
            low  = mid + 1;
        }
    }
    return 0x8000;                                                           //返回值最高位置1，表示没有找到值
}

uint16_t GetDataAddrr(uint16_t addr, PageTpye *PageData)
{
    uint16_t low=0 , mid = 0 , high= ((PageData->TableSize) & 0x7FFF) - 1; 
    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;
        if(addr == PageData->Page[mid].SrcAdd){
            return PageData->Page[mid].Value;
        }else if(PageData->Page[mid].SrcAdd > addr){
            if(!mid){
                break;
            }
            high =  mid - 1;
            if(high==0)high=1;                                               //
        }else{
            if(mid < 1)mid = 1;
            low  = mid + 1;
        }
    }
    return 0xFFFF;                                                           //返回值最高位置1，表示没有找到值
}


uint8_t GetRealPagehlflag(const Real_PageTpye *src_real_page, Real_PageTpye *dst_real_page)//获取接收的监控表的高低位
{
    uint16_t low=0 , mid = 0 ,high = 0;
	for(uint16_t i=0;i<dst_real_page->cnt;i++){
		high= ((src_real_page->cnt) & 0x7FFF) - 1;
    while(low <= high && high != 0){
	        mid = (low+high) >> 1 ;
	        if(dst_real_page->SrcAdd[i] == src_real_page->SrcAdd[mid]){
	            dst_real_page->hl_flag[i] = src_real_page->hl_flag[mid];
				break;
	        }else if(src_real_page->SrcAdd[mid] > dst_real_page->SrcAdd[i]){
	            if(!mid){
	                break;
	            }
	            high =  mid - 1;
	            if(high==0)high=1;                                               //
	        }else{
	            if(mid < 1)mid = 1;
	            low  = mid + 1;
	        }
	    }
	}
    return 0;                                                           //返回值最高位置1，表示没有找到值
}



