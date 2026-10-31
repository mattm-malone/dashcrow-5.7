#include "spi.h"
#include <SPI.h>

// Hardware SPI (FSPI) on the panel's SCK/MOSI pins; CS stays a GPIO. The transaction is opened once
// and never closed: the panel is the only device on this bus.
static SPIClass gEpdSpi(FSPI);
static bool gSpiStarted = false;

void EPD_GPIOInit(void)
{
    if (!gSpiStarted) {
        gEpdSpi.begin(SCK, -1, MOSI, -1);
        gEpdSpi.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
        gSpiStarted = true;
    }
    pinMode(RES, OUTPUT);
    pinMode(DC, OUTPUT);
    pinMode(CS, OUTPUT);
    pinMode(BUSY, INPUT);
}

/**
 * @brief       IO模拟SPI发送一个字节数据
 * @param       dat: 需要发送的字节数据
 * @retval      无
 */
void EPD_WR_Bus(uint8_t dat)
{
    EPD_CS_Clr();
    gEpdSpi.transfer(dat);
    EPD_CS_Set();
}

/**
 * @brief       向液晶写寄存器命令
 * @param       reg: 要写的命令
 * @retval      无
 */
void EPD_WR_REG(uint8_t reg)
{
    EPD_DC_Clr();
    EPD_WR_Bus(reg);
    EPD_DC_Set();
}

/**
 * @brief       向液晶写一个字节数据
 * @param       dat: 要写的数据
 * @retval      无
 */
void EPD_WR_DATA8(uint8_t dat)
{
    EPD_DC_Set();
    EPD_WR_Bus(dat);
    EPD_DC_Set();
}

void EPD_WR_DATA_BUF(const uint8_t *buf, size_t n)
{
    EPD_DC_Set();
    EPD_CS_Clr();
    gEpdSpi.writeBytes(buf, n);
    EPD_CS_Set();
}
