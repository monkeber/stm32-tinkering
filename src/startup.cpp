extern "C"
{
	extern unsigned long _estack;
	void _reset();
}

int main();
void SysTick_Handler();

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
