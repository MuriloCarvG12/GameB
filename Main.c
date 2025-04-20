#include <stdio.h>
#pragma warning(push, 0)
#include <windows.h>
#include <stdint.h>
#pragma warning(pop)

#include "GameB.h"

BOOL g_game_is_running;

HANDLE g_window_handle = 0;

RECT g_Game_window_size;

GAME_BIT_MAP g_backbuffer; // this is our backbuffer basically the artist that draws our graphics

GAMEB_PERFORMANCE_DATA g_gameperformance;





int MonitorWidth;
int MonitorHeight;


int  WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	UNREFERENCED_PARAMETER(hInstPrev);
	UNREFERENCED_PARAMETER(cmdline);
	UNREFERENCED_PARAMETER(cmdshow);
	uint64_t FrameStart;
	uint64_t FrameEnd;
	uint64_t CounterMicroseconds = 0;
	uint64_t ElapsedMicroSecondsPerFrame = 0;

	if(game_running_check() == TRUE)
	{
		MessageBox(NULL, "You already opened this!", "Error", MB_ICONEXCLAMATION);
		goto EXIT;
	}



	create_main_window();

	MSG messages;

	QueryPerformanceFrequency(&g_gameperformance.PerfFrequency);

	// here we are registering our bitmap info 
	g_backbuffer.BitMapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
	// make sure we are supplying a negative height for windows to make use of our buffer top -> down supplying it with a positive inverts this direction and for some reason pixels are not drawn
	g_backbuffer.BitMapInfo.bmiHeader.biHeight = -GAME_RES_HEIGHT;
	g_backbuffer.BitMapInfo.bmiHeader.biSize = sizeof(g_backbuffer.BitMapInfo.bmiHeader);
	g_backbuffer.BitMapInfo.bmiHeader.biCompression = BI_RGB;
	g_backbuffer.BitMapInfo.bmiHeader.biBitCount = BPP;
	g_backbuffer.BitMapInfo.bmiHeader.biPlanes = 1;

	if ((g_backbuffer.memory_canvas = VirtualAlloc(NULL, GAME_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) == NULL)
	{
		return (0);
	}



	memset(g_backbuffer.memory_canvas,0x00, GAME_AREA_MEMORY_SIZE); // sets our backbuffer bits to white


	g_game_is_running = TRUE;

	// this loop below is basically a frame for our game during each iteration it checks our inputs, renders the player input and the game graphics
	while(g_game_is_running == TRUE)
	{
		QueryPerformanceCounter(&FrameStart);

		while(PeekMessageA(&messages, g_window_handle, 0 ,0, PM_REMOVE))
		{
			DispatchMessageA(&messages);
		}

		ProcessPlayerInput();
		RenderFrameGraphics(); // needs to be called every 16 milliseconds

		QueryPerformanceCounter(&FrameEnd);
		ElapsedMicroSecondsPerFrame = FrameEnd - FrameStart;
		ElapsedMicroSecondsPerFrame *= 1000000;
		ElapsedMicroSecondsPerFrame /= g_gameperformance.PerfFrequency;

		Sleep(1); // this function allows our process to be shared across multiple threads! so our game wont use 25% of our cpu...

		g_gameperformance.TotalFramesRendered++;
		CounterMicroseconds += ElapsedMicroSecondsPerFrame;

		if(g_gameperformance.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES == 0)
		{
			uint64_t AverageMicroSecondsPerFrame = CounterMicroseconds / CALCULATE_AVG_FPS_EVERY_X_FRAMES;
			float AverageMiliSecondsPerFrame = AverageMicroSecondsPerFrame * 0.001f;

			float avg_fps = 1000000.0f / ElapsedMicroSecondsPerFrame; // divides one second (in this case written in microsecconds by the time it took to draw the frame
			char str[64] = { 0 };
			_snprintf_s(str, _countof(str), _TRUNCATE ," Approx FPS: %.2f , Approx Time: %.2f \n",avg_fps, AverageMiliSecondsPerFrame);
			OutputDebugStringA(str);

			CounterMicroseconds = 0;
		}
	}

EXIT:

	return (0);


   
}



