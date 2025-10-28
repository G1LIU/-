#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"
#include <stdint.h>

// ---------- 配置区（根据实际接线修改） ----------
#define OLED_SCL_GPIO_PORT    GPIOB
#define OLED_SCL_PIN          GPIO_Pin_8

#define OLED_SDA_GPIO_PORT    GPIOB
#define OLED_SDA_PIN          GPIO_Pin_9

// SSD1306 I2C address (not used for bit-bang, kept for reference)
#define SSD1306_ADDR 0x3C

// 屏幕分辨率
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// 基本 API
void OLED_Init(void);
void OLED_DisplayOn(void);//OLED显示开启
void OLED_DisplayOff(void);//OLED显示关闭
void OLED_Clear(void);
void OLED_UpdateScreen(void); // 把内存 buffer 刷到屏上（若使用页面模式则直接写）
void OLED_SetPos(uint8_t x, uint8_t y);//定义OLED的坐标

// 简单绘图与文字（文字依赖 font 数据）
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_Fill(uint8_t color);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len);
void OLED_ShowHexNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
void OLED_ShowBinNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
uint32_t OLED_Pow(uint32_t base, uint32_t exp);


// 若要使用或替换字体，请在工程其他文件定义 Font6x8，类型如下：
// extern const uint8_t Font6x8[][6]; // 每字符6字节，ASCII 32~127
// 本驱动中如未提供完整字体数组，则需自行添加或替换显示函数实现。
// ------------------------------------------------

#endif /* __OLED_H */
