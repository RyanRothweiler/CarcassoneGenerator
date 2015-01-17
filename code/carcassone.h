

#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}

#define global_variable static
#define internal static
#define local_persist static

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

#define GAME_UPDATE_AND_RENDER(name) void name()
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);