#include "math.h"

inline uint32 
RoundReal32ToUInt32(real32 Real32)
{
	uint32 Result = (uint32)roundf((Real32));
	return (Result);
}

inline int32 
RoundReal32ToInt32(real32 Real32)
{
	int32 Result = (int32)roundf((Real32));
	return (Result);
}

struct bit_scan_result
{
	bool32 Found;
	uint32 Index;
};

inline bit_scan_result
FindLeastSignificantSetBit(uint32 Value)
{
	bit_scan_result Result = {};

	for(uint32 Test = 0;
	    Test < 32;
	    ++Test)
	{
		if (Value & (1 << Test))
		{
			Result.Index = Test;
			Result.Found = true;
			break;
		}
	}

	return (Result);
}