/*******************************************************************************
*   Filename:       bsp_mlx90632.h
*   Revised:        All copyrights reserved to Roger.
*   Date:           2022-05-02
*   Revision:       v1.0
*   Writer:	        Roger-WY.
*
*   Description:    非接触式红外温度传感器模块驱动(数字式)  头文件
*******************************************************************************/
#ifndef __BSP_MLX90632_H__
#define __BSP_MLX90632_H__

#include "stm32f10x.h"
#include "bsp_timer.h"

#define MLX90632_TEMP_READ_ERR_CODE -99
/* Solve errno not defined values */
#ifndef ETIMEDOUT
#define ETIMEDOUT 110 /**< From linux errno.h */
#endif
#ifndef EINVAL
#define EINVAL 22 /**< From linux errno.h */
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT 93 /**< From linux errno.h */
#endif

typedef struct __mlx90632calibrationpar
{
    int32_t PR;
    int32_t PG;
    int32_t PO;
    int32_t PT;
    int32_t Ea;
    int32_t Eb;
    int32_t Fa;
    int32_t Fb;
    int32_t Ga;
    int16_t Gb;
    int16_t Ha;
    int16_t Hb;
    int16_t Ka;
}stMlx90632CalibraParTypeDef;

/* MLX90632 内部寄存器地址定义 */
/* Private defines -----------------------------------------------------------*/
/* Definition of I2C address of MLX90632 */
#define MLX90632_ADDR       0x3A
#define MLX90632_ADDR_WR   (MLX90632_ADDR << 1)

#define BITS_PER_LONG 32

/* BIT, GENMASK and ARRAY_SIZE macros are imported from kernel */
#ifndef BIT
#define BIT(x) (1UL << (x))
#endif
#ifndef GENMASK
#ifndef BITS_PER_LONG
#warning "Using default BITS_PER_LONG value"
#define BITS_PER_LONG 64 /**< Define how many bits per long your CPU has */
#endif
#define GENMASK(h, l) \
    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0])) /**< Return number of elements in array */
#endif

/* Memory sections addresses */
#define MLX90632_ADDR_RAM   0x4000 /**< Start address of ram */
#define MLX90632_ADDR_EEPROM    0x2480 /**< Start address of user eeprom */

/* EEPROM addresses - used at startup */
#define MLX90632_EE_CTRL    0x24d4 /**< Control register initial value */
#define MLX90632_EE_CONTROL MLX90632_EE_CTRL /**< More human readable for Control register */

#define MLX90632_EE_I2C_ADDRESS 0x24d5 /**< I2C address register initial value */
#define MLX90632_EE_VERSION 0x240b /**< EEPROM version reg - assumed 0x101 */

#define MLX90632_EE_P_R     0x240c /**< Calibration constant ambient reference register 32bit */
#define MLX90632_EE_P_G     0x240e /**< Calibration constant ambient gain register 32bit */
#define MLX90632_EE_P_T     0x2410 /**< Calibration constant ambient tc2 register 32bit	*/
#define MLX90632_EE_P_O     0x2412 /**< Calibration constant ambient offset register 32bit */
#define MLX90632_EE_Aa      0x2414 /**< Aa calibration const register 32bit */
#define MLX90632_EE_Ab      0x2416 /**< Ab calibration const register 32bit */
#define MLX90632_EE_Ba      0x2418 /**< Ba calibration const register 32bit */
#define MLX90632_EE_Bb      0x241a /**< Bb calibration const register 32bit */
#define MLX90632_EE_Ca      0x241c /**< Ca calibration const register 32bit */
#define MLX90632_EE_Cb      0x241e /**< Cb calibration const register 32bit */
#define MLX90632_EE_Da      0x2420 /**< Da calibration const register 32bit */
#define MLX90632_EE_Db      0x2422 /**< Db calibration const register 32bit */
#define MLX90632_EE_Ea      0x2424 /**< Ea calibration constant register 32bit */
#define MLX90632_EE_Eb      0x2426 /**< Eb calibration constant register 32bit */
#define MLX90632_EE_Fa      0x2428 /**< Fa calibration constant register 32bit */
#define MLX90632_EE_Fb      0x242a /**< Fb calibration constant register 32bit */
#define MLX90632_EE_Ga      0x242c /**< Ga calibration constant register 32bit */

#define MLX90632_EE_Gb      0x242e /**< Ambient Beta calibration constant 16bit */
#define MLX90632_EE_Ka      0x242f /**< IR Beta calibration constant 16bit */

#define MLX90632_EE_Ha      0x2481 /**< Ha customer calibration value register 16bit */
#define MLX90632_EE_Hb      0x2482 /**< Hb customer calibration value register 16bit */

/* Register addresses - volatile */
#define MLX90632_REG_I2C_ADDR   0x3000 /**< Chip I2C address register */

/* Control register address - volatile */
#define MLX90632_REG_CTRL   0x3001 /**< Control Register address */
#define MLX90632_CFG_SOC_SHIFT 3 /**< Start measurement in step mode */
#define MLX90632_CFG_SOC_MASK BIT(MLX90632_CFG_SOC_SHIFT)
#define MLX90632_CFG_PWR_MASK GENMASK(2, 1) /**< PowerMode Mask */
/* PowerModes statuses */
#define MLX90632_PWR_STATUS(ctrl_val) (ctrl_val << 1)
#define MLX90632_PWR_STATUS_HALT MLX90632_PWR_STATUS(0) /**< Pwrmode hold */
#define MLX90632_PWR_STATUS_SLEEP_STEP MLX90632_PWR_STATUS(1) /**< Pwrmode sleep step*/
#define MLX90632_PWR_STATUS_STEP MLX90632_PWR_STATUS(2) /**< Pwrmode step */
#define MLX90632_PWR_STATUS_CONTINUOUS MLX90632_PWR_STATUS(3) /**< Pwrmode continuous*/

/* Device status register - volatile */
#define MLX90632_REG_STATUS         0x3fff /**< Device status register */
#define MLX90632_STAT_BUSY          BIT(10) /**< Device busy indicator */
#define MLX90632_STAT_EE_BUSY       BIT(9) /**< Device EEPROM busy indicator */
#define MLX90632_STAT_BRST          BIT(8) /**< Device brown out reset indicator */
#define MLX90632_STAT_CYCLE_POS     GENMASK(6, 2) /**< Data position in measurement table */
#define MLX90632_STAT_DATA_RDY      BIT(0) /**< Data ready indicator */

/* RAM_MEAS address-es for each channel */
#define MLX90632_RAM_1(meas_num)    (MLX90632_ADDR_RAM + 3 * meas_num)
#define MLX90632_RAM_2(meas_num)    (MLX90632_ADDR_RAM + 3 * meas_num + 1)
#define MLX90632_RAM_3(meas_num)    (MLX90632_ADDR_RAM + 3 * meas_num + 2)

/* Timings (ms) */
#define MLX90632_TIMING_EEPROM      100 /**< Time between EEPROM writes */

/* Magic constants */
#define MLX90632_EEPROM_VERSION     0x105 /**< EEPROM DSP version for constants */
#define MLX90632_EEPROM_WRITE_KEY   0x554C /**< EEPROM write key 0x55 and 0x4c */
#define MLX90632_RESET_CMD          0x0006 /**< Reset sensor (address or global) */
#define MLX90632_MAX_MEAS_NUM       31 /**< Maximum number of measurements in list */
#define MLX90632_EE_SEED            0x3f6d /**< Seed for the CRC calculations */
#define MLX90632_REF_12             12.0 /**< ResCtrlRef value of Channel 1 or Channel 2 */
#define MLX90632_REF_3              12.0 /**< ResCtrlRef value of Channel 3 */


/* MLX90632 供外部调用函数 */
int8_t bsp_Mlx90632Init(void);
int8_t bsp_Mlx90632ReadCalibraParFromEeprom(stMlx90632CalibraParTypeDef *pPar);
void bsp_Mlx90632ReadCalibraParInit(stMlx90632CalibraParTypeDef *pPar);
int bsp_Mlx90632StartMeasurement(void);
int32_t bsp_Mlx90632ChannelNewSelect(int32_t ret, uint8_t *channel_new, uint8_t *channel_old);
int32_t bsp_Mlx90632ReadTempAmbientRaw(int16_t *ambient_new_raw, int16_t *ambient_old_raw);
void bsp_Mlx90632SetEmissivity(double value);
double bsp_Mlx90632CalcTempObject(int32_t object, int32_t ambient,
                                 int32_t Ea, int32_t Eb, int32_t Ga, int32_t Fa, int32_t Fb,
                                 int16_t Ha, int16_t Hb);
double bsp_Mlx90632CalcTempAmbient(int16_t ambient_new_raw, int16_t ambient_old_raw, int32_t P_T,
                                  int32_t P_R, int32_t P_G, int32_t P_O, int16_t Gb);
double bsp_Mlx90632PreprocessTempObject(int16_t object_new_raw, int16_t object_old_raw,
                                       int16_t ambient_new_raw, int16_t ambient_old_raw,
                                       int16_t Ka);
double bsp_Mlx90632PreprocessTempAmbient(int16_t ambient_new_raw, int16_t ambient_old_raw, int16_t Gb);
int32_t bsp_Mlx90632ReadTempRaw(int16_t *ambient_new_raw, int16_t *ambient_old_raw,
                               int16_t *object_new_raw, int16_t *object_old_raw);

extern stMlx90632CalibraParTypeDef stMlxCalibraPar;

#endif
/***************************** (END OF FILE) **********************************/
