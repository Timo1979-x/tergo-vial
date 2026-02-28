#pragma once

#include_next <mcuconf.h>

#undef RP_SIO_USE_UART0
#define RP_SIO_USE_UART0 TRUE
#undef RP_SIO_USE_UART1
#define RP_SIO_USE_UART1 FALSE

#undef RP_I2C_USE_I2C0
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C0 TRUE
#define RP_I2C_USE_I2C1 FALSE

// Для того, чтобы скомпилировалась поддержка обычной подсветки с использованием аппаратного PWM:
#undef RP_PWM_USE_PWM7
#define RP_PWM_USE_PWM7 TRUE
