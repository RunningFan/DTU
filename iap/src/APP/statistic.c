/*****************************************************************************
Copyright: 2014 GREE ELECTRIC APPLIANCES,INC.
File name: data_statistic.c
Description: 用于统计机组的各类参数，本文件主要包含两个函数，GetData_can函数用于从CAN总线
上取出需要的数据，需要传入参数，传参类型为CanMsgType结构体，包含在handle_can.h头文件中。以
全局变量的方式传出参数，传出结构体stat_buffer，包含时间累积和次数累积。
Author:  zhh
Version: V1.0
Date:    2015/12/07
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

#define Min(a,b)   (a<b)?a:b
#define Max(a,b)   (a>b)?a:b
#define bit_type  1
#define byte_type 2

uint32_t TIM2_count ;

uint16_t time_account(timetype * time) 
{
    if(time->bgTime == 0 )
    time->bgTime = TIM2_count;
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

uint16_t cold_ability = 0,hot_ability = 0;
uint8_t oil_flag,frest_flag;
uint8_t sys_mode;
uint8_t sys_machine;
int8_t temp_outside;
int8_t temp_frest[4] = {-101,-101,-101,-101};
int8_t H_press,L_press;
uint8_t machine1[4],machine2[4],machine3,machine4;
uint16_t subcooler[4],hotvalve[4];
uint8_t wind_frequence[4];
uint8_t machine_frequence1[4],machine_frequence2[4],machine_frequence3;
uint16_t machine1_voltage[4],machine2_voltage[4],machine3_voltage;
uint8_t work_station[125];
uint16_t capability[125];
uint16_t sys_ability;
uint16_t machineID;
int8_t keding1[4],keding2[4];
int8_t paiqi1[4],paiqi2[4],temp[10],tenp[10];
int8_t paiqi3,paiqi4;
int8_t H_module[4],L_module[4];
uint8_t guoredu[10];
uint8_t qipangtong[4];
uint8_t warm_band1[4],warm_band2[4],warm_band3;
uint8_t junyoufa1[4],junyoufa2[4],huiyoufa1[4],huiyoufa2[4],pinghengfa1[4],pinghengfa2[4];
uint8_t huiyoufa3,huiyoufa4,pinghengfa3;
uint8_t machine1_fail[4],machine2_fail[4],machine3_fail;
buffertype stat_buffer;
void   GetData_can(CanMsgType    m_Convert) 
{
    uint8_t module_num;
    uint16_t com_data16;
    uint8_t com_data;
    int8_t icom_data;
    if(m_Convert.FunCode== 0xf7) {
        switch(m_Convert.DataType) {
        case 0x10:
            if((m_Convert.CAN1IP-8)<4) {
                com_data16 = (uint16_t)Get_data(5,byte_type,m_Convert)<<8|Get_data(4,byte_type,m_Convert);//机型ID
                if(recieve_flag == 1) {
                    machineID =com_data16;
                    recieve_flag = 0;
                }
            }
            break;

        case 0x11:
            if((work_station[m_Convert.CAN1IP] == 1)&&(m_Convert.CAN1IP>=32)&&(m_Convert.CAN1IP<=111)) { //内机的开关机状态
                com_data16 = ((uint16_t)Get_data(3,byte_type,m_Convert)<<8)|Get_data(2,byte_type,m_Convert);//内机的额定容量
                if(recieve_flag == 1) {
                    capability[m_Convert.CAN1IP] =com_data16;
                    recieve_flag = 0;
                }

            } else if((work_station[m_Convert.CAN1IP] == 0)&&(m_Convert.CAN1IP>=32)&&(m_Convert.CAN1IP<=111))
                capability[m_Convert.CAN1IP] = 0;

            if((m_Convert.CAN1IP-8)<4) { //外机的额定容量计算
                com_data16  =  ((uint16_t)Get_data(3,byte_type,m_Convert)<<8)|Get_data(2,byte_type,m_Convert);//过冷器
                //                        recv_data(capability[m_Convert.CAN1IP],com_data);
                if(recieve_flag == 1) {
                    capability[m_Convert.CAN1IP] =com_data16;
                    recieve_flag = 0;
                }
            }
            break;
        case 0x1d:
            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                com_data    = Get_data(165,bit_type,m_Convert);//压缩机1
                if(recieve_flag == 1) {
                    machine2_fail[module_num] = com_data;
                    recieve_flag = 0;
                }

            }

            if((m_Convert.CAN1IP==8)) {
                com_data  = Get_data(189,bit_type,m_Convert);//压缩机1
                if(recieve_flag == 1) {
                    machine3_fail= com_data;
                    recieve_flag = 0;
                }

            }

            break;
        case 0x1e:
            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                com_data    = Get_data(157,bit_type,m_Convert);//压缩机1
                if(recieve_flag == 1) {
                    machine1_fail[module_num] = com_data;
                    recieve_flag = 0;
                }

            }
            break;

        case 0x1f:
            com_data  = Get_data(81,bit_type,m_Convert);//系统压缩机的运行情况
            recv_data(&sys_machine,com_data);
            
            com_data  = Get_data(92,bit_type,m_Convert);//回油标志位
            recv_data(&oil_flag,com_data);


            com_data  = Get_data(93,bit_type,m_Convert);//化霜标志位
            recv_data(&frest_flag,com_data);

            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                com_data    = Get_data(104,bit_type,m_Convert);//压缩机1
                // recv_data(machine1[module_num],com_data);
                if(recieve_flag == 1) {
                    machine1[module_num] = com_data;
                    recieve_flag = 0;
                }

                com_data    = Get_data(105,bit_type,m_Convert);//压缩机2
                //recv_data(machine2[module_num],com_data);
                if(recieve_flag == 1) {
                    machine2[module_num] = com_data;
                    recieve_flag = 0;
                }

                if(module_num == 0) {
                    com_data    = Get_data(106,bit_type,m_Convert);//压缩机3  仅限于单机
                    //recv_data(machine2[module_num],com_data);
                    if(recieve_flag == 1) {
                        machine3 = com_data;
                        recieve_flag = 0;
                    }

                    com_data    = Get_data(107,bit_type,m_Convert);//压缩机4  仅限于单机
                    //recv_data(machine2[module_num],com_data);
                    if(recieve_flag == 1) {
                        machine4= com_data;
                        recieve_flag = 0;
                    }
                }

            }

//            com_data  = Get_data(81,bit_type,m_Convert);//系统压缩机运行状态
//            recv_data(&sys_machine,com_data);

            if((m_Convert.CAN1IP>=32)&&(m_Convert.CAN1IP<=111)) {
                com_data = Get_data(0,bit_type,m_Convert);
                // recv_data(work_station[m_Convert.CAN1IP],com_data);
                if(recieve_flag == 1) {
                    work_station[m_Convert.CAN1IP] = com_data;
                    recieve_flag = 0;
                }
            }

            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                icom_data   = Get_data(122,bit_type,m_Convert) ;//温度传感器
                if(recieve_flag == 1) {
                    qipangtong[module_num] = icom_data;
                    recieve_flag = 0;
                }
            }

            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;


                icom_data   = Get_data(118,bit_type,m_Convert) ;//压缩机2辅热标志
                if(recieve_flag == 1) {
                    warm_band2[module_num] = icom_data;
                    recieve_flag = 0;
                }

                icom_data   = Get_data(119,bit_type,m_Convert) ;//压缩机1辅热标志
                if(recieve_flag == 1) {
                    warm_band1[module_num] = icom_data;
                    recieve_flag = 0;
                }

                icom_data   = Get_data(146,bit_type,m_Convert) ;//压缩机3辅热标志
                if(recieve_flag == 1) {
                    warm_band3 = icom_data;
                    recieve_flag = 0;
                }
            }

            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                icom_data   = Get_data(130,bit_type,m_Convert) ;//均油阀1
                if(recieve_flag == 1) {
                    junyoufa1[module_num] = icom_data;
                    recieve_flag = 0;
                }

                icom_data   = Get_data(131,bit_type,m_Convert) ;//均油阀2
                if(recieve_flag == 1) {
                    junyoufa2[module_num] = icom_data;
                    recieve_flag = 0;
                }

            }

            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                icom_data   = Get_data(128,bit_type,m_Convert) ;//回油阀1
                if(recieve_flag == 1) {
                    huiyoufa1[module_num] = icom_data;
                    recieve_flag = 0;
                }

                icom_data   = Get_data(129,bit_type,m_Convert) ;//回油阀2
                if(recieve_flag == 1) {
                    huiyoufa2[module_num] = icom_data;
                    recieve_flag = 0;
                }
           
            
            if((m_Convert.CAN1IP==8)) {
                icom_data   = Get_data(148,bit_type,m_Convert) ;//回油阀3
                if(recieve_flag == 1) {
                    huiyoufa3 = icom_data;
                    recieve_flag = 0;
                }
                icom_data   = Get_data(151,bit_type,m_Convert) ;//回油阀4
                if(recieve_flag == 1) {
                    huiyoufa4 = icom_data;
                    recieve_flag = 0;
                }
            }
            }
            
            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                icom_data   = Get_data(133,bit_type,m_Convert) ;//压缩机1平衡阀
                if(recieve_flag == 1) {
                    pinghengfa1[module_num] = icom_data;
                    recieve_flag = 0;
                }

                icom_data   = Get_data(134,bit_type,m_Convert) ;//压缩机2平衡阀
                if(recieve_flag == 1) {
                    pinghengfa2[module_num] = icom_data;
                    recieve_flag = 0;
                }

                if((m_Convert.CAN1IP==8)) {
                    icom_data   = Get_data(147,bit_type,m_Convert) ;//平衡阀3
                    if(recieve_flag == 1) {
                        pinghengfa3 = icom_data;
                        recieve_flag = 0;
                    }
                }

            }
            break;
        case 0x20:
            if((m_Convert.CAN1IP-8)==0) {
            icom_data= Get_data(80,byte_type,m_Convert)-100;//室外环境温度
            recv_data2(&temp_outside,icom_data);
            }
            icom_data  =  Get_data(81,byte_type,m_Convert)-100;  //系统高压值
            recv_data2(&H_press,icom_data);
            icom_data  =  Get_data(82,byte_type,m_Convert)-100;  //系统地压值
            recv_data2(&L_press,icom_data);
            if((m_Convert.CAN1IP-8)<4) {
                module_num =m_Convert.CAN1IP-8;
                icom_data  = Get_data(105,byte_type,m_Convert) - 100;//模块高压值
                // recv_data2(H_module[module_num],com_data);
                if(recieve_flag == 1) {
                    H_module[module_num] = icom_data;
                    recieve_flag = 0;
                }
                icom_data  = Get_data(106,byte_type,m_Convert) - 100;//模块低压值
                if(recieve_flag == 1) {
                    L_module[module_num] = icom_data;
                    recieve_flag = 0;
                }
                // recv_data2(L_module[module_num],com_data);
            }
            if((m_Convert.CAN1IP-8)<4) {
                module_num = m_Convert.CAN1IP-8;
                icom_data   = Get_data(97,byte_type,m_Convert) - 100;//化霜温度传感器
                //recv_data2(temp_frest[module_num],com_data);
                if(recieve_flag == 1) {
                    temp_frest[module_num] = icom_data;
                    recieve_flag = 0;
                }
            }

            if((machine1[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
                icom_data = Get_data(87,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi1[0] ,icom_data);
                icom_data = Get_data(93,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding1[0] ,icom_data);
                if(keding1[0] != -100){
                temp[0] = Min(paiqi1[0],keding1[0]);
                tenp[0] = Max(paiqi1[0],keding1[0]);}
                else {
                    temp[0] = paiqi1[0];
                    tenp[0] = paiqi1[0];}
            }
            if((machine1[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
                icom_data = Get_data(87,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi1[1] ,icom_data);
                icom_data = Get_data(93,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding1[1] ,icom_data);
                if(keding1[1] != -100){
                temp[1] = Min(paiqi1[1],keding1[1]);
                tenp[1] = Max(paiqi1[1],keding1[1]);}
                else {
                    temp[1] = paiqi1[1];
                    tenp[1] = paiqi1[1];
                }
            }
            if((machine1[2] == 1)&&((m_Convert.CAN1IP-8)==2)) {
                icom_data = Get_data(87,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi1[2] ,icom_data);
                icom_data = Get_data(93,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding1[2] ,icom_data);
                if(keding1[2] != -100){
                temp[2] = Min(paiqi1[2],keding1[2]);
                tenp[2] = Max(paiqi1[2],keding1[2]);}
                else{
                    temp[2] = paiqi1[2];
                    tenp[2] = paiqi1[2];
                }
            }
            if((machine1[3] == 1)&&((m_Convert.CAN1IP-8)==3)) {
                icom_data = Get_data(87,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi1[3] ,icom_data);
                icom_data = Get_data(93,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding1[3] ,icom_data);
                if(keding1[2] != -100){
                temp[3] = Min(paiqi1[3],keding1[3]);
                tenp[3] = Max(paiqi1[3],keding1[3]);}
                else{
                    temp[3] = paiqi1[3];
                    tenp[3] = paiqi1[3];
                }
            }

            if((machine2[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
                icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi2[0] ,icom_data);
                icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding2[0] ,icom_data);
                if(keding2[0] != -100){
                temp[4] = Min(paiqi2[0],keding2[0]);
                tenp[4] = Max(paiqi2[0],keding2[0]);}
                else{
                   temp[4] = paiqi2[0];
                   tenp[4] = paiqi2[0];     
                }
            }
            if((machine2[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
                icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi2[1] ,icom_data);
                icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding2[1] ,icom_data);
                if(keding2[1] != -100){
                temp[5] = Min(paiqi2[1],keding2[1]);
                tenp[5] = Max(paiqi2[1],keding2[1]);}
                else {
                    temp[5] = paiqi2[1];
                    tenp[5] = paiqi2[1];
                }
            }
            if((machine2[2] == 1)&&((m_Convert.CAN1IP-8)==2)) {
                icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi2[2] ,icom_data);
                icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding2[2] ,icom_data);
                if(keding2[2] != -100){
                temp[6] = Min(paiqi2[2],keding2[2]);
                tenp[6] = Max(paiqi2[2],keding2[2]);}
                else{
                    temp[6] = paiqi2[2];
                    tenp[6] = paiqi2[2]; 
                }
            }
            if((machine2[3] == 1)&&((m_Convert.CAN1IP-8)==3)) {
                icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
                recv_data2(&paiqi2[3] ,icom_data);
                icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
                recv_data2(&keding2[3] ,icom_data);
                if(keding2[3] != -100){
                temp[7] = Min(paiqi2[3],keding2[3]);
                tenp[7] = Max(paiqi2[3],keding2[3]);}
                else {
                    temp[7] = paiqi2[3];
                    tenp[7] = paiqi2[3];
                }
            }

            if((machine3 == 1)&&((m_Convert.CAN1IP-8)==0)) {
                icom_data = Get_data(89,byte_type,m_Convert)-100; //压缩机3排气温度
                recv_data2(&paiqi3 ,icom_data);
                temp[8] = paiqi3;
                tenp[8] = paiqi3;
            }
            if((machine4 == 1)&&((m_Convert.CAN1IP-8)==0)) {
                icom_data = Get_data(90,byte_type,m_Convert)-100; //压缩机4排气温度
                recv_data2(&paiqi4 ,icom_data);
                temp[9] = paiqi4;
                tenp[9] = paiqi4;
            }
            break;
//
//            case 0x6030:
//                if((machine1[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[0] ,icom_data);
//                    icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
//                    recv_data2(&keding1[0] ,icom_data);
//                    temp[0] = Min(paiqi1[0],keding1[0]);
//                    tenp[0] = Max(paiqi1[0],keding1[0]);
//                }
//                if((machine1[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[1] ,icom_data);
//                    icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
//                    recv_data2(&keding1[1] ,icom_data);
//                    temp[1] = Min(paiqi1[1],keding1[1]);
//                    tenp[1] = Max(paiqi1[1],keding1[1]);
//                }
//                if((machine1[2] == 1)&&((m_Convert.CAN1IP-8)==2)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[2] ,icom_data);
//                    icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
//                    recv_data2(&keding1[2] ,icom_data);
//                    temp[2] = Min(paiqi1[2],keding1[2]);
//                    tenp[2] = Max(paiqi1[2],keding1[2]);
//                }
//                if((machine1[3] == 1)&&((m_Convert.CAN1IP-8)==3)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[3] ,icom_data);
//                    icom_data = Get_data(94,byte_type,m_Convert)-100; //壳顶温度
//                    recv_data2(&keding1[3] ,icom_data);
//                    temp[3] = Min(paiqi1[3],keding1[3]);
//                    tenp[3] = Max(paiqi1[3],keding1[3]);
//                }
//
//                if((machine2[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[0] ,icom_data);
//                    temp[4] =paiqi2[0];
//                    tenp[4] = paiqi2[0];
//                }
//                if((machine2[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[1] ,icom_data);
//                    temp[5] = paiqi2[1];
//                    tenp[5] = paiqi2[1];
//                }
//                if((machine2[2] == 1)&&((m_Convert.CAN1IP-8)==2)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[2] ,icom_data);
//                    temp[6] =paiqi2[2];
//                    tenp[6] = paiqi2[2];
//                }
//                if((machine2[3] == 1)&&((m_Convert.CAN1IP-8)==3)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[3] ,icom_data);
//                    temp[7] = paiqi2[3];
//                    tenp[7] = paiqi2[3];
//                }
//            case 0x6063:
//                if((machine1[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
//                    icom_data = Get_data(87,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[0] ,icom_data);
//                    icom_data = Get_data(93,byte_type,m_Convert)-100; //壳顶温度
//                    recv_data2(&keding1[0] ,icom_data);
//                    temp[0] = Min(paiqi1[0],keding1[0]);
//                    tenp[0] = Max(paiqi1[0],keding1[0]);
//                }
//                if((machine2[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[0] ,icom_data);
//                    temp[1] =paiqi2[0];
//                    tenp[1] =paiqi2[0];
//                }
//                if((machine1[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
//                    icom_data = Get_data(89,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[1] ,icom_data);
//                    temp[2] = paiqi1[1];
//                    tenp[1] =paiqi1[1];
//                }
//                if((machine2[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
//                    icom_data = Get_data(90,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[1] ,icom_data);
//                    temp[3] = paiqi2[1];
//                    tenp[3] = paiqi2[1];
//                }
//            case 0x6062:
//                if((machine1[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
//                    icom_data = Get_data(87,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[0] ,icom_data);
//                    temp[0] = paiqi1[0];
//                    tenp[0] = paiqi1[0];
//                }
//                if((machine2[0] == 1)&&((m_Convert.CAN1IP-8)==0)) {
//                    icom_data = Get_data(88,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[0] ,icom_data);
//                    temp[1] =paiqi2[0];
//                    tenp[1] =paiqi2[0];
//                }
//                if((machine1[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
//                    icom_data = Get_data(89,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi1[1] ,icom_data);
//                    temp[2] = paiqi1[1];
//                    tenp[2] = paiqi1[1];
//                }
//                if((machine2[1] == 1)&&((m_Convert.CAN1IP-8)==1)) {
//                    icom_data = Get_data(90,byte_type,m_Convert)-100; //排气温度
//                    recv_data2(&paiqi2[1] ,icom_data);
//                    temp[3] = paiqi2[1];
//                    tenp[3] = paiqi2[1];
//                }
//
//            }
//            break;
        case 0x21:
            com_data = Get_data(81,byte_type,m_Convert);
            recv_data(&sys_mode,com_data);
            break;
        case 0x28:

            if((m_Convert.CAN1IP-8)<4) {
                module_num =m_Convert.CAN1IP-8;
                com_data  = Get_data(137,byte_type,m_Convert);
                recv_data(&machine_frequence1[module_num],com_data);//压缩机运行频率
                com_data   = Get_data(179,byte_type,m_Convert);
                recv_data(&machine_frequence2[module_num] ,com_data);
                if(module_num ==0) {
                    com_data   = Get_data(193,byte_type,m_Convert);
                    recv_data(&machine_frequence3 ,com_data);
                }
                com_data = Get_data(149,byte_type,m_Convert);//风机频率
                recv_data(&wind_frequence[module_num],com_data);
            }

            if((m_Convert.CAN1IP-8)<4) {
                module_num =m_Convert.CAN1IP-8;
                com_data16  =  ((uint16_t)Get_data(131,byte_type,m_Convert)<<8)|Get_data(130,byte_type,m_Convert);//过冷器
                if(recieve_flag == 1) {
                    machine1_voltage[module_num] =com_data16;
                    recieve_flag = 0;
                }

                com_data16  =  ((uint16_t)Get_data(173,byte_type,m_Convert)<<8)|Get_data(172,byte_type,m_Convert);//过冷器
                if(recieve_flag == 1) {
                    machine2_voltage[module_num] =com_data16;
                    recieve_flag = 0;
                }

                if(module_num == 0) {
                    com_data16  =  ((uint16_t)Get_data(187,byte_type,m_Convert)<<8)|Get_data(186,byte_type,m_Convert);//过冷器
                    if(recieve_flag == 1) {
                        machine3_voltage =com_data16;
                        recieve_flag = 0;
                    }
                }
            }

            break;
        case 0x31:
            //                    cold_ability = ((uint16_t)Get_data(81,byte_type,m_Convert)<<8)|Get_data(80,byte_type,m_Convert);//制冷能力需求
            //                    hot_ability  = ((uint16_t)Get_data(83,byte_type,m_Convert)<<8)|Get_data(82,byte_type,m_Convert);//制热能力需求
            if((m_Convert.CAN1IP-8)<4) {
                module_num =m_Convert.CAN1IP-8;
                com_data16   = (uint16_t )Get_data(142,byte_type,m_Convert)<<8|Get_data(141,byte_type,m_Convert);//过冷器
                if(recieve_flag == 1) {
                    subcooler[module_num] =com_data16;    //过冷器
                    recieve_flag = 0;
                }
            }
            if((m_Convert.CAN1IP-8)<4) {
                module_num =m_Convert.CAN1IP-8;
                com_data16   = (uint16_t )Get_data(140,byte_type,m_Convert)<<8|Get_data(139,byte_type,m_Convert);//过冷器
                if(recieve_flag == 1) {
                    hotvalve[module_num] =com_data16;
                    recieve_flag = 0;
                }
            }

            com_data16 = (uint16_t )Get_data(85,byte_type,m_Convert)<<8|Get_data(84,byte_type,m_Convert);//整机运行能力
            if(recieve_flag == 1) {
                sys_ability =com_data16;//整机运行能力
                recieve_flag = 0;
            }
            break;

        }
    }
}


timetype timer1,timer2,timer3,timer4,timer5,timer6,timer7,timer8,timer9,timer10, timer11,timer12;
timetype timerbf1[6],timerbf2[6],timerbf3[6],timerbf4[6],timerbf5[6],timerbf6[6],timerbf7[6],timerbf8[6],timerbf9[6],timerbf10[6],timerbf11[6],timerbf12[6],timerbf13[6],timerbf14[6];
uint8_t current_frest,oldflag_frest;
uint32_t start,time_s;
uint8_t new_state;
float capa_percent,work_percent;
uint8_t QPT_flag[4],JYF_flag1[4],JYF_flag2[4],JYF_flag3;
uint8_t HYF_flag1[4],HYF_flag2[4],HYF_flag3,HYF_flag4;
uint8_t PHF_flag1[4],PHF_flag2[4],PHF_flag3;
uint8_t oldflag1_fail[4],oldflag2_fail[4],oldflag3_fail;
void Statistic_CanData() 
{
    uint8_t i = 0 ;
    uint16_t indoor_capability = 0;
    uint16_t outdoor_capability = 0;
    /***********************整机得电时间统计**********************/
