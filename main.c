#include <stdio.h>

#pragma warning(push, 0)

#pragma warning(pop)

#include "projectdeclarations.h"
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

GAME_BIT_MAP g_backbuffer;
game_performance_info game_performance;

BOOL g_game_is_running;
HANDLE g_window_handle = 0;
DWORD create_main_window();
void processInput();
void rendergraphics();
DWORD InitializePlayer();
VOID Load32BppIntoBackBuffer(GAME_BIT_MAP *, int , int );

__m128i data;
game_info GInfo;
Player g_Player;
BOOL gWindowHasFocus;
GAME_BIT_MAP *g_CurrentSprite;

int  WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    UNREFERENCED_PARAMETER(hInstPrev);
    UNREFERENCED_PARAMETER(cmdline);
    UNREFERENCED_PARAMETER(cmdshow);
    uint64_t FrameStart = 0;
    uint64_t FrameEnd = 0;
    uint64_t ElapsedFrameTime = 0;
    uint64_t i64_AverageMicroSecondsPerFrame = 0;
    uint64_t i64_ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
    uint64_t i64_ElapsedMicrosecondsPerFrameAccumulatorCooked = 0;
    FILETIME ProcessCreation;
    FILETIME ProcessExitTime;
    uint64_t CurrentKernelCpuTime;
    uint64_t PreviousKernelCpuTime;
    uint64_t CurrentUserCpuTime;
    uint64_t PreviousUserCpuTime;


    HMODULE NtDllModuleHandle;
    if ((NtDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL)
    {
        MessageBoxA(NULL, "Couldn't load ntdll.dll!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }
    _NtQueryTimerResolution NtQueryTimerResolution;

    if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(NtDllModuleHandle, "NtQueryTimerResolution")) == NULL)
    {
        MessageBoxA(NULL, "Couldn't find the NtQueryTimerResolution function in ntdll.dll!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    NtQueryTimerResolution(&game_performance.MinimumTimerResolution, &game_performance.MaximumTimerResolution, &game_performance.CurrentTimerResolution);
    GetSystemInfo(&game_performance.SystemInfo);

    create_main_window();

    MSG messages;
    g_game_is_running = TRUE;
    game_performance.DebugModeOn = FALSE;

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

    if(InitializePlayer() != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Failed to initialize hero!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    g_CurrentSprite = &g_Player.PlayerSprite[character_sprite_down_standing];

    memset(g_backbuffer.memory_canvas,0x00, GAME_AREA_MEMORY_SIZE);

    QueryPerformanceFrequency(&game_performance.TickFrequency );


    GInfo.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(g_window_handle, MONITOR_DEFAULTTOPRIMARY), &GInfo.MonitorInfo) == 0)// this takes 2 input parameters a monitor handle a and monitor data structure our monitor handle is a function call that returns a handle for our monitor
    {
        goto EXIT;
    };

    GInfo.monitor_width = GInfo.MonitorInfo.rcMonitor.right - GInfo.MonitorInfo.rcMonitor.left;
    GInfo.monitor_height =  GInfo.MonitorInfo.rcMonitor.bottom - GInfo.MonitorInfo.rcMonitor.top ;

    if (timeBeginPeriod(1) == TIMERR_NOCANDO)
    {
        MessageBoxA(NULL, "Failed to set global timer resolution!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }
    /**
    if (SetPriorityClass(g_window_handle, HIGH_PRIORITY_CLASS) == 0)
    {
        MessageBoxA(NULL, "Failed to set process priority!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }
     **/
    /**
    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == 0)
    {
        MessageBoxA(NULL, "Failed to set thread priority!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }
    **/
    while(g_game_is_running == TRUE)
    {
        QueryPerformanceCounter(&FrameStart);
        while(PeekMessageA(&messages, g_window_handle, 0 ,0, PM_REMOVE))


        {
            DispatchMessageA(&messages);
        }

        // render frame graphicz
        processInput();
        rendergraphics();

        QueryPerformanceCounter(&FrameEnd);
        ElapsedFrameTime = FrameEnd - FrameStart;
        //convert to MicroSeconds
        ElapsedFrameTime *= 1000000;
        ElapsedFrameTime /= game_performance.TickFrequency;
        game_performance.TotalFramesRendered++;


        Sleep(1);


        i64_ElapsedMicrosecondsPerFrameAccumulatorRaw += ElapsedFrameTime;

        while (ElapsedFrameTime < TARGET_MICROSECONDS_PER_FRAME)
        {
            QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);
            ElapsedFrameTime = FrameEnd - FrameStart;
            ElapsedFrameTime *= 1000000;
            ElapsedFrameTime /= game_performance.TickFrequency;
            if (ElapsedFrameTime < (TARGET_MICROSECONDS_PER_FRAME * 0.75f) )
            {
                Sleep(1); // Could be anywhere from 1ms to a full system timer tick? (~15.625ms)
            }
        }

        i64_ElapsedMicrosecondsPerFrameAccumulatorCooked += ElapsedFrameTime;

        if(game_performance.TotalFramesRendered % CALCULATE_FPS_EVERY_X_FRAMES == 0)
        {
            GetSystemTimeAsFileTime(&game_performance.CurrentSystemTime);
            GetProcessTimes
            (
                    GetCurrentProcess(),
                    &ProcessCreation,
                    &ProcessExitTime,
                    (FILETIME*) &CurrentKernelCpuTime,
                    (FILETIME*) &CurrentUserCpuTime
                    );



            GetProcessHandleCount(GetCurrentProcess(), &game_performance.HandleCount);
            GetProcessMemoryInfo(GetCurrentProcess(),&game_performance.MemoryInfo, sizeof(game_performance.MemoryInfo));

            int64_t i64_AverageMicrosecondsPerFrameRaw = i64_ElapsedMicrosecondsPerFrameAccumulatorRaw / CALCULATE_FPS_EVERY_X_FRAMES;

            int64_t i64_AverageMicrosecondsPerFrameCooked = i64_ElapsedMicrosecondsPerFrameAccumulatorCooked / CALCULATE_FPS_EVERY_X_FRAMES;

            game_performance.AvgFpsRaw = 1.0f / ((i64_ElapsedMicrosecondsPerFrameAccumulatorRaw / 60) * 0.000001f);
            game_performance.AvgFpsCooked = 1.0f / ((i64_ElapsedMicrosecondsPerFrameAccumulatorCooked / 60) * 0.000001f);

            game_performance.CpuPercentage = (CurrentKernelCpuTime - PreviousKernelCpuTime) +  \
                                                (CurrentUserCpuTime - PreviousUserCpuTime);

            game_performance.CpuPercentage /= (float)(game_performance.CurrentSystemTime - game_performance.PreviousSystemTime);

            game_performance.CpuPercentage /= game_performance.SystemInfo.dwNumberOfProcessors;

            game_performance.CpuPercentage *= 100;

            printf("Avg milliseconds/frame raw: %lu Avg FPS Cooked: %.01f Avg FPS Raw: %.01f\n",
                   i64_AverageMicrosecondsPerFrameRaw,
                   game_performance.AvgFpsCooked,
                   game_performance.AvgFpsRaw);


            i64_ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
            i64_ElapsedMicrosecondsPerFrameAccumulatorCooked = 0;

            PreviousKernelCpuTime = CurrentKernelCpuTime;
            PreviousUserCpuTime = CurrentUserCpuTime;
            game_performance.PreviousSystemTime = game_performance.CurrentSystemTime;
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
        case WM_ACTIVATE:


        {
            if (wparam == 0)
            {
                // Our window has lost focus
                gWindowHasFocus = FALSE;
            }
            else
            {
                // Our window has gained focus
                ShowCursor(FALSE);
                gWindowHasFocus = TRUE;

            }
            break;
        }
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
    window_class.hbrBackground = CreateSolidBrush(RGB(180,90, 100));
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = "Project_Class";
    window_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (RegisterClassExA(&window_class) == 0)
    {
        MessageBox(NULL, "Window Class Registration Failed", "Error", MB_ICONEXCLAMATION);
        goto EXIT;
    }

    g_window_handle = CreateWindowExA
            (
                    WS_EX_CLIENTEDGE, window_class.lpszClassName, "WINAPI32REVIEW", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, NULL, NULL
            );
    if (g_window_handle == NULL)
    {
        MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION);
        goto EXIT;
    }
   EXIT:
    return (0);
}

void processInput()
{
    short Esc_Key_is_down = GetAsyncKeyState(VK_ESCAPE);
    short Debug_key_is_down = GetAsyncKeyState(VK_F12);
    short RunKeyIsDown = GetAsyncKeyState(VK_LSHIFT);
    short UpKeyIsDown = GetAsyncKeyState('W');
    short LeftKeyIsDown = GetAsyncKeyState('A');
    short RightKeyIsDown = GetAsyncKeyState('D');
    short DownKeyIsDown = GetAsyncKeyState('S');
    static short LeftKeyWasDown;
    static short RightKeyWasDown;
    static short debug_key_was_down;

    if(Esc_Key_is_down)
    {
        SendMessageA(g_window_handle, WM_CLOSE, 0, 0);
    }

    if(Debug_key_is_down && !debug_key_was_down)
    {
        game_performance.DebugModeOn = !game_performance.DebugModeOn ;
    }

    if(LeftKeyIsDown)
    {
        if(g_Player.ScreenPosX > 0)
        {
            g_Player.ScreenPosX -= 1;
            g_CurrentSprite = &g_Player.PlayerSprite[character_sprite_left_standing];

        }

    }

    if(RightKeyIsDown)
    {
        if(g_Player.ScreenPosX < GAME_RES_WIDTH - 16)
        {
            g_Player.ScreenPosX += 1;
            g_CurrentSprite = &g_Player.PlayerSprite[character_sprite_right_standing];
        }

    }

    if(UpKeyIsDown)
    {
        if(g_Player.ScreenPosY > 0)
        {
            g_Player.ScreenPosY -= 1;
            g_CurrentSprite = &g_Player.PlayerSprite[character_sprite_up_standing];
        }

    }

    if(DownKeyIsDown)
    {
        if(g_Player.ScreenPosY < (GAME_RES_HEIGHT - 16))
        {
            g_Player.ScreenPosY += 1;
            g_CurrentSprite = &g_Player.PlayerSprite[character_sprite_down_standing];
        }
    }

    if(UpKeyIsDown && RunKeyIsDown)
    {
        if(g_Player.ScreenPosY > 1)
        {
            g_Player.ScreenPosY -= 2;
        }
    }

    if(DownKeyIsDown && RunKeyIsDown)
    {
        if(g_Player.ScreenPosY < (GAME_RES_HEIGHT - 17))
        {
            g_Player.ScreenPosY += 2;
        }
    }


    debug_key_was_down = Debug_key_is_down;
    RightKeyWasDown = RightKeyIsDown;
    LeftKeyWasDown = LeftKeyIsDown;

}

void rendergraphics()
{


    HDC DeviceContext = GetDC(g_window_handle);

#ifdef SIMD
    uint32_t color = 0xFF9900FF;
    base_screen(&color);
#else

    base_screen();
#endif

    //const int bytes_per_pixel = BPP / 8; // == 4
    //uint32_t *pixels = (uint32_t *) g_backbuffer.memory_canvas;
    // base_index = screenY * GAME_RES_WIDTH + screenX;

    Load32BppIntoBackBuffer(g_CurrentSprite, g_Player.ScreenPosX, g_Player.ScreenPosY);

    StretchDIBits(DeviceContext, 0, 0, GInfo.monitor_width, GInfo.monitor_height, 0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT, g_backbuffer.memory_canvas, &g_backbuffer.BitMapInfo, DIB_RGB_COLORS, SRCCOPY);

    if(game_performance.DebugModeOn == TRUE)
    {
        char c_DebugBuffer[64] = {0};

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Avg FPS Raw: %.01f", game_performance.AvgFpsRaw);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,0, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Avg FPS Cooked!: %.01f", game_performance.AvgFpsCooked);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,15, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Minimum Timer!: %.01f", (float) game_performance.MinimumTimerResolution / 10000.0f);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,30, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Maximum Timer!: %.01f", (float) game_performance.MaximumTimerResolution / 10000.0f);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,45, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Current Timer!: %.01f",(float) game_performance.CurrentTimerResolution / 10000.0f);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,60, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Handles!: %lu", game_performance.HandleCount);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,75, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Memory currently being used!: %lu KB" , (game_performance.MemoryInfo.PrivateUsage / 1024));
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,90, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Memory currently being used!: %.02f%%" , (game_performance.CpuPercentage));
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,105, c_DebugBuffer, (int) strlen(c_DebugBuffer));
    }




    ReleaseDC(g_window_handle, DeviceContext);
}

