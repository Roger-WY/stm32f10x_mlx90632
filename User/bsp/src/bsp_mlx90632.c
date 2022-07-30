/*******************************************************************************
*   Filename:       bsp_mlx90632.c
*   Revised:        All copyrights reserved to Roger.
*   Date:           2020-10-17
*   Revision:       v1.0
*   Writer:	        Roger-WY.
*
*   Description:    非接触式红外温度传感器模块驱动(数字式)
*
*
*   Notes:          https://www.melexis.com/zh/product/MLX90632/MLX90632
*   All copyrights reserved to Roger-WY
*******************************************************************************/
#include "bsp_mlx90632.h"
#include <math.h>


//----------------------------------------------------------------------------//
#define MLX90632_I2C_SCL_RCC     RCC_APB2Periph_GPIOC
#define MLX90632_I2C_SCL_PIN	 GPIO_Pin_4			/* 连接到SCL时钟线的GPIO */
#define MLX90632_I2C_SCL_PORT    GPIOC

#define MLX90632_I2C_SDA_RCC     RCC_APB2Periph_GPIOC
#define MLX90632_I2C_SDA_PIN	 GPIO_Pin_5			/* 连接到SDA数据线的GPIO */
#define MLX90632_I2C_SDA_PORT    GPIOC


/* 定义读写SCL和SDA的宏 */
#define MLX90632_I2C_SCL_1()     MLX90632_I2C_SCL_PORT->BSRR = MLX90632_I2C_SCL_PIN			/* SCL = 1 */
#define MLX90632_I2C_SCL_0()     MLX90632_I2C_SCL_PORT->BRR  = MLX90632_I2C_SCL_PIN			/* SCL = 0 */

#define MLX90632_I2C_SDA_1()     MLX90632_I2C_SDA_PORT->BSRR = MLX90632_I2C_SDA_PIN			/* SDA = 1 */
#define MLX90632_I2C_SDA_0()     MLX90632_I2C_SDA_PORT->BRR  = MLX90632_I2C_SDA_PIN			/* SDA = 0 */

#define MLX90632_I2C_SDA_READ()  ((MLX90632_I2C_SDA_PORT->IDR & MLX90632_I2C_SDA_PIN) != 0)	/* 读SDA口线状态 */
#define MLX90632_I2C_SCL_READ()  ((MLX90632_I2C_SCL_PORT->IDR & MLX90632_I2C_SCL_PIN) != 0)	/* 读SCL口线状态 */

#define MLX90632_DELAY_MS(a) bsp_DelayNms(a);

//----------------------------------------------------------------------------//
#define POW10 10000000000LL

/* 灵敏度 */
static double emissivity = 0.0;

stMlx90632CalibraParTypeDef stMlxCalibraPar;
//============================================================================//


/*
********************************************************************************
*	函 数 名: Mlx90632_i2c_Delay
*	功能说明: I2C总线位延迟，最快400KHz
*	形    参:  无
*	返 回 值: 无
********************************************************************************
*/
static void Mlx90632_i2c_Delay(void)
{
    volatile uint8_t i;

    for(i = 0; i < 50; i++);
}


static void Mlx90632_i2c_Start(void)
{
    /* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
    MLX90632_I2C_SDA_1();
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SCL_1();
    Mlx90632_i2c_Delay();

    MLX90632_I2C_SDA_0();
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SCL_0();
//	Mlx90632_i2c_Delay();
}

/*******************************************************************************
 * 名    称： i2c_Stop
 * 功    能： CPU发起I2C总线停止信号
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注： 停止时序
 *            SCL _____/ˉˉˉˉˉˉˉ
 *            SDA _________/ˉˉˉˉˉ
 *                       |   |
 *                       STOP
 *******************************************************************************/
static void Mlx90632_i2c_Stop(void)
{
    /* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */

    MLX90632_I2C_SCL_0();
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SDA_0();
    Mlx90632_i2c_Delay();

    MLX90632_I2C_SCL_1();
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SDA_1();
//	Mlx90632_i2c_Delay();
}

/*******************************************************************************
 * 名    称： i2c_WaitAck
 * 功    能： CPU产生一个时钟，并读取器件的ACK应答信号
 * 入口参数： 无
 * 出口参数： 返回0表示正确应答，1表示无器件响应
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static uint8_t Mlx90632_i2c_WaitAck(void)
{
    uint8_t re;
    uint8_t TimeOutCnt = 20;  /* 超时计数器 */

    MLX90632_I2C_SDA_1();	/* CPU释放SDA总线 */
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
    Mlx90632_i2c_Delay();

    while(TimeOutCnt --)
    {
        if(MLX90632_I2C_SDA_READ())	  /* CPU读取SDA口线状态 */
        {
            re = 1;
        }
        else
        {
            re = 0;
        }
    }
    MLX90632_I2C_SCL_0();
    Mlx90632_i2c_Delay();
    return re;
}

