#include <stdio.h>

#pragma warning(push, 0)

#pragma warning(pop)

#include "projectdeclarations.h"
#include "menu.h"
#include <xaudio2.h>

#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")


GAME_BIT_MAP g_backbuffer;
game_performance_info game_performance;
GAME_BIT_MAP g_Game_Font;
BOOL g_game_is_running;
HANDLE g_window_handle = 0;
DWORD create_main_window();
void processInput();
void rendergraphics();
DWORD Load32BppFile(_In_ char * FilePath, _Inout_ GAME_BIT_MAP *GAME_BIT_MAP);
DWORD InitializePlayer();
GameCoordinate FindFontSprite(char Message);
void BlitStringIntoBuffer(GAME_BIT_MAP *Sprite, int ScreenX, int ScreenY, char *Text, PIXEL32 FontColor);
VOID Load32BppIntoBackBuffer(GAME_BIT_MAP *, int , int );
GameLogSeverity G_game_log_severity;
void WriteLog(_In_ DWORD LogLevel, _In_ char* Message, _In_ ...) ;
DWORD LoadRegistryParameters(void);


__m128i data;
game_info GInfo;
Player g_Player;
BOOL gWindowHasFocus;
GAME_BIT_MAP *g_CurrentSprite;

game_states g_CurrentGameState = GAME_MAIN_MENU_STATE;

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

GAME_SOUND gMenuNavigate;

BOOL GameInProgress = FALSE;

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

    if ((Load32BppFile("assets\\Game_Font_Sprite.bmp", &g_Game_Font) != ERROR_SUCCESS))
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

    if(InitializePlayer() != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Failed to initialize hero!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto EXIT;
    }

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
    short EnterKeyIsDown = GetAsyncKeyState(VK_RETURN);

    static short EnterKeyWasDown;
    static short LeftKeyWasDown;
    static short RightKeyWasDown;
    static short debug_key_was_down;
    static short UpKeyWasDown;
    static short DownKeyWasDown;

    uint8_t CurrentPixelPosition = 0;

    switch (g_CurrentGameState)
    {
        case GAME_MAIN_MENU_STATE:
        {

            if (UpKeyIsDown && !UpKeyWasDown)
            {
                if (GameInProgress)
                {
                    if (MainGameMenu.SelectedItem == 0)
                    {
                        MainGameMenu.SelectedItem = 2;
                    }
                    else
                    {
                        MainGameMenu.SelectedItem < 4 && MainGameMenu.SelectedItem++;
                    }
                }
                else
                {
                    if (MainGameMenu.SelectedItem == 0)
                    {
                        MainGameMenu.SelectedItem = 1;
                    }
                    else
                    {
                        MainGameMenu.SelectedItem < 4 && MainGameMenu.SelectedItem++;
                    }
                }

                PlayGameSound(&gMenuNavigate);
            }

            if (DownKeyIsDown && !DownKeyWasDown)
            {
                if (GameInProgress)
                {
                    if (MainGameMenu.SelectedItem == 2)
                    {
                        MainGameMenu.SelectedItem = 0;
                    }
                    else
                    {
                        MainGameMenu.SelectedItem > 0 && MainGameMenu.SelectedItem--;
                    }
                }
                else
                {
                    if (MainGameMenu.SelectedItem == 2)
                    {
                        MainGameMenu.SelectedItem = 1;
                    }
                    else
                    {
                        MainGameMenu.SelectedItem > 1 && MainGameMenu.SelectedItem--;
                    }
                }


                PlayGameSound(&gMenuNavigate);
            }

            if (EnterKeyIsDown && !EnterKeyWasDown)
            {
                MainGameMenu.Items[MainGameMenu.SelectedItem]->action();
            }

            EnterKeyWasDown = EnterKeyIsDown;
            UpKeyWasDown = UpKeyIsDown;
            DownKeyWasDown = DownKeyIsDown;
        }

        case GAME_YESORNOEXITMENU_STATE:
        {
            if (UpKeyIsDown && !UpKeyWasDown)
            {
                g_ExitYesOrNoMenu.SelectedItem < 1 && g_ExitYesOrNoMenu.SelectedItem++;
                PlayGameSound(&gMenuNavigate);
            }

            if (DownKeyIsDown && !DownKeyWasDown)
            {
                g_ExitYesOrNoMenu.SelectedItem > 0 && g_ExitYesOrNoMenu.SelectedItem--;
                PlayGameSound(&gMenuNavigate);
            }

            if (EnterKeyIsDown && !EnterKeyWasDown)
            {
                g_ExitYesOrNoMenu.Items[g_ExitYesOrNoMenu.SelectedItem]->action();
                PlayGameSound(&gMenuNavigate);
            }

            EnterKeyWasDown = EnterKeyIsDown;
            UpKeyWasDown = UpKeyIsDown;
            DownKeyWasDown = DownKeyIsDown;

        }

        case GAME_OPTIONS_STATE:
        {
            if (UpKeyIsDown && !UpKeyWasDown)
            {
                g_mi_OptionsMenu.SelectedItem < 3 && g_mi_OptionsMenu.SelectedItem++;
                PlayGameSound(&gMenuNavigate);
            }

            if (DownKeyIsDown && !DownKeyWasDown)
            {
                g_mi_OptionsMenu.SelectedItem > 0 && g_mi_OptionsMenu.SelectedItem--;
                PlayGameSound(&gMenuNavigate);
            }

            if (EnterKeyIsDown && !EnterKeyWasDown)
            {
                g_mi_OptionsMenu.Items[g_mi_OptionsMenu.SelectedItem]->action();
                PlayGameSound(&gMenuNavigate);
            }

            EnterKeyWasDown = EnterKeyIsDown;
            UpKeyWasDown = UpKeyIsDown;
            DownKeyWasDown = DownKeyIsDown;
            break;
        }

        case GAME_OVERWORLD_STATE:
        {
            if(Esc_Key_is_down)
            {
                SendMessageA(g_window_handle, WM_CLOSE, 0, 0);
            }

            if(Debug_key_is_down && !debug_key_was_down)
            {
                game_performance.DebugModeOn = !game_performance.DebugModeOn ;
            }

            if(DownKeyIsDown)
            {
                if(g_Player.ScreenPosY < (GAME_RES_HEIGHT - 16))
                {
                    g_Player.ScreenPosY += 1;
                    g_Player.PixelPosition += 1;
                    g_Player.Direction = character_direction_down;
                }
            }

            if(LeftKeyIsDown)
            {
                if(g_Player.ScreenPosX > 0)
                {
                    g_Player.ScreenPosX -= 1;
                    g_Player.PixelPosition += 1;
                    g_Player.Direction = character_direction_left;

                }

            }

            if(RightKeyIsDown)
            {
                if(g_Player.ScreenPosX < GAME_RES_WIDTH - 16)
                {
                    g_Player.ScreenPosX += 1;
                    g_Player.PixelPosition += 1;
                    g_Player.Direction = character_direction_right;
                }

            }

            if(UpKeyIsDown)
            {
                if(g_Player.ScreenPosY > 0)
                {
                    g_Player.ScreenPosY -= 1;
                    g_Player.PixelPosition += 1;
                    g_Player.Direction = character_direction_up;
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

            if(g_Player.PixelPosition > 12)
            {
                g_Player.PixelPosition = 0;
            }
            else
            {
                switch(g_Player.PixelPosition)
                {
                    case 0:
                    {
                        g_Player.SpriteIndex = character_animation_standing;
                        break;
                    }
                    case 4:
                    {
                        g_Player.SpriteIndex = character_animation_cycle_one;
                        break;
                    }

                    case 8:
                    {
                        g_Player.SpriteIndex = character_animation_standing;
                        break;
                    }
                    case 12:
                    {
                        g_Player.SpriteIndex = character_animation_cycle_two;
                        break;
                    }
                }
            }

            debug_key_was_down = Debug_key_is_down;
            RightKeyWasDown = RightKeyIsDown;
            LeftKeyWasDown = LeftKeyIsDown;
        }
    }

}

void rendergraphics()
{
    HDC DeviceContext = GetDC(g_window_handle);
    switch (g_CurrentGameState)
    {
        case GAME_MAIN_MENU_STATE: {
            //uint32_t colorBlack = 0xFF111111;
            uint32_t colorBlack = 0xFF111111;
            PIXEL32 FontColor;
            FontColor.Blue = 0xFF;
            FontColor.Green = 0xFF;
            FontColor.Red = 0xFF;

            base_screen(&colorBlack);


            char GameNameString[20] = "";
            strncpy(GameNameString, MainGameMenu.MenuText, strlen(MainGameMenu.MenuText));
            BlitStringIntoBuffer (&g_Game_Font, 50, 20, GameNameString, FontColor);

            for (uint8_t MenuItemIterator = 0; MenuItemIterator < MainGameMenu.ItemCount; MenuItemIterator++)
            {
                if (MainGameMenu.Items[MenuItemIterator]->ItemIsActive == TRUE)
                {
                    BlitStringIntoBuffer (&g_Game_Font, 35, MainGameMenu.Items[MainGameMenu.SelectedItem]->Y, ">", FontColor);

                    char CurrentMenuItemString[40] = "";
                    strncpy(CurrentMenuItemString,  MainGameMenu.Items[MenuItemIterator]->ItemTitle, strlen(MainGameMenu.MenuText));
                    BlitStringIntoBuffer (&g_Game_Font, 50, MainGameMenu.Items[MenuItemIterator]->Y, CurrentMenuItemString, FontColor);
                }


            }
            break;
        }
        case GAME_YESORNOEXITMENU_STATE :
        {
            uint32_t colorBlack = 0xFF111111;
            PIXEL32 FontColor;
            FontColor.Blue = 0xFF;
            FontColor.Green = 0xFF;
            FontColor.Red = 0xFF;

            base_screen(&colorBlack);

            char MenuItemMessage[40] = "";
            strncpy(MenuItemMessage, g_ExitYesOrNoMenu.MenuText, strlen(g_ExitYesOrNoMenu.MenuText));
            BlitStringIntoBuffer (&g_Game_Font, 50, 50, MenuItemMessage, FontColor);

            for (uint8_t MenuItemIterator = 0; MenuItemIterator < g_ExitYesOrNoMenu.ItemCount; MenuItemIterator++)
            {
                if (g_ExitYesOrNoMenu.Items[MenuItemIterator]->ItemIsActive != TRUE) { continue; }
                int YOffset = 30 + 15 * MenuItemIterator;

                if (g_ExitYesOrNoMenu.SelectedItem == MenuItemIterator)
                {
                    BlitStringIntoBuffer (&g_Game_Font, 35, 50 + YOffset, ">", FontColor);
                }

                char CurrentMenuItemString[40] = "";
                strncpy(CurrentMenuItemString,  g_ExitYesOrNoMenu.Items[MenuItemIterator]->ItemTitle, strlen(MainGameMenu.MenuText));
                BlitStringIntoBuffer (&g_Game_Font, 50, 50 + YOffset, CurrentMenuItemString, FontColor);
            }


            break;
        }

        case GAME_OPTIONS_STATE: {
            uint32_t colorBlack = 0xFF111111;
            PIXEL32 FontColor;
            FontColor.Blue = 0xFF;
            FontColor.Green = 0xFF;
            FontColor.Red = 0xFF;

            base_screen(&colorBlack);

            char MenuItemMessage[40] = "";
            strncpy(MenuItemMessage, g_mi_OptionsMenu.MenuText, strlen(g_mi_OptionsMenu.MenuText));
            BlitStringIntoBuffer (&g_Game_Font, 50, 50, MenuItemMessage, FontColor);

            for (uint8_t MenuItemIterator = 0; MenuItemIterator < g_mi_OptionsMenu.ItemCount; MenuItemIterator++)
            {
                if (g_mi_OptionsMenu.Items[MenuItemIterator]->ItemIsActive != TRUE) { continue; }
                int YOffset = 30 + 15 * MenuItemIterator;

                if (g_mi_OptionsMenu.SelectedItem == MenuItemIterator)
                {
                    BlitStringIntoBuffer (&g_Game_Font, 35, 50 + YOffset, ">", FontColor);
                }

                char CurrentMenuItemString[40] = "";
                strncpy(CurrentMenuItemString,  g_mi_OptionsMenu.Items[MenuItemIterator]->ItemTitle, strlen(g_mi_OptionsMenu.Items[MenuItemIterator]->ItemTitle));
                BlitStringIntoBuffer (&g_Game_Font, 50, 50 + YOffset, CurrentMenuItemString, FontColor);
            }
            PIXEL32 ActiveFontColor;
            ActiveFontColor.Blue = 0xFF;
            ActiveFontColor.Green = 0xFF;
            ActiveFontColor.Red = 0xFF;
            ActiveFontColor.Padding = 0xFF;

            PIXEL32 InactiveFontColor;
            InactiveFontColor.Blue = 0xAA;
            InactiveFontColor.Green = 0xAA;
            InactiveFontColor.Red = 0xAA;
            InactiveFontColor.Padding = 0xFF;

            const int SoundEffectYOffset = 50;
            int SoundEffectXOffset = 165;
            for (uint8_t SoundEffectIndicatorIterator = 0; SoundEffectIndicatorIterator < 10; SoundEffectIndicatorIterator++)
            {
                float SoundEffectIndicatorValue = (float)SoundEffectIndicatorIterator / 10.0f;



                SoundEffectXOffset = SoundEffectXOffset + 6;
                if (SoundEffectIndicatorValue < G_Current_Game_SoundEffect_Volume)
                {
                   BlitStringIntoBuffer (&g_Game_Font, SoundEffectXOffset, 30 + SoundEffectYOffset, "\xf2" , ActiveFontColor);
                }
                else
                {
                    BlitStringIntoBuffer (&g_Game_Font, SoundEffectXOffset, 30 + SoundEffectYOffset, "\xf2" , InactiveFontColor);
                }
            }

            const int MusicYOffset = 50;
            int MusicXOffset = strlen(g_mi_OptionsMenu.Items[1]->ItemTitle) * (8) + 8;
            for (uint8_t SoundMusicIndicatorIterator = 0; SoundMusicIndicatorIterator < 10; SoundMusicIndicatorIterator++)
            {
                float SoundMusicIndicatorIteratorValue = (float)SoundMusicIndicatorIterator / 10.0f;

                MusicXOffset = MusicXOffset + 6;
                if (SoundMusicIndicatorIteratorValue < G_Current_Game_Music_Volume)
                {
                    BlitStringIntoBuffer (&g_Game_Font, MusicXOffset, 45 + MusicYOffset, "\xf2" , ActiveFontColor);
                }
                else
                {
                    BlitStringIntoBuffer (&g_Game_Font, MusicXOffset, 45 + MusicYOffset, "\xf2" , InactiveFontColor);
                }
            }

            break;
        }

        case GAME_OVERWORLD_STATE: {
            char Text[] = "TESTANDO AQUI";
            char Text2[] = "DESENHANDO FONTES PERSONALIZADAS COM C!";

            #ifdef SIMD
                uint32_t color = 0xFF9900FF;
                base_screen(&color);
            #else
                base_screen();
            #endif
            PIXEL32 FontColor;
            FontColor.Blue = 0x00;
            FontColor.Green = 0x00;
            FontColor.Red = 0x00;

            BlitStringIntoBuffer (&g_Game_Font, 50, 50, Text, FontColor);
            BlitStringIntoBuffer (&g_Game_Font, 50, 75, Text2, FontColor);
            Load32BppIntoBackBuffer(&g_Player.PlayerSprite[g_Player.Direction + g_Player.SpriteIndex], g_Player.ScreenPosX, g_Player.ScreenPosY);
            break;
        }
        default:
            int *i;
            *i = 3.0;
    }
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

DWORD InitializePlayer(void)
{
    DWORD Error = ERROR_SUCCESS;
    g_Player.ScreenPosX = 25;
    g_Player.ScreenPosY = 25;
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
    int MessageLength = (sizeof(Message) / sizeof(char));
    const int FontXSize = 6;
    const int FontYSize = 7;
    GameCoordinate GameCoordinate;

    for (int i = 0; i < MessageLength;)
    {
        int CurrentChar = Message;
        if (CurrentChar >= 'A' && CurrentChar <= 'Z')
        {
            GameCoordinate.X = FontXSize * (CurrentChar - 0x41);
            GameCoordinate.Y = FontYSize;
            return GameCoordinate;
        }
        else if (CurrentChar >= 'a' && CurrentChar <= 'z')
        {
            GameCoordinate.X = FontXSize * (CurrentChar - 0x41);
            GameCoordinate.Y = FontYSize;
            return GameCoordinate;
        }
        else
        {
            switch (CurrentChar) {
                case '0':
                {
                    GameCoordinate.X = FontXSize * 52;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                }
                case '1':
                {
                    GameCoordinate.X = FontXSize * 53;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '2':
                {
                    GameCoordinate.X = FontXSize * 54;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '3':
                {
                    GameCoordinate.X = FontXSize * 55;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '4':
                {
                    GameCoordinate.X = FontXSize * 56;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '5':
                {
                    GameCoordinate.X = FontXSize * 57;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '6':
                {
                    GameCoordinate.X = FontXSize * 58;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '7':
                {
                    GameCoordinate.X = FontXSize * 59;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '8':
                {
                    GameCoordinate.X = FontXSize * 60;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '9':
                {
                    GameCoordinate.X = FontXSize * 61;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '`':
                {
                    GameCoordinate.X = FontXSize * 62;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '~':
                {
                    GameCoordinate.X = FontXSize * 63;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '!':
                {
                    GameCoordinate.X = FontXSize * 64;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '@':
                {
                    GameCoordinate.X = FontXSize * 65;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '#':
                {
                    GameCoordinate.X = FontXSize * 66;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '$':
                {
                    GameCoordinate.X = FontXSize * 67;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '%':
                {
                    GameCoordinate.X = FontXSize * 68;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '^':
                {
                    GameCoordinate.X = FontXSize * 69;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '&':
                {
                    GameCoordinate.X = FontXSize * 70;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '*':
                {
                    GameCoordinate.X = FontXSize * 71;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '(':
                {
                    GameCoordinate.X = FontXSize * 72;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case ')':
                {
                    GameCoordinate.X = FontXSize * 73;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '-':
                {
                    GameCoordinate.X = FontXSize * 74;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '=':
                {
                    GameCoordinate.X = FontXSize * 75;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '_':
                {
                    GameCoordinate.X = FontXSize * 76;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '+':
                {
                    GameCoordinate.X = FontXSize * 77;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '\\':
                {
                    GameCoordinate.X = FontXSize * 78;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '|':
                {
                    GameCoordinate.X = FontXSize * 79;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '[':
                {
                    GameCoordinate.X = FontXSize * 80;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case ']':
                {
                    GameCoordinate.X = FontXSize * 81;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '{':
                {
                    GameCoordinate.X = FontXSize * 82;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '}':
                {
                    GameCoordinate.X = FontXSize * 83;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case ';':
                {
                    GameCoordinate.X = FontXSize * 84;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '\'':
                {
                    GameCoordinate.X = FontXSize * 85;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case ':':
                {
                    GameCoordinate.X = FontXSize * 86;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '"':
                {
                    GameCoordinate.X = FontXSize * 87;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case ',':
                {
                    GameCoordinate.X = FontXSize * 88;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '<':
                {
                    GameCoordinate.X = FontXSize * 89;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '>':
                {
                    GameCoordinate.X = FontXSize * 90;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '.':
                {
                    GameCoordinate.X = FontXSize * 91;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '/':
                {
                    GameCoordinate.X = FontXSize * 92;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '?':
                {
                    GameCoordinate.X = FontXSize * 93;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case ' ':
                {
                    GameCoordinate.X = FontXSize * 94;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                case '\xf2':
                {
                    GameCoordinate.X = FontXSize * 97;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                    break;
                }
                default:
                {
                    GameCoordinate.X = FontXSize * 93;
                    GameCoordinate.Y = FontYSize;
                    return GameCoordinate;
                }
            }


        }
    }

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
void g_mi_StartGameAction(void){};
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

void g_mi_ScreenResolutionAction(void){};

void g_mi_OptionsBackAction(void)
{
    g_CurrentGameState = GAME_MAIN_MENU_STATE;
};