//    stat_buffer.time_buffer[FLAG] = f4_time++;
    f4_time++;
    stat_buffer.time_buffer[WORK_TIME]++;
    /***********************室外环境温度累积时间统计***************/
    if((sys_mode == 2)||(sys_mode == 3)) { //制冷时外环境温度时间累积
        if(timer4.ctTime != 0)   memset(&timer4,0,sizeof(timer4));
        if(timer5.ctTime != 0)   memset(&timer5,0,sizeof(timer5));
        if(timer6.ctTime != 0)   memset(&timer6,0,sizeof(timer6));
        if(timer7.ctTime != 0)   memset(&timer7,0,sizeof(timer7));
        if(timer8.ctTime != 0)   memset(&timer8,0,sizeof(timer8));

        if(temp_outside <=10) {
            if((time_account(&timer1))>=180)  stat_buffer.time_buffer[COLD_TIME1]++;
            if(timer2.ctTime != 0)   memset(&timer2,0,sizeof(timer2));
            if(timer3.ctTime != 0)   memset(&timer3,0,sizeof(timer3));
        } else if((temp_outside >10)&&(temp_outside <=27)) {
            if((time_account(&timer2))>=180)  stat_buffer.time_buffer[COLD_TIME2]++;
            if(timer1.ctTime != 0)   memset(&timer1,0,sizeof(timer1));
            if(timer3.ctTime != 0)   memset(&timer3,0,sizeof(timer3));
        } else if(temp_outside >27) {
            if((time_account(&timer3))>=180)  stat_buffer.time_buffer[COLD_TIME3]++;
            if(timer1.ctTime != 0)   memset(&timer1,0,sizeof(timer1));
            if(timer2.ctTime != 0)   memset(&timer2,0,sizeof(timer2));
        }
    }else if(sys_mode == 5) {
        if(timer2.ctTime != 0){
            memset(&timer2,0,sizeof(timer2));
        }
        if(timer3.ctTime != 0){
            memset(&timer3,0,sizeof(timer3));
        }
        if(timer1.ctTime != 0){
            memset(&timer1,0,sizeof(timer1));
        }
        if(temp_outside <=(-15)) {  //制热时室外环境温度时间累积
            if((time_account(&timer4))>=180)  stat_buffer.time_buffer[HOT_TIME1]++;
            if(timer5.ctTime != 0)   memset(&timer5,0,sizeof(timer5));
            if(timer6.ctTime != 0)   memset(&timer6,0,sizeof(timer6));
            if(timer7.ctTime != 0)   memset(&timer7,0,sizeof(timer7));
            if(timer8.ctTime != 0)   memset(&timer8,0,sizeof(timer8));
        } else if((temp_outside >(-15))&&(temp_outside <=(-7))) {
            if((time_account(&timer5))>=180)  stat_buffer.time_buffer[HOT_TIME2]++;
            if(timer4.ctTime != 0)   memset(&timer4,0,sizeof(timer4));
            if(timer6.ctTime != 0)   memset(&timer6,0,sizeof(timer6));
            if(timer7.ctTime != 0)   memset(&timer7,0,sizeof(timer7));
            if(timer8.ctTime != 0)   memset(&timer8,0,sizeof(timer8));
        } else if((temp_outside >(-7))&&(temp_outside <=(0))) {
            if((time_account(&timer6))>=180)  stat_buffer.time_buffer[HOT_TIME3]++;
            if(timer5.ctTime != 0)   memset(&timer5,0,sizeof(timer5));
            if(timer4.ctTime != 0)   memset(&timer4,0,sizeof(timer4));
            if(timer7.ctTime != 0)   memset(&timer7,0,sizeof(timer7));
            if(timer8.ctTime != 0)   memset(&timer8,0,sizeof(timer8));
        } else if((temp_outside >0)&&(temp_outside <=15)) {
            if((time_account(&timer7))>=180)  stat_buffer.time_buffer[HOT_TIME4]++;
            if(timer5.ctTime != 0)   memset(&timer5,0,sizeof(timer5));
            if(timer6.ctTime != 0)   memset(&timer6,0,sizeof(timer6));
            if(timer4.ctTime != 0)   memset(&timer4,0,sizeof(timer4));
            if(timer8.ctTime != 0)   memset(&timer8,0,sizeof(timer8));
        } else if(temp_outside >15) {
            if((time_account(&timer8))>=180)  stat_buffer.time_buffer[HOT_TIME5]++;
            if(timer5.ctTime != 0)   memset(&timer5,0,sizeof(timer5));
            if(timer6.ctTime != 0)   memset(&timer6,0,sizeof(timer6));
            if(timer7.ctTime != 0)   memset(&timer7,0,sizeof(timer7));
            if(timer4.ctTime != 0)   memset(&timer4,0,sizeof(timer4));
        }
    }
    /***********************模块过冷器，电子膨胀阀开度统计*************************/
    for(i = 0; i<4; i++) { //共有四个模块，计算每个模块的过冷器,电子膨胀阀开度
        if(((sys_mode==2)||(sys_mode == 3))&&(oil_flag!= 1)&&((machine1[i])||(machine2[i]))) {
            if((time_account(&timerbf1[i]))>=300) {
                if(subcooler[i] <=100) stat_buffer.time_buffer[CEXV0_TIME1+i*3]++;
                else if((subcooler[i] >100)&&(subcooler[i] <=400)) stat_buffer.time_buffer[CEXV0_TIME2+i*3]++;
                else if(subcooler[i] >400) stat_buffer.time_buffer[CEXV0_TIME3+i*3]++;
            }

        } else{  
            memset(&timerbf1[i],0,sizeof(timerbf1[i]));
        }
        if((sys_mode==5)&&(oil_flag!= 1)&&(frest_flag!= 1)&&((machine1[i])||(machine2[i]))) {
            if((time_account(&timerbf2[i]))>=300) {
                if(subcooler[i] <=100) stat_buffer.time_buffer[HEXV0_TIME1+i*3]++;
                else if((subcooler[i] >100)&&(subcooler[i] <=400)) stat_buffer.time_buffer[HEXV0_TIME2+i*3]++;
                else if(subcooler[i] >400) stat_buffer.time_buffer[HEXV0_TIME3+i*3]++;

                if(hotvalve[i] <=100) stat_buffer.time_buffer[HDXV0_TIME1+i*3]++;
                else if((hotvalve[i] >100)&&(hotvalve[i] <=400)) stat_buffer.time_buffer[HDXV0_TIME2+i*3]++;
                else if(hotvalve[i] >400) stat_buffer.time_buffer[HDXV0_TIME3+i*3]++;
            }
        } else   memset(&timerbf2[i],0,sizeof(timerbf2[i]));
    }
    /*******************************系统高低压统计***********************************/
    if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(sys_machine == 1)) {
        if((time_account(&timer9))>=300) {
            if(H_press<=10) stat_buffer.time_buffer[HPRSN_TIME1]++;//正常运行时候系统高低压统计
            else if((H_press>10)&&(H_press<=30)) stat_buffer.time_buffer[HPRSN_TIME2]++;
            else if((H_press>30)&&(H_press<=55)) stat_buffer.time_buffer[HPRSN_TIME3]++;
            else if(H_press>55)  stat_buffer.time_buffer[HPRSN_TIME4]++;

            if(L_press<=(-30)) stat_buffer.time_buffer[LPRSN_TIME1]++;
            else if((L_press>(-30))&&(L_press<=(-10))) stat_buffer.time_buffer[LPRSN_TIME2]++;
            else if((L_press>(-10))&&(L_press<=0)) stat_buffer.time_buffer[LPRSN_TIME3]++;
            else if((L_press>(0))&&(L_press<=7)) stat_buffer.time_buffer[LPRSN_TIME4]++;
            else if(L_press>7)  stat_buffer.time_buffer[LPRSN_TIME5]++;

        }
    } else {
        memset(&timer9,0,sizeof(timer9));
    }
    /*******************************制冷回油时高低压统计***********************************/
    if(((sys_mode==2)||(sys_mode == 3))&&(oil_flag== 1)) { //无需运行5分钟

        if(H_press<=10) stat_buffer.time_buffer[HPRSC_TIME1]++;//正常运行时候系统高低压统计
        else if((H_press>10)&&(H_press<=30)) stat_buffer.time_buffer[HPRSC_TIME2]++;
        else if((H_press>30)&&(H_press<=55)) stat_buffer.time_buffer[HPRSC_TIME3]++;
        else if(H_press>55)  stat_buffer.time_buffer[HPRSC_TIME4]++;

        if(L_press<=(-30)) stat_buffer.time_buffer[LPRSC_TIME1]++;
        else if((L_press>(-30))&&(L_press<=(-10))) stat_buffer.time_buffer[LPRSC_TIME2]++;
        else if((L_press>(-10))&&(L_press<=0)) stat_buffer.time_buffer[LPRSC_TIME3]++;
        else if((L_press>(0))&&(L_press<=7)) stat_buffer.time_buffer[LPRSC_TIME4]++;
        else if(L_press>7)  stat_buffer.time_buffer[LPRSC_TIME5]++;
    }
    /*******************************化霜时高低压统计***********************************/
    if((sys_mode==5)&&(frest_flag== 1)) { //无需运行5分钟
        if(H_press<=10) stat_buffer.time_buffer[HPRSH_TIME1]++;//正常运行时候系统高低压统计
        else if((H_press>10)&&(H_press<=30)) stat_buffer.time_buffer[HPRSH_TIME2]++;
        else if(H_press>30) stat_buffer.time_buffer[HPRSH_TIME3]++;

        if(L_press<=(-30)) stat_buffer.time_buffer[LPRSH_TIME1]++;
        else if((L_press>(-30))&&(L_press<=(-10))) stat_buffer.time_buffer[LPRSH_TIME2]++;
        else if(L_press>(-10)) stat_buffer.time_buffer[LPRSH_TIME3]++;

    }
    /*****************************制热化霜时高压低压次数以及化霜感温包温度次数统计**************************************/
    if(frest_flag == 0) {
        current_frest = frest_flag;
    } else {
        if(current_frest == 0) {
            if(H_press<=10) { //高压次数
                stat_buffer.count_buffer[HPRSH_COUNT1]++;
            } else if((H_press>10)&&(H_press<=30)) {
                stat_buffer.count_buffer[HPRSH_COUNT2]++;
            } else if(H_press>30) {
                stat_buffer.count_buffer[HPRSH_COUNT3]++;
            }

            if(L_press<=(-30)) { //低压次数
                stat_buffer.count_buffer[LPRSH_COUNT1]++;
            } else if((L_press>(-30))&&(L_press<=(-10))) {
                stat_buffer.count_buffer[LPRSH_COUNT2]++;
            } else if(L_press>(-10)) {
                stat_buffer.count_buffer[LPRSH_COUNT3]++;
            }

            if((-100)<=temp_frest[0]<=(-20)) { //模块1的化霜感温包统计
                stat_buffer.count_buffer[FREST1_COUNT1]++;
            } else if((temp_frest[0]>(-20))&&(temp_frest[0]<=(-7))) {
                stat_buffer.count_buffer[FREST1_COUNT2]++;
            } else if(temp_frest[0]>(-7)) {
                stat_buffer.count_buffer[FREST1_COUNT3]++;
            }

            if((-100)<=temp_frest[1]<=(-20)) { //模块2的化霜感温包统计
                stat_buffer.count_buffer[FREST2_COUNT1]++;
            } else if((temp_frest[1]>(-20))&&(temp_frest[0]<=(-7))) {
                stat_buffer.count_buffer[FREST2_COUNT2]++;
            } else if(temp_frest[1]>(-7)) {
                stat_buffer.count_buffer[FREST2_COUNT3]++;
            }

            if((-100)<=temp_frest[2]<=(-20)) { //模块3的化霜感温包统计
                stat_buffer.count_buffer[FREST3_COUNT1]++;
            } else if((temp_frest[2]>(-20))&&(temp_frest[0]<=(-7))) {
                stat_buffer.count_buffer[FREST3_COUNT2]++;
            } else if(temp_frest[2]>(-7)) {
                stat_buffer.count_buffer[FREST3_COUNT3]++;
            }

            if((-100)<=temp_frest[3]<=(-20)) { //模块4的化霜感温包统计
                stat_buffer.count_buffer[FREST4_COUNT1]++;
            } else if((temp_frest[3]>(-20))&&(temp_frest[0]<=(-7))) {
                stat_buffer.count_buffer[FREST4_COUNT2]++;
            } else if(temp_frest[3]>(-7)) {
                stat_buffer.count_buffer[FREST4_COUNT3]++;
            }

            current_frest = 1;
        }
    }
    /*****************************制热化霜的时间次数统计**************************************/
    if(frest_flag ==1) {
        if(oldflag_frest == 0) {
            start = RTC_GetCounter();
            oldflag_frest = 1;
        }
    } else {
        if(oldflag_frest == 1) time_s = RTC_GetCounter() - start;
        oldflag_frest = 0 ;
    }

    if(time_s!=0) {
        if(time_s<=300) {
            stat_buffer.count_buffer[TIME_COUNT1]++;
            time_s = 0;
        } else if((time_s>300)&&(time_s<=600)) {
            stat_buffer.count_buffer[TIME_COUNT2]++;
            time_s = 0;
        } else if(time_s>600) {
            stat_buffer.count_buffer[TIME_COUNT3]++;
            time_s = 0;
        }
    }
    /**********************************风机频率统计***********************************/
    for(i = 0 ; i <4; i++) {
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&((machine1[i])||(machine2[i]))) {
            if((time_account(&timerbf4[i]))>=300) {
                if(wind_frequence[i] <= 20)  stat_buffer.time_buffer[WIND1_STATIC1+i*4]++;
                else if((wind_frequence[i]>20)&&(wind_frequence[i]<=40)) stat_buffer.time_buffer[WIND1_STATIC2+i*4]++;
                else if((wind_frequence[i]>40)&&(wind_frequence[i]<=55)) stat_buffer.time_buffer[WIND1_STATIC3+i*4]++;
                else if(wind_frequence[i]>55)  stat_buffer.time_buffer[WIND1_STATIC4+i*4]++;
            }
        }

        else memset(&timerbf4[i],0,sizeof(timerbf4[i]));
    }
    /**********************************负荷能力统计***********************************/
    new_state++;
    if(new_state >= 30) {
        for(i = 32 ; i<=111; i++) {
            indoor_capability += capability[i];
        }
        for(i = 8 ; i<=12; i++) {
            outdoor_capability += capability[i];
        }
        capa_percent = (float)indoor_capability/(float)outdoor_capability;
        work_percent = (float)sys_ability/(float)outdoor_capability;
        new_state = 0;
    }

    if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(sys_machine == 1)) {
        if((time_account(&timer10))>=300) {
            if(capa_percent<= (0.25))  stat_buffer.time_buffer[BURTHEN_STATIC1]++;
            else if((capa_percent>(0.25))&&(capa_percent<=0.5))  stat_buffer.time_buffer[BURTHEN_STATIC2]++;
            else if((capa_percent>0.5)&&(capa_percent<=0.75))    stat_buffer.time_buffer[BURTHEN_STATIC3]++;
            else if((capa_percent>(0.75))&&(capa_percent<=1))  stat_buffer.time_buffer[BURTHEN_STATIC4]++;
            else if((capa_percent>1)&&(capa_percent<=1.2))    stat_buffer.time_buffer[BURTHEN_STATIC5]++;
            else if(capa_percent>1.2)  stat_buffer.time_buffer[BURTHEN_STATIC6]++;

            if(work_percent<= (0.25))  stat_buffer.time_buffer[WORKBURTHEN_STATIC1]++;
            else if((work_percent>(0.25))&&(work_percent<=0.5))  stat_buffer.time_buffer[WORKBURTHEN_STATIC2]++;
            else if((work_percent>0.5)&&(work_percent<=0.75))    stat_buffer.time_buffer[WORKBURTHEN_STATIC3]++;
            else if((work_percent>(0.75))&&(work_percent<1))  stat_buffer.time_buffer[WORKBURTHEN_STATIC4]++;
            else if(work_percent==1)  stat_buffer.time_buffer[WORKBURTHEN_STATIC5]++;
        }
    } else memset(&timer10,0,sizeof(timer10));

    /**********************************压缩机频率***********************************/
