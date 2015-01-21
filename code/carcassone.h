

#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}

#define global_variable static
#define internal static
#define local_persist static

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)
#define Terrabytes(value) (Megabytes(value) * 1024)

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float real32;
typedef double real64;

typedef int32 bool32;

struct game_screen_information
{
	void *PixelMemory;
	uint32 Width;
	uint32 Height;
	uint32 Pitch;
	uint32 BytesPerPixel;
};

struct game_input
{
	real32 StickAverageX;
	real32 StickAverageY;

	bool32 DPadUp;
	bool32 DPadDown;
	bool32 DPadLeft;
	bool32 DPadRight;

	bool32 ActionUp;
	bool32 ActionDown;
	bool32 ActionRight;
	bool32 ActionLeft;

	bool32 LeftShoulder;
	bool32 RightShoulder;

	bool32 RightBumper;
	bool32 LeftBumper;

	bool32 Start;
	bool32 Select;

	bool32 Home;
};

struct game_memory
{
	bool IsInitialized;

	uint64 PermanentStorageSize;
	void *PermanentStorage;
};

struct vector2
{
	uint32 X;
	uint32 Y;
};

struct game_state
{
	vector2 SquarePosition;
	uint32 SquareSideLength;
};

struct color
{
	uint32 R;
	uint32 G;
	uint32 B;
	uint32 A;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_input GameInput, game_screen_information *GameScreenInformation, game_memory *GameMemory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);