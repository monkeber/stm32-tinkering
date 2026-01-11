#include "hal.hpp"
#include "stm32f103xb.h"
#include <cstdio>

static volatile std::uint32_t s_ticks;

extern "C"
{
	void SysTick_Handler()
	{
		++s_ticks;
	}

	void SystemInit(void)
	{
		// Set latency wait states.
		FLASH->ACR |= FLASH_LATENCY | get_bit(4);

		// Enable HSI.
		RCC->CR |= get_bit(0);

		// Clear PLL related registers.
		RCC->CFGR &=
			~(get_bit(16) | get_bit(17) | get_bit(18) | get_bit(19) | get_bit(20) | get_bit(21));
		// Set PLL multiplicator.
		RCC->CFGR |= get_bit(18) | get_bit(19) | get_bit(20) | get_bit(21);
		// Enable PLL and wait until it's on.
		RCC->CR |= get_bit(24);
		while ((RCC->CR & get_bit(25)) == 0)
		{
			spin(1);
		}

		// Set prescalers for AHB, APB1 and APB2.
		RCC->CFGR &= ~(get_bit(4) | get_bit(5) | get_bit(6) | get_bit(7));
		RCC->CFGR &= ~(get_bit(8) | get_bit(9) | get_bit(10));
		RCC->CFGR |= get_bit(10);
		RCC->CFGR &= ~(get_bit(11) | get_bit(12) | get_bit(13));
		// Set PLL as system clock.
		RCC->CFGR |= get_bit(1);
		while ((RCC->CFGR & get_bit(3)) == 0)
		{
			spin(1);
		}

		SysTick_Config(FREQUENCY / 1000);

		RCC->CFGR |= get_bit(24) | get_bit(26);
	}
}

void blink()
{
	std::uint32_t led = get_pin('A', 5);
	// Move bit 2 bits to the left because that's where IO pins start on APB2ENR.
	RCC->APB2ENR |= (get_bit(get_pin_bank(led)) << 2);
	GPIO_TypeDef* gpio = get_gpio(0);
	gpio->CRL &= ~(0xFU << 20);	   // Clear MODE5[1:0] and CNF5[1:0]
	gpio->CRL |= (0x1U << 20);	   // MODE5 = 01

	std::uint32_t timer = 0, period = 500;
	for (;;)
	{
		if (timer_expired(&timer, period, s_ticks))
		{
			static bool on{ true };
			gpio_write(led, on);

			on = !on;

			// uart_write_buf(USART2, "hi\r\n", 4);
			std::printf("LED: %d, tick: %lu\r\n", on, static_cast<unsigned long>(s_ticks));
		}
	}
}

void uart_init(USART_TypeDef* uart, const std::uint32_t baud)
{
	// We need to enable both usart and configure required pins to work on GPIO.
	RCC->APB1ENR |= get_bit(17);
	RCC->APB2ENR |= get_bit(2);

	GPIO_TypeDef* gpio = get_gpio(0);
	gpio->CRL &= ~(0xFFU << 8);
	gpio->CRL |= get_bit(11) | get_bit(9) | get_bit(8);
	gpio->CRL |= get_bit(14);

	uart->CR1 = 0;
	uart->BRR = FREQUENCY / baud;
	uart->CR1 |= get_bit(13) | get_bit(2) | get_bit(3);
}

int main()
{
	uart_init(USART2, 115200);
	blink();
	return 0;
}
