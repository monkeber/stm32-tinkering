#pragma once

#include <cstdint>

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

bool timer_expired(std::uint32_t* expTime, const std::uint32_t period, const std::uint32_t now)
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

struct gpio
{
	volatile std::uint32_t CRL, CRH, IDR, ODR, BSRR, BRR, LCKR;
};

gpio* get_gpio(const std::uint32_t bank)
{
	return reinterpret_cast<gpio*>(0x40010800 + 0x400 * (bank));
}

// Enum values are per datasheet: 0, 1, 2, 3
enum class GPIO_MODE
{
	INPUT,
	OUTPUT,
	AF,
	ANALOG
};

static inline void gpio_set_mode(const std::uint32_t pin, const std::uint8_t mode)
{
	gpio* gpio = get_gpio(get_pin_bank(pin));	 // GPIO bank
	const std::uint32_t n{ get_pin_no(pin) };	 // Pin number
	gpio->CRL &= ~(3U << (n * 2));				 // Clear existing setting
	gpio->CRL |= (mode & 3u) << (n * 2u);		 // Set new mode
}

static inline void gpio_write(std::uint32_t pin, bool val)
{
	gpio* gpio = get_gpio(get_pin_bank(pin));
	gpio->BSRR = (1u << get_pin_no(pin)) << (val ? 0 : 16);
}

//
// RCC
//

struct rcc
{
	volatile std::uint32_t CR, CFGR, CIR, APB2RSTR, APB1RSTR, AHBENR, APB2ENR, APB1ENR, BDCR, CSR,
		AHBRSTR, CFGR2;
};

rcc* const RCC{ reinterpret_cast<rcc*>(0x40021000) };
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

struct uart
{
	volatile std::uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
};

uart* const USART2{ reinterpret_cast<uart*>(0x40004400) };

constexpr std::uint32_t FREQUENCY{ 8'000'000 };

std::int32_t uart_read_ready(uart* uart)
{
	return uart->SR & get_bit(5);	 // If RXNE bit is set, data is ready
}

std::uint8_t uart_read_byte(uart* uart)
{
	return static_cast<std::uint8_t>(uart->DR & 255);	 // If RXNE bit is set, data is ready
}

void uart_write_byte(uart* uart, std::uint8_t byte)
{
	uart->DR = byte;
	while ((uart->SR & get_bit(7)) == 0)
	{
		spin(1);
	}
}

void uart_write_buf(uart* uart, const char* buf, std::size_t len)
{
	while (len-- > 0)
	{
		uart_write_byte(uart, *reinterpret_cast<const std::uint8_t*>(buf++));
	}
}
