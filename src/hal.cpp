#include "hal.hpp"

GPIO_TypeDef* get_gpio(const std::uint32_t bank)
{
	return reinterpret_cast<GPIO_TypeDef*>(GPIOA_BASE + 0x400U * (bank));
}

std::int32_t uart_read_ready(USART_TypeDef* uart)
{
	return uart->SR & get_bit(5);	 // If RXNE bit is set, data is ready
}

std::uint8_t uart_read_byte(USART_TypeDef* uart)
{
	return static_cast<std::uint8_t>(uart->DR & 255);	 // If RXNE bit is set, data is ready
}

void uart_write_byte(USART_TypeDef* uart, std::uint8_t byte)
{
	uart->DR = byte;
	while ((uart->SR & get_bit(7)) == 0)
	{
		spin(1);
	}
}
