#include "OLED.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "font.h"

// --------------- 配置 / 全局 ---------------
static uint8_t OLED_Buffer[OLED_WIDTH * OLED_HEIGHT / 8]; // page buffer (128*64/8=1024)

static void I2C_GPIO_Config(void);
static void SDA_Set(void);
static void SDA_Clr(void);
static void SCL_Set(void);
static void SCL_Clr(void);
static uint8_t SDA_Read(void);

static void I2C_Start(void);
static void I2C_Stop(void);
static void I2C_SendByte(uint8_t byte);
static uint8_t I2C_WaitAck(void);
static void I2C_SendCommand(uint8_t cmd);
static void OLED_I2C_Send_Byte(uint8_t data);


// -------------- 低速 I2C (bit-bang) --------------
static void I2C_Delay(void)
{
    // 这里简单做短延时：可根据主频调整
    volatile int i = 40;
    while (i--) __NOP();
}

static void I2C_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 使能端口时钟（按需加入其它端口）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStructure);

    // 初始高电平（I2C 空闲）
    SCL_Set();
    SDA_Set();
}

static void SDA_Set(void)  { GPIO_SetBits(GPIOB, GPIO_Pin_9); }
static void SDA_Clr(void)  { GPIO_ResetBits(GPIOB, GPIO_Pin_9); }
static void SCL_Set(void)  { GPIO_SetBits(GPIOB,GPIO_Pin_8); }
static void SCL_Clr(void)  { GPIO_ResetBits(GPIOB, GPIO_Pin_8); }

// 配置 SDA 为输入并读取（用于 ACK）。为了简单，直接切换模式。
static uint8_t SDA_Read(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 设置为输入浮空
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    uint8_t val = GPIO_ReadInputDataBit(OLED_SDA_GPIO_PORT, OLED_SDA_PIN);

    // 恢复推挽输出模式（上层会设置SDA为高/低）
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    return val;
}

static void I2C_Start(void)
{
    SDA_Set();
    SCL_Set();
    I2C_Delay();
    SDA_Clr();
    I2C_Delay();
    SCL_Clr();
}

static void I2C_Stop(void)
{
    SCL_Clr();
    SDA_Clr();
    I2C_Delay();
    SCL_Set();
    I2C_Delay();
    SDA_Set();
    I2C_Delay();
}

static void I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        SCL_Clr();
        if (byte & 0x80) SDA_Set(); else SDA_Clr();
        I2C_Delay();
        SCL_Set();
        I2C_Delay();
        byte <<= 1;
    }
    SCL_Clr();
}

static uint8_t I2C_WaitAck(void)
{
    SCL_Clr();
    SDA_Set(); // release SDA
    I2C_Delay();
    SCL_Set();
    I2C_Delay();
    // read SDA
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    uint8_t ack = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
    SCL_Clr();

    // restore to output push-pull
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    return (ack == 0) ? 1 : 0;
}

// 0x00: command, 0x40: data (SSD1306 over I2C uses control byte before payload)
static void I2C_SendCommand(uint8_t cmd)
{
    I2C_Start();
    I2C_SendByte((SSD1306_ADDR << 1) | 0); // address + write
    I2C_WaitAck();
    I2C_SendByte(0x00); // control byte: Co = 0, D/C# = 0 (command)
    I2C_WaitAck();
    I2C_SendByte(cmd);
    I2C_WaitAck();
    I2C_Stop();
}

static void OLED_I2C_Send_Byte(uint8_t data)
{
    I2C_Start();
    I2C_SendByte((SSD1306_ADDR << 1) | 0);
    I2C_WaitAck();
    I2C_SendByte(0x40); // control byte: Co = 0, D/C# = 1 (data)
    I2C_WaitAck();
    I2C_SendByte(data);
    I2C_WaitAck();
    I2C_Stop();
}