#ifdef SIMD
void base_screen(uint32_t *pixel_color)
{
    //FOUR PIXELS SMASHED TOGETHER DOWN HERE! BECAUSE OUR SCREEN RES IS A MULTIPLE OF 4 IT WILL OWRK NICELY

    uint32_t *pixelstest = (uint32_t*) g_backbuffer.memory_canvas;
    const size_t totalPixels = (size_t)GAME_RES_WIDTH * GAME_RES_HEIGHT;

    // A solid magenta-ish color: B=0xFF, G=0x00, R=0x99, A=0xFF
    uint32_t *pixelValue = pixel_color;

    __m128i QuadPixel = _mm_set1_epi32(*pixelValue);

    // Fill 4 pixels at a time

    for (size_t i = 0; i < totalPixels; i += 4) {
        _mm_storeu_si128((__m128i*)(pixelstest + i), QuadPixel);
    }
}
#else
void base_screen()
{

    PIXEL32 Pixel = { 0 };

    Pixel.Blue = 0xff;
    Pixel.Green = 0;
    Pixel.Red = 0xaa;
    Pixel.Alpha = 0xff;

    for(int x = 0 ; x < GAME_RES_WIDTH * GAME_RES_HEIGHT; x++)
    {
        memcpy_s((PIXEL32*)g_backbuffer.memory_canvas + x, sizeof(PIXEL32), &Pixel, sizeof(PIXEL32));
    }

}
#endif

DWORD Load32BppFile(_In_ char * FilePath, _Inout_ GAME_BIT_MAP *GAME_BIT_MAP)
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    DWORD Error = ERROR_SUCCESS;
    WORD BitMapHeader = 0;
    DWORD NumBytesRead = 2;
    DWORD PixelDataOffSet = 0;

    if((FileHandle = CreateFileA(FilePath,GENERIC_READ,FILE_SHARE_READ,NULL,
                                 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))
    == INVALID_HANDLE_VALUE )
    {
        Error = GetLastError();
        goto Exit;
    }

    if(ReadFile(FileHandle, &BitMapHeader, 2, &NumBytesRead, NULL ) == 0 )
    {
        Error = GetLastError();
        goto Exit;
    }

    if(BitMapHeader != 0x4d42) // "If its a valid bitmap file the first two bytes should be 42 4D or BM"
    {
        Error = ERROR_BAD_FILE_TYPE;
        goto Exit;
    }

    if (SetFilePointer(FileHandle, 0xA, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();
        goto Exit;
    }

    if (ReadFile(FileHandle, &PixelDataOffSet, sizeof(DWORD), &NumBytesRead, NULL) == 0)
    {
        Error = GetLastError();
        goto Exit;
    }

    if(SetFilePointer(FileHandle, 0xE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();
        goto Exit;
    }

    if(ReadFile(FileHandle, &GAME_BIT_MAP->BitMapInfo.bmiHeader, sizeof(BITMAPINFOHEADER), &NumBytesRead, NULL) == 0)
    {
        Error = GetLastError();
        goto Exit;
    }

    if ((GAME_BIT_MAP->memory_canvas = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GAME_BIT_MAP->BitMapInfo.bmiHeader.biSizeImage)) == NULL)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    if (SetFilePointer(FileHandle, PixelDataOffSet, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();
        goto Exit;
    }

    if (ReadFile(FileHandle, GAME_BIT_MAP->memory_canvas, GAME_BIT_MAP->BitMapInfo.bmiHeader.biSizeImage, &NumBytesRead, NULL) == 0)
    {
        Error = GetLastError();
        goto Exit;
    }



Exit:

        if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
        {
            CloseHandle(FileHandle);
        }

        return(Error);
}

VOID Load32BppIntoBackBuffer(GAME_BIT_MAP *Sprite, int ScreenX, int ScreenY)
{
    int spriteHeight = abs(Sprite->BitMapInfo.bmiHeader.biHeight);
    int spriteWidth= abs(Sprite->BitMapInfo.bmiHeader.biWidth);
    int gameScreenX = ScreenX;
    int gameScreenY = ScreenY;
    int gameHeight = GAME_RES_HEIGHT;
    int gameWidth = GAME_RES_WIDTH;
    PIXEL32 CurrentSpritePixel = { 0 };
    PIXEL32 *spritePixels = (PIXEL32*)Sprite->memory_canvas;
    PIXEL32 *bufferPixels = (PIXEL32*)g_backbuffer.memory_canvas;

    BOOL bmpIsBottomUp = (Sprite->BitMapInfo.bmiHeader.biHeight > 0);

    for(int y = 0; y < spriteHeight; y++)
    {
        int spriteY = bmpIsBottomUp ? (spriteHeight - 1 - y) : y;
        int dstY = gameScreenY + y;

        if(dstY >= gameHeight)
        {
            continue;
        }
        for(int x = 0; x < spriteWidth; x++)
        {
            int dstX = gameScreenX + x;
            if(dstX >= gameWidth)
            {
                continue;
            }
            int srcIndex = ((spriteY * spriteWidth) + x);
            int destIndex = ((dstY * GAME_RES_WIDTH) + dstX);

            CurrentSpritePixel = spritePixels[srcIndex];
            if(CurrentSpritePixel.Alpha == 0xFF)
            {
                bufferPixels[destIndex] = CurrentSpritePixel;
            }


        }
    }
}

DWORD InitializePlayer(void)
{
    DWORD Error = ERROR_SUCCESS;
    g_Player.ScreenPosX = 25;
    g_Player.ScreenPosY = 25;

    if ((Error = Load32BppFile(SpriteAssets, &g_Player.PlayerSprite[character_sprite_down_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Down_Walk1.bmpx", &g_Player.PlayerSprite[character_sprite_down_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

//if ((Error = Load32BppFile("assets\\Hero_Suit0_Down_Walk2.bmpx", &g_Player.PlayerSprite[character_sprite_down_walk_two])) != ERROR_SUCCESS)
  //  {
  //      MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
   //     goto Exit;
   // }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Left_Standing.bmpx", &g_Player.PlayerSprite[character_sprite_left_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Left_Walk1.bmpx", &g_Player.PlayerSprite[character_sprite_left_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Left_Walk2.bmpx", &g_Player.PlayerSprite[character_sprite_left_walk_two])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Right_Standing.bmpx", &g_Player.PlayerSprite[character_sprite_right_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Right_Walk1.bmpx", &g_Player.PlayerSprite[character_sprite_right_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Right_Walk2.bmpx", &g_Player.PlayerSprite[character_sprite_right_walk_two])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Up_Standing.bmpx", &g_Player.PlayerSprite[character_sprite_up_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Up_Walk1.bmpx", &g_Player.PlayerSprite[character_sprite_up_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Up_Walk2.bmpx", &g_Player.PlayerSprite[character_sprite_up_walk_two])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    Exit:
        return(Error);
}