/*******************************************************************************
 * 名    称： i2c_Ack
 * 功    能： CPU产生一个ACK信号
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void Mlx90632_i2c_Ack(void)
{
    MLX90632_I2C_SDA_0();	/* CPU驱动SDA = 0 */
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SCL_1();	/* CPU产生1个时钟 */
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SCL_0();
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SDA_1();	/* CPU释放SDA总线 */
}

/*******************************************************************************
 * 名    称： i2c_NAck
 * 功    能： CPU产生1个NACK信号
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void Mlx90632_i2c_NAck(void)
{
    MLX90632_I2C_SDA_1();	/* CPU驱动SDA = 1 */
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SCL_1();	/* CPU产生1个时钟 */
    Mlx90632_i2c_Delay();
    MLX90632_I2C_SCL_0();
    Mlx90632_i2c_Delay();
}

/*******************************************************************************
* 名    称： bsp_InitI2C
* 功    能： 配置I2C总线的GPIO，采用模拟IO的方式实现
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2018-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void Mlx90632_InitI2C(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(MLX90632_I2C_SCL_RCC | MLX90632_I2C_SDA_RCC, ENABLE);	/* 打开GPIO时钟 */

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;	        /* 开漏输出模式 */

    GPIO_InitStructure.GPIO_Pin = MLX90632_I2C_SCL_PIN;
    GPIO_Init(MLX90632_I2C_SCL_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MLX90632_I2C_SDA_PIN;
    GPIO_Init(MLX90632_I2C_SDA_PORT, &GPIO_InitStructure);

    /* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
    Mlx90632_i2c_Stop();
}

/*******************************************************************************
 * 名    称： i2c_SendByte
 * 功    能： CPU向I2C总线设备发送8bit数据
 * 入口参数： _ucByte ： 等待发送的字节
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void Mlx90632_i2c_SendByte(uint8_t _ucByte)
{
    uint8_t i;

    /* 先发送字节的高位bit7 */
    for(i = 0; i < 8; i++)
    {
        MLX90632_I2C_SCL_0();
        Mlx90632_i2c_Delay();

        if(_ucByte & 0x80)
        {
            MLX90632_I2C_SDA_1();
        }
        else
        {
            MLX90632_I2C_SDA_0();
        }

        _ucByte <<= 1;	/* 左移一个bit */

        Mlx90632_i2c_Delay();

        MLX90632_I2C_SCL_1();
        Mlx90632_i2c_Delay();
    }
    MLX90632_I2C_SCL_0();
    Mlx90632_i2c_Delay();

}

/*******************************************************************************
 * 名    称： Mlx90632_i2c_ReadByte
 * 功    能： CPU从I2C总线设备读取8bit数据
 * 入口参数： 无
 * 出口参数： 读到的数据
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static uint8_t Mlx90632_i2c_ReadByte(void)
{
    uint8_t i;
    uint8_t value;

    /* 读到第1个bit为数据的bit7 */
    value = 0;
    for(i = 0; i < 8; i++)
    {
        value <<= 1;
        MLX90632_I2C_SCL_0();
        Mlx90632_i2c_Delay();
        MLX90632_I2C_SCL_1();
        Mlx90632_i2c_Delay();

        if(MLX90632_I2C_SDA_READ())
        {
            value++;
        }
    }
    MLX90632_I2C_SCL_0();
    Mlx90632_i2c_Delay();

    return value;
}


//============================================================================//


