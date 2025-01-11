#include <stdio.h>
#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

#include "GameB.h"

BOOL g_game_is_running;

HANDLE g_window_handle = 0;


void ProcessPlayerInput(void);

int  WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	UNREFERENCED_PARAMETER(hInstPrev);
	UNREFERENCED_PARAMETER(cmdline);
	UNREFERENCED_PARAMETER(cmdshow);


	if(game_running_check() == TRUE)
	{
		MessageBox(NULL, "You already opened this!", "Error", MB_ICONEXCLAMATION);
		goto EXIT;
	}


	create_main_window();

	MSG messages;


	g_game_is_running = TRUE;

	while(g_game_is_running == TRUE)
	{
		while(PeekMessageA(&messages, g_window_handle, 0 ,0, PM_REMOVE))
		{
			DispatchMessageA(&messages);
		}

		ProcessPlayerInput();
		// render frame graphicz

		Sleep(1);
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

DWORD create_main_window()
{
	DWORD RESULT = 0;
	
	WNDCLASSEXA window_class = { sizeof(WNDCLASSEXA) };
	


	window_class.style = 0;
	window_class.lpfnWndProc = MainWindowProcedure;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = GetModuleHandleA(NULL);
	window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = "Project_Class";
	window_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (RegisterClassExA(&window_class) == 0)
	{
		MessageBox(NULL, "Window Class Registration Failed", "Error", MB_ICONEXCLAMATION);
		goto EXIT;
	}


	g_window_handle = CreateWindowExA(
		WS_EX_CLIENTEDGE, window_class.lpszClassName, GAME_NAME, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, NULL, NULL);

	if (g_window_handle == NULL)
	{
		MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION);
		goto EXIT;
	}

EXIT:

	return (0);

}


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