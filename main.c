#include <stdio.h>

#pragma warning(push, 0)

#pragma warning(pop)

#include "projectdeclarations.h"
#include "GameOpeningSplashScreen.h"
#include <xaudio2.h>

#include <mmsystem.h>

#include "GameCharacterNamingScreen.h"
#include "GameMainMenuScreen.h"
#include "GameOptionsScreen.h"
#include "GameOverworldScreen.h"
#include "GameYesOrNoExitMenu.h"
#pragma comment(lib, "Winmm.lib")

// Map any char value to an offset dictated by the g6x7Font ordering.
int gFontCharacterPixelOffset[] = {
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
    93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
//     !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
    94,64,87,66,67,68,70,85,72,73,71,77,88,74,91,92,52,53,54,55,56,57,58,59,60,61,86,84,89,75,90,93,
//  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
    65,0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,80,78,81,69,76,
//  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  ..
    62,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,82,79,83,63,93,
//  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
    93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
//  .. .. .. .. .. .. .. .. .. .. .. «  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. »  .. .. .. ..
    93,93,93,93,93,93,93,93,93,93,93,96,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,95,93,93,93,93,
//  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
    93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
//  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. F2 .. .. .. .. .. .. .. .. .. .. .. .. ..
    93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,97,93,93,93,93,93,93,93,93,93,93,93,93,93
};

int gPassableTiles[] = {1};

//Functions
DWORD create_main_window();
DWORD Load32BppFile(_In_ char * FilePath, _Inout_ GAME_BIT_MAP *GAME_BIT_MAP);
DWORD InitializePlayer();
DWORD LoadRegistryParameters(void);
DWORD SaveRegistryParameters(void);

void WriteLog(_In_ DWORD LogLevel, _In_ char* Message, _In_ ...) ;
void processInput();
void rendergraphics();
VOID Load32BppIntoBackBuffer(GAME_BIT_MAP *, int , int );
VOID Load32BppOverworldIntoBackBuffer(GAME_BIT_MAP *, int , int );
DWORD LoadTmxFile( char* , TileMap* );

// windows variables

BOOL GameInProgress = FALSE;
BOOL gWindowHasFocus;
BOOL g_game_is_running;

GAME_BIT_MAP g_backbuffer;
GAME_BIT_MAP g_Game_Font;

GAME_BIT_MAP *g_CurrentSprite;
__m128i data;

HANDLE g_window_handle = 0;

//audio variables

float G_Current_Game_SoundEffect_Volume = 1.0;

float G_Current_Game_Music_Volume = 1.0;

IXAudio2* G_XAudio;

IXAudio2MasteringVoice* G_XAudio_Mastering_Voice;

IXAudio2SourceVoice *G_Game_SoundEffects_Audio[MAX_NUMBER_GAME_SOUND_EFFECTS];

IXAudio2SourceVoice *G_Game_Music_Audio;

uint8_t Game_SoundEffects_Audio_Selector;

typedef HRESULT (__stdcall *PFN_XAudio2Create)(
    IXAudio2 **ppXAudio2,
    UINT32 Flags,
    XAUDIO2_PROCESSOR XAudio2Processor
);

HMODULE G_XAudio2_DLL = NULL;
PFN_XAudio2Create G_pXAudio2Create = NULL;

GAME_SOUND   gMenuNavigate;
GAME_SOUND   gIntroEffect;

