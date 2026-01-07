#include "hal.hpp"

#include <sys/stat.h>

extern "C"
{
	int _write(int fd, char* ptr, int len)
	{
		if (fd == 1)
		{
			uart_write_buf(USART2, ptr, static_cast<std::size_t>(len));
		}

		return -1;
	}

	int _fstat(int fd, struct stat* st)
	{
		if (fd < 0)
			return -1;
		st->st_mode = S_IFCHR;
		return 0;
	}

	void* _sbrk(int incr)
	{
		extern char _end;
		static unsigned char* heap = nullptr;
		unsigned char* prev_heap;
		if (heap == nullptr)
		{
			heap = (unsigned char*)&_end;
		}
		prev_heap = heap;
		heap += incr;

		return prev_heap;
	}

	int _close([[maybe_unused]] int fd)
	{
		return -1;
	}

	int _isatty([[maybe_unused]] int fd)
	{
		return 1;
	}

	int _read([[maybe_unused]] int fd, [[maybe_unused]] char* ptr, [[maybe_unused]] int len)
	{
		return -1;
	}

	int _lseek([[maybe_unused]] int fd, [[maybe_unused]] int ptr, [[maybe_unused]] int dir)
	{
		return 0;
	}

	void _init(void)
	{
	}
}
