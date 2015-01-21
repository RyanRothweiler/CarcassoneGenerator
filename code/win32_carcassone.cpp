#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#include "carcassone.h"
#include "win32_carcassone.h"


global_variable bool GlobalRunning;
global_variable win32_window_pixel_information WindowPixelInformation;


internal win32_window_dimensions
Win32GetWindowDimensions(HWND WindowHandle)
{
	win32_window_dimensions Result;

	RECT ClientRect; 
	GetClientRect(WindowHandle, &ClientRect);
	Result.Width = ClientRect.right;
	Result.Height = ClientRect.bottom;

	return (Result);
}

internal void 
ResizeDIBSection(HWND WindowHandle, win32_window_pixel_information *PixelInformation)
{
	if (PixelInformation->PixelMemory)
	{
		VirtualFree(&PixelInformation->PixelMemory, 0, MEM_RELEASE);
	}

	win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(WindowHandle);

	PixelInformation->BitMapInfo.bmiHeader.biSize = sizeof(PixelInformation->BitMapInfo.bmiHeader);
	PixelInformation->BitMapInfo.bmiHeader.biWidth = WindowDimensions.Width;
	PixelInformation->BitMapInfo.bmiHeader.biHeight = -1 * (int32)WindowDimensions.Height;
	PixelInformation->BitMapInfo.bmiHeader.biPlanes = 1;
	PixelInformation->BitMapInfo.bmiHeader.biBitCount = 32;
	PixelInformation->BitMapInfo.bmiHeader.biCompression = BI_RGB;

	PixelInformation->BytesPerPixel = 4;
	PixelInformation->Pitch = WindowDimensions.Width * PixelInformation->BytesPerPixel;

	int PixelsMemorySize = (WindowDimensions.Width * WindowDimensions.Height * PixelInformation->BytesPerPixel);
	PixelInformation->PixelMemory = VirtualAlloc(0, PixelsMemorySize, MEM_COMMIT, PAGE_READWRITE);

	uint8 *Row = (uint8 *)PixelInformation->PixelMemory;
	for (uint32 Y = 0;
	     Y < WindowDimensions.Height;
	     ++Y)
	{
		uint8 *Pixel = (uint8 *)Row;
		for (uint32 X = 0;
		     X < WindowDimensions.Width;
		     ++X)
		{
			*Pixel = 0;
			++Pixel;

			*Pixel = 0;
			++Pixel;

			*Pixel = 0;
			++Pixel;

			*Pixel = 0;
			++Pixel;

		}
		Row += PixelInformation->Pitch;
	}
}

internal void 
Win32DisplayPixelInformation(HWND WindowHandle, HDC DeviceContext, win32_window_pixel_information *WindowPixelInformation)
{
	win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(WindowHandle);
	StretchDIBits(DeviceContext,
	              0, 0, WindowDimensions.Width, WindowDimensions.Height,
	              0, 0, WindowDimensions.Width, WindowDimensions.Height,
	              WindowPixelInformation->PixelMemory,
	              &WindowPixelInformation->BitMapInfo,
	              DIB_PAL_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32WindowCallback(HWND WindowHandle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_SIZE:
		{
			win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(WindowHandle);
			ResizeDIBSection(WindowHandle, &WindowPixelInformation);
		}break;

		case WM_CLOSE:
		{
			GlobalRunning = false;
			DestroyWindow(WindowHandle);
		}break;

		case WM_DESTROY:
		{
			GlobalRunning = false;
			PostQuitMessage(0);
		}break;

		case WM_PAINT:
		{
			PAINTSTRUCT PaintStruct;
			HDC DeviceContext = BeginPaint(WindowHandle, &PaintStruct);

			win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(WindowHandle);
			Win32DisplayPixelInformation(WindowHandle, DeviceContext, &WindowPixelInformation);

			EndPaint(WindowHandle, &PaintStruct);
		}break;

		default:
		{
			return DefWindowProc(WindowHandle, msg, wParam, lParam);
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

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName)
{
	win32_game_code Result = {};

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

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
	WNDCLASS WindowClass = {};
	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32WindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.lpszClassName = "CarcaassoneGeneratorWindowClass";

	if(RegisterClass(&WindowClass))
	{
		HWND WindowHandle = 
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
			win32_state Win32State = {};
			Win32GetEXEFileName(&Win32State);

			char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
			Win32BuildEXEPathFileName(&Win32State, "carcassone.dll",
			                          sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
			char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
			Win32BuildEXEPathFileName(&Win32State, "carcassone_temp.dll",
			                          sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

			win32_window_pixel_information WindowPixelInformation = {};
			ResizeDIBSection(WindowHandle, &WindowPixelInformation);

			win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath);
			uint32 LoadCounter = 0;


			game_input GameInput = {};


			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = Gigabytes(1);

			LPVOID BaseAddress = (LPVOID)Terrabytes((uint64)2);
			Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)GameMemory.PermanentStorageSize,
			                                          MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			GameMemory.PermanentStorage = Win32State.GameMemoryBlock;

			if (GameMemory.PermanentStorage)
			{
				GlobalRunning = true;
				while (GlobalRunning)
				{
					MSG Message;
					while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&Message);
						DispatchMessage(&Message);
					}	


					if (LoadCounter > 120)
					{
						LoadCounter = 0;
						Win32UnloadGameCode(&Game);
						Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath);
					}
					LoadCounter += 1;


					DWORD dwResult;    
					for (DWORD ControllerIndex = 0; 
					     ControllerIndex < XUSER_MAX_COUNT; 
					     ControllerIndex++)
					{
						XINPUT_STATE ControllerState;
						ZeroMemory(&ControllerState, sizeof(XINPUT_STATE));

						dwResult = XInputGetState(ControllerIndex, &ControllerState);

						if(dwResult == ERROR_SUCCESS)
						{
							XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

							GameInput.DPadUp = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
							GameInput.DPadDown = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
							GameInput.DPadLeft = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
							GameInput.DPadRight = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
						}
						else
						{
            		// Controller is not connected 
						}
					}

					game_screen_information GameScreenInformation = {};
					win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(WindowHandle);
					GameScreenInformation.PixelMemory = &WindowPixelInformation.PixelMemory;
					GameScreenInformation.Width = WindowDimensions.Width;
					GameScreenInformation.Height = WindowDimensions.Height;
					GameScreenInformation.Pitch = WindowPixelInformation.Pitch;
					GameScreenInformation.BytesPerPixel = WindowPixelInformation.BytesPerPixel;


					if (Game.UpdateAndRender)
					{
						Game.UpdateAndRender(GameInput, &GameScreenInformation, &GameMemory);
					}


					HDC DeviceContext = GetDC(WindowHandle);
					win32_window_dimensions Dimensions = Win32GetWindowDimensions(WindowHandle);
					Win32DisplayPixelInformation(WindowHandle, DeviceContext, &WindowPixelInformation);
					ReleaseDC(WindowHandle, DeviceContext);


					GameInput.DPadUp = false;
					GameInput.DPadDown = false;
					GameInput.DPadLeft = false;
					GameInput.DPadRight = false;
					GameInput.ActionUp = false;
					GameInput.ActionDown = false;
					GameInput.ActionLeft = false;
					GameInput.ActionRight = false;
					GameInput.RightShoulder = false;
					GameInput.LeftShoulder = false;
					GameInput.RightBumper = false;
					GameInput.LeftBumper = false;
					GameInput.Start = false;
					GameInput.Select = false;
					GameInput.Home = false;

				}
			}
		}
	}
	return (0);
}