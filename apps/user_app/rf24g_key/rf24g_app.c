
#include "system/includes.h"
#include "task.h"
#include "event.h"
#include "btstack/btstack_typedef.h"
#include "app_config.h"
#include "rf24g_app.h"
#include "WS2812FX.H"
#include "led_strip_sys.h"

#include "../../../apps/user_app/one_wire/one_wire.h"
#include "../../../apps/user_app/led_strip/led_strand_effect.h"

#if (TCFG_RF24GKEY_ENABLE)

/* ==============================================================

                2.4g遥控初始化

    备注：该2.4G遥控的代码逻辑，是根据遥控而定的，可能不适宜其他遥控
=================================================================*/

// 天奕2.4G遥控
#define HEADER1 0X55
#define HEADER2 0XAA

rf24g_ins_t rf24g_ins; // 2.4g遥控数据包格式

#define SUPPORT_LONG (1)

struct RF24G_PARA is_rf24g_ = {

    .clink_delay_up = 0,
    .long_press_cnt = 0,
    .last_key_v = NO_KEY,
    .last_dynamic_code = 0,
    .rf24g_rx_flag = 0,
    .hold_pess_cnt = 0,
    .is_long_time = 1000,
    .rf24g_key_state = 0,
#if SUPPORT_LONG
    .is_click = 35, // 因没有长按功能，缩短抬起判断时间
#else
    .is_click = 10, // 因没有长按功能，缩短抬起判断时间
#endif
    .is_long = 1,
    ._sacn_t = 10,
};

static u8 Get_24G_KeyValue()
{

    if (is_rf24g_.rf24g_rx_flag)
    {
        is_rf24g_.rf24g_rx_flag = 0;

        if ((is_rf24g_.last_dynamic_code != rf24g_ins.dynamic_code) && rf24g_ins.remoter_id == 0x00)
        {

            is_rf24g_.clink_delay_up = 0;
            is_rf24g_.hold_pess_cnt = 0;
            is_rf24g_.last_key_v = rf24g_ins.key_v;
            is_rf24g_.last_dynamic_code = rf24g_ins.dynamic_code;
            return rf24g_ins.key_v; // 当前2.4G架构无用
        }
#if SUPPORT_LONG
        else
#else
        else if (rf24g_ins.remoter_id == 0x01)
#endif
        {
            is_rf24g_.clink_delay_up = 0;
            if (is_rf24g_.hold_pess_cnt >= is_rf24g_.is_long_time)
            {
                is_rf24g_.long_press_cnt++;
                is_rf24g_.hold_pess_cnt = 0;
                is_rf24g_.rf24g_key_state = KEY_EVENT_LONG;
                is_rf24g_.last_key_v = rf24g_ins.key_v;
                return rf24g_ins.key_v; // 当前2.4G架构无用
            }
        }
    }
    return NO_KEY;
}

struct key_driver_para rf24g_scan_para = {
    .scan_time = 2,         // 按键扫描频率, 单位: ms
    .last_key = NO_KEY,     // 上一次get_value按键值, 初始化为NO_KEY;
    .filter_time = 0,       // 按键消抖延时;
    .long_time = 200,       // 按键判定长按数量
    .hold_time = 200,       // 按键判定HOLD数量
    .click_delay_time = 20, // 按键被抬起后等待连击延时数量
    .key_type = KEY_DRIVER_TYPE_RF24GKEY,
    .get_value = Get_24G_KeyValue,
};

/* 长按计数器 */
void RF24G_Key_Long_Scan(void)
{

    if (rf24g_ins.remoter_id == 0x01)
    {

        is_rf24g_.hold_pess_cnt += (is_rf24g_._sacn_t + 2);
    }
    else
    {
        is_rf24g_.hold_pess_cnt = 0;
    }
}

