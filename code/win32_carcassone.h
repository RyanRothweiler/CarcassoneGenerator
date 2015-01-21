

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH


struct win32_window_pixel_information
{
	void *PixelMemory;
	int BytesPerPixel;
	BITMAPINFO BitMapInfo;
	uint32 Pitch; 
};

struct win32_window_dimensions
{
	uint32 Width;
	uint32 Height;
};

struct win32_state
{
	char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
	char *OnePastLastEXEFileNameSlash;

	uint64 TotalSize;
	void *GameMemoryBlock;
};

struct win32_game_code
{
	HMODULE GameCodeDLL;
	FILETIME DLLLastWriteTime;
	
	game_update_and_render *UpdateAndRender;

	bool32 IsValid;
};
