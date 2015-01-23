

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

inline uint32
SafeTruncateUInt62(uint64 Value)
{
	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return (Result);
}

#include "vector2.h"

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

	bool32 RightTrigger;
	bool32 LeftTrigger;

	bool32 Start;
	bool32 Select;

	bool32 Home;

	real32 dtForFrame;
};

struct read_file_result
{
	uint32 FileSize;
	void *FileData;
};

#define PLATFORM_READ_FILE(name) read_file_result name(char *Filename)
typedef PLATFORM_READ_FILE(platform_read_file);

struct bitmap_image
{
	int32 Width;
	int32 Height;
	uint32 *Pixels;
};

struct game_memory
{
	bool IsInitialized;

	uint64 PermanentStorageSize;
	void *PermanentStorage;

	platform_read_file *PlatformReadFile;
};

struct tile
{
/*
	1 = Road
	2 = Farm
	3 = City
	100 = Taken
*/
	int32 TopType;
	int32 BottomType;
	int32 RightType;
	int32 LeftType;

	vector2 TilePosition;

	bitmap_image Sprite;
	bool32 TileHasBeenSet;
};

struct tile_map
{
	int32 TileCountX = 255;
	int32 TileCountY = 256;

	int32 TileWidth;

	tile Tiles [256][256];
};

struct game_state
{
	vector2 CameraPosition;
	vector2 LastCameraPosition;

	bitmap_image RoadRoadImage;

	tile_map WorldTileMap;
};

struct color
{
	real32 R;
	real32 G;
	real32 B;
	real32 A;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_input GameInput, game_screen_information *GameScreenInformation, game_memory *GameMemory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);