/*******************************************************************************
* 名    称： bsp_Mlx90632Init
* 功    能： Mlx90632传感器的初始化
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2018-08-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
int8_t bsp_Mlx90632Init(void)
{
    int8_t ucError      = 0;

    Mlx90632_InitI2C();

    bsp_Mlx90632ReadCalibraParInit(&stMlxCalibraPar);

    ucError = bsp_Mlx90632ReadCalibraParFromEeprom(&stMlxCalibraPar);
    return (ucError);

}

/*******************************************************************************
 * 名    称： bsp_Mlx90632WriteReg
 * 功    能： 对寄存器写值
 * 入口参数： devAddr：设备地址 regAddr：寄存器地址 data：写入寄存器的值
 * 出口参数： 读到的数据
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int8_t bsp_Mlx90632WriteReg(uint8_t devAddr, uint16_t regAddr, uint16_t data)
{
    uint8_t ucAck = 0;
    uint8_t sendBuffer[5] = {0}; //used to save the data to send

    sendBuffer[0] = devAddr << 1;
    sendBuffer[1] = (uint8_t)(regAddr >> 8);
    sendBuffer[2] = (uint8_t)regAddr;
    sendBuffer[3] = (uint8_t)(data >> 8);
    sendBuffer[4] = (uint8_t)data;

    Mlx90632_i2c_Start();
    for(uint8_t i = 0; i < 5; i ++)
    {
        Mlx90632_i2c_SendByte(sendBuffer[i]);
        ucAck = Mlx90632_i2c_WaitAck();
        if(ucAck)           /* 如果Mlx90632，没有应答 */
        {
            goto cmd_fail;	/* 器件无应答 */
        }
    }
    /* 发送I2C总线停止信号 */
    Mlx90632_i2c_Stop();
    return 0;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
    /* 发送I2C总线停止信号 */
    Mlx90632_i2c_Stop();
    return -1;

}

/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadReg
 * 功    能： 从寄存器里读值
 * 入口参数： devAddr：设备地址 regAddr：寄存器地址 ppBuf：读取寄存器值的缓存区 nBytes：读取寄存器的字节数
 * 出口参数： 读到的数据
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int8_t bsp_Mlx90632ReadReg(uint8_t devAddr, uint16_t regAddr, uint8_t* ppBuf, uint8_t nBytes)
{
    uint8_t ucAck = 0;
    uint8_t ValBuf[6] = {0};
    uint8_t prcRegVal = 0;
    uint8_t i = 0;

    ValBuf[0] = devAddr << 1;
    ValBuf[1] = (uint8_t)(regAddr >> 8);
    ValBuf[2] = (uint8_t)regAddr;

    ValBuf[3] = (devAddr << 1) | 0x01;


    Mlx90632_i2c_Start();
    for(i = 0; i < 3; i++)
    {
        Mlx90632_i2c_SendByte(ValBuf[i]);
        ucAck = Mlx90632_i2c_WaitAck();
        if(ucAck)           /* 如果没有应答 */
        {
            goto cmd_fail;	/* 器件无应答 */
        }
    }

    //------------------------------------------------------------------------//
    Mlx90632_i2c_Start();
    Mlx90632_i2c_SendByte(ValBuf[3]);
    ucAck = Mlx90632_i2c_WaitAck();
    if(ucAck)           /* 如果没有应答 */
    {
        goto cmd_fail;	/* 器件无应答 */
    }

    while(nBytes)
    {
        *ppBuf = Mlx90632_i2c_ReadByte();
        if(nBytes == 1)
            Mlx90632_i2c_NAck();
        else
            Mlx90632_i2c_Ack();

        nBytes--;
        ppBuf++;
    }

    /* 发送I2C总线停止信号 */
    Mlx90632_i2c_Stop();
    return 0;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
    /* 发送I2C总线停止信号 */
    Mlx90632_i2c_Stop();
    return -1;
}


/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadWord
 * 功    能： 从寄存器里读一个字(16-bit)
 * 入口参数： regAddr：寄存器地址 value：读取寄存器值的缓存区
 * 出口参数： 0：读取成功 其他值，读取失败
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int8_t bsp_Mlx90632ReadWord(uint16_t regAddr, uint16_t* value)
{
    int8_t ret = 0;
    uint8_t buf[2] = {0};
    ret = bsp_Mlx90632ReadReg(MLX90632_ADDR, regAddr, buf, 2);
    *value = buf[1] | (buf[0] << 8);
    return ret;
}

