#include "led_strip_sys.h"
#include "led_strip_drive.h"
#include "asm/adc_api.h"
#include "asm/mcpwm.h"
#include "led_strand_effect.h"

#define LEDC_PIN IO_PORTB_07
// #define LEDC_PIN    IO_PORTA_01

#define _R_PIN IO_PORTA_01
#define _G_PIN IO_PORTA_02
#define _B_PIN IO_PORTA_07
#define _C_PIN IO_PORTB_06
#define _W_PIN IO_PORTB_05

#define MIC_PIN IO_PORTA_08
#define MIC_CH AD_CH_PA8

/********** 幻彩灯输出口 初始化*****************/
/********** 幻彩灯输出口 初始化*****************/

const struct ledc_platform_data ledc_data =
    {
        .index = 0,       // 控制器号
        .port = LEDC_PIN, // 输出引脚
        .idle_level = 1,  // 当前帧的空闲电平，0：低电平， 1：高电平
        .out_inv = 1,     // 起始电平，0：高电平开始， 1：低电平开始
        .bit_inv = 1,     // 取数据时高低位镜像，0：不镜像，1：8位镜像，2:16位镜像，3:32位镜像
        .t_unit = t_42ns, // 时间单位
        .t1h_cnt = 21,    // 1码的高电平时间 = t1h_cnt * t_unit;21*42=882
        .t1l_cnt = 7,     // 1码的低电平时间 = t1l_cnt * t_unit;7*42=294
        .t0h_cnt = 8,     // 0码的高电平时间 = t0h_cnt * t_unit;8*42=336
        .t0l_cnt = 30,    // 0码的低电平时间 = t0l_cnt * t_unit;*/30*42=1260

        .t_rest_cnt = 20000, // 复位信号时间 = t_rest_cnt * t_unit;20000*42=840000
        .cbfun = NULL,       // 中断回调函数
};

void led_state_init(void)
{
    ledc_init(&ledc_data);
}

/********** 七彩灯输出口 初始化*****************/
/********** 七彩灯输出口 初始化*****************/
void led_gpio_init(void)
{

    gpio_set_die(_R_PIN, 1);
    gpio_direction_output(_R_PIN, 0);

    gpio_set_die(_G_PIN, 1);
    gpio_direction_output(_G_PIN, 0);

    gpio_set_die(_B_PIN, 1);
    gpio_direction_output(_B_PIN, 0);

    gpio_set_die(_C_PIN, 1);
    gpio_direction_output(_C_PIN, 0);

#if LED_STRIP_RGBCW
    gpio_set_die(_W_PIN, 1);
    gpio_direction_output(_W_PIN, 0);

#endif
}

void led_pwm_init(void)
{
    // R
    struct pwm_platform_data pwm_p_data;
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch0;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = _R_PIN;                      // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // G
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch2;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = _G_PIN;                      // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // B
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch1;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = _B_PIN;                      // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // W
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch3;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = _C_PIN;                      // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);

#if LED_STRIP_RGBCW

    extern void timer_pwm_init(JL_TIMER_TypeDef * JL_TIMERx, u32 pwm_io, u32 fre, u32 duty);
    timer_pwm_init(JL_TIMER3, _W_PIN, 1000, 0); // 调timer做pwm  通道号 R c

#endif
}

/*********************************mic脚IO口初始化***************************************************************/

void mic_gpio_init()
{

    adc_add_sample_ch(MIC_CH); // 注意：初始化AD_KEY之前，先初始化ADC
    gpio_set_die(MIC_PIN, 0);
    gpio_set_direction(MIC_PIN, 1);
    gpio_set_pull_down(MIC_PIN, 0);
}

u16 check_mic_adc(void)
{
    return adc_get_value(MIC_CH);
}

// PWM控制函数
void fc_rgbw_driver(u8 r, u8 g, u8 b, u8 w)
{
    u32 duty1, duty2, duty3, duty4;
    duty1 = r * 10000 / 255; // 占空比转为0~10000
    duty2 = b * 10000 / 255; // 占空比转为0~10000
    duty3 = g * 10000 / 255; // 占空比转为0~010000
    duty4 = w * 10000 / 255; // 占空比转为0~010000
    // 设置一个通道的占空比
    mcpwm_set_duty(pwm_ch0, duty1); // R
    mcpwm_set_duty(pwm_ch1, duty2); // G
    mcpwm_set_duty(pwm_ch2, duty3); // B
    mcpwm_set_duty(pwm_ch3, duty4); // W
}

void fc_rgb_driver(u8 r, u8 g, u8 b)
{
    u32 duty1, duty2, duty3, duty4;
    duty1 = r * 10000 / 255; // 占空比转为0~10000
    duty2 = b * 10000 / 255; // 占空比转为0~10000
    duty3 = g * 10000 / 255; // 占空比转为0~010000

    // 设置一个通道的占空比
    mcpwm_set_duty(pwm_ch0, duty1); // R
    mcpwm_set_duty(pwm_ch1, duty2); // G
    mcpwm_set_duty(pwm_ch2, duty3); // B
}

// 色温PWM控制
// 不能放在fc_drive 否则调节时会闪
void cw_driver(u16 C, u16 W)
{
    printf("c  =%d, w =%d", C, W);
    // 占空比是0~10000
    mcpwm_set_duty(pwm_ch3, C * 100);
    set_timer_pwm_duty(JL_TIMER3, W * 100);
}

// 关闭色温
void cw_off(void)
{
    mcpwm_set_duty(pwm_ch3, 0);
    set_timer_pwm_duty(JL_TIMER3, 0);
}

/**
 * @brief 关机时调用
 *
 */
void turn_Off_Lamp(void)
{
    mcpwm_set_duty(pwm_ch0, 0);
    mcpwm_set_duty(pwm_ch1, 0);
    mcpwm_set_duty(pwm_ch2, 0);
    cw_off();
}

// 由 ws281x_show() 调用
void fc_rgbcw_driver(u8 r, u8 g, u8 b)
{
    u32 duty1, duty2, duty3, duty4;
    duty1 = r * 10000 / 255; // 占空比转为0~10000
    duty2 = b * 10000 / 255; // 占空比转为0~10000
    duty3 = g * 10000 / 255; // 占空比转为0~010000

    if (fc_effect.Now_state == ACT_CW)
    {
        mcpwm_set_duty(pwm_ch0, 0);
        mcpwm_set_duty(pwm_ch1, 0);
        mcpwm_set_duty(pwm_ch2, 0);
    }
    else if (fc_effect.Now_state == IS_light_scene &&
                 (MODE_COOL_WHITE_BREATHING == fc_effect.dream_scene.change_type) ||
             (MODE_WARM_WHITE_BREATHING == fc_effect.dream_scene.change_type))
    {
        // 处于冷白呼吸、暖白呼吸模式时，不关闭控制CW的pwm
    }
    else
    {
        // 设置一个通道的占空比
        mcpwm_set_duty(pwm_ch0, duty1); // R
        mcpwm_set_duty(pwm_ch1, duty2); // G
        mcpwm_set_duty(pwm_ch2, duty3); // B
        cw_off();
    }
}