//struct variables
GameCoordinate g_CameraPosition = { 0, 0 };
game_states g_CurrentGameState = GAME_OVERWORLD_STATE;
game_performance_info game_performance;
game_info GInfo;
Player g_Player;
game_registry_info g_game_registry_info;
GameLogSeverity G_game_log_severity;
GameOverworldInfo G_game_overworld_info;

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

    if (LoadRegistryParameters() != ERROR_SUCCESS)
    {
        goto EXIT;
    }

    if (InitializeSoundEngine() != S_OK)
    {
        MessageBoxA(NULL, "InitializeSoundEngine failed!", "Error!", MB_ICONERROR | MB_OK);

        goto EXIT;
    }

    if (LoadWavFileFromDisk(".\\Assets\\MenuNavigate.wav", &gMenuNavigate) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWavFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto EXIT;
    }

    if (LoadWavFileFromDisk(".\\Assets\\SplashScreen.wav", &gIntroEffect) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWavFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto EXIT;
    }

    if ((Load32BppFile("assets\\Game_Font_Sprite.bmp", &g_Game_Font) != ERROR_SUCCESS))
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    if ((Load32BppFile("assets\\Overworld01.bmp", &G_game_overworld_info.OverWorldBackGroundSprite) != ERROR_SUCCESS))
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    if(InitializePlayer() != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Failed to initialize hero!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    LoadTmxFile("assets\\Overworld01.tmx", &G_game_overworld_info.TileMap);

    g_CurrentSprite = &g_Player.PlayerSprite[character_sprite_down_standing];

    if (GameInProgress == FALSE) {
        MainGameMenu.SelectedItem = 1;
        MainGameMenu.Items[1]->ItemIsActive = TRUE;
        MainGameMenu.Items[0]->ItemIsActive = FALSE;
    }

    memset(g_backbuffer.memory_canvas,0x00, GAME_AREA_MEMORY_SIZE);

    QueryPerformanceFrequency(&game_performance.TickFrequency);


    GInfo.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(g_window_handle, MONITOR_DEFAULTTOPRIMARY), &GInfo.MonitorInfo) == 0)// this takes 2 input parameters a monitor handle a and monitor data structure our monitor handle is a function call that returns a handle for our monitor
    {
        goto EXIT;
    };

    GInfo.monitor_width = GInfo.MonitorInfo.rcMonitor.right - GInfo.MonitorInfo.rcMonitor.left;
    GInfo.monitor_height =  GInfo.MonitorInfo.rcMonitor.bottom - GInfo.MonitorInfo.rcMonitor.top ;

    for (int counter = 1; counter < 12; counter++)
    {
        if (GAME_RES_WIDTH * counter > GInfo.monitor_width  || GAME_RES_HEIGHT * counter > GInfo.monitor_height) {
            game_performance.MaxGameResScaleFactor = counter - 1;
            break;
        }
    }

    if (g_game_registry_info.CURRENT_GAME_RES_SCALE_FACTOR == 0)
    {
        game_performance.CurrentGameResScaleFactor  = game_performance.MaxGameResScaleFactor;
    }
    else
    {
        game_performance.CurrentGameResScaleFactor = g_game_registry_info.CURRENT_GAME_RES_SCALE_FACTOR;
    }

    if (timeBeginPeriod(1) == TIMERR_NOCANDO)
    {
        MessageBoxA(NULL, "Failed to set global timer resolution!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    SetWindowLongA(g_window_handle, GWL_STYLE, WS_VISIBLE);

    SetWindowPos(
        g_window_handle,
        HWND_TOP,
        GInfo.MonitorInfo.rcMonitor.left,
        GInfo.MonitorInfo.rcMonitor.top,
        GInfo.MonitorInfo.rcMonitor.right - GInfo.MonitorInfo.rcMonitor.left,
        GInfo.MonitorInfo.rcMonitor.bottom - GInfo.MonitorInfo.rcMonitor.top,
        SWP_NOZORDER | SWP_FRAMECHANGED
    );

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
    switch (g_CurrentGameState) {
        case GAME_MAIN_MENU_STATE:
        {
            ProcessGameMainMenuScreenInput();
            break;
        }

        case GAME_YESORNOEXITMENU_STATE:
        {
            ProcessYesOrNoExitMenuInput();
            break;
        }

        case GAME_OPTIONS_STATE:
        {
            ProcessOptionsMenuInput();
            break;
        }

        case GAME_NAME_CHARACTER_STATE:
        {
            ProcessCharacterNamingMenuInput();
            break;
        }

        case GAME_OVERWORLD_STATE: {
            ProcessGameOverWorldScreenInput();
            break;
        }
    }
}

void rendergraphics()
{
    HDC DeviceContext = GetDC(g_window_handle);
    switch (g_CurrentGameState)
    {
        case GAME_INTRO_STATE:
        {
            DrawGameOpeningSplashScreen();
            break;
        }
        case GAME_MAIN_MENU_STATE: {
            GameMainMenuScreen();
            break;
        }
        case GAME_YESORNOEXITMENU_STATE :
        {
            DrawYesOrNoExitMenu();
            break;
        }

        case GAME_OPTIONS_STATE: {
            DrawOptionsScreen();
            break;
        }

        case GAME_NAME_CHARACTER_STATE:
        {
            DrawCharacterNamingScreen();
            break;
        }

        case GAME_OVERWORLD_STATE: {
            DrawGameOverWorldScreen();
            break;
        }
        default:
            int *i;
            *i = 3.0;
    }
    StretchDIBits
    (DeviceContext,
        (GInfo.monitor_width / 2) - ((GAME_RES_WIDTH * game_performance.CurrentGameResScaleFactor) / 2),
        (GInfo.monitor_height / 2) - ((GAME_RES_HEIGHT * game_performance.CurrentGameResScaleFactor) / 2),
        GAME_RES_WIDTH * game_performance.CurrentGameResScaleFactor,
        GAME_RES_HEIGHT * game_performance.CurrentGameResScaleFactor,
        0,
        0,
        GAME_RES_WIDTH,
        GAME_RES_HEIGHT,
        g_backbuffer.memory_canvas,
        &g_backbuffer.BitMapInfo,
        DIB_RGB_COLORS, SRCCOPY);

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

PIXEL32 *Convert24BppInto32Bpp(GAME_BIT_MAP *Sprite)
{
    int spriteHeight = abs(Sprite->BitMapInfo.bmiHeader.biHeight);
    int spriteWidth= abs(Sprite->BitMapInfo.bmiHeader.biWidth);

    PIXEL32 *ConvertedSprite = malloc(sizeof(PIXEL32) * spriteWidth * spriteHeight);

    BYTE *spritePixels = (BYTE*)Sprite->memory_canvas;

    int BytesPerRow = spriteWidth * 3;
    int PaddedBytesPerRow = (4 - (BytesPerRow % 4)) % 4;
    int stride = BytesPerRow + PaddedBytesPerRow;

    BOOL bmpIsBottomUp = (Sprite->BitMapInfo.bmiHeader.biHeight > 0);

    for(int y = 0; y < spriteHeight; y++)
    {
        int spriteY = bmpIsBottomUp ? (spriteHeight - 1 - y) : y;
        BYTE *currentSpriteRow = (spriteY * stride) + spritePixels;
        PIXEL32 *currentConvertedSpriteRow = ConvertedSprite + (spriteY * spriteWidth);

        for(int x = 0; x < spriteWidth; x++)
        {
            currentConvertedSpriteRow[x].Blue = currentSpriteRow[x * 3 + 0];
            currentConvertedSpriteRow[x].Green = currentSpriteRow[x * 3 + 1];
            currentConvertedSpriteRow[x].Red = currentSpriteRow[x * 3 + 2];
            currentConvertedSpriteRow[x].Padding = 0xFF;
        }
    }
    return ConvertedSprite;
}

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

    void *oldPixels = GAME_BIT_MAP->memory_canvas;
    GAME_BIT_MAP->memory_canvas = Convert24BppInto32Bpp(GAME_BIT_MAP);
    HeapFree(GetProcessHeap(), 0, oldPixels);



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
    int PixelVector[3] = { 0 };
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

            BOOL isMagenta = (CurrentSpritePixel.Red == 0xFF && CurrentSpritePixel.Green == 0x00 && CurrentSpritePixel.Blue == 0xFF);

            if(!isMagenta)
            {
                bufferPixels[destIndex] = CurrentSpritePixel;
            }


        }
    }
}

VOID Load32BppOverworldIntoBackBuffer(GAME_BIT_MAP *Sprite, int ScreenX, int ScreenY)
{
    int spriteHeight = abs(Sprite->BitMapInfo.bmiHeader.biHeight);
    int spriteWidth= abs(Sprite->BitMapInfo.bmiHeader.biWidth);
    int gameScreenX = ScreenX;
    int gameScreenY = ScreenY;
    int gameHeight = GAME_RES_HEIGHT;
    int gameWidth = GAME_RES_WIDTH;
    int PixelVector[3] = { 0 };
    PIXEL32 CurrentSpritePixel = { 0 };
    PIXEL32 *spritePixels = (PIXEL32*)Sprite->memory_canvas;
    PIXEL32 *bufferPixels = (PIXEL32*)g_backbuffer.memory_canvas;
    int currentCameraY = g_CameraPosition.Y;
    int currentCameraX = g_CameraPosition.X;

    BOOL bmpIsBottomUp = (Sprite->BitMapInfo.bmiHeader.biHeight > 0);

    for(int y = 0; y < spriteHeight - currentCameraY; y++)
    {
        int spriteY = bmpIsBottomUp ? (spriteHeight - 1 - y - currentCameraY) : y + currentCameraY;

        int dstY = gameScreenY + y;

        if(dstY >= gameHeight)
        {
            continue;
        }
        for(int x = 0; x < spriteWidth - currentCameraX; x++)
        {
            int dstX = gameScreenX + x;
            if(dstX >= gameWidth)
            {
                continue;
            }
            int srcIndex = ((spriteY * spriteWidth) + (currentCameraX + x));
            int destIndex = ((dstY * GAME_RES_WIDTH) + dstX);


            CurrentSpritePixel = spritePixels[srcIndex];

            BOOL isMagenta = (CurrentSpritePixel.Red == 0xFF && CurrentSpritePixel.Green == 0x00 && CurrentSpritePixel.Blue == 0xFF);

            if(!isMagenta)
            {
                bufferPixels[destIndex] = CurrentSpritePixel;
            }
        }
    }
}



