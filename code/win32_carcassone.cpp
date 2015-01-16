#include <windows.h>
#include <stdint.h>

#include "carcassone.h"


global_variable bool GlobalRunning;


internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
	LRESULT Result = 0;

	switch(Message)
	{
		case WM_DESTROY:
		{
			GlobalRunning = false;
		} break;

		case WM_SETCURSOR:
		{

		} break;

		case WM_CLOSE:
		{
			GlobalRunning = false;
		} break;

		case WM_ACTIVATEAPP:
		{
		} break;

		// case WM_PAINT:
		// {
		// 	PAINTSTRUCT Paint;
		// 	HDC DeviceContext = BeginPaint(Window, &Paint);
		// 	win32_window_dimensions Dimensions = Win32GetWindowDimension(Window);
		// 	Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimensions.Width, Dimensions.Height);
		// 	EndPaint(Window, &Paint);
		// } break;

		default:
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return(Result);
}



int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{

	WNDCLASS WindowClass = {};
	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if(RegisterClass(&WindowClass))
	{
		HWND Window = 
		CreateWindowEx(
		               0,
		               WindowClass.lpszClassName,
		               "Handmade Hero",
		               WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		               CW_USEDEFAULT,
		               CW_USEDEFAULT,
		               CW_USEDEFAULT,
		               CW_USEDEFAULT,
		               0,
		               0,
		               Instance,
		               0);
		if (Window)
		{
			GlobalRunning = true;
			while (GlobalRunning)
			{

			}
		}
	}
	return (0);
}