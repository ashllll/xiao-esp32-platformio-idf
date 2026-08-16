#pragma once

#include "driver/gpio.h"

/* Seeed Studio XIAO ESP32-S3 with the external OLED encoder console. */
#define BOARD_NAME "Seeed Studio XIAO ESP32-S3 + SH1106 Encoder Console"
#define BOARD_ID "seeed_xiao_esp32s3_sh1106_encoder_console"
#define BOARD_EXPECTED_FLASH_BYTES (8U * 1024U * 1024U)
#define BOARD_EXPECTED_PSRAM_BYTES (8U * 1024U * 1024U)

/* External 1.3-inch SH1106 OLED: 128x64 over I2C. */
#define BOARD_OLED_SDA_GPIO GPIO_NUM_9  /* XIAO D10 */
#define BOARD_OLED_SCL_GPIO GPIO_NUM_8  /* XIAO D9 */
#define BOARD_OLED_I2C_ADDRESS 0x3CU

/* Integrated encoder and buttons. Inputs are active-low with 3.3 V pull-ups. */
#define BOARD_ENCODER_PUSH_GPIO GPIO_NUM_1 /* XIAO D0 */
#define BOARD_ENCODER_A_GPIO GPIO_NUM_2    /* XIAO D1 */
#define BOARD_ENCODER_B_GPIO GPIO_NUM_3    /* XIAO D2 */
#define BOARD_CONFIRM_GPIO GPIO_NUM_44     /* XIAO D7 */
#define BOARD_BACK_GPIO GPIO_NUM_7         /* XIAO D8 */

/* Four-wire 12 V fan control and open-collector tachometer feedback. */
#define BOARD_FAN_PWM_GPIO GPIO_NUM_4       /* XIAO D3 */
#define BOARD_FAN_TACH_GPIO GPIO_NUM_5      /* XIAO D4 */

/* Native USB is reserved and never reconfigured by this profile. */
#define BOARD_USB_D_MINUS_GPIO GPIO_NUM_19
#define BOARD_USB_D_PLUS_GPIO GPIO_NUM_20