//    for(i = 0 ; i <4; i++) {
//        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine1[i])) {
//
//            if((time_account(&timerbf5[i]))>=300) {
//                if((i == 0)&&((machineID ==0x6063)||(machineID ==0x6062))) {
//                    if(machine_frequence1[i]<=30)  stat_buffer.time_buffer[MACHINE00_STATIC1]++;
//                    else if((machine_frequence1[i]>30)&&(machine_frequence1[i]<=70))  stat_buffer.time_buffer[MACHINE00_STATIC2]++;
//                    else if(machine_frequence1[i]>70)    stat_buffer.time_buffer[MACHINE00_STATIC3]++;
//
//                    if(machine_frequence3<=30)  stat_buffer.time_buffer[MACHINE3_STATIC1]++;
//                    else if((machine_frequence3>30)&&(machine_frequence3<=70))  stat_buffer.time_buffer[MACHINE3_STATIC2]++;
//                    else if(machine_frequence3>70)    stat_buffer.time_buffer[MACHINE3_STATIC3]++;
//                }
//                if((i == 0)&&((machineID ==0x6030)||(machineID ==0x6031)||(machineID ==0x6039))) {
//                    if(machine_frequence1[i]<=30)  stat_buffer.time_buffer[MACHINE00_STATIC1]++;
//                    else if((machine_frequence1[i]>30)&&(machine_frequence1[i]<=70))  stat_buffer.time_buffer[MACHINE00_STATIC2]++;
//                    else if(machine_frequence1[i]>70)    stat_buffer.time_buffer[MACHINE00_STATIC3]++;
//                }
//
//                if(i != 0 ) {
//                    if(machine_frequence1[i]<=30)  stat_buffer.time_buffer[MACHINE10_STATIC1+(i-1)*6]++;
//                    else if((machine_frequence1[i]>30)&&(machine_frequence1[i]<=70))  stat_buffer.time_buffer[MACHINE10_STATIC1+(i-1)*6]++;
//                    else if(machine_frequence1[i]>70)    stat_buffer.time_buffer[MACHINE10_STATIC1+(i-1)*6]++;
//                }
//            }
//        } else memset(&timerbf5[i],0,sizeof(timerbf5[i]));
    
    for(i = 0 ; i <4; i++) {
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine1[i])) {

            if((time_account(&timerbf5[i]))>=300) {
                if((i == 0)) {
                    if(machine_frequence1[i]<=30)  stat_buffer.time_buffer[MACHINE00_STATIC1]++;
                    else if((machine_frequence1[i]>30)&&(machine_frequence1[i]<=70))  stat_buffer.time_buffer[MACHINE00_STATIC2]++;
                    else if(machine_frequence1[i]>70)    stat_buffer.time_buffer[MACHINE00_STATIC3]++;             
                }

                if(i != 0 ) {
                    if(machine_frequence1[i]<=30)  stat_buffer.time_buffer[MACHINE10_STATIC1+(i-1)*6]++;
                    else if((machine_frequence1[i]>30)&&(machine_frequence1[i]<=70))  stat_buffer.time_buffer[MACHINE10_STATIC2+(i-1)*6]++;
                    else if(machine_frequence1[i]>70)    stat_buffer.time_buffer[MACHINE10_STATIC3+(i-1)*6]++;
                }
            }
        } else memset(&timerbf5[i],0,sizeof(timerbf5[i]));

        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine2[i])) {

            if((time_account(&timerbf6[i]))>=300) {

                if((i == 0)) {
                    if(machine_frequence2[i]<=30)  stat_buffer.time_buffer[MACHINE01_STATIC1]++;
                    else if((machine_frequence2[i]>30)&&(machine_frequence2[i]<=70))  stat_buffer.time_buffer[MACHINE01_STATIC2]++;
                    else if(machine_frequence2[i]>70)    stat_buffer.time_buffer[MACHINE01_STATIC3]++;
                }

                if(i != 0 ) {
                    if(machine_frequence2[i]<=30)  stat_buffer.time_buffer[MACHINE11_STATIC1+(i-1)*6]++;
                    else if((machine_frequence2[i]>30)&&(machine_frequence2[i]<=70))  stat_buffer.time_buffer[MACHINE11_STATIC2+(i-1)*6]++;
                    else if(machine_frequence2[i]>70)    stat_buffer.time_buffer[MACHINE11_STATIC3+(i-1)*6]++;
                }
            }
        } else memset(&timerbf6[i],0,sizeof(timerbf6[i]));
    }

     if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine3)) {

        if((time_account(&timerbf5[4]))>=300) {
        if(machine_frequence3<=30)  stat_buffer.time_buffer[MACHINE3_STATIC1]++;
        else if((machine_frequence3>30)&&(machine_frequence3<=70))  stat_buffer.time_buffer[MACHINE3_STATIC2]++;
        else if(machine_frequence3>70)    stat_buffer.time_buffer[MACHINE3_STATIC3]++;}
     } else {
         memset(&timerbf5[4],0,sizeof(timerbf5[4]));}
    /**********************************正常运行压缩机排气过热度***********************************/
    for(i = 0 ; i <4; i++) {
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine1[i])) {
            if((temp[i] !=0)){
            if((time_account(&timerbf7[i]))>=300) {              
                    guoredu[i]=temp[i]-H_module[i];
                if(i == 0 ){
                    if(guoredu[i]<=5)  stat_buffer.time_buffer[GUOREDUN1_STATIC1]++;
                    else if((guoredu[i]>5)&&(guoredu[i]<=10))  stat_buffer.time_buffer[GUOREDUN1_STATIC2]++;
                    else if((guoredu[i]>10)&&(guoredu[i]<=25))  stat_buffer.time_buffer[GUOREDUN1_STATIC3]++;
                    else if(guoredu[i]>25)    stat_buffer.time_buffer[GUOREDUN1_STATIC4]++;
                } else {
                    if(guoredu[i]<=5)  stat_buffer.time_buffer[GUOREDUN2_STATIC1+(i-1)*8]++;
                    else if((guoredu[i]>5)&&(guoredu[i]<=10))  stat_buffer.time_buffer[GUOREDUN2_STATIC2+(i-1)*8]++;
                    else if((guoredu[i]>10)&&(guoredu[i]<=25))  stat_buffer.time_buffer[GUOREDUN2_STATIC3+(i-1)*8]++;
                    else if(guoredu[i]>25)    stat_buffer.time_buffer[GUOREDUN2_STATIC4+(i-1)*8]++;
                }
               
        } 
        }
        }else memset(&timerbf7[i],0,sizeof(timerbf7[i])); 
        
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine2[i])) {
            if((temp[i+4] !=0)){
            if((time_account(&timerbf8[i]))>=300) {       
                guoredu[i+4]=temp[i+4]-H_module[i];
                if(i == 0 ) {
                    if(guoredu[i+4]<=5)  stat_buffer.time_buffer[GUOREDUN1_STATIC5]++;
                    else if((guoredu[i+4]>5)&&(guoredu[i+4]<=10))  stat_buffer.time_buffer[GUOREDUN1_STATIC6]++;
                    else if((guoredu[i+4]>10)&&(guoredu[i+4]<=25))  stat_buffer.time_buffer[GUOREDUN1_STATIC7]++;
                    else if(guoredu[i+4]>25)    stat_buffer.time_buffer[GUOREDUN1_STATIC8]++;
                } else {
                    if(guoredu[i+4]<=5)  stat_buffer.time_buffer[GUOREDUN2_STATIC5+(i-1)*8]++;
                    else if((guoredu[i+4]>5)&&(guoredu[i+4]<=10))  stat_buffer.time_buffer[GUOREDUN2_STATIC6+(i-1)*8]++;
                    else if((guoredu[i+4]>10)&&(guoredu[i+4]<=25))  stat_buffer.time_buffer[GUOREDUN2_STATIC7+(i-1)*8]++;
                    else if(guoredu[i+4]>25)    stat_buffer.time_buffer[GUOREDUN2_STATIC8+(i-1)*8]++;
                }           
                }       
        }
        }else memset(&timerbf8[i],0,sizeof(timerbf8[i]));
        }
    
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine3)) {
            if((temp[8] !=0)){
            if((time_account(&timerbf8[4]))>=300) {
                guoredu[8]=temp[8]-H_module[0];
                if(guoredu[8]<=5)  stat_buffer.time_buffer[GUOREDUN13_STATIC1]++;
                else if((guoredu[8]>5)&&(guoredu[8]<=10))  stat_buffer.time_buffer[GUOREDUN13_STATIC2]++;
                else if((guoredu[8]>10)&&(guoredu[8]<=25))  stat_buffer.time_buffer[GUOREDUN13_STATIC3]++;
                else if(guoredu[8]>25)    stat_buffer.time_buffer[GUOREDUN13_STATIC4]++;
            }            
        }
        }else memset(&timerbf8[4],0,sizeof(timerbf8[4]));
    
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine4)) {
            if((temp[9] !=0)){
            if((time_account(&timerbf8[5]))>=300) {
                guoredu[9]=temp[9]-H_module[0];
                if(guoredu[9]<=5)  stat_buffer.time_buffer[GUOREDUN14_STATIC5]++;
                else if((guoredu[9]>5)&&(guoredu[9]<=10))  stat_buffer.time_buffer[GUOREDUN14_STATIC6]++;
                else if((guoredu[9]>10)&&(guoredu[9]<=25))  stat_buffer.time_buffer[GUOREDUN14_STATIC7]++;
                else if(guoredu[9]>25)    stat_buffer.time_buffer[GUOREDUN14_STATIC8]++;
            }       
        }
        }else memset(&timerbf8[5],0,sizeof(timerbf8[5]));
    /**********************************制冷回油时压缩机排气过热度***********************************/

    for(i = 0 ; i <4; i++) {
        if(((sys_mode==2)||(sys_mode == 3))&&(oil_flag == 1)&&(machine1[i])) {
                if((temp[i] !=0)){
                guoredu[i]=temp[i]-H_module[i];
                if(i == 0 ) {
                    if(guoredu[i]<=5)  stat_buffer.time_buffer[GUOREDUC1_STATIC1]++;
                    else if((guoredu[i]>5)&&(guoredu[i]<=10))  stat_buffer.time_buffer[GUOREDUC1_STATIC2]++;
                    else if((guoredu[i]>10)&&(guoredu[i]<=25))  stat_buffer.time_buffer[GUOREDUC1_STATIC3]++;
                    else if(guoredu[i]>25)    stat_buffer.time_buffer[GUOREDUC1_STATIC4]++;
                } else {
                    if(guoredu[i]<=5)  stat_buffer.time_buffer[GUOREDUC2_STATIC1+(i-1)*8]++;
                    else if((guoredu[i]>5)&&(guoredu[i]<=10))  stat_buffer.time_buffer[GUOREDUC2_STATIC2+(i-1)*8]++;
                    else if((guoredu[i]>10)&&(guoredu[i]<=25))  stat_buffer.time_buffer[GUOREDUC2_STATIC3+(i-1)*8]++;
                    else if(guoredu[i]>25)    stat_buffer.time_buffer[GUOREDUC2_STATIC4+(i-1)*8]++;
                }
            }
        }

        if(((sys_mode==2)||(sys_mode == 3))&&(oil_flag == 1)&&(machine2[i])) {

                if((temp[i+4] !=0)){
                guoredu[i+4]=temp[i+4]-H_module[i];
                if(i == 0 ) {
                    if(guoredu[i+4]<=5)  stat_buffer.time_buffer[GUOREDUC1_STATIC5]++;
                    else if((guoredu[i+4]>5)&&(guoredu[i+4]<=10))  stat_buffer.time_buffer[GUOREDUC1_STATIC6]++;
                    else if((guoredu[i+4]>10)&&(guoredu[i+4]<=25))  stat_buffer.time_buffer[GUOREDUC1_STATIC7]++;
                    else if(guoredu[i+4]>25)    stat_buffer.time_buffer[GUOREDUC1_STATIC8]++;
                } else {
                    if(guoredu[i+4]<=5)  stat_buffer.time_buffer[GUOREDUC2_STATIC5+(i-1)*8]++;
                    else if((guoredu[i+4]>5)&&(guoredu[i+4]<=10))  stat_buffer.time_buffer[GUOREDUC2_STATIC6+(i-1)*8]++;
                    else if((guoredu[i+4]>10)&&(guoredu[i+4]<=25))  stat_buffer.time_buffer[GUOREDUC2_STATIC7+(i-1)*8]++;
                    else if(guoredu[i+4]>25)    stat_buffer.time_buffer[GUOREDUC2_STATIC8+(i-1)*8]++;
                }
            }
            }
        }
        if(((sys_mode==2)||(sys_mode == 3))&&(oil_flag == 1)&&(machine3)) {

                if((temp[8] !=0)){
                guoredu[8]=temp[8]-H_module[0];
                if(guoredu[8]<=5)  stat_buffer.time_buffer[GUOREDUC13_STATIC1]++;
                else if((guoredu[8]>5)&&(guoredu[8]<=10))  stat_buffer.time_buffer[GUOREDUC13_STATIC2]++;
                else if((guoredu[8]>10)&&(guoredu[8]<=25))  stat_buffer.time_buffer[GUOREDUC13_STATIC3]++;
                else if(guoredu[8]>25)    stat_buffer.time_buffer[GUOREDUC13_STATIC4]++;
            }
        }

        if(((sys_mode==2)||(sys_mode == 3))&&(oil_flag == 1)&&(machine4)) {

                if((temp[9] !=0)){
                guoredu[9]=temp[9]-H_module[0];
                if(guoredu[9]<=5)  stat_buffer.time_buffer[GUOREDUC14_STATIC5]++;
                else if((guoredu[9]>5)&&(guoredu[9]<=10))  stat_buffer.time_buffer[GUOREDUC14_STATIC6]++;
                else if((guoredu[9]>10)&&(guoredu[9]<=25))  stat_buffer.time_buffer[GUOREDUC14_STATIC7]++;
                else if(guoredu[9]>25)    stat_buffer.time_buffer[GUOREDUC14_STATIC8]++;
            }
        }
        
    

    /**********************************制热回油化霜时压缩机排气过热度***********************************/

    for(i = 0 ; i <4; i++) {
        if((sys_mode == 5)&&((oil_flag == 1)||(frest_flag ==1))&&(machine1[i])) {

                if((temp[i] !=0)){
                guoredu[i]=temp[i]-H_module[i];
                if(i == 0 ) {
                    if(guoredu[i]<=5)  stat_buffer.time_buffer[GUOREDUH1_STATIC1]++;
                    else if((guoredu[i]>5)&&(guoredu[i]<=10))  stat_buffer.time_buffer[GUOREDUH1_STATIC2]++;
                    else if((guoredu[i]>10)&&(guoredu[i]<=25))  stat_buffer.time_buffer[GUOREDUH1_STATIC3]++;
                    else if(guoredu[i]>25)    stat_buffer.time_buffer[GUOREDUH1_STATIC4]++;
                } else {
                    if(guoredu[i]<=5)  stat_buffer.time_buffer[GUOREDUH2_STATIC1+(i-1)*8]++;
                    else if((guoredu[i]>5)&&(guoredu[i]<=10))  stat_buffer.time_buffer[GUOREDUH2_STATIC2+(i-1)*8]++;
                    else if((guoredu[i]>10)&&(guoredu[i]<=25))  stat_buffer.time_buffer[GUOREDUH2_STATIC3+(i-1)*8]++;
                    else if(guoredu[i]>25)    stat_buffer.time_buffer[GUOREDUH2_STATIC4+(i-1)*8]++;
                }
            }
            }   
       

        if((sys_mode == 5)&&((oil_flag == 1)||(frest_flag ==1))&&(machine2[i])) {

                if((temp[i+4] !=0)){
                guoredu[i+4]=temp[i+4]-H_module[i];
                if(i == 0 ) {
                    if(guoredu[i+4]<=5)  stat_buffer.time_buffer[GUOREDUH1_STATIC5]++;
                    else if((guoredu[i+4]>5)&&(guoredu[i+4]<=10))  stat_buffer.time_buffer[GUOREDUH1_STATIC6]++;
                    else if((guoredu[i+4]>10)&&(guoredu[i+4]<=25))  stat_buffer.time_buffer[GUOREDUH1_STATIC7]++;
                    else if(guoredu[i+4]>25)    stat_buffer.time_buffer[GUOREDUH1_STATIC8]++;
                } else {
                    if(guoredu[i+4]<=5)  stat_buffer.time_buffer[GUOREDUH2_STATIC5+(i-1)*8]++;
                    else if((guoredu[i+4]>5)&&(guoredu[i+4]<=10))  stat_buffer.time_buffer[GUOREDUH2_STATIC6+(i-1)*8]++;
                    else if((guoredu[i+4]>10)&&(guoredu[i+4]<=25))  stat_buffer.time_buffer[GUOREDUH2_STATIC7+(i-1)*8]++;
                    else if(guoredu[i+4]>25)    stat_buffer.time_buffer[GUOREDUH2_STATIC8+(i-1)*8]++;
                }
            }
        }
        }
        
        if((sys_mode == 5)&&((oil_flag == 1)||(frest_flag ==1))&&(machine3)) {

            if((temp[8] !=0)){
            guoredu[8]=temp[8]-H_module[0];
            if(guoredu[8]<=5)  stat_buffer.time_buffer[GUOREDUH13_STATIC1]++;
            else if((guoredu[8]>5)&&(guoredu[8]<=10))  stat_buffer.time_buffer[GUOREDUH13_STATIC2]++;
            else if((guoredu[8]>10)&&(guoredu[8]<=25))  stat_buffer.time_buffer[GUOREDUH13_STATIC3]++;
            else if(guoredu[8]>25)    stat_buffer.time_buffer[GUOREDUH13_STATIC4]++;
        }
        }

        if((sys_mode == 5)&&((oil_flag == 1)||(frest_flag ==1))&&(machine4)) {
            if((temp[9] !=0)){
            guoredu[9]=temp[9]-H_module[0];
            if(guoredu[9]<=5)  stat_buffer.time_buffer[GUOREDUH14_STATIC5]++;
            else if((guoredu[9]>5)&&(guoredu[9]<=10))  stat_buffer.time_buffer[GUOREDUH14_STATIC6]++;
            else if((guoredu[9]>10)&&(guoredu[9]<=25))  stat_buffer.time_buffer[GUOREDUH14_STATIC7]++;
            else if(guoredu[9]>25)    stat_buffer.time_buffer[GUOREDUH14_STATIC8]++;
        }
        }
    

    /**********************************正常运行时的排气温度***********************************/

    for(i = 0 ; i <4; i++) {
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine1[i])) {
            if(tenp[i]!= 0){
            if((time_account(&timerbf13[i]))>=300) {
                if(i == 0 ) {
                    if(tenp[i]<=50)  stat_buffer.time_buffer[PAIQI1_STATIC1]++;
                    else if((tenp[i]>50)&&(tenp[i]<=70))  stat_buffer.time_buffer[PAIQI1_STATIC2]++;
                    else if((tenp[i]>70)&&(tenp[i]<=107))  stat_buffer.time_buffer[PAIQI1_STATIC3]++;
                    else if(tenp[i]>107)    stat_buffer.time_buffer[PAIQI1_STATIC4]++;
                } else {
                    if(tenp[i]<=50)  stat_buffer.time_buffer[PAIQI2_STATIC1+(i-1)*8]++;
                    else if((tenp[i]>50)&&(tenp[i]<=70))  stat_buffer.time_buffer[PAIQI2_STATIC2+(i-1)*8]++;
                    else if((tenp[i]>70)&&(tenp[i]<=107))  stat_buffer.time_buffer[PAIQI2_STATIC3+(i-1)*8]++;
                    else if(tenp[i]>107)    stat_buffer.time_buffer[PAIQI2_STATIC4+(i-1)*8]++;
                }
            }
            }
        } else {
            memset(&timerbf13[i],0,sizeof(timerbf13[i]));
        }
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine2[i])) {
             if(tenp[i+4]!= 0){
            if((time_account(&timerbf14[i]))>=300) {
                if(i == 0 ) {
                    if(tenp[i+4]<=50)  stat_buffer.time_buffer[PAIQI1_STATIC5]++;
                    else if((tenp[i+4]>50)&&(tenp[i+4]<=70))  stat_buffer.time_buffer[PAIQI1_STATIC6]++;
                    else if((tenp[i+4]>70)&&(tenp[i+4]<=107))  stat_buffer.time_buffer[PAIQI1_STATIC7]++;
                    else if(tenp[i+4]>107)    stat_buffer.time_buffer[PAIQI1_STATIC8]++;
                } else {
                    if(tenp[i+4]<=50)  stat_buffer.time_buffer[PAIQI2_STATIC5+(i-1)*8]++;
                    else if((tenp[i+4]>50)&&(tenp[i+4]<=70))  stat_buffer.time_buffer[PAIQI2_STATIC6+(i-1)*8]++;
                    else if((tenp[i+4]>70)&&(tenp[i+4]<=107))  stat_buffer.time_buffer[PAIQI2_STATIC7+(i-1)*8]++;
                    else if(tenp[i+4]>107)    stat_buffer.time_buffer[PAIQI2_STATIC8+(i-1)*8]++;
                }
            }
            }
        } else {
            memset(&timerbf14[i],0,sizeof(timerbf14[i]));
        }
        }
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine3)) {
             if(tenp[i]!= 0){
            if((time_account(&timerbf13[4]))>=300) {
                if(tenp[8]<=50)  stat_buffer.time_buffer[PAIQI13_STATIC1]++;
                else if((tenp[8]>50)&&(tenp[8]<=70))  stat_buffer.time_buffer[PAIQI13_STATIC2]++;
                else if((tenp[8]>70)&&(tenp[8]<=107))  stat_buffer.time_buffer[PAIQI13_STATIC3]++;
                else if(tenp[8]>107)    stat_buffer.time_buffer[PAIQI13_STATIC4]++;
            }
            }
        } else memset(&timerbf13[4],0,sizeof(timerbf13[4]));
        
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&(oil_flag!= 1)&&(frest_flag!=1)&&(machine4)) {
            if(tenp[i]!= 0){
            if((time_account(&timerbf13[5]))>=300) {
                if(tenp[9]<=50)  stat_buffer.time_buffer[PAIQI14_STATIC5]++;
                else if((tenp[9]>50)&&(tenp[9]<=70))  stat_buffer.time_buffer[PAIQI14_STATIC6]++;
                else if((tenp[9]>70)&&(tenp[9]<=107))  stat_buffer.time_buffer[PAIQI14_STATIC7]++;
                else if(tenp[9]>107)    stat_buffer.time_buffer[PAIQI14_STATIC8]++;
            }
            }
        } else {
            memset(&timerbf13[5],0,sizeof(timerbf13[5]));
        }
    
    /**********************************气旁通统计***********************************/
    for(i = 0 ; i <4; i++) {
        if(qipangtong[i])   stat_buffer.time_buffer[QIPANGTONG1_TIME+i]++;           // 时间统计

        if(QPT_flag[i] !=qipangtong[i]) {
            stat_buffer.count_buffer[QIPANGTONG1_COUNT+i]++;          //次数统计
            QPT_flag[i]  = qipangtong[i];
        }
    }
    /**********************************外机辅热统计***********************************/
    for(i = 0 ; i <4; i++) {
        if(warm_band1[i]){
            stat_buffer.time_buffer[WARMBAND11_TIME+i*2]++;           // 时间统计
        }
        if(warm_band2[i]){   
            stat_buffer.time_buffer[WARMBAND12_TIME+i*2]++;
        }
    }
    if(warm_band3)   stat_buffer.time_buffer[WARMBAND3_TIME+i*2]++;
    /**********************************均油阀统计***********************************/
    for(i = 0 ; i <4; i++) {
        if(junyoufa1[i]){  
            stat_buffer.time_buffer[JUNYOUFA11_TIME+i*2]++;           // 时间统计
        }
        if(junyoufa2[i]){  
            stat_buffer.time_buffer[JUNYOUFA12_TIME+i*2]++;
        }
        if(JYF_flag1[i] !=junyoufa1[i]) {
            stat_buffer.count_buffer[JUNYOUFA11_COUNT+i*2]++;
            JYF_flag1[i] = junyoufa1[i];
        }
        if(JYF_flag2[i] !=junyoufa2[i]) {
            stat_buffer.count_buffer[JUNYOUFA12_COUNT+i*2]++;
            JYF_flag2[i] = junyoufa2[i];
        }
    }
    /**********************************回油阀统计***********************************/
    for(i = 0 ; i <4; i++) {
        if(huiyoufa1[i]){
            stat_buffer.time_buffer[HUIYOUFA11_TIME+i*2]++;           // 时间统计
        }
        if(huiyoufa2[i]){
            stat_buffer.time_buffer[HUIYOUFA12_TIME+i*2]++;
        }
        if(HYF_flag1[i] !=huiyoufa1[i]) {
            stat_buffer.count_buffer[HUIYOUFA11_COUNT+i*2]++;
            HYF_flag1[i] = huiyoufa1[i];
        }
        if(HYF_flag2[i] !=huiyoufa2[i]) {
            stat_buffer.count_buffer[HUIYOUFA12_COUNT+i*2]++;
            HYF_flag2[i] = huiyoufa2[i];
        }
    }
    if(huiyoufa3){
        stat_buffer.time_buffer[HUIYOUFA3_TIME]++;
    }
    if(huiyoufa4){
        stat_buffer.time_buffer[HUIYOUFA4_TIME]++;
    }