// --------------- SSD1306 基本命令 ----------------
void OLED_Init(void)
{
    I2C_GPIO_Config();

    // 初始化 sequence（参考 SSD1306 手册）
    I2C_SendCommand(0xAE); // display off
    I2C_SendCommand(0x20); // set memory addressing mode
    I2C_SendCommand(0x10); // 00: horizontal, 01: vertical, 10: page
    I2C_SendCommand(0xB0); // set page start address for page addressing mode
    I2C_SendCommand(0xC8); // COM output scan direction
    I2C_SendCommand(0x00); // low column address
    I2C_SendCommand(0x10); // high column address
    I2C_SendCommand(0x40); // start line = 0
    I2C_SendCommand(0x81); // contrast control
    I2C_SendCommand(0xFF);
    I2C_SendCommand(0xA1); // segment remap
    I2C_SendCommand(0xA6); // normal display
    I2C_SendCommand(0xA8); // multiplex ratio
    I2C_SendCommand(0x3F); // 1/64
    I2C_SendCommand(0xA4); // output follows RAM content
    I2C_SendCommand(0xD3); // display offset
    I2C_SendCommand(0x00);
    I2C_SendCommand(0xD5); // display clock divide ratio/oscillator freq
    I2C_SendCommand(0x80);
    I2C_SendCommand(0xD9); // pre-charge period
    I2C_SendCommand(0xF1);
    I2C_SendCommand(0xDA); // com pins hardware config
    I2C_SendCommand(0x12);
    I2C_SendCommand(0xDB); // vcomh
    I2C_SendCommand(0x40);
    I2C_SendCommand(0x8D); // charge pump
    I2C_SendCommand(0x14);
    I2C_SendCommand(0xAF); // display ON

    OLED_Fill(0x00);
    OLED_UpdateScreen();
}

void OLED_DisplayOn(void)  { I2C_SendCommand(0xAF); }
void OLED_DisplayOff(void) { I2C_SendCommand(0xAE); }

void OLED_Clear(void)
{
    for (uint16_t i = 0; i < sizeof(OLED_Buffer); i++) OLED_Buffer[i] = 0x00;
}

void OLED_UpdateScreen(void)
{
    // SSD1306 page addressing: 8 pages (0..7) for 64 pixels height
    for (uint8_t page = 0; page < 8; page++)
    {
        I2C_SendCommand(0xB0 + page); // set page
        I2C_SendCommand(0x00);        // set lower column start address
        I2C_SendCommand(0x10);        // set higher column start address

        // send all 128 bytes in this page
        I2C_Start();
        I2C_SendByte((SSD1306_ADDR << 1) | 0); I2C_WaitAck();
        I2C_SendByte(0x40); I2C_WaitAck(); // control byte: data
        for (uint8_t col = 0; col < 128; col++)
        {
            I2C_SendByte(OLED_Buffer[page * 128 + col]);
            I2C_WaitAck();
        }
        I2C_Stop();
    }
}

void OLED_SetPos(uint8_t x, uint8_t y)
{
    // x: 0..127, y: page 0..7
    I2C_SendCommand(0xB0 + y);
    I2C_SendCommand(((x & 0xF) | 0x00));      // low col
    I2C_SendCommand(((x >> 4) | 0x10));       // high col
}

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint16_t index = (x) + (y / 8) * OLED_WIDTH;
    if (color)
        OLED_Buffer[index] |=  (1 << (y % 8));
    else
        OLED_Buffer[index] &= ~(1 << (y % 8));
}

void OLED_Fill(uint8_t color)
{
    uint8_t fill = color ? 0xFF : 0x00;
    for (uint16_t i = 0; i < sizeof(OLED_Buffer); i++) OLED_Buffer[i] = fill;
}

// NOTE: 显示字符依赖外部提供的 6x8 字体表 Font6x8 （每字符6字节，ASCII 32~127）
// 若工程中没有字体，请自行添加。下面是示范的简单实现（依赖 Font6x8）。
extern const uint8_t Font6x8[][6];

void OLED_ShowChar(uint8_t x, uint8_t y, char ch)
{
    uint8_t c = ch - 32; // 字体数组通常从 ASCII 32（空格）开始
    if (x > OLED_WIDTH - 6) return;
    if (c > (127 - 32)) return; // 超出支持范围
    for (uint8_t i = 0; i < 6; i++)
    {
        OLED_Buffer[y * 128 + x + i] = Font6x8[c][i];
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    while (*str)
    {
        OLED_ShowChar(x, y, *str);
        x += 6;
        if (x + 6 > OLED_WIDTH) { x = 0; y++; } // 换行（按 page）
        str++;
    }
}
