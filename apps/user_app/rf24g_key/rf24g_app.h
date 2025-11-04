#ifndef _RF24G_APP_H_
#define _RF24G_APP_H_

#include "board_ac632n_demo_cfg.h"

#if TCFG_RF24GKEY_ENABLE

#pragma pack(1)
// 遥控配�??
typedef struct
{
    u8 pair[3];
    u8 flag; // 0:表示该数组没使用�?0xAA：表示改数组已配对使�?
} rf24g_pair_t;

// 2.4G遥控 不同的遥控，该结构体不一�?
typedef struct
{
    u8 header1;      //
    u8 header2;      //
    u8 sum_h;        //
    u8 sum_l;        //
    u16 vid;         //
    u8 dynamic_code; //  动态码 包号
    u8 type;         // 不变的，
    u16 signatures;  // 不变的厂�?
    u8 key_v;
    u8 remoter_id;
    u8 k_crc;
} rf24g_ins_t; // 指令数据

struct RF24G_PARA
{

    u8 rf24g_rx_flag;
    u8 last_dynamic_code;
    u8 last_key_v;
    u8 rf24g_key_state;
    u8 clink_delay_up;
    u8 long_press_cnt;
    u16 hold_pess_cnt;
    const u16 is_long_time;
    const u8 is_click;
    const u8 is_long;
    const u8 _sacn_t;
};

#pragma pack()

// 硬件按键值定义
// 天奕幻彩流星灯
#define RF24_K1 0x06
// #define RF24_K2 0x01
#define RF24_K2 0x1E // 测试时使用
#define RF24_K3 0x05
#define RF24_K4 0x0C
#define RF24_K5 0x0B
// #define RF24_K5 0x2E // 测试时使用
#define RF24_K6 0x07
#define RF24_K7 0x11
#define RF24_K8 0x12
#define RF24_K9 0x0D
#define RF24_K10 0x17
#define RF24_K11 0x18
#define RF24_K12 0x13

// 用户键值定制
// 天奕幻彩流星灯
#define RF24_1H RF24_K1
#define RF24_ON_OFF RF24_K2
#define RF24_BRT_PUL RF24_K3
#define RF24_WARM RF24_K4
#define RF24_M RF24_K5
#define RF24_WHITE RF24_K6
#define RF24_BRT_SUB RF24_K7
#define RF24_ONE RF24_K8
#define RF24_READ RF24_K9
#define RF24_TWO RF24_K10
#define RF24_THREE RF24_K11
#define RF24_NIGHT RF24_K12

extern rf24g_pair_t rf24g_pair[]; // 需要写flash
extern rf24g_ins_t rf24g_ins;

void RF24G_Key_Handle(void);

#endif
#endif
