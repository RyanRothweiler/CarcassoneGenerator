#include "math.h"

inline uint32 
RoundReal32ToUInt32(real32 Real32)
{
	uint32 Result = (uint32)roundf((Real32));
	return (Result);
}
