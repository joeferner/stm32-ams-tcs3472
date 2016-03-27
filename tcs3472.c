
#include "tcs3472.h"
#include <utils/time.h>
#include <utils/utils.h>

#ifdef TCS3472_DEBUG
#define TCS3472_DEBUG_OUT(format, ...) printf("%s:%d: TCS3472: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define TCS3472_DEBUG_OUT(format, ...)
#endif

#define TCS3472_ADDR_ENABLE  0x00
#define TCS3472_ADDR_ATIME   0x01
#define TCS3472_ADDR_WTIME   0x03
#define TCS3472_ADDR_AILTL   0x04
#define TCS3472_ADDR_AILTH   0x05
#define TCS3472_ADDR_AIHTL   0x06
#define TCS3472_ADDR_AIHTH   0x07
#define TCS3472_ADDR_PERS    0x0c
#define TCS3472_ADDR_CONFIG  0x0d
#define TCS3472_ADDR_CONTROL 0x0f
#define TCS3472_ADDR_ID      0x12
#define TCS3472_ADDR_STATUS  0x13
#define TCS3472_ADDR_CDATAL  0x14
#define TCS3472_ADDR_CDATAH  0x15
#define TCS3472_ADDR_RDATAL  0x16
#define TCS3472_ADDR_RDATAH  0x17
#define TCS3472_ADDR_GDATAL  0x18
#define TCS3472_ADDR_GDATAH  0x19
#define TCS3472_ADDR_BDATAL  0x1a
#define TCS3472_ADDR_BDATAH  0x1b

#define TCS3472_REG_CMD_BIT 0x80

#define TCS3472_REG_CMD_TYPE_BYTE     (0b00 << 5)
#define TCS3472_REG_CMD_TYPE_AUTO_INC (0b01 << 5)
#define TCS3472_REG_CMD_TYPE_SF       (0b11 << 5)

#define TCS3472_REG_ENABLE_AIEN 0x10
#define TCS3472_REG_ENABLE_WEN  0x08
#define TCS3472_REG_ENABLE_AEN  0x02
#define TCS3472_REG_ENABLE_PON  0x01

#define TCS3472_REG_CONFIG_WLONG 0x02

#define TCS3472_REG_ID_34721_34725  0x44
#define TCS3472_REG_ID_34723_34727  0x4d

#define TCS3472_REG_STATUS_AVALID 0x01
#define TCS3472_REG_STATUS_AINT   0x10

HAL_StatusTypeDef _TCS3472_init(TCS3472* tcs3472);
HAL_StatusTypeDef _TCS3472_read8bitReg(TCS3472* tcs3472, uint8_t reg, uint8_t* data);
HAL_StatusTypeDef _TCS3472_read16bitReg(TCS3472* tcs3472, uint8_t reg, uint16_t* data);
HAL_StatusTypeDef _TCS3472_write8bitReg(TCS3472* tcs3472, uint8_t reg, uint8_t value);

HAL_StatusTypeDef TCS3472_setup(TCS3472* tcs3472, I2C_HandleTypeDef* i2c, uint8_t address) {
  tcs3472->i2c = i2c;
  tcs3472->address = address;
  return _TCS3472_init(tcs3472);
}

HAL_StatusTypeDef _TCS3472_init(TCS3472* tcs3472) {
  HAL_StatusTypeDef ret;
  uint8_t id;

  ret = TCS3472_readId(tcs3472, &id);
  if (ret != HAL_OK) {
    return ret;
  }

  if (id != TCS3472_REG_ID_34721_34725 && id != TCS3472_REG_ID_34723_34727) {
    TCS3472_DEBUG_OUT("invalid id: %d", id);
    return HAL_ERROR;
  }

  return HAL_OK;
}

HAL_StatusTypeDef TCS3472_readId(TCS3472* tcs3472, uint8_t* id) {
  return _TCS3472_read8bitReg(tcs3472, TCS3472_ADDR_ID, id);
}