//    if(HYF_flag3 !=huiyoufa3) {
//        stat_buffer.count_buffer[HUIYOUFA11_COUNT+i*2]++;
//        HYF_flag3 = huiyoufa3;
//    }
//    if(HYF_flag4 !=huiyoufa4) {
//        stat_buffer.count_buffer[HUIYOUFA11_COUNT+i*2]++;
//        HYF_flag4 =huiyoufa4;
//    }
    if(HYF_flag3 !=huiyoufa3) {
        stat_buffer.count_buffer[HUIYOUFA3_COUNT]++;
        HYF_flag3 = huiyoufa3;
    }
    if(HYF_flag4 !=huiyoufa4) {
        stat_buffer.count_buffer[HUIYOUFA4_COUNT]++;
        HYF_flag4 =huiyoufa4;
    }
    /**********************************平衡阀统计***********************************/
    for(i = 0 ; i <4; i++) {
        if(pinghengfa1[i]) {
            stat_buffer.time_buffer[PINGHENGFA11_TIME+i*2]++;           // 时间统计
        }
        if(pinghengfa2[i]) {
            stat_buffer.time_buffer[PINGHENGFA12_TIME+i*2]++;
        }
        if(PHF_flag1[i] !=pinghengfa1[i]) {
            stat_buffer.count_buffer[PINGHENGFA11_COUNT+i*2]++;
            PHF_flag1[i] = pinghengfa1[i];
        }
        if(PHF_flag2[i] !=pinghengfa2[i]) {
            stat_buffer.count_buffer[PINGHENGFA12_COUNT+i*2]++;
            PHF_flag2[i] = pinghengfa2[i];
        }
    }
    if(pinghengfa3)   stat_buffer.time_buffer[PINGHENGFA3_TIME]++;
    if(PHF_flag3 !=pinghengfa3) {
        stat_buffer.count_buffer[PINGHENGFA3_COUNT+i*2]++;
        PHF_flag3 =pinghengfa3;
    }
    /**********************************压缩机启动失败次数***********************************/
    for(i = 0 ; i <4; i++) {
        if(machine1_fail[i] ==1) {
            if(oldflag1_fail[i] == 1) {
                oldflag1_fail[i]= 0;
                stat_buffer.count_buffer[MACHINE11FAIL_COUNT+i*2]++;
            }
        } else {
            oldflag1_fail[i]= 1 ;
        }
        if(machine2_fail[i] ==1) {
            if(oldflag2_fail[i] == 1) {
                oldflag2_fail[i]= 0;
                stat_buffer.count_buffer[MACHINE12FAIL_COUNT+i*2]++;
            }
        } else {
            oldflag2_fail[i]= 1 ;
        }
    }
    if(machine3_fail ==1) {
        if(oldflag3_fail == 1) {
            oldflag3_fail= 0;
            stat_buffer.count_buffer[MACHINE3FAIL_COUNT]++;
        }
    } else {
        oldflag3_fail= 1 ;
    }
    /**********************************直流母线电压统计***********************************/
    for(i = 0 ; i <4; i++) {
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&machine1[i]) {
            if(machine1_voltage[i] < 370 )  stat_buffer.time_buffer[MACHINE1_VOLTAGE11+i*3]++;
            else if((machine1_voltage[i] >= 370 )&&(machine1_voltage[i] <= 780 ))  stat_buffer.time_buffer[MACHINE1_VOLTAGE12+i*3]++;
            else if(machine1_voltage[i] > 780)    stat_buffer.time_buffer[MACHINE1_VOLTAGE13+i*3]++;
        }
        if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&machine2[i]) {
            if(machine2_voltage[i] < 370 )  stat_buffer.time_buffer[MACHINE2_VOLTAGE11+i*3]++;
            else if((machine2_voltage[i] >= 370 )&&(machine2_voltage[i] <= 780 ))  stat_buffer.time_buffer[MACHINE2_VOLTAGE12+i*3]++;
            else if(machine2_voltage[i] > 780)    stat_buffer.time_buffer[MACHINE2_VOLTAGE13+i*3]++;
        }
    }
    if(((sys_mode==2)||(sys_mode == 3)||(sys_mode == 5))&&machine3) {
        if(machine3_voltage < 370 )  stat_buffer.time_buffer[MACHINE3_VOLTAGE1]++;
        else if((machine3_voltage >= 370 )&&(machine3_voltage <= 780 ))  stat_buffer.time_buffer[MACHINE3_VOLTAGE2]++;
        else if(machine3_voltage> 780)    stat_buffer.time_buffer[MACHINE3_VOLTAGE3]++;
    }
}

