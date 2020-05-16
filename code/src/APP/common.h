
#ifndef _COMMON_H_
#define _COMMON_H_

    /*   扇区大小，64KBytes  */
#define SECTOR_SIZE                 0x00010000

#define SECTOR_SIZE_8K				0x00002000


    /*  子扇区，大小4KByte  s  */
#define SUB_SECTOR_SIZE             0x00001000


    /*  统计数据区，大小2KBytes  */
#define STATISTIC_WRITE_SIZE        0x00000800


    /*  CAN缓存，大小1KBytes */
#define CAN_WRITE_SIZE              0x00000400

    /*故障缓存区，大小12MBytes*/
#define ERR_CAHCE_START_ADDR        0x00000000
#define ERR_CAHCE_END_ADDR          0x00BAFFFF

/*
*       2^20 = 1M = 1048576  = 0x100000
*       2^10 = 1K = 1024    = 0x400
*/

    /* 用户行为习惯，大小 1 M */
#define USERHABIT_START_ADDR    0x00BB0000
#define USERHABIT_END_ADDR      0x00CAFFFF

/*******************************大数据采集数据缓存区********************************/
/****************************第一小时大数据采集数据缓存区    **************************/
#define	BIGDATA_SAMPLE_START_ONE_ADDR       0x00CB0000
#define BIGDATA_SAMPLE_END_ONE_ADDR         0x00DAFFFF

/****************************第二小时大数据采集数据缓存区    ************************/
#define BIGDATA_SAMPLE_START_TWO_ADDR       0x00DB0000
#define	BIGDATA_SAMPLE_END_TWO_ADDR         0x00EAFFFF

    /*升级程序缓存区1M*/
#define PARAM_UPDATE_START_ADDR     0x00EB0000
#define PARAM_UPDATE_END_ADDR       0x00FAFFFF

/*程序备份区  256k*/
#define PARAM_BACKUP_START_ADDR     0x00FB0000
#define PARAM_BACKUP_END_ADDR       0x00FEFFFF

    /*故障信息区，大小4KBytes*/
/******** g_Upload *******/

#define ERR_INFO_START_ADDR         0x00FF0000
#define ERR_INFO_END_ADDR           0x00FF0FFF

    /*GPRS掉电参数区，大小4KBytes*/
#define GPRS_INFO_START_ADDR        0x00FF1000
#define GPRS_INFO_END_ADDR          0x00FF1FFF

    /*功率帧掉电保存区，大小4KBytes*/
#define POWER_BUFF_START_ADDR     0x00FF2000
#define POWER_BUFF_END_ADDR       0x00FF2FFF

    /*GPRS统计参数区，大小4KBytes*/
#define STATISTIC_START_ADDR        0xFF3000
#define STATISTIC_END_ADDR          0xFF3FFF

/*GPRS掉电记录当前运行模式区，大小4KBytes*/
#define VAR_START_ADDR        0xFF4000
#define VAR_END_ADDR          0xFF4FFF

/*GPRS统计参数备份区域，大小4KBytes*/
#define BACKUP_STATISTIC_START_ADDR   0xFF5000
#define BACKUP_STATISTIC_END_ADDR     0xFF5FFF

/*参数配置表，大小4KBytes*/
#define RECEIVE_PAGE_START_ADDR   0xFF6000
#define RECEIVE_PAGE_END_ADDR     0xFF6FFF

/*程序升级信息区 大小4kBytes*/
#define UPDATE_INFO_START_ADDR     0xFF7000
#define UPDATE_INFO_END_ADDR       0xFF7FFF

//注册不上开飞行，复位次数
#define UPDATE_PARAM_START_ADDR     0xFF8000
#define  UPDATE_PARAM_END_ADDR       0xFF8FFF

#endif