HAL_StatusTypeDef TCS3472_enable(TCS3472* tcs3472) {
  returnNonOKHALStatus(_TCS3472_write8bitReg(tcs3472, TCS3472_ADDR_ENABLE, TCS3472_REG_ENABLE_PON));
  sleep_ms(3);
  returnNonOKHALStatus(_TCS3472_write8bitReg(tcs3472, TCS3472_ADDR_ENABLE, TCS3472_REG_ENABLE_PON | TCS3472_REG_ENABLE_AEN));
  return HAL_OK;
}

HAL_StatusTypeDef TCS3472_disable(TCS3472* tcs3472) {
  uint8_t value;
  returnNonOKHALStatus(_TCS3472_read8bitReg(tcs3472, TCS3472_ADDR_ENABLE, &value));
  value = value & ~(TCS3472_REG_ENABLE_PON | TCS3472_REG_ENABLE_AEN);
  returnNonOKHALStatus(_TCS3472_write8bitReg(tcs3472, TCS3472_ADDR_ENABLE, value));
  return HAL_OK;
}

HAL_StatusTypeDef TCS3472_setIntegrationTime(TCS3472* tcs3472, uint8_t value) {
  return _TCS3472_write8bitReg(tcs3472, TCS3472_ADDR_ATIME, value);
}

HAL_StatusTypeDef TCS3472_setGain(TCS3472* tcs3472, TCS3472_Gain value) {
  return _TCS3472_write8bitReg(tcs3472, TCS3472_ADDR_CONTROL, (uint8_t)value);
}

HAL_StatusTypeDef TCS3472_readColor(TCS3472* tcs3472, TCS3472_Color* colorData) {
  returnNonOKHALStatus(_TCS3472_read16bitReg(tcs3472, TCS3472_ADDR_CDATAL, &colorData->c));
  returnNonOKHALStatus(_TCS3472_read16bitReg(tcs3472, TCS3472_ADDR_RDATAL, &colorData->r));
  returnNonOKHALStatus(_TCS3472_read16bitReg(tcs3472, TCS3472_ADDR_GDATAL, &colorData->g));
  returnNonOKHALStatus(_TCS3472_read16bitReg(tcs3472, TCS3472_ADDR_BDATAL, &colorData->b));
  return HAL_OK;
}

HAL_StatusTypeDef _TCS3472_read8bitReg(TCS3472* tcs3472, uint8_t reg, uint8_t* data) {
  HAL_StatusTypeDef ret;

  uint8_t cmd = TCS3472_REG_CMD_BIT | TCS3472_REG_CMD_TYPE_BYTE | reg;
  ret = HAL_I2C_Master_Transmit(tcs3472->i2c, tcs3472->address, &cmd, 1, TCS3472_TIMEOUT);
  if (ret != HAL_OK) {
    return ret;
  }

  ret = HAL_I2C_Master_Receive(tcs3472->i2c, tcs3472->address, data, 1, TCS3472_TIMEOUT);
  if (ret != HAL_OK) {
    return ret;
  }

  return HAL_OK;
}

HAL_StatusTypeDef _TCS3472_read16bitReg(TCS3472* tcs3472, uint8_t reg, uint16_t* data) {
  HAL_StatusTypeDef ret;

  uint8_t cmd = TCS3472_REG_CMD_BIT | TCS3472_REG_CMD_TYPE_BYTE | reg;
  ret = HAL_I2C_Master_Transmit(tcs3472->i2c, tcs3472->address, &cmd, 1, TCS3472_TIMEOUT);
  if (ret != HAL_OK) {
    return ret;
  }

  uint8_t temp[2];
  ret = HAL_I2C_Master_Receive(tcs3472->i2c, tcs3472->address, temp, 2, TCS3472_TIMEOUT);
  if (ret != HAL_OK) {
    return ret;
  }

  *data = ((uint16_t)temp[1] << 8) | (uint16_t)temp[0];

  return HAL_OK;
}

HAL_StatusTypeDef _TCS3472_write8bitReg(TCS3472* tcs3472, uint8_t reg, uint8_t value) {
  uint8_t data[2];
  data[0] = TCS3472_REG_CMD_BIT | TCS3472_REG_CMD_TYPE_BYTE | reg;
  data[1] = value;
  return HAL_I2C_Master_Transmit(tcs3472->i2c, tcs3472->address, data, 2, TCS3472_TIMEOUT);
}