// 底层按键扫描，由__resolve_adv_report()调用
void rf24g_scan(u8 *pBuf)
{
    rf24g_ins_t *p = (rf24g_ins_t *)pBuf;
    if (p->header1 == HEADER1 && p->header2 == HEADER2)
    {
        //  printf_buf(pBuf,sizeof(rf24g_ins_t));
        memcpy((u8 *)&rf24g_ins, pBuf, sizeof(rf24g_ins_t));
        is_rf24g_.rf24g_rx_flag = 1;
    }
}

/* ==============================================================

                2.4g遥控按键功能


=================================================================*/
void RF24G_Key_Handle(void)
{
    u8 key_value = 0;
    if (is_rf24g_.clink_delay_up < 0xFF)
        is_rf24g_.clink_delay_up++;
    // printf("is_rf24g_.clink_delay_up=%d",is_rf24g_.clink_delay_up);
    if ((is_rf24g_.clink_delay_up > is_rf24g_.is_click) && (is_rf24g_.long_press_cnt < is_rf24g_.is_long))
    {
        key_value = is_rf24g_.last_key_v;
        if (key_value == NO_KEY)
            return;
        is_rf24g_.last_key_v = NO_KEY;
        is_rf24g_.clink_delay_up = 0;
        is_rf24g_.long_press_cnt = 0;
        printf(" ------------------------duanna key_value = %x", key_value);

        /*  code  */

        // 开/关
        if (key_value == RF24_ON_OFF)
        {
            // motor_Init();
            if (DEVICE_OFF == fc_effect.on_off_flag)
            {
                // printf("fc_effect.base_ins.period = %u\n", (u16)fc_effect.base_ins.period);
                // printf("fc_effect.star_speed_index = %u\n", (u16)fc_effect.star_speed_index);
                // printf("send_base_ins = 0x%x\n", (u16)send_base_ins);

                // 开机前，可能关机前电机就开着，或者关机前电机就已经关了，开机后保持状态不变

                if (fc_effect.star_speed_index >= ARRAY_SIZE(motor_period))
                {
                    // 如果开机前，电机的周期索引超过了电机的周期数组大小，说明电机在开机前就是关着的
                    one_wire_set_mode(6); // 关闭电机
                    fc_effect.motor_on_off = DEVICE_OFF;
                }
                else
                {
                    // 如果开机前，电机的周期索引还在电机的周期数组大小内，说明电机在开机前是开着的
                    one_wire_set_period(motor_period[fc_effect.star_speed_index]);
                    one_wire_set_mode(4); // 360正转
                    fc_effect.motor_on_off = DEVICE_ON;
                }

                // if (DEVICE_OFF == fc_effect.motor_on_off &&
                //     fc_effect.star_speed_index >= ARRAY_SIZE(motor_period))
                // {
                //     // 如果电机之前是关闭的，并且当前索引已经超过数组长度
                //     fc_effect.star_speed_index = 0; // 设置电机转动速度的索引，默认从0开始
                //     one_wire_set_period(motor_period[fc_effect.star_speed_index]);
                //     one_wire_set_mode(4); // 360正转
                //     fc_effect.motor_on_off = DEVICE_ON;
                // }
                // else if (DEVICE_OFF == fc_effect.motor_on_off)
                // {
                //     // 电机是关闭的，但是索引没有超过数组长度
                //     one_wire_set_period(motor_period[fc_effect.star_speed_index]);
                //     one_wire_set_mode(4); // 360正转
                //     fc_effect.motor_on_off = DEVICE_ON;
                // }

                os_taskq_post("msg_task", 1, MSG_SEQUENCER_ONE_WIRE_SEND_INFO);

                soft_turn_on_the_light(); // 开灯

                // printf("fc_effect.base_ins.mode = %u\n", (u16)fc_effect.base_ins.mode);
                // printf("fc_effect.base_ins.period = %u\n", (u16)fc_effect.base_ins.period);
                // printf("fc_effect.base_ins.dir = %u\n", (u16)fc_effect.base_ins.dir);
                // printf("fc_effect.base_ins.music_mode = %u\n", (u16)fc_effect.base_ins.music_mode);
                // printf("fc_effect.motor_on_off = %u\n", (u16)fc_effect.motor_on_off);
            }
        }

        /*
            用电机短按按键事件来测试电池是否能够启动和停止：
        */
        // if(key_value == RF24_M) // 电机按键短按
        // {
        //     motor_Init();
        //     if (fc_effect.motor_on_off == DEVICE_ON)
        //     {
        //         fc_effect.motor_on_off = DEVICE_OFF;
        //         one_wire_set_mode(4); // 电机正转
        //     }
        //     else
        //     {
        //         fc_effect.motor_on_off = DEVICE_ON;
        //         one_wire_set_mode(6); // 电机停止
        //     }

        //     // enable_one_wire();
        //     os_taskq_post("msg_task", 1, MSG_SEQUENCER_ONE_WIRE_SEND_INFO);
        // }

        if (get_on_off_state())
        {

            if (key_value == RF24_BRT_SUB)
            {

                ls_sub_bright();
                ls_add_speed();
                ls_sub_sensitive();
            }
            if (key_value == RF24_BRT_PUL)
            {
                ls_add_bright();
                ls_sub_speed();
                ls_add_sensitive();
            }

            // 暖光
            if (key_value == RF24_WARM)
            {

                ls_Sub_CW();
            }
            // 冷光
            if (key_value == RF24_WHITE)
            {
                ls_Add_CW();
            }

            // 短按电机转速选择（电机360度连续转动）
            if (key_value == RF24_M)
            {
                // ls_set_motor_speed();
                if (DEVICE_OFF == fc_effect.motor_on_off &&
                    fc_effect.star_speed_index >= ARRAY_SIZE(motor_period))
                {
                    // 如果电机之前是关闭的，并且当前索引已经超过数组长度
                    fc_effect.star_speed_index = 0; // 设置电机转动速度的索引，默认从0开始
                    one_wire_set_period(motor_period[fc_effect.star_speed_index]);
                    one_wire_set_mode(4); // 360正转
                    fc_effect.motor_on_off = DEVICE_ON;
                }
                else if (DEVICE_OFF == fc_effect.motor_on_off)
                {
                    // 电机是关闭的，但是索引没有超过数组长度
                    one_wire_set_period(motor_period[fc_effect.star_speed_index]);
                    one_wire_set_mode(4); // 360正转
                    fc_effect.motor_on_off = DEVICE_ON;
                }
                else
                {
                    fc_effect.star_speed_index++;

                    // 超过了电机的数组索引，让索引值清零，下一次开启电机时重新从数组中取值
                    if (fc_effect.star_speed_index >= ARRAY_SIZE(motor_period) + 1)
                    {
                        fc_effect.star_speed_index = 0;
                    }

                    if (fc_effect.star_speed_index >= ARRAY_SIZE(motor_period))
                    {
                        // 超过了电机数组的索引，关闭电机
                        one_wire_set_mode(6); // 关闭电机

                        fc_effect.motor_on_off = DEVICE_OFF;
                    }
                    else
                    {
                        // 没有超过电机数组的索引，启动电机
                        one_wire_set_mode(4); // 360正转
                        one_wire_set_period(motor_period[fc_effect.star_speed_index]);

                        fc_effect.motor_on_off = DEVICE_ON;
                    }
                }

                os_taskq_post("msg_task", 1, MSG_SEQUENCER_ONE_WIRE_SEND_INFO);
                fb_motor_speed();

                // printf("fc_effect.star_speed_index %u\n", (u16)fc_effect.star_speed_index); // 打印电机的速度索引

                // printf("fc_effect.base_ins.mode = %u\n", (u16)fc_effect.base_ins.mode);
                // printf("fc_effect.base_ins.period = %u\n", (u16)fc_effect.base_ins.period);
                // printf("fc_effect.base_ins.dir = %u\n", (u16)fc_effect.base_ins.dir);
                // printf("fc_effect.base_ins.music_mode = %u\n", (u16)fc_effect.base_ins.music_mode);
                // printf("fc_effect.motor_on_off = %u\n", (u16)fc_effect.motor_on_off);
            }

            // 1H
            if (key_value == RF24_1H)
            {
                ls_1H_close();
            }
            // 1 7色跳变
            if (key_value == RF24_ONE)
            {
                ls_chose_mode_InAPP(1, 3, 0x08, 0x08);
            }
            // 2 七色渐变
            if (key_value == RF24_TWO)
            {
                ls_chose_mode_InAPP(1, 3, 0x0a, 0x0a);
            }
            // 3 七色呼吸
            if (key_value == RF24_THREE)
            {

                ls_chose_mode_InAPP(1, 0, 0x0B, 0x11);
            }
            // read 多种音乐律动切换
            if (key_value == RF24_READ)
            {

                ls_cycle_music_effect();
            }
            // ninght 多种RGB变化模式选择，RGB 静态颜色选择
            if (key_value == RF24_NIGHT)
            {

                ls_function_one();
            }
        }

        save_user_data_area3();
    }
    else if (is_rf24g_.clink_delay_up > is_rf24g_.is_click)
    {
        is_rf24g_.clink_delay_up = 0;
        is_rf24g_.long_press_cnt = 0;
        is_rf24g_.last_key_v = NO_KEY;
    }

    if (is_rf24g_.long_press_cnt > is_rf24g_.is_long)
    {

        key_value = is_rf24g_.last_key_v;
        if (key_value == NO_KEY)
            return;
        is_rf24g_.last_key_v = NO_KEY;
        /*  因为执行这里，清了键值，而长按时，1s才获取一次键值 */
        printf(" ================================longkey_value = %d", key_value);

#if SUPPORT_LONG
        /* code */
        if (key_value == RF24_ON_OFF)
        {
            // motor_Init();
            // ls_set_motor_off(); // 关电机

            fc_effect.motor_on_off = DEVICE_OFF;
            one_wire_set_mode(6); // 停止电机
            os_taskq_post("msg_task", 1, MSG_SEQUENCER_ONE_WIRE_SEND_INFO);

            soft_turn_off_lights(); // 关灯

            // printf("fc_effect.base_ins.mode = %u\n", (u16)fc_effect.base_ins.mode);
            // printf("fc_effect.base_ins.period = %u\n", (u16)fc_effect.base_ins.period);
            // printf("fc_effect.base_ins.dir = %u\n", (u16)fc_effect.base_ins.dir);
            // printf("fc_effect.base_ins.music_mode = %u\n", (u16)fc_effect.base_ins.music_mode);
            // printf("fc_effect.motor_on_off = %u\n", (u16)fc_effect.motor_on_off);
        }

        if (get_on_off_state())
        {

            // 长按电机停止（对孔）
            if (key_value == RF24_M)
            {
                // motor_Init();
                // ls_set_motor_off();
                fc_effect.motor_on_off = DEVICE_OFF;
                one_wire_set_mode(6); // 停止电机
                os_taskq_post("msg_task", 1, MSG_SEQUENCER_ONE_WIRE_SEND_INFO);

                // printf("fc_effect.base_ins.mode = %u\n", (u16)fc_effect.base_ins.mode);
                // printf("fc_effect.base_ins.period = %u\n", (u16)fc_effect.base_ins.period);
                // printf("fc_effect.base_ins.dir = %u\n", (u16)fc_effect.base_ins.dir);
                // printf("fc_effect.base_ins.music_mode = %u\n", (u16)fc_effect.base_ins.music_mode);
                // printf("fc_effect.motor_on_off = %u\n", (u16)fc_effect.motor_on_off);
            }
        }

        // 暖光
        if (key_value == RF24_WARM)
        {
            ls_Slight_Sub_CW();
        }
        // 冷光
        if (key_value == RF24_WHITE)
        {
            ls_Slight_Add_CW();
        }
        if (key_value == RF24_BRT_SUB)
        {
            ls_slight_sub_bright();
        }
        if (key_value == RF24_BRT_PUL)
        {
            ls_slight_add_bright();
        }

#endif
    }
}

#endif