uint8_t F4_CONNECT; 
uint16_t F4_COUNT ;
uint8_t sign_everyday(uint8_t curSta) 
{
    uint8_t err = 0 , res = 0 ;
    uint8_t flash_flag = 0;
    uint8_t flag_flash = 0;
    static uint16_t prevHour = 0;
    static uint16_t nextHour = 0;
    //nextHour = (stat_buffer.time_buffer[WORK_TIME] )/ 3600;//根据测试结果对一个小时的累积数据进行了修正
    nextHour = (f4_time)/ 3600;
//    if(prevHour == 0){prevHour = nextHour;}
    if((nextHour - prevHour)){
        flash_flag = 0xBB; 
        connect_flag = 1;  
        if(F4_COUNT == 65535) F4_COUNT = 0;
        else F4_COUNT++;
        
        /*调试程序，用于打印写flash之前和之后的数据*/
//         if(dbgPrintf)(*dbgPrintf)("/******F4 data befor Write flash !******/\r\n");
//         if(dbgPrintf)(*dbgPrintf)("F4_COUNT:%d ; \t WORK_TIME :%d",F4_COUNT,stat_buffer.time_buffer[WORK_TIME]);
//         if(dbgPrintf)(*dbgPrintf)("/****************************************/\r\n");
        /*over*/ 
         
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
        sFLASH_WriteBuffer(&flash_flag ,STATISTIC_START_ADDR,1);
        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,STATISTIC_START_ADDR+1,2);
//        memset((uint8_t *)&stat_buffer,0,sizeof(stat_buffer));
        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,STATISTIC_START_ADDR+3,sizeof(stat_buffer));
        OSMutexPost(MutexFlash);
       
