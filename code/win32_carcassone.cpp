#include <math.h>
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#include "carcassone.h"
#include "win32_carcassone.h"


global_variable bool GlobalRunning;
global_variable int64 GlobalPerfCountFrequency;
global_variable win32_window_pixel_information WindowPixelInformation;
global_variable game_input GameInput;


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

		case WM_KEYUP:
		{
			uint32 VKCode = (uint32)wParam;

			if (VKCode == 'W')
			{
				GameInput.DPadUp = false;
			}
			if (VKCode == 'A')
			{
				GameInput.DPadLeft = false;
			}
			if (VKCode == 'S')
			{
				GameInput.DPadDown = false;
			}
			if (VKCode == 'D')
			{
				GameInput.DPadRight = false;
			}
			if (VKCode == 0x10)
			{
				GameInput.Shift = false;
			}
		}break;

		case WM_KEYDOWN:
		{
			uint32 VKCode = (uint32)wParam;

			if (VKCode == 'W')
			{
				GameInput.DPadUp = true;
			}
			if (VKCode == 'A')
			{
				GameInput.DPadLeft = true;
			}
			if (VKCode == 'S')
			{
				GameInput.DPadDown = true;
			}
			if (VKCode == 'D')
			{
				GameInput.DPadRight = true;
			}

			if (VKCode == 0x10)
			{
				GameInput.Shift = true;
			}
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

internal real32
Win32ProcessStickInput(short Value, SHORT DeadZoneThreshold)
{
	real32 Result = 0;

	if (Value < -DeadZoneThreshold)
	{
		Result = (real32)(Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold);
	}
	else if (Value > DeadZoneThreshold)
	{
		Result = (real32)(Value  + DeadZoneThreshold) / (32767.0f - DeadZoneThreshold);
	}

	return (Result);
}

PLATFORM_READ_FILE(PlatformReadFile)
{
	read_file_result Result = {};

	HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(FileHandle, &FileSize))
		{
			uint32 FileSize32 = SafeTruncateUInt62(FileSize.QuadPart);
			Result.FileData = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (Result.FileData)
			{
				DWORD BytesRead;
				if (ReadFile(FileHandle, Result.FileData, FileSize32, &BytesRead, 0) && 
					(FileSize32 == BytesRead))
				{
					Result.FileSize = FileSize32;
				}
				else if (Result.FileData)
				{
					VirtualFree(Result.FileData, 0, MEM_RELEASE);
					Result.FileSize = 0;
				}
			}
			else
			{

			}
		}
		else
		{

		}

		CloseHandle(FileHandle);
	}
	else
	{

	}

	return (Result);
}

inline LARGE_INTEGER 
Win32GetWallClock()
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return (Result);
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	real32 Result = ((real32)(End.QuadPart - Start.QuadPart) / (real32)GlobalPerfCountFrequency);
	return (Result);
}

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{

	win32_state Win32State = {};
	Win32GetEXEFileName(&Win32State);

	char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32BuildEXEPathFileName(&Win32State, "carcassone.dll",
		sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
	char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32BuildEXEPathFileName(&Win32State, "carcassone_temp.dll",
		sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

	LARGE_INTEGER PerfCountFrequencyReult;
	QueryPerformanceFrequency(&PerfCountFrequencyReult);
	GlobalPerfCountFrequency = PerfCountFrequencyReult.QuadPart;

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
			win32_window_pixel_information WindowPixelInformation = {};
			ResizeDIBSection(WindowHandle, &WindowPixelInformation);

			LARGE_INTEGER LastCounter = Win32GetWallClock();
			LARGE_INTEGER FlipWallClock = Win32GetWallClock();


			win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath);
			uint32 LoadCounter = 0;

			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = Gigabytes(1);
			GameMemory.PlatformReadFile = PlatformReadFile;

			LPVOID BaseAddress = (LPVOID)Terrabytes((uint64)2);
			Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)GameMemory.PermanentStorageSize,
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			GameMemory.PermanentStorage = Win32State.GameMemoryBlock;

			if (GameMemory.PermanentStorage)
			{
				int64 LastCycleCount = __rdtsc();

				GlobalRunning = true;
				while (GlobalRunning)
				{
					MSG Message;
					while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&Message);
						if (Message.message == WM_SYSKEYDOWN)
						{
							Assert(0);
						}
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

							GameInput.StickAverageX = Win32ProcessStickInput(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							GameInput.StickAverageY = Win32ProcessStickInput(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

							GameInput.DPadUp = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
							GameInput.DPadDown = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
							GameInput.DPadLeft = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
							GameInput.DPadRight = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

							GameInput.RightShoulder = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
							GameInput.LeftShoulder = Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;

							GameInput.RightTrigger = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
							GameInput.LeftTrigger = Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
						}
						else
						{
            			// Controller is not connected 
						}
					}

					game_screen_information GameScreenInformation = {};
					win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(WindowHandle);
					GameScreenInformation.PixelMemory = WindowPixelInformation.PixelMemory;
					GameScreenInformation.Width = WindowDimensions.Width;
					GameScreenInformation.Height = WindowDimensions.Height;
					GameScreenInformation.Pitch = WindowPixelInformation.Pitch;
					GameScreenInformation.BytesPerPixel = WindowPixelInformation.BytesPerPixel;


					if (Game.UpdateAndRender)
					{
						Game.UpdateAndRender(GameInput, &GameScreenInformation, &GameMemory);
					}


					LARGE_INTEGER EndCounter = Win32GetWallClock();
					real32 MSPerFrame = 1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter);   
					LastCounter = EndCounter;

					HDC DeviceContext = GetDC(WindowHandle);
					win32_window_dimensions Dimensions = Win32GetWindowDimensions(WindowHandle);
					Win32DisplayPixelInformation(WindowHandle, DeviceContext, &WindowPixelInformation);
					ReleaseDC(WindowHandle, DeviceContext);

					FlipWallClock = Win32GetWallClock();


					// GameInput.DPadUp = false;
					// GameInput.DPadDown = false;
					// GameInput.DPadLeft = false;
					// GameInput.DPadRight = false;
					GameInput.ActionUp = false;
					GameInput.ActionDown = false;
					GameInput.ActionLeft = false;
					GameInput.ActionRight = false;
					GameInput.RightShoulder = false;
					GameInput.LeftShoulder = false;
					GameInput.RightTrigger = false;
					GameInput.LeftTrigger = false;
					GameInput.Start = false;
					GameInput.Select = false;
					GameInput.Home = false;


					uint64 EndCycleCount = __rdtsc();
					uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
					LastCycleCount = EndCycleCount;

					real64 FPS = 0.0f;
					real64 MCPF = ((real64)CyclesElapsed / (1000.0f * 1000.0f));

					char FPSBuffer[256];
					_snprintf_s(FPSBuffer, sizeof(FPSBuffer),
						"%.02fms/f,  %.02ff/s,  %.02fmc/f\n", MSPerFrame, FPS, MCPF);
					OutputDebugStringA(FPSBuffer);

				}
			}
		}
	}
	return (0);
}