#pragma once

#define GAME_NAME  "GameB"




LRESULT CALLBACK MainWindowProcedure(HWND, UINT, WPARAM, LPARAM);


DWORD create_main_window();


BOOL game_running_check (void);