//        if(isGetTime == 1){ 
        if(!ConSta.Status){
            if(!connectToServer()){
                while(!disConnectToServer());
                return curSta; 
            }
        if(!send89Frame(&g_TcpType)){
            while(!disConnectToServer());
            return curSta;
        }
        
        if(!sendF3Frame(&g_TcpType)){
            while(!disConnectToServer());
            return curSta;
        }
#if 1
        if(!send91Frame(&g_TcpType , 0xFF)){
            while(!disConnectToServer());
            return curSta;
        }        
#endif    
        F4_CONNECT = 1;

        }
//      }

//        if((ConSta.Status)&&(isGetTime == 1)){   
        if(ConSta.Status){
//            F4_COUNT ++; 
            getLocalTime(&(&g_TcpType)->ValidData.frameF4Tx.Time);

            res = sendF4Frame(&g_TcpType , 0,0x01,(uint8_t *)stat_buffer.time_buffer, 150*sizeof(uint32_t) ,F4_COUNT);
            if(!res){
                return curSta;
            }

            res = sendF4Frame(&g_TcpType , 150*sizeof(uint32_t),0x01,(uint8_t *)stat_buffer.time_buffer, (MACHINE3_VOLTAGE3-150+1)*sizeof(uint32_t),F4_COUNT);

            if(!res){
                return curSta;
            }
         
            res = sendF4Frame(&g_TcpType , 0,0x02,(uint8_t *)stat_buffer.count_buffer, (MACHINE3FAIL_COUNT+1)*sizeof(uint16_t),F4_COUNT);
       
            if(res){        
                prevHour = nextHour;  
                
                /*调试程序，用于打印写flash之前和之后的数据*/
//                sFLASH_ReadBuffer(&flag_flash, STATISTIC_START_ADDR, 1);
//                if(dbgPrintf)(*dbgPrintf)("/******F4 data after Write flash !******/\r\n");
//                if(dbgPrintf)(*dbgPrintf)("F4_COUNT:%d ; \t WORK_TIME :%d \t flash_flag :%d",F4_COUNT,stat_buffer.time_buffer[WORK_TIME],flag_flash);
//                if(dbgPrintf)(*dbgPrintf)("/****************************************/\r\n");
                /*over*/ 
                
                return curSta;
            }
        }
        return curSta;
    }else{
        return curSta;
    }
}