DWORD InitializePlayer(void)
{
    DWORD Error = ERROR_SUCCESS;
    g_Player.ScreenPosX = 100;
    g_Player.ScreenPosY = 100;
    g_Player.Direction = character_direction_down;
    g_Player.SpriteIndex = character_animation_standing;
    g_Player.PixelPosition = 0;

    if ((Error = Load32BppFile(SpriteAssets, &g_Player.PlayerSprite[character_sprite_down_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Down_Walk1.bmp", &g_Player.PlayerSprite[character_sprite_down_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

   if ((Error = Load32BppFile("assets\\Hero_Suit0_Down_Walk2.bmp", &g_Player.PlayerSprite[character_sprite_down_walk_two])) != ERROR_SUCCESS)
   {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
       goto Exit;
   }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Left_Standing.bmp", &g_Player.PlayerSprite[character_sprite_left_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Left_Walk1.bmp", &g_Player.PlayerSprite[character_sprite_left_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Left_Walk2.bmp", &g_Player.PlayerSprite[character_sprite_left_walk_two])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Right_Standing.bmp", &g_Player.PlayerSprite[character_sprite_right_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Right_Walk1.bmp", &g_Player.PlayerSprite[character_sprite_right_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Right_Walk2.bmp", &g_Player.PlayerSprite[character_sprite_right_walk_two])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Up_Standing.bmp", &g_Player.PlayerSprite[character_sprite_up_standing])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Up_Walk1.bmp", &g_Player.PlayerSprite[character_sprite_up_walk_one])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((Error = Load32BppFile("assets\\Hero_Suit0_Up_Walk2.bmp", &g_Player.PlayerSprite[character_sprite_up_walk_two])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    Exit:
        return(Error);
}

void BlitStringIntoBuffer(GAME_BIT_MAP *Sprite, int ScreenX, int ScreenY, char Text[], PIXEL32 FontColor)
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
    BOOL IsSpriteInverted = Sprite->BitMapInfo.bmiHeader.biHeight > 0;
    GameCoordinate BitMapSymbolCoordinate = { 0 };

    for (int i = 0; Text[i] != '\0'; i++) {
        BitMapSymbolCoordinate = FindFontSprite(Text[i]);

        for (int y = 0; y < 7; y++)
        {
            int glyphSrcY =  y;
            int dstY = ScreenY + y;
            int srcY = IsSpriteInverted
                ? (spriteHeight - 1 - glyphSrcY)
                : glyphSrcY;

            for (int x = 0; x < 6; x++)
            {
                int dstX = gameScreenX + x;

                if (dstX < 0 || dstX >= GAME_RES_WIDTH ||
                    dstY < 0 || dstY >= GAME_RES_HEIGHT)
                                {
                                    continue;
                                }
                int destIndex = ((dstY * GAME_RES_WIDTH) + dstX);
                int SpriteIndex = srcY * spriteWidth + (BitMapSymbolCoordinate.X + x);

                CurrentSpritePixel = spritePixels[SpriteIndex];

                BOOL isMagenta = (CurrentSpritePixel.Blue == 0xFF && CurrentSpritePixel.Green == 0x00 && CurrentSpritePixel.Red == 0xFF);
                if (!isMagenta)
                {
                    CurrentSpritePixel.Blue =FontColor.Blue;
                    CurrentSpritePixel.Green = FontColor.Green;
                    CurrentSpritePixel.Red = FontColor.Red;
                    bufferPixels[destIndex] = CurrentSpritePixel;
                }
            }
        }
        gameScreenX += 6;
    }
}

GameCoordinate FindFontSprite(char Message)
{
    const int FontXSize = 6;
    const int FontYSize = 7;
    GameCoordinate GameCoordinate;

    int CurrentChar = Message;
    GameCoordinate.X = (FontXSize * gFontCharacterPixelOffset[(uint8_t)CurrentChar]);
    GameCoordinate.Y = FontYSize;
    return(GameCoordinate);
}

DWORD LoadRegistryParameters(void)
{
    DWORD Result = ERROR_SUCCESS;
    HKEY RegKey = NULL;
    DWORD dwDisposition = 0;
    DWORD RegBytesRead = sizeof(DWORD);
    DWORD RegSubResult = ERROR_SUCCESS;
    HKEY RegSubKey = NULL;

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\GAME_B" , 0, NULL, 0,
                   KEY_ALL_ACCESS, NULL, &RegKey, &dwDisposition);

    G_game_log_severity.LogLevel = log_severity_debug;
    if (Result != ERROR_SUCCESS)
    {
        WriteLog(log_severity_error, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (dwDisposition  == REG_CREATED_NEW_KEY)
    {
        WriteLog(log_severity_info, "The Registry Key for the game didnt exist! created a new registry key", __FUNCTION__);
        RegSubResult = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\GAME_B\\LogLevel" , 0, NULL, 0,
                   KEY_ALL_ACCESS, NULL, &RegSubKey, &dwDisposition);

        if (RegSubResult != ERROR_SUCCESS)
        {
            WriteLog(log_severity_error, "[%s] The Sub Key Creation for the Registry of Game B Failed!", __FUNCTION__, Result);
        }
        else
        {
            WriteLog(log_severity_info, "The Registry Sub Key for the game didnt exist! created a new registry key", __FUNCTION__);
            DWORD DefaultLogLevel = log_severity_error ;
            RegSetValueExA( RegSubKey, "LogLevel", 0, REG_DWORD, (const BYTE*)&DefaultLogLevel, sizeof(REG_DWORD));
        }
    }
    else
    {
        WriteLog(log_severity_info, "The Registry Key for the game already exists!", __FUNCTION__);
    }

    Result = RegGetValueA(RegKey, "LogLevel" , "LogLevel", RRF_RT_DWORD, NULL, (BYTE*)&G_game_log_severity.LogLevel, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;
            WriteLog(log_severity_info, "The Registry Sub Key for the game didnt exist! created a new registry key", __FUNCTION__);
            DWORD DefaultLogLevel = log_severity_error ;
            RegSetValueExA( RegSubKey, "LogLevel", 0, REG_DWORD, (const BYTE*)&DefaultLogLevel, sizeof(REG_DWORD));
            G_game_log_severity.LogLevel = log_severity_error;
        }
        else
        {
            WriteLog(log_severity_error, "[%s] Failed to read the 'LogLevel' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    WriteLog(log_severity_info, "[%s] LogLevel is %d.", __FUNCTION__, G_game_log_severity.LogLevel);

    RegBytesRead = sizeof(DWORD);
    Result =
        RegGetValueA(RegKey, NULL , "SoundEffectsVolume", RRF_RT_DWORD, NULL, (BYTE*)&g_game_registry_info.GAME_SOUND_EFFECTS_VOLUME_LEVEL, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;
            WriteLog(log_severity_info, "The Registry Key for the game sound effect doesnt exist using a default value of 0.5!", __FUNCTION__);

            g_game_registry_info.GAME_SOUND_EFFECTS_VOLUME_LEVEL = 50;
            G_Current_Game_SoundEffect_Volume = (float) g_game_registry_info.GAME_SOUND_EFFECTS_VOLUME_LEVEL/100;
        }
        else
        {
            WriteLog(log_severity_error, "[%s] Failed to read the game sound effect' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }
    else
    {
        G_Current_Game_SoundEffect_Volume = (float)g_game_registry_info.GAME_SOUND_EFFECTS_VOLUME_LEVEL / 10;
        WriteLog(log_severity_info, "[%s] SoundEffectsVolume raw=%lu, applied=%.2f",
         __FUNCTION__,
         g_game_registry_info.GAME_SOUND_EFFECTS_VOLUME_LEVEL,
         G_Current_Game_SoundEffect_Volume);

    }

    RegBytesRead = sizeof(DWORD);
    Result =
      RegGetValueA(RegKey, NULL , "MusicVolume", RRF_RT_DWORD, NULL, (BYTE*)&g_game_registry_info.GAME_MUSIC_VOLUME_LEVEL, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;
            WriteLog(log_severity_info, "The Registry Key for the game music volume doesnt exist using a default value of 0.5!", __FUNCTION__);

            g_game_registry_info.GAME_MUSIC_VOLUME_LEVEL = 50;
            G_Current_Game_Music_Volume = (float) g_game_registry_info.GAME_MUSIC_VOLUME_LEVEL/100;
        }
        else
        {
            WriteLog(log_severity_error, "[%s] Failed to read the game music volume registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }
    else
    {
        G_Current_Game_Music_Volume = (float)g_game_registry_info.GAME_MUSIC_VOLUME_LEVEL / 10;
        WriteLog(log_severity_info, "[%s] MusicVolume raw=%lu, applied=%.2f",
         __FUNCTION__,
         g_game_registry_info.GAME_MUSIC_VOLUME_LEVEL,
         G_Current_Game_Music_Volume);
    }

    RegBytesRead = sizeof(DWORD);
    Result =
      RegGetValueA(RegKey, NULL , "ScaleFactor", RRF_RT_DWORD, NULL, (BYTE*)&g_game_registry_info.CURRENT_GAME_RES_SCALE_FACTOR, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;
            WriteLog(log_severity_info, "The Registry Key for the game window width doesnt exist using a default value of 0!", __FUNCTION__);

            g_game_registry_info.CURRENT_GAME_RES_SCALE_FACTOR = 0;
        }
        else
        {
            WriteLog(log_severity_error, "[%s] Failed to read the game window width registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }
    else
    {
        game_performance.CurrentGameResScaleFactor = g_game_registry_info.CURRENT_GAME_RES_SCALE_FACTOR;
    }


    Exit:

    if (RegKey)
    {
        RegCloseKey(RegKey);
        RegCloseKey(RegSubKey);
    }

    return(Result);
}

DWORD SaveRegistryParameters(void)
{
    DWORD Result = ERROR_SUCCESS;
    HKEY RegKey = NULL;
    DWORD dwDisposition = 0;
    DWORD RegBytesRead = sizeof(DWORD);
    DWORD RegSubResult = ERROR_SUCCESS;
    HKEY RegSubKey = NULL;
    DWORD RegistrySoundEffectsVolume = G_Current_Game_SoundEffect_Volume * 10;
    DWORD RegistryMusicVolume = G_Current_Game_Music_Volume * 10;
    DWORD RegistryScaleFactor = game_performance.CurrentGameResScaleFactor;

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\GAME_B" , 0, NULL, 0,
                   KEY_ALL_ACCESS, NULL, &RegKey, &dwDisposition);

    G_game_log_severity.LogLevel = log_severity_debug;
    if (Result != ERROR_SUCCESS)
    {
        WriteLog(log_severity_error, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    Result = RegSetValueExA(RegKey, "SoundEffectsVolume",0 ,REG_DWORD,(BYTE*) &RegistrySoundEffectsVolume, sizeof(DWORD));
    if (Result != ERROR_SUCCESS)
    {
        WriteLog(log_severity_error, "[%s] Failed to save SoundEffectsVolume!", __FUNCTION__, Result);
        goto Exit;
    }

    Result = RegSetValueExA(RegKey, "MusicVolume",0 ,REG_DWORD,(BYTE*) &RegistryMusicVolume, sizeof(DWORD));
    if (Result != ERROR_SUCCESS)
    {
        WriteLog(log_severity_error, "[%s] Failed to save MusicVolume!", __FUNCTION__, Result);
        goto Exit;
    }

    Result = RegSetValueExA(RegKey, "ScaleFactor",0 ,REG_DWORD,(BYTE*) &RegistryScaleFactor, sizeof(DWORD));
    if (Result != ERROR_SUCCESS)
    {
        WriteLog(log_severity_error, "[%s] Failed to save ScaleFactor!", __FUNCTION__, Result);
        goto Exit;
    }

    Exit:
    if (RegKey)
    {
        RegCloseKey(RegKey);
        RegCloseKey(RegSubKey);
    }

    return(Result);
}

void WriteLog(_In_ DWORD LogLevel, _In_ char* Message, _In_ ...) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD NumberOfBytesWritten = 0;
    char LogMessageSeverityPrefix[8] = { 0 };
    char LogTimeStamp[24];
    char LogMessage[4096] = { 0 };
    BYTE utf8Bom[] = { 0xEF, 0xBB, 0xBF };
    SYSTEMTIME SystemTime;

    if (G_game_log_severity.LogLevel < LogLevel)
    {
        return;
    }

    switch (LogLevel)
    {
        case log_severity_none:
            strcpy_s(LogMessageSeverityPrefix, sizeof(LogMessageSeverityPrefix), "=NONE= ");
            break;

        case log_severity_info:
            strcpy_s(LogMessageSeverityPrefix, sizeof(LogMessageSeverityPrefix), "=INFO= ");
            break;

        case log_severity_warning:
            strcpy_s(LogMessageSeverityPrefix, sizeof(LogMessageSeverityPrefix), "=WARN= ");
            break;

        case log_severity_error:
            strcpy_s(LogMessageSeverityPrefix, sizeof(LogMessageSeverityPrefix), "=ERROR= ");
            break;

        case log_severity_debug:
            strcpy_s(LogMessageSeverityPrefix, sizeof(LogMessageSeverityPrefix), "=DEBUG= ");
            break;

        default:
            return;
    }

    GetSystemTime(&SystemTime);

    _snprintf_s(LogTimeStamp, sizeof(LogTimeStamp), _TRUNCATE, "[%02u/%02u/%u %02u:%02u:%02u", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);

    va_list ArgPointer;

    va_start(ArgPointer, Message);

    _vsnprintf_s(LogMessage, sizeof(LogMessage), _TRUNCATE, Message, ArgPointer);

    va_end(ArgPointer);


    if ((hFile = CreateFileA("LOG_GAMEB.log", FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {

        return;
    }

    WriteFile(hFile, LogTimeStamp, strlen(LogTimeStamp), &NumberOfBytesWritten, NULL);
    WriteFile(hFile, LogMessageSeverityPrefix, strlen(LogMessageSeverityPrefix), &NumberOfBytesWritten, NULL);
    WriteFile(hFile, LogMessage, strlen(LogMessage), &NumberOfBytesWritten, NULL);
    WriteFile(hFile, LogMessageSeverityPrefix, strlen(LogMessageSeverityPrefix), &NumberOfBytesWritten, NULL);
    WriteFile(hFile, "\r\n", 2, &NumberOfBytesWritten, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
}

HRESULT InitializeSoundEngine(void)
{
    HRESULT Result = S_OK;

    WAVEFORMATEX SoundEffectWave = { 0 };
    WAVEFORMATEX MusicWave = { 0 };

    Result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (Result != S_OK)
    {
        WriteLog(log_severity_info, "Call to CoInitializeEx Failed!", __FUNCTION__);
        goto Exit;
    }

    G_XAudio2_DLL = LoadLibraryA("XAudio2_9.dll");
    if (G_XAudio2_DLL == NULL)
    {
        Result = E_FAIL;
        WriteLog(log_severity_info, "LoadLibrary Failed!", __FUNCTION__);
        goto Exit;
    }

    G_pXAudio2Create = (PFN_XAudio2Create)GetProcAddress(G_XAudio2_DLL, "XAudio2Create");
    if (G_pXAudio2Create == NULL)
    {
        Result = E_FAIL;
        WriteLog(log_severity_info, "GetProcAddress Failed!", __FUNCTION__);
        goto Exit;
    }

    Result =  G_pXAudio2Create(&G_XAudio, 0, XAUDIO2_ANY_PROCESSOR);

    if (FAILED(Result))
    {
        WriteLog(log_severity_info,  "Call to XAuido2Create Failed!", __FUNCTION__, Result);
        goto Exit;
    }

    Result = G_XAudio->lpVtbl->CreateMasteringVoice(G_XAudio, &G_XAudio_Mastering_Voice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL, 0);

    if (FAILED(Result))
    {
        WriteLog(log_severity_info,  "Call to CreateMasteringVoice Failed!", __FUNCTION__, Result);

        goto Exit;
    }
    // initializing SoundEffects
    SoundEffectWave.wFormatTag = WAVE_FORMAT_PCM;

    SoundEffectWave.nChannels = 1;

    SoundEffectWave.nSamplesPerSec = 44100;

    SoundEffectWave.nAvgBytesPerSec = SoundEffectWave.nSamplesPerSec * SoundEffectWave.nChannels * 2;

    SoundEffectWave.nBlockAlign = SoundEffectWave.nChannels * 2;

    SoundEffectWave.wBitsPerSample = 16;

    SoundEffectWave.cbSize = 0x6164;

    for (int CurrentSoundEffect = 0; CurrentSoundEffect < MAX_NUMBER_GAME_SOUND_EFFECTS; CurrentSoundEffect++)
    {
        //initializing the soundeffect at index CurrentSoundEffect
        Result = G_XAudio->lpVtbl->CreateSourceVoice(G_XAudio, &G_Game_SoundEffects_Audio[CurrentSoundEffect], &SoundEffectWave, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);
        if (FAILED(Result))
        {
            WriteLog(log_severity_info,  "The initialization of G_Game_SoundEffects_Audio failed!", __FUNCTION__, Result);

            goto Exit;
        }
        //after initailizing it set the volume
        G_Game_SoundEffects_Audio[CurrentSoundEffect]->lpVtbl->SetVolume(G_Game_SoundEffects_Audio[CurrentSoundEffect], G_Current_Game_SoundEffect_Volume, XAUDIO2_COMMIT_NOW);
    }
    //Initializing GameMusic

    MusicWave.wFormatTag = WAVE_FORMAT_PCM;

    MusicWave.nChannels = 2;

    MusicWave.nSamplesPerSec = 44100;

    MusicWave.nAvgBytesPerSec = MusicWave.nSamplesPerSec * MusicWave.nChannels * 2;

    MusicWave.nBlockAlign = MusicWave.nChannels * 2;

    MusicWave.wBitsPerSample = 16;

    MusicWave.cbSize = 0;

    Result = G_XAudio->lpVtbl->CreateSourceVoice(G_XAudio, &G_Game_Music_Audio, &MusicWave, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);

    if (FAILED(Result))
    {
        WriteLog(log_severity_info,  "The initialization of G_Game_Music_Audio failed!", __FUNCTION__, Result);

        goto Exit;
    }

    G_Game_Music_Audio->lpVtbl->SetVolume(G_Game_Music_Audio, G_Current_Game_Music_Volume, XAUDIO2_COMMIT_NOW);

    Exit:
    return Result;
}

DWORD LoadWavFileFromDisk(_In_ char* WavFileName, _Inout_ GAME_SOUND* game_sound)
{
    DWORD Result = ERROR_SUCCESS;

    DWORD NumberOfBytesRead = 0;

    DWORD FileHeaderValidation = 0;

    uint16_t DataChunkOffset = 0;

    DWORD DataChunkSearcher = 0;

    BOOL DataChunkFound = FALSE;

    DWORD DataChunkSize = 0;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    FileHandle = CreateFileA(WavFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
    if ( FileHandle == INVALID_HANDLE_VALUE ) {
        WriteLog(log_severity_info,  "Failed to open the WAV file handle", __FUNCTION__, Result);
        goto EXIT;
    }

    ReadFile(FileHandle, &FileHeaderValidation, sizeof(DWORD), &NumberOfBytesRead, NULL);

    if (FileHeaderValidation != 0x46464952) // check if the FileHeaderFormat is "RIFF" in little edian
    {
        Result = ERROR_FILE_INVALID;
        WriteLog(log_severity_info,  "Invalid WAV file ", __FUNCTION__, Result);
        goto EXIT;
    }

    if (SetFilePointer(FileHandle, 20, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Result = GetLastError();
        WriteLog(log_severity_info, "[%s] SetFilePointer failed with 0x%08lx!", __FUNCTION__, Result);
        goto EXIT;
    }

    if (ReadFile(FileHandle, &game_sound->WaveFormat, sizeof(WAVEFORMATEX), &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();
        WriteLog(log_severity_info, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Result);
        goto EXIT;
    }

    if (game_sound->WaveFormat.nBlockAlign != ((game_sound->WaveFormat.nChannels * game_sound->WaveFormat.wBitsPerSample) / 8) ||
        (game_sound->WaveFormat.wFormatTag != WAVE_FORMAT_PCM) ||
        (game_sound->WaveFormat.wBitsPerSample != 16))
    {
        Result = ERROR_DATATYPE_MISMATCH;

        WriteLog(log_severity_info, "[%s] This wav file did not meet the format requirements! Only PCM format, 44.1KHz, 16 bits per sample wav files are supported. 0x%08lx!", __FUNCTION__, Result);

        goto EXIT;
    }

    while (DataChunkFound == FALSE)
    {
        if (SetFilePointer(FileHandle, DataChunkOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        {
            Result = GetLastError();

            WriteLog(log_severity_info, "[%s] SetFilePointer failed with 0x%08lx!", __FUNCTION__, Result);

            goto EXIT;
        }

        if (ReadFile(FileHandle, &DataChunkSearcher, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
        {
            Result = GetLastError();

            WriteLog(log_severity_info, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Result);

            goto EXIT;
        }

        if (DataChunkSearcher == 0x61746164) // 'data', backwards
        {
            DataChunkFound = TRUE;

            break;
        }
        else
        {
            DataChunkOffset += 4;
        }

        if (DataChunkOffset > 256)
        {
            Result = ERROR_DATATYPE_MISMATCH;

            WriteLog(log_severity_info, "[%s] Data chunk not found within first 256 bytes of this file! 0x%08lx!", __FUNCTION__, Result);

            goto EXIT;
        }
    }

    if (SetFilePointer(FileHandle, DataChunkOffset + 4, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Result = GetLastError();
        WriteLog(log_severity_info, "[%s] SetFilePointer failed with 0x%08lx!", __FUNCTION__, Result);
        goto EXIT;
    };

    if (ReadFile(FileHandle, &DataChunkSize, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();
        WriteLog(log_severity_info, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Result);
        goto EXIT;
    }

    game_sound->Buffer.pAudioData = HeapAlloc(GetProcessHeap(), 0, DataChunkSize);

    if (game_sound->Buffer.pAudioData == NULL) {
        Result = ERROR_NOT_ENOUGH_MEMORY;
        WriteLog(log_severity_info, "Failed to allocate memory for the WAV file", __FUNCTION__, Result);
        goto EXIT;
    }

    game_sound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    game_sound->Buffer.AudioBytes = DataChunkSize;

    if (SetFilePointer(FileHandle, DataChunkOffset + 8, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Result = GetLastError();
        WriteLog(log_severity_info, "Failed to reach the Sound Bytes of the WAV file", __FUNCTION__, Result);
        goto EXIT;
    }

    if (ReadFile(FileHandle, game_sound->Buffer.pAudioData, DataChunkSize, &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();
        WriteLog(log_severity_info, "Failed to Write Sound bytes", __FUNCTION__, Result);
        goto EXIT;
    }

    EXIT:
        if (Result == ERROR_SUCCESS)
        {
            WriteLog(log_severity_info, "[%s] Successfully loaded %s.", __FUNCTION__, WavFileName);
        }
        else
        {
            WriteLog(log_severity_info, "[%s] Failed to load %s! Error: 0x%08lx!", __FUNCTION__, WavFileName, Result);
        }

    if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
    {
        CloseHandle(FileHandle);
    }

    return(Result);
}

void PlayGameSound(_In_ GAME_SOUND* GameSound)
{
    G_Game_SoundEffects_Audio[Game_SoundEffects_Audio_Selector]->lpVtbl->SubmitSourceBuffer(G_Game_SoundEffects_Audio[Game_SoundEffects_Audio_Selector], &GameSound->Buffer, NULL);

    G_Game_SoundEffects_Audio[Game_SoundEffects_Audio_Selector]->lpVtbl->Start(G_Game_SoundEffects_Audio[Game_SoundEffects_Audio_Selector], 0, XAUDIO2_COMMIT_NOW);

    Game_SoundEffects_Audio_Selector++;

    if (Game_SoundEffects_Audio_Selector > (MAX_NUMBER_GAME_SOUND_EFFECTS - 1))
    {
        Game_SoundEffects_Audio_Selector = 0;
    }
}

void g_mi_ResumeGameAction(void){};
void g_mi_StartGameAction(void) {
    g_CurrentGameState = GAME_NAME_CHARACTER_STATE;
};
void g_mi_SaveGameAction(void){};
void g_mi_OptionGameAction(void)
{
    g_CurrentGameState = GAME_OPTIONS_STATE;
};

void g_mi_CloseGameAction(void)
{
    g_CurrentGameState = GAME_YESORNOEXITMENU_STATE;
};

void g_mi_ExitGameAction(void)
{
    SendMessageA(g_window_handle, WM_CLOSE, 0, 0);
};
void g_mi_DontExitGameAction(void)
{
    g_CurrentGameState = GAME_MAIN_MENU_STATE;
};

void g_mi_OptionsSoundLevelAction(void)
{
    float f_CurrentSoundVolumeLevel = G_Current_Game_SoundEffect_Volume;
    if (f_CurrentSoundVolumeLevel + 0.1 > 1) {
        f_CurrentSoundVolumeLevel = 0;
    }
    else {
        f_CurrentSoundVolumeLevel += 0.1;
    }

    for (uint8_t CurrentVoice = 0; CurrentVoice < MAX_NUMBER_GAME_SOUND_EFFECTS; CurrentVoice++)
    {
        G_Game_SoundEffects_Audio[CurrentVoice]->lpVtbl->SetVolume(G_Game_SoundEffects_Audio[CurrentVoice], f_CurrentSoundVolumeLevel, XAUDIO2_COMMIT_NOW);
    }
    G_Current_Game_SoundEffect_Volume = f_CurrentSoundVolumeLevel;
};

void g_mi_MusicSoundLevelAction(void)
{
    float f_CurrentSoundVolumeLevel = G_Current_Game_Music_Volume;
    if (f_CurrentSoundVolumeLevel + 0.1 > 1) {
        f_CurrentSoundVolumeLevel = 0;
    }
    else {
        f_CurrentSoundVolumeLevel += 0.1;
    }

    for (uint8_t CurrentVoice = 0; CurrentVoice < MAX_NUMBER_GAME_SOUND_EFFECTS; CurrentVoice++)
    {
        G_Game_Music_Audio->lpVtbl->SetVolume(G_Game_Music_Audio, f_CurrentSoundVolumeLevel, XAUDIO2_COMMIT_NOW);
    }
    G_Current_Game_Music_Volume = f_CurrentSoundVolumeLevel;
};

void g_mi_ScreenResolutionAction(void) {
    if (game_performance.CurrentGameResScaleFactor < game_performance.MaxGameResScaleFactor)
    {
        game_performance.CurrentGameResScaleFactor++;
    }
    else
    {
        game_performance.CurrentGameResScaleFactor = 1;
    }
    InvalidateRect(g_window_handle, NULL, TRUE);
};

void g_mi_OptionsBackAction(void)
{
    g_CurrentGameState = GAME_MAIN_MENU_STATE;
    if (SaveRegistryParameters() != ERROR_SUCCESS)
    {
        WriteLog(log_severity_error, "[%s] Failed to save Options", __FUNCTION__);
    }
};

void g_mi_CharacterNameBack(void)
{
    int CurrentNameChars = strlen(g_Player.name);
    if (CurrentNameChars > 0)
    {
        g_Player.name[CurrentNameChars - 1] = '\0';
    }
    else
    {
        g_CurrentGameState = GAME_MAIN_MENU_STATE;
    }

}

void g_mi_CharacterNameAddChar(void)
{
    int currentNameChar = strlen(g_Player.name);

    if (currentNameChar < 12) {
        strncpy(&g_Player.name[currentNameChar], g_mi_CharacterNamingScreenMenu.Items[g_mi_CharacterNamingScreenMenu.SelectedItem]->ItemTitle, 1);
    }
}

void g_mi_CharacterNameConfirmName(void) {
    if (strlen(g_Player.name) > 0)
    {
        g_CurrentGameState = GAME_OVERWORLD_STATE;
    }
}
DWORD LoadTmxFile(_In_ char* TmxFilePath,_Inout_ TileMap *TileMap)
{
    DWORD Result = ERROR_SUCCESS;

    DWORD NumberOfBytesRead = 0;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    void* FileBuffer = NULL;

    LARGE_INTEGER FileSize = { 0 };

    char* FileCursor;

    char *AuxNumberBuffer;

    int TileMapWidth = 0;

    int TileMapHeight = 0;

    int *TileBuffer;

    FileHandle = CreateFileA(TmxFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
    if ( FileHandle == INVALID_HANDLE_VALUE ) {
        WriteLog(log_severity_info,  "Failed to open the TMX file handle", __FUNCTION__, Result);
        goto EXIT;
    }

    GetFileSizeEx(FileHandle,&FileSize);

    if (FileSize.QuadPart == 0) {
        WriteLog(log_severity_info,  "Failed to extract the tmx file size", __FUNCTION__, Result);
        goto EXIT;
    }

    FileBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, FileSize.QuadPart);

    if (FileBuffer == NULL)
    {
        Result = ERROR_OUTOFMEMORY;

        WriteLog(log_severity_info, "[%s] HeapAlloc failed with 0x%08lx!", __FUNCTION__, Result);

        goto EXIT;
    }

    if (ReadFile(FileHandle, FileBuffer, (DWORD) FileSize.QuadPart, &NumberOfBytesRead, NULL) == 0) {
        WriteLog(log_severity_info,  "Failed to read the TMX FILE", __FUNCTION__, Result);
        goto EXIT;
    }


    FileCursor = strstr(FileBuffer, "width=");

    if (FileCursor == NULL)
    {
        WriteLog(log_severity_info,  "the TMX FILE doesnt have a width= parameter in its header", __FUNCTION__, Result);
        goto EXIT;
    }

    FileCursor += strlen("width="); // moves to the index after the width=

    for (int FindOpeningQuoute = 0; FindOpeningQuoute < 8; FindOpeningQuoute++)
    {

        if (FileCursor[FindOpeningQuoute] == '\"')
        {
            int QuoteIndex = FindOpeningQuoute;
            QuoteIndex++;
            FileCursor += QuoteIndex;
            break;
        }
    }

    AuxNumberBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CHAR) * 3);
    // locating the height we are assuming the game wont have more than triple digit roles
    for (int FindOpeningQuoute = 0; FindOpeningQuoute < 3; FindOpeningQuoute++)
    {
        if (*FileCursor == '\"')
        {
            break;
        }
        AuxNumberBuffer[FindOpeningQuoute] = *FileCursor;
        FileCursor++;
    }

    TileMapWidth = atoi(AuxNumberBuffer);
    TileMap->TileMapWidth = TileMapWidth;


    FileCursor = strstr(FileBuffer, "height=");

    FileCursor += strlen("height="); // moves to the index after the height=

    for (int FindOpeningQuoute = 0; FindOpeningQuoute < 8; FindOpeningQuoute++)
    {
        if (FileCursor[FindOpeningQuoute] == '\"')
        {
            int QuoteIndex = FindOpeningQuoute;
            QuoteIndex++;
            FileCursor += QuoteIndex;
            break;
        }
    }

    for (int FindOpeningQuoute = 0; FindOpeningQuoute < 3; FindOpeningQuoute++)
    {
        if (*FileCursor == '\"')
        {
            break;
        }
        AuxNumberBuffer[FindOpeningQuoute] = *FileCursor;
        FileCursor++;
    }

    TileMapHeight = atoi(AuxNumberBuffer);
    TileMap->TileMapHeight = TileMapHeight;

    TileBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int) * TileMapHeight * TileMapWidth);

    if (TileBuffer == NULL) {
        Result = ERROR_OUTOFMEMORY;
        WriteLog(log_severity_info, "[%s] failed to allocate the TileBuffer %s.", __FUNCTION__, TmxFilePath);
        goto EXIT;
    }

    FileCursor = strstr(FileBuffer, "csv");

    FileCursor += strlen("csv\">\n");

    for ( int row = 0; row < TileMapHeight; row++) {
        for ( int column = 0; column < TileMapWidth; column++)
        {
            char *End;
            TileBuffer[row * TileMapWidth + column] = (int)strtol(FileCursor, &End, 10);
            FileCursor = End;
            if (*FileCursor == ',')
            {
                FileCursor++;
            }
            else
            {
                break;
            }
        }
    }

    TileMap->Map = TileBuffer;

    EXIT:
        if (Result == ERROR_SUCCESS)
        {
            WriteLog(log_severity_info, "[%s] Successfully loaded %s.", __FUNCTION__, TmxFilePath);
        }
        else
        {
            WriteLog(log_severity_info, "[%s] Failed to load %s! Error: 0x%08lx!", __FUNCTION__, TmxFilePath, Result);
        }

    if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
    {
        CloseHandle(FileHandle);
    }

    return(Result);
}