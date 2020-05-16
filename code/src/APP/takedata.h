#pragma once
//#ifndef __TAKEDATA_H__
//#define __TAKEDATA_H__

#include "duplicate.h"

#define TAKE_PHOTO_TIME		300
#define TAKE_PHOTO_DEFAULT_TIME		1800
#define HABITPARAMNUM	5
#define PRECISIONPARAMNUM	18
//#define PRECISION_DATA_TYPE		8
#define PRECISIONPARAMNUM_OUTDOOR	15
#define PRECISIONPARAMNUM_INDOOR	8
#define PRECISION_DATA_TYPE_OUTDOOR		8
#define PRECISION_DATA_TYPE_INDOOR		3

#define user_habit_buff_size 50
#define RECV_STATE		0u
#define RECV_OVER		1u

#define INDOOR_TYPE		0x01
#define OUTDOOR_TYPE	0x00

#define INDOOR_TMP_ADDR		0x2000
#define OUTDOOR_TMP_ADDR	0x205F

#define REPEAT_YES	0x01
#define REPEAT_NO	0x00

#define END_FLAG_YES	0x01
#define END_FLAG_NO		0x00

//#define USING_USER_HABIT



#ifdef USING_USER_HABIT
typedef struct _user_habit_param{
	uint16_t src_add[HABITPARAMNUM];
	uint8_t high_low_flag[HABITPARAMNUM];
	uint8_t cnt;
}user_habit_param;

typedef struct _user_habit_t{
    uint32_t write_addr;
    uint32_t read_addr;  
}user_habit_t;
#endif


//typedef struct _power_data_t{
//    uint32_t write_addr;
//    uint32_t read_addr;  
//}power_data_t;


typedef struct _control_precision{
	uint16_t src_add[PRECISIONPARAMNUM];
	uint8_t high_low_flag[PRECISIONPARAMNUM];
	uint8_t second_get[PRECISIONPARAMNUM];//第二次取到该值时，才开始降精度处理
	uint8_t	cnt;
}control_precision;

typedef struct _indoor_bigdata_control_precision{
	uint16_t src_add[PRECISION_DATA_TYPE_INDOOR][PRECISIONPARAMNUM_INDOOR];
	uint8_t high_low_flag[PRECISION_DATA_TYPE_INDOOR][PRECISIONPARAMNUM_INDOOR];
//	uint8_t second_get[PRECISION_DATA_TYPE_INDOOR][PRECISIONPARAMNUM_INDOOR];//第二次取到该值时，才开始降精度处理
	uint8_t precision[PRECISION_DATA_TYPE_INDOOR];
	uint8_t	cnt[PRECISION_DATA_TYPE_INDOOR];	
}indoor_bigdata_control_precision;

typedef struct _outdoor_bigdata_control_precision{
	uint16_t src_add[PRECISION_DATA_TYPE_OUTDOOR][PRECISIONPARAMNUM_OUTDOOR];
	uint8_t high_low_flag[PRECISION_DATA_TYPE_OUTDOOR][PRECISIONPARAMNUM_OUTDOOR];
//	uint8_t second_get[PRECISION_DATA_TYPE_OUTDOOR][PRECISIONPARAMNUM_OUTDOOR];//第二次取到该值时，才开始降精度处理
	uint8_t precision[PRECISION_DATA_TYPE_OUTDOOR];
	uint8_t cnt[PRECISION_DATA_TYPE_OUTDOOR];

}outdoor_bigdata_control_precision;

uint8_t Find96Take(uint16_t SrcAdd, const Real_PageTpye *real_PageData);
#ifdef USING_USER_HABIT
uint8_t read_user_data();
extern user_habit_t user_data;
#endif
extern OS_EVENT * MSemExitWait;

extern uint8_t take_photo_style;
void Quick_Photo(void);
#ifdef USING_USER_HABIT
void user_habit(const SrcDataTyp  *Page, uint8_t *m_Data, const CanMsgType *m_Msg, indoorinfoType * indoorinfo, uint8_t bit_location);
#endif
uint8_t judge_precision(uint16_t SrcAdd, uint8_t lnewvalue, uint8_t loldvalue, uint8_t hnewvalue, uint8_t holdvalue);
//uint8_t bigdata_judge_precision(uint16_t SrcAdd, uint8_t lnewvalue, uint8_t loldvalue, uint8_t hnewvalue, uint8_t holdvalue,uint8_t param_type);
uint8_t bigdata_judge_precision(uint16_t SrcAdd,const CanMsgType *m_Msg, uint8_t location_can, uint8_t *m_data, uint8_t location_data,uint8_t param_type);
//uint8_t judge_precision_second();
uint8_t judge_precision_second(uint16_t SrcAdd,uint8_t *m_data, uint8_t location_data, uint8_t *m_data_pri, uint8_t location_data_pri,uint8_t param_type);




void take_a_photo(const PageTpye *PageData,uint8_t *m_Data, uint8_t *d_Changflag, outdoorinfoType *outdoorinfo, indoorinfoType *indoorinfo,uint8_t param_type);

//void online_transmit_table(uint8_t systemnum);
void take_photo(uint16_t SrcAdd, uint8_t value, outdoorinfoType * outdoorinfo, indoorinfoType *indoorinfo, uint8_t paramtype, uint8_t endflag);
void quick_photo(void);

//#endif



















