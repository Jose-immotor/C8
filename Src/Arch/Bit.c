
#include "Bit.h"

int Uint32_t_getMaskBit(uint32_t value)
{
	for (int i = 0; i < 32; i++)
	{
		if (value == (1 << i))
			return i;
	}

	return -1;
}
