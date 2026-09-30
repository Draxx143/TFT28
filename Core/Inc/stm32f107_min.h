/**
 * stm32f107_min.h — minimal register definitions for STM32F107VCT6.
 *
 * Deliberately dependency-free: no CMSIS, no HAL, no ST headers needed.
 * Only what the LCD demo touches (RCC + GPIO + SysTick-free busy delay).
 */
#ifndef STM32F107_MIN_H
#define STM32F107_MIN_H

#include <stdint.h>

#define __IO volatile

/* ---------------- GPIO ---------------- */
typedef struct {
  __IO uint32_t CRL;   /* 0x00 config pins 0..7   */
  __IO uint32_t CRH;   /* 0x04 config pins 8..15  */
  __IO uint32_t IDR;   /* 0x08 */
  __IO uint32_t ODR;   /* 0x0C */
  __IO uint32_t BSRR;  /* 0x10 set [15:0] / reset [31:16] */
  __IO uint32_t BRR;   /* 0x14 */
  __IO uint32_t LCKR;  /* 0x18 */
} GPIO_T;

#define GPIOA ((GPIO_T *)0x40010800U)
#define GPIOB ((GPIO_T *)0x40010C00U)
#define GPIOC ((GPIO_T *)0x40011000U)
#define GPIOD ((GPIO_T *)0x40011400U)
#define GPIOE ((GPIO_T *)0x40011800U)

/* ---------------- RCC ---------------- */
typedef struct {
  __IO uint32_t CR;
  __IO uint32_t CFGR;
  __IO uint32_t CIR;
  __IO uint32_t APB2RSTR;
  __IO uint32_t APB1RSTR;
  __IO uint32_t AHBENR;
  __IO uint32_t APB2ENR;
  __IO uint32_t APB1ENR;
  __IO uint32_t BDCR;
  __IO uint32_t CSR;
} RCC_T;

#define RCC ((RCC_T *)0x40021000U)

#define RCC_APB2ENR_AFIOEN  (1U << 0)
#define RCC_APB2ENR_IOPAEN  (1U << 2)
#define RCC_APB2ENR_IOPBEN  (1U << 3)
#define RCC_APB2ENR_IOPCEN  (1U << 4)
#define RCC_APB2ENR_IOPDEN  (1U << 5)
#define RCC_APB2ENR_IOPEEN  (1U << 6)

/* ---------------- GPIO helpers (F1 style CRL/CRH) ---------------- */
/* MODE: 00 input, 01 out 10MHz, 10 out 2MHz, 11 out 50MHz
   CNF (output): 00 push-pull, 01 open-drain, 10 AF-PP, 11 AF-OD
   CNF (input) : 00 analog, 01 floating, 10 pull-up/down            */
#define GPIO_CFG_OUT_PP_50MHZ  (0x3U)   /* CNF=00 MODE=11 */
#define GPIO_CFG_IN_FLOATING   (0x4U)   /* CNF=01 MODE=00 */
#define GPIO_CFG_IN_PULL       (0x8U)   /* CNF=10 MODE=00 */

static inline void gpio_config(GPIO_T *port, uint32_t pin, uint32_t cfg)
{
  if (pin < 8U) {
    uint32_t s = pin * 4U;
    port->CRL = (port->CRL & ~(0xFU << s)) | (cfg << s);
  } else {
    uint32_t s = (pin - 8U) * 4U;
    port->CRH = (port->CRH & ~(0xFU << s)) | (cfg << s);
  }
}

static inline void gpio_set(GPIO_T *port, uint32_t pinmask)   { port->BSRR = pinmask; }
static inline void gpio_clear(GPIO_T *port, uint32_t pinmask) { port->BSRR = pinmask << 16; }

#endif /* STM32F107_MIN_H */