LRESULT CALLBACK MainWindowProcedure(
	HWND hwnd,
	UINT message,
	WPARAM wparam,
	LPARAM lparam
)
{
	switch (message)
	{

	case WM_CLOSE:
		g_game_is_running = FALSE;
		PostQuitMessage(0);
		break;
		

	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
		break;
	}


EXIT:

	return (0);
}
/**
This function is responsible for initializing the creation of our game window, we begin by creating a variable type WNDCLASSEXA and fill up its values


*/
DWORD create_main_window()
{
	
	DWORD RESULT = 0;
	
	// registering our window class information
	WNDCLASSEXA window_class = { sizeof(WNDCLASSEXA) };
	
	

	window_class.style = 0;
	window_class.lpfnWndProc = MainWindowProcedure;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = GetModuleHandleA(NULL);
	window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hbrBackground = CreateSolidBrush(RGB(169, 3, 252)); // this sets the color of our background to a srong purple
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = "Project_Class";
	window_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	// end of our window class registration

	// check if our window registration was sucessfull, that is no information was forgotten
	if (RegisterClassExA(&window_class) == 0)
	{
		MessageBox(NULL, "Window Class Registration Failed", "Error", MB_ICONEXCLAMATION);
		goto EXIT;
	}

	// here we assign our global window handle as the window we have created
	g_window_handle = CreateWindowExA(
		WS_EX_CLIENTEDGE, window_class.lpszClassName, GAME_NAME, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, NULL, NULL);

	g_gameperformance.MonitorInfo.cbSize = sizeof(MONITORINFO);

	if (GetMonitorInfoA(MonitorFromWindow(g_window_handle, MONITOR_DEFAULTTOPRIMARY), &g_gameperformance.MonitorInfo) == 0)// this takes 2 input parameters a monitor handle a and monitor data structure our monitor handle is a function call that returns a handle for our monitor
	{
		RESULT = ERROR_MONITOR_NO_DESCRIPTOR;
		goto EXIT;
	};

	 g_gameperformance.MonitorWidth = g_gameperformance.MonitorInfo.rcMonitor.right - g_gameperformance.MonitorInfo.rcMonitor.left;
	 g_gameperformance.MonitorHeight = g_gameperformance.MonitorInfo.rcMonitor.bottom - g_gameperformance.MonitorInfo.rcMonitor.top;

	// check if our g_window_handle was sucssfully assigned
	if (g_window_handle == NULL)
	{
		MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION);
		goto EXIT;
	}

EXIT:

	return (0);

}

// game running check will verify if theres no other process of this game open!
BOOL game_running_check(void)
{
	HANDLE MUTEX = NULL;

	MUTEX = CreateMutexA(NULL, GAME_NAME, "GAME_MUTEX");

	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}

void ProcessPlayerInput(void)
{

	 short Esc_Key_is_down = GetAsyncKeyState(VK_ESCAPE);
	

		if(Esc_Key_is_down)
		{
			SendMessageA(g_window_handle, WM_CLOSE, 0, 0);
		}

		
}

void RenderFrameGraphics(void) // when we want to draw into a winddow we need a device context and then release it
{
	int test = 8;
	memset(g_backbuffer.memory_canvas, 0xFF, test);

	HDC DeviceContext = GetDC(g_window_handle); // this is important its used to tell windows where to draw our backbuffer!
	// now lets plaster this backbuffer into the window!
	// todo this we will need the size of the window

	PIXEL_32 Pixel = { 0 };

	Pixel.blue = 0xC8;
	Pixel.red = 0xC8;
	Pixel.green = 0;
	Pixel.alpha = 0;

	

	StretchDIBits(DeviceContext, 0, 0, g_gameperformance.MonitorWidth, g_gameperformance.MonitorHeight, 0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT, g_backbuffer.memory_canvas, &g_backbuffer.BitMapInfo, DIB_RGB_COLORS, SRCCOPY);
	// function StretchDIBits is responsible for stretching our backbuffer into  the screen itself this gets drawn as many times as the loop gets called!

	ReleaseDC(g_window_handle, DeviceContext);
}