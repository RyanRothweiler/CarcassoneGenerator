#include <windows.h>
#include <stdint.h>

#include "carcassone.h"
#include "win32_carcassone.h"


global_variable bool GlobalRunning;
global_variable HWND WindowHandle;


LRESULT CALLBACK Win32MessageProcessor(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CLOSE:
		{
			GlobalRunning = false;
			DestroyWindow(hwnd);
		}break;

		case WM_DESTROY:
		{
			GlobalRunning = false;
			PostQuitMessage(0);
		}break;

		default:
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	return 0;
}

inline FILETIME
Win32GetLastWriteTime(char *FileName)
{
	FILETIME LastWriteTime = {};

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesEx(FileName, GetFileExInfoStandard, &Data))
	{
		LastWriteTime = Data.ftLastWriteTime;
	}

	return (LastWriteTime);
}

struct win32_game_code
{
	HMODULE GameCodeDLL;
	FILETIME DLLLastWriteTime;
	
	game_update_and_render *UpdateAndRender;

	bool32 IsValid;
};

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName)
{
	win32_game_code Result = {};

	// WIN32_FILE_ATTRIBUTE_DATA Ignored;
	// if (!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
	// {
	Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);

	CopyFile(SourceDLLName, TempDLLName, FALSE);

	Result.GameCodeDLL = LoadLibraryA(TempDLLName);
	if (Result.GameCodeDLL)
	{
		Result.UpdateAndRender = (game_update_and_render *)GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");

		if (Result.UpdateAndRender)
		{
			Result.IsValid = true;
		}
	}

	if (!Result.IsValid)
	{
		Result.UpdateAndRender = 0;
	}
	// }

	return (Result);
}

internal void 
Win32UnloadGameCode(win32_game_code *GameCode)
{
	if (GameCode->GameCodeDLL)
	{
		if (!FreeLibrary(GameCode->GameCodeDLL))
		{
			Assert(0);
		}
		GameCode->GameCodeDLL = 0;
	}

	GameCode->IsValid = false;
	GameCode->UpdateAndRender = 0;
}

internal int
StringLength(char *String)
{
	int Count = 0;
	while (*String++)
	{
		++Count;
	}
	return (Count);
}

void 
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
	for (int Index = 0;
	     Index < SourceACount;
	     ++Index)
	{
		*Dest++ = *SourceA++;
	}

	for (int Index = 0;
	     Index < SourceBCount;
	     ++Index)
	{
		*Dest++ = *SourceB++;
	}

	*Dest++ = 0;
}


internal void
Win32GetEXEFileName(win32_state *State)
{
	DWORD SizeOfFileName = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
	State->OnePastLastEXEFileNameSlash = State->EXEFileName;
	for (char *Scan = State->EXEFileName;
	     *Scan;
	     ++Scan)
	{
		if (*Scan == '\\')
		{
			State->OnePastLastEXEFileNameSlash = Scan + 1;
		}
	}
}

internal void 
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
	CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName, 
	           StringLength(FileName), FileName, 
	           DestCount, Dest);
}

void 
Win32MainLoop()
{
	win32_state Win32State = {};
	Win32GetEXEFileName(&Win32State);

	char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32BuildEXEPathFileName(&Win32State, "carcassone.dll",
	                          sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
	char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32BuildEXEPathFileName(&Win32State, "carcassone_temp.dll",
	                          sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

	win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath);
	uint32 LoadCounter = 0;

	GlobalRunning = true;
	while (GlobalRunning)
	{
		if (LoadCounter > 120)
		{
			LoadCounter = 0;
			Win32UnloadGameCode(&Game);
			Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath);
		}
		LoadCounter += 1;

		if (Game.UpdateAndRender)
		{
			Game.UpdateAndRender();
		}

		MSG Message;
		while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}	
	}
}

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{

	WNDCLASS WindowClass = {};
	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MessageProcessor;
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.lpszClassName = "CarcaassoneGeneratorWindowClass";

	if(RegisterClass(&WindowClass))
	{
		WindowHandle = 
		CreateWindowEx(
		               0,
		               WindowClass.lpszClassName,
		               "Carcassone Generator",
		               WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		               CW_USEDEFAULT,
		               CW_USEDEFAULT,
		               CW_USEDEFAULT,
		               CW_USEDEFAULT,
		               0,
		               0,
		               Instance,
		               0);
		if (WindowHandle)
		{
			Win32MainLoop();
		}
	}
	return (0);
}