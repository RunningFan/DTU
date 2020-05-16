
#ifndef _COMMON_H_
#define _COMMON_H_

    /*   扇区大小，64KBytes  */
#define SECTOR_SIZE                 0x00010000
    /*  子扇区，大小4KByte  s  */
#define SUB_SECTOR_SIZE             0x00001000
    /*  统计数据区，大小2KBytes  */
#define STATISTIC_WRITE_SIZE        0x00000800
    /*  CAN缓存，大小1KBytes */
#define CAN_WRITE_SIZE              0x00000400

    /*故障缓存区，大小15MBytes*/
#define ERR_CAHCE_START_ADDR        0x00000000
#define ERR_CAHCE_END_ADDR          0x00EAFFFF
/******************************************************************************/
    /* 参数预留区，大小952KB */
//#define PARAM_RESERVE_START_ADDR    0x00F00000
//#define PARAM_RESERVE_END_ADDR      0x00FF0000

    /*升级程序缓存区1M*/
#define PARAM_UPDATE_START_ADDR     0x00EB0000
#define PARAM_UPDATE_END_ADDR       0x00FAFFFF

/*程序备份区  256k*/
#define PARAM_BACKUP_START_ADDR     0x00FB0000    //不能变动
#define PARAM_BACKUP_END_ADDR       0x00FEFFFF
//    /*故障信息区，大小4KBytes*/
//#define ERR_INFO_START_ADDR         0x00FFE000
//#define ERR_INFO_END_ADDR           0x00FFEFFF
//    /*GPRS掉电参数区，大小4KBytes*/
//#define GPRS_INFO_START_ADDR        0x00FFF000
//#define GPRS_INFO_END_ADDR          0x00FFFFFF

    /*故障信息区，大小4KBytes*/
#define ERR_INFO_START_ADDR         0x00FF0000
#define ERR_INFO_END_ADDR           0x00FF0FFF
    /*GPRS掉电参数区，大小4KBytes*/
#define GPRS_INFO_START_ADDR        0x00FF1000
#define GPRS_INFO_END_ADDR          0x00FF1FFF

    /*参数配置表，大小4KBytes*/
#define PARAM_CONFIG_START_ADDR     0x00FF2000
#define PARAM_CONFIG_END_ADDR       0x00FF2FFF
    /*GPRS掉电参数区，大小4KBytes*/
#define STATISTIC_START_ADDR        0x00FF3000
#define STATISTIC_END_ADDR          0x00FF3FFF

/*GPRS掉电记录当前运行模式区，大小4KBytes*/
#define VAR_START_ADDR        0x00FF4000
#define VAR_END_ADDR          0x00FF4FFF

/*GPRS掉电记录重连次数，大小4KBytes*/
#define RECONNECT_START_ADDR   0x00FF5000
#define RECONNECT_END_ADDR     0x00FF5FFF

/*参数配置表，大小4KBytes*/
#define RECEIVE_PAGE_START_ADDR   0x00FF6000
#define RECEIVE_PAGE_END_ADDR     0x00FF6FFF
/*程序升级信息区 大小4kBytes*/
#define UPDATE_INFO_START_ADDR     0x00FF7000   //不能变动
#define UPDATE_INFO_END_ADDR       0x00FF7FFF

//注册不上开飞行，复位次数
#define  UPDATE_PARAM_START_ADDR     0x00FF8000
#define  UPDATE_PARAM_END_ADDR       0x00FF8FFF
#endif