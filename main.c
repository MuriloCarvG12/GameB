#include <stdio.h>

#pragma warning(push, 0)

#include <windows.h>
#include <stdint.h>
#include <emmintrin.h>


#pragma warning(pop)
#include "projectdeclarations.h"
GAME_BIT_MAP g_backbuffer;
game_performance_info game_performance;

BOOL g_game_is_running;
HANDLE g_window_handle = 0;
DWORD create_main_window();
void processInput();
void rendergraphics();
__m128i data;
game_info GInfo;



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

    memset(g_backbuffer.memory_canvas,0x00, GAME_AREA_MEMORY_SIZE);

    QueryPerformanceFrequency(&game_performance.TickFrequency );


    GInfo.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(g_window_handle, MONITOR_DEFAULTTOPRIMARY), &GInfo.MonitorInfo) == 0)// this takes 2 input parameters a monitor handle a and monitor data structure our monitor handle is a function call that returns a handle for our monitor
    {

        goto EXIT;
    };

    GInfo.monitor_width = GInfo.MonitorInfo.rcMonitor.right - GInfo.MonitorInfo.rcMonitor.left;
    GInfo.monitor_height =  GInfo.MonitorInfo.rcMonitor.bottom - GInfo.MonitorInfo.rcMonitor.top ;
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

        while (ElapsedFrameTime <= TARGET_MICROSECONDS_PER_FRAME)
        {
            QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);
            ElapsedFrameTime = FrameEnd - FrameStart;
            ElapsedFrameTime *= 1000000;
            ElapsedFrameTime /= game_performance.TickFrequency;
            if (ElapsedFrameTime <= ((int64_t)TARGET_MICROSECONDS_PER_FRAME - game_performance.CurrentTimerResolution))
            {
                Sleep(1); // Could be anywhere from 1ms to a full system timer tick? (~15.625ms)
            }
        }

        i64_ElapsedMicrosecondsPerFrameAccumulatorCooked += ElapsedFrameTime;

        if(game_performance.TotalFramesRendered % CALCULATE_FPS_EVERY_X_FRAMES == 0)
        {

            int64_t i64_AverageMicrosecondsPerFrameRaw = i64_ElapsedMicrosecondsPerFrameAccumulatorRaw / CALCULATE_FPS_EVERY_X_FRAMES;

            int64_t i64_AverageMicrosecondsPerFrameCooked = i64_ElapsedMicrosecondsPerFrameAccumulatorCooked / CALCULATE_FPS_EVERY_X_FRAMES;

            game_performance.AvgFpsRaw = 1.0f / ((i64_ElapsedMicrosecondsPerFrameAccumulatorRaw / 60) * 0.000001f);
            game_performance.AvgFpsCooked = 1.0f / ((i64_ElapsedMicrosecondsPerFrameAccumulatorCooked / 60) * 0.000001f);


            printf("Avg milliseconds/frame raw: %lu Avg FPS Cooked: %.01f Avg FPS Raw: %.01f\n",
                   i64_AverageMicrosecondsPerFrameRaw,
                   game_performance.AvgFpsCooked,
                   game_performance.AvgFpsRaw);


            i64_ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
            i64_ElapsedMicrosecondsPerFrameAccumulatorCooked = 0;
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
    short W_Key_is_down = GetAsyncKeyState('W');
    short Debug_key_is_down = GetAsyncKeyState(VK_F12);
    static short debug_key_was_down;

    if(Esc_Key_is_down)
    {
        SendMessageA(g_window_handle, WM_CLOSE, 0, 0);
    }

    if(W_Key_is_down )
    {
        MessageBox(NULL, "Going Foward", "Error", MB_ICONEXCLAMATION);
    }

    if(Debug_key_is_down && !debug_key_was_down)
    {
        game_performance.DebugModeOn = !game_performance.DebugModeOn ;
    }

    debug_key_was_down = Debug_key_is_down;

}

void rendergraphics()
{


    HDC DeviceContext = GetDC(g_window_handle);

#ifdef SIMD
    base_screen(0xFF9900FFu);
#else
    PIXEL32 Pixel = {0xFF, 0x99 , 0x00, 0xFF};
    base_screen(&Pixel)
#endif

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

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Minimum Timer!: %.01f", game_performance.MinimumTimerResolution / 10000.0f);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,30, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Maximum Timer!: %.01f", game_performance.MaximumTimerResolution / 10000.0f);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,45, c_DebugBuffer, (int) strlen(c_DebugBuffer));

        sprintf_s(c_DebugBuffer, sizeof(c_DebugBuffer), "Current Timer!: %.01f", game_performance.CurrentTimerResolution / 10000.0f);
        //this TextOut function prints stuff onto our program!
        TextOutA(DeviceContext, 0,60, c_DebugBuffer, (int) strlen(c_DebugBuffer));

    }




    ReleaseDC(g_window_handle, DeviceContext);
}

#ifdef SIMD
void base_screen(uint32_t pixel_color)
{
    //FOUR PIXELS SMASHED TOGETHER DOWN HERE! BECAUSE OUR SCREEN RES IS A MULTIPLE OF 4 IT WILL OWRK NICELY

    uint32_t *pixelstest = (uint32_t*) g_backbuffer.memory_canvas;
    const size_t totalPixels = (size_t)GAME_RES_WIDTH * GAME_RES_HEIGHT;

    // A solid magenta-ish color: B=0xFF, G=0x00, R=0x99, A=0xFF
    uint32_t pixelValue = pixel_color;

    __m128i QuadPixel = _mm_set1_epi32((int)pixelValue);

    // Fill 4 pixels at a time

    for (size_t i = 0; i < totalPixels; i += 4) {
        _mm_storeu_si128((__m128i*)(pixelstest + i), QuadPixel);
    }
}
#else
void base_screen(PIXEL32* Pixel)
{

   for (int x = 0; x < GAME_RES_WIDTH * GAME_RES_HEIGHT; x++)
    {
        memcpy((PIXEL32*)g_backbuffer.memory_canvas + x, Pixel, sizeof(PIXEL32));
    }

}
#endif