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
		//   RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;    // Enable SYSCFG
		SysTick_Config(FREQUENCY / 1000);
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

	// systick_init(8'000'000 / 1000);

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
