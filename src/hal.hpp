#pragma once

#include <cstdint>

extern "C"
{

#include <stm32f103xb.h>
}

//
// Pin operations.
//

constexpr std::uint32_t get_bit(const std::uint32_t x)
{
	return std::uint32_t{ 1 } << x;
}
constexpr std::uint32_t get_pin(const std::uint32_t bank, const std::uint32_t num)
{
	return ((bank - 'A') << 8) | num;
}

constexpr std::uint32_t get_pin_no(const std::uint32_t pin)
{
	return pin & 255;
}

constexpr std::uint32_t get_pin_bank(const std::uint32_t pin)
{
	return pin >> 8;
}

//
// General.
//

static inline void spin(volatile std::uint32_t count)
{
	while (count--)
	{
		(void)0;
	}
}

constexpr bool timer_expired(
	std::uint32_t* expTime, const std::uint32_t period, const std::uint32_t now)
{
	if ((now + period) < *expTime)
	{
		*expTime = 0;
	}
	if (*expTime == 0)
	{
		*expTime = now + period;
	}
	if (*expTime > now)
	{
		return false;
	}

	*expTime = (now - *expTime) > period ? now + period : *expTime + period;

	return true;
}

//
// GPIO
//

GPIO_TypeDef* get_gpio(const std::uint32_t bank);

static inline void gpio_set_mode(const std::uint32_t pin, const std::uint8_t mode)
{
	GPIO_TypeDef* gpio = get_gpio(get_pin_bank(pin));	 // GPIO bank
	const std::uint32_t n{ get_pin_no(pin) };			 // Pin number
	gpio->CRL &= ~(3U << (n * 2));						 // Clear existing setting
	gpio->CRL |= (mode & 3u) << (n * 2u);				 // Set new mode
}

static inline void gpio_write(std::uint32_t pin, bool val)
{
	GPIO_TypeDef* gpio = get_gpio(get_pin_bank(pin));
	gpio->BSRR = (1u << get_pin_no(pin)) << (val ? 0 : 16);
}

//
// RCC
//

// User led is connected to PB13.

//
// SysTick
//

struct systick
{
	volatile std::uint32_t CSR, RVR, CVR, CALIB;
};

systick* const SYSTICK{ reinterpret_cast<systick*>(0xe000e010) };

constexpr void systick_init(std::uint32_t ticks)
{
	if ((ticks - 1) > 0xffffff)
	{
		return;
	}

	SYSTICK->RVR = ticks - 1;
	SYSTICK->CVR = 0;
	SYSTICK->CSR = get_bit(0) | get_bit(1) | get_bit(2);
}

//
// USART
//

constexpr std::uint32_t FREQUENCY{ 64'000'000 };

std::int32_t uart_read_ready(USART_TypeDef* uart);

std::uint8_t uart_read_byte(USART_TypeDef* uart);

void uart_write_byte(USART_TypeDef* uart, std::uint8_t byte);

constexpr void uart_write_buf(USART_TypeDef* uart, const char* buf, std::size_t len)
{
	while (len-- > 0)
	{
		uart_write_byte(uart, *reinterpret_cast<const std::uint8_t*>(buf++));
	}
}

enum
{
	APB1_PRE = 5 /* AHB clock / 4 */,
	APB2_PRE = 4 /* AHB clock / 2 */
};
enum
{
	PLL_HSI = 16,
	PLL_M = 8,
	PLL_N = 180,
	PLL_P = 2
};	  // Run at 180 Mhz
constexpr std::uint32_t FLASH_LATENCY{ 2 };
#define SYS_FREQUENCY (72 * 1000000)
#define APB2_FREQUENCY (SYS_FREQUENCY / (BIT(APB2_PRE - 3)))
#define APB1_FREQUENCY (SYS_FREQUENCY / (BIT(APB1_PRE - 3)))
