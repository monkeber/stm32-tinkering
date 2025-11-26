#include <cstdint>

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
	struct gpio* gpio = get_gpio(get_pin_bank(pin));	// GPIO bank
	const std::uint32_t n{ get_pin_no(pin) };			// Pin number
	gpio->CRL &= ~(3U << (n * 2));						// Clear existing setting
	gpio->CRL |= (mode & 3u) << (n * 2u);				// Set new mode
}

struct rcc
{
	volatile std::uint32_t CR, CFGR, CIR, APB2RSTR, APB1RSTR, AHBENR, APB2ENR, APB1ENR, BDCR, CSR,
		AHBRSTR, CFGR2;
};

rcc* const RCC{ reinterpret_cast<rcc*>(0x40021000) };
// User led is connected to PB13.

static inline void gpio_write(std::uint32_t pin, bool val)
{
	struct gpio* gpio = get_gpio(get_pin_bank(pin));
	gpio->BSRR = (1u << get_pin_no(pin)) << (val ? 0 : 16);
}

static inline void spin(volatile std::uint32_t count)
{
	while (count--)
	{
		(void)0;
	}
}

int main()
{
	std::uint32_t led = get_pin('A', 5);
	// RCC->APB2ENR |= get_bit(get_pin_bank(led));
	RCC->APB2ENR |= (1 << 2);
	struct gpio* gpio = get_gpio(0);
	gpio->CRL &= ~(0xFU << 20);	   // Clear MODE5[1:0] and CNF5[1:0]
	gpio->CRL |= (0x1U << 20);	   // MODE5 = 01 (Output mode, max speed 10 MHz)
	for (;;)
	{
		gpio_write(led, true);
		spin(999999);
		gpio_write(led, false);
		spin(999999);
	}

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
};
