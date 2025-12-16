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

// constexpr void gpio_set_af(std::uint32_t pin, std::uint8_t af_num)
// {
// 	gpio* gpio{ get_gpio(get_pin_bank(pin)) };
// 	const std::uint32_t n{ get_pin_no(pin) };
// }

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

static volatile std::uint32_t s_ticks;
void SysTick_Handler()
{
	++s_ticks;
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

void blink()
{
	std::uint32_t led = get_pin('A', 5);
	// Move bit 2 bits to the left because that's where IO pins start on APB2ENR.
	RCC->APB2ENR |= (get_bit(get_pin_bank(led)) << 2);
	struct gpio* gpio = get_gpio(0);
	gpio->CRL &= ~(0xFU << 20);	   // Clear MODE5[1:0] and CNF5[1:0]
	gpio->CRL |= (0x1U << 20);	   // MODE5 = 01

	systick_init(8'000'000 / 1000);

	std::uint32_t timer = 0, period = 500;
	for (;;)
	{
		if (timer_expired(&timer, period, s_ticks))
		{
			static bool on{ true };
			gpio_write(led, on);

			on = !on;

			uart_write_buf(USART2, "hi\r\n", 4);
		}
	}
}

void uart_init(uart* uart, const std::uint32_t baud)
{
	// const std::uint32_t rx{ get_pin('A', 3) }, tx{ get_pin('A', 2) };
	RCC->APB1ENR |= get_bit(17);
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

extern "C"
{
	extern unsigned long _estack;
	void _reset();
}

// Startup code
// naked=don't generate prologue/epilogue
// noreturn=it will never exit
__attribute__((naked, noreturn)) void _reset()
{
	// memset .bss to zero, and copy .data section to RAM region
	// linker symbols declared in the .ld file
	extern long _sbss, _ebss, _sdata, _edata, _sidata;
	for (long* dst = &_sbss; dst < &_ebss; dst++)
	{
		*dst = 0;
	}
	for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;)
	{
		*dst++ = *src++;
	}

	main();
	for (;;)
	{
		(void)0;	// Infinite loop in the case if main() returns
	}
}

// 16 standard and 91 STM32-specific handlers
// defines the interrupt vector table and dumps it into .vectors
// via the linker script
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {
	reinterpret_cast<void (*)(void)>(&_estack),
	_reset,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	SysTick_Handler
};
