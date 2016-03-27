
#ifndef _TCS3472_H_
#define _TCS3472_H_

#include <platform_config.h>
#include <stdbool.h>
#include <stdint.h>

#define TCS34721_I2C_ADDRESS (0x39 << 1)
#define TCS34723_I2C_ADDRESS (0x39 << 1)
#define TCS34725_I2C_ADDRESS (0x29 << 1)
#define TCS34727_I2C_ADDRESS (0x29 << 1)

#ifndef TCS3472_TIMEOUT
#  define TCS3472_TIMEOUT 1000
#endif

typedef enum {
  TCS3472_REG_CONTROL_AGAIN_1X  = 0x00,
  TCS3472_REG_CONTROL_AGAIN_4X  = 0x01,
  TCS3472_REG_CONTROL_AGAIN_16X = 0x02,
  TCS3472_REG_CONTROL_AGAIN_60X = 0x03
} TCS3472_Gain;

typedef struct {
  I2C_HandleTypeDef* i2c;
  uint8_t address;
} TCS3472;

typedef struct {
  uint16_t r;
  uint16_t g;
  uint16_t b;
  uint16_t c;
} TCS3472_Color;

HAL_StatusTypeDef TCS3472_setup(TCS3472* tcs3472, I2C_HandleTypeDef* i2c, uint8_t address);
HAL_StatusTypeDef TCS3472_readId(TCS3472* tcs3472, uint8_t* id);
HAL_StatusTypeDef TCS3472_enable(TCS3472* tcs3472);
HAL_StatusTypeDef TCS3472_disable(TCS3472* tcs3472);
HAL_StatusTypeDef TCS3472_readColor(TCS3472* tcs3472, TCS3472_Color* colorData);

/**
 * The RGBC timing register controls the internal integration time
 * of the RGBC clear and IR channel ADCs in 2.4-ms increments.
 * Max RGBC Count = (256 − ATIME) × 1024 up to a maximum of
 * 65535.
 *
 * 0xff    1  2.4ms
 * 0xf6   10   24ms
 * 0xd5   42  101ms
 * 0xc0   64  154ms
 * 0x00  256  700ms
 */
HAL_StatusTypeDef TCS3472_setIntegrationTime(TCS3472* tcs3472, uint8_t value);

HAL_StatusTypeDef TCS3472_setGain(TCS3472* tcs3472, TCS3472_Gain value);

#endif