/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadDoubleWord
 * 功    能： 从寄存器里读双字(32-bit)
 * 入口参数： regAddr：寄存器地址 value：读取寄存器值的缓存区
 * 出口参数： 0：读取成功 其他值，读取失败
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int8_t bsp_Mlx90632ReadDoubleWord(uint16_t regAddr, uint32_t* value)
{
    int8_t ret = 0;
    uint8_t buf[4] = {0};
    ret = bsp_Mlx90632ReadReg(MLX90632_ADDR, regAddr, buf, 4);
    *value = buf[2] << 24 | buf[3] << 16 | buf[0] << 8 | buf[1];
    return ret;
}
/*******************************************************************************
 * 名    称： bsp_Mlx90632WriteWord
 * 功    能： 往寄存器里写入一个字(16-bit)
 * 入口参数： regAddr：寄存器地址 value：读取寄存器值的缓存区
 * 出口参数： 0：读取成功 其他值，读取失败
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int8_t bsp_Mlx90632WriteWord(uint16_t regAddr, uint16_t data)
{
    int8_t ret = 0;
    ret = bsp_Mlx90632WriteReg(MLX90632_ADDR, regAddr, data) ;
    return (ret);
}

/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadCalibraParInit
 * 功    能： 校准参数初始化，如果eeprom的校准参数读出来为空，就是使用此默认值
 * 入口参数： pPar：校准参数结构体指针
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_Mlx90632ReadCalibraParInit(stMlx90632CalibraParTypeDef* pPar)
{
    pPar->PR = 0x00587f5b;
    pPar->PG = 0x04a10289;
    pPar->PT = 0xfff966f8;
    pPar->PO = 0x00001e0f;
    pPar->Ea = 4859535;
    pPar->Eb = 5686508;
    pPar->Fa = 53855361;
    pPar->Fb = 42874149;
    pPar->Ga = -14556410;
    pPar->Ha = 16384;
    pPar->Hb = 0;
    pPar->Gb = 9728;
    pPar->Ka = 10752;
}


/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadCalibraParFromEeprom
 * 功    能： 从传感器内部EEPROM中读取校准参数
 * 入口参数： pPar：校准参数结构体指针
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int8_t bsp_Mlx90632ReadCalibraParFromEeprom(stMlx90632CalibraParTypeDef* pPar)
{
    int8_t ret = 0;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_P_R, (uint32_t*)&pPar->PR);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_P_G, (uint32_t*)&pPar->PG);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_P_O, (uint32_t*)&pPar->PO);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_P_T, (uint32_t*)&pPar->PT);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_Ea, (uint32_t*)&pPar->Ea);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_Eb, (uint32_t*)&pPar->Eb);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_Fa, (uint32_t*)&pPar->Fa);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_Fb, (uint32_t*)&pPar->Fb);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadDoubleWord(MLX90632_EE_Ga, (uint32_t*)&pPar->Ga);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadWord(MLX90632_EE_Gb, (uint16_t*)&pPar->Gb);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadWord(MLX90632_EE_Ha, (uint16_t*)&pPar->Ha);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadWord(MLX90632_EE_Hb, (uint16_t*)&pPar->Hb);
    if(ret < 0)
        return ret;
    ret = bsp_Mlx90632ReadWord(MLX90632_EE_Ka, (uint16_t*)&pPar->Ka);
    if(ret < 0)
        return ret;
    return 0;
}


/*******************************************************************************
 * 名    称： bsp_Mlx90632StartMeasurement
 * 功    能： 触发传感器开始测量，但是只是触发测量周期，等待数据准备就绪，它不读取任何内容，只是触发并等待完成。
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注： 此处超时等待是阻塞的
 *******************************************************************************/
int bsp_Mlx90632StartMeasurement(void)
{
    int ret, tries = 100;
    uint16_t reg_status;

    ret = bsp_Mlx90632ReadWord(MLX90632_REG_STATUS, &reg_status);
    if(ret < 0)
        return ret;

    ret = bsp_Mlx90632WriteWord(MLX90632_REG_STATUS, reg_status & (~MLX90632_STAT_DATA_RDY));
    if(ret < 0)
        return ret;

    while(tries-- > 0)
    {
        ret = bsp_Mlx90632ReadWord(MLX90632_REG_STATUS, &reg_status);
        if(ret < 0)
            return ret;
        if(reg_status & MLX90632_STAT_DATA_RDY)
            break;
        /* minimum wait time to complete measurement
         * should be calculated according to refresh rate
         * atm 10ms - 11ms
         */
        //usleep(10000, 11000);
        MLX90632_DELAY_MS(10);  //提供延时函数，此处延时10ms左右
    }

    if(tries < 0)
    {
        // data not ready
        return -ETIMEDOUT;
    }

    return (reg_status & MLX90632_STAT_CYCLE_POS) >> 2;
}

/*******************************************************************************
 * 名    称： bsp_Mlx90632ChannelNewSelect
 * 功    能： 根据 bsp_Mlx90632StartMeasurement 的返回值，获取通道的顺序。
 * 入口参数： ret：bsp_Mlx90632StartMeasurement 的返回值  *channel_new：指向存储新通道值的内存位置的指针 *channel_new：指向存储旧通道值的内存位置的指针
 * 出口参数： 0：正常
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注： 如果返回值不是1/2 则不分配通道序列
 *******************************************************************************/
int32_t bsp_Mlx90632ChannelNewSelect(int32_t ret, uint8_t* channel_new, uint8_t* channel_old)
{
    switch(ret)
    {
    case 1:
        *channel_new = 1;
        *channel_old = 2;
        break;

    case 2:
        *channel_new = 2;
        *channel_old = 1;
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadTempAmbientRaw
 * 功    能： 读取环境原始旧值和新值
 * 入口参数： ambient_new_raw：原始环境温度新值  ambient_old_raw：原始环境温度旧值
 * 出口参数： 0：正常
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int32_t bsp_Mlx90632ReadTempAmbientRaw(int16_t* ambient_new_raw, int16_t* ambient_old_raw)
{
    int32_t ret;
    uint16_t read_tmp;

    ret = bsp_Mlx90632ReadWord(MLX90632_RAM_3(1), &read_tmp);
    if(ret < 0)
        return ret;
    *ambient_new_raw = (int16_t)read_tmp;

    ret = bsp_Mlx90632ReadWord(MLX90632_RAM_3(2), &read_tmp);
    if(ret < 0)
        return ret;
    *ambient_old_raw = (int16_t)read_tmp;

    return ret;
}

/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadTempObjectRaw
 * 功    能： 读取对象原始温度新值和旧值
 * 入口参数： start_measurement_ret： 返回值 object_new_raw：物体原始温度新值  object_old_raw：物体原始温度旧值
 * 出口参数： 0：正常
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int32_t bsp_Mlx90632ReadTempObjectRaw(int32_t start_measurement_ret,
                                      int16_t* object_new_raw, int16_t* object_old_raw)
{
    int32_t ret;
    uint16_t read_tmp;
    int16_t read;
    uint8_t channel, channel_old;

    ret = bsp_Mlx90632ChannelNewSelect(start_measurement_ret, &channel, &channel_old);
    if(ret != 0)
        return -EINVAL;

    ret = bsp_Mlx90632ReadWord(MLX90632_RAM_2(channel), &read_tmp);
    if(ret < 0)
        return ret;

    read = (int16_t)read_tmp;

    ret = bsp_Mlx90632ReadWord(MLX90632_RAM_1(channel), &read_tmp);
    if(ret < 0)
        return ret;
    *object_new_raw = (read + (int16_t)read_tmp) / 2;

    ret = bsp_Mlx90632ReadWord(MLX90632_RAM_2(channel_old), &read_tmp);
    if(ret < 0)
        return ret;
    read = (int16_t)read_tmp;

    ret = bsp_Mlx90632ReadWord(MLX90632_RAM_1(channel_old), &read_tmp);
    if(ret < 0)
        return ret;
    *object_old_raw = (read + (int16_t)read_tmp) / 2;

    return ret;
}
/*******************************************************************************
 * 名    称： bsp_Mlx90632ReadTempRaw
 * 功    能： 读取温度原始值
 * 入口参数： ambient_new_raw：环境原始温度新值  ambient_old_raw：环境原始温度旧值
*             object_new_raw：物体原始温度新值  object_old_raw：物体原始温度旧值
 * 出口参数： 0：正常
 * 作　　者： Roger-WY
 * 创建日期： 2022-05-04
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int32_t bsp_Mlx90632ReadTempRaw(int16_t* ambient_new_raw, int16_t* ambient_old_raw,
                                int16_t* object_new_raw, int16_t* object_old_raw)
{
    int32_t ret, start_measurement_ret;

    // trigger and wait for measurement to complete
    start_measurement_ret = bsp_Mlx90632StartMeasurement();
    if(start_measurement_ret < 0)
        return start_measurement_ret;

    /** Read new and old **ambient** values from sensor */
    ret = bsp_Mlx90632ReadTempAmbientRaw(ambient_new_raw, ambient_old_raw);
    if(ret < 0)
        return ret;

    /** Read new and old **object** values from sensor */
    ret = bsp_Mlx90632ReadTempObjectRaw(start_measurement_ret, object_new_raw, object_old_raw);

    return ret;
}

/* DSPv5 */
double bsp_Mlx90632PreprocessTempAmbient(int16_t ambient_new_raw, int16_t ambient_old_raw, int16_t Gb)
{
    double VR_Ta, kGb;

    kGb = ((double)Gb) / 1024.0;

    VR_Ta = ambient_old_raw + kGb * (ambient_new_raw / (MLX90632_REF_3));
    return ((ambient_new_raw / (MLX90632_REF_3)) / VR_Ta) * 524288.0;
}

double bsp_Mlx90632PreprocessTempObject(int16_t object_new_raw, int16_t object_old_raw,
                                        int16_t ambient_new_raw, int16_t ambient_old_raw,
                                        int16_t Ka)
{
    double VR_IR, kKa;

    kKa = ((double)Ka) / 1024.0;

    VR_IR = ambient_old_raw + kKa * (ambient_new_raw / (MLX90632_REF_3));
    return ((((object_new_raw + object_old_raw) / 2) / (MLX90632_REF_12)) / VR_IR) * 524288.0;
}

double bsp_Mlx90632CalcTempAmbient(int16_t ambient_new_raw, int16_t ambient_old_raw, int32_t P_T,
                                   int32_t P_R, int32_t P_G, int32_t P_O, int16_t Gb)
{
    double Asub, Bsub, Ablock, Bblock, Cblock, AMB;

    AMB = bsp_Mlx90632PreprocessTempAmbient(ambient_new_raw, ambient_old_raw, Gb);

    Asub = ((double)P_T) / (double)17592186044416.0;
    Bsub = (double)AMB - ((double)P_R / (double)256.0);
    Ablock = Asub * (Bsub * Bsub);
    Bblock = (Bsub / (double)P_G) * (double)1048576.0;
    Cblock = (double)P_O / (double)256.0;

    return Bblock + Ablock + Cblock;
}


/*******************************************************************************
* 名    称： bsp_Mlx90632CalcTempObjectIteration
* 功    能： 物体温度的迭代计算
* 入口参数： prev_object_temp： 上次计算的物体温度值，如果没有，默认为25.0摄氏度
*            object： 通过 bsp_Mlx90632PreprocessTempObject 函数获取的物体温度值
*            剩余的参数都是校正值
* 出口参数： 0：正常
* 作　　者： Roger-WY
* 创建日期： 2022-05-04
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static double bsp_Mlx90632CalcTempObjectIteration(double prev_object_temp, int32_t object, double TAdut,
        int32_t Ga, int32_t Fa, int32_t Fb, int16_t Ha, int16_t Hb,
        double emissivity)
{
    double calcedGa, calcedGb, calcedFa, TAdut4, first_sqrt;
    // temp variables
    double KsTAtmp, Alpha_corr;
    double Ha_customer, Hb_customer;


    Ha_customer = Ha / ((double)16384.0);
    Hb_customer = Hb / ((double)1024.0);
    calcedGa = ((double)Ga * (prev_object_temp - 25)) / ((double)68719476736.0);
    KsTAtmp = (double)Fb * (TAdut - 25);
    calcedGb = KsTAtmp / ((double)68719476736.0);
    Alpha_corr = (((double)(Fa * POW10)) * Ha_customer * (double)(1 + calcedGa + calcedGb)) /
                 ((double)70368744177664.0);
    calcedFa = object / (emissivity * (Alpha_corr / POW10));
    TAdut4 = (TAdut + 273.15) * (TAdut + 273.15) * (TAdut + 273.15) * (TAdut + 273.15);

    first_sqrt = sqrt(calcedFa + TAdut4);

    return sqrt(first_sqrt) - 273.15 - Hb_customer;
}


void bsp_Mlx90632SetEmissivity(double value)
{
    emissivity = value;
}

double bsp_mlx90632GetEmissivity(void)
{
    if(emissivity == 0.0)
    {
        return 1.0;
    }
    else
    {
        return emissivity;
    }
}

double bsp_Mlx90632CalcTempObject(int32_t object, int32_t ambient,
                                  int32_t Ea, int32_t Eb, int32_t Ga, int32_t Fa, int32_t Fb,
                                  int16_t Ha, int16_t Hb)
{
    double kEa, kEb, TAdut;
    double temp = 25.0;
    double tmp_emi = bsp_mlx90632GetEmissivity();
    int8_t i;

    kEa = ((double)Ea) / ((double)65536.0);
    kEb = ((double)Eb) / ((double)256.0);
    TAdut = (((double)ambient) - kEb) / kEa + 25;

    //iterate through calculations
    for(i = 0; i < 5; ++i)
    {
        temp = bsp_Mlx90632CalcTempObjectIteration(temp, object, TAdut, Ga, Fa, Fb, Ha, Hb, tmp_emi);
    }
    return temp;
}



/***************************** (END OF FILE) **********************************/
