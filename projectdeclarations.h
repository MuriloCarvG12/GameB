//
// Created by Murilo on 31/08/2025.
//

#ifndef UNTITLED7_PROJECTDECLARATIONS_H
#define UNTITLED7_PROJECTDECLARATIONS_H

#include <windows.h>
#include <stdint.h>
#include <emmintrin.h>
#include <psapi.h>
#include <xaudio2.h>


#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define BPP 32 // defines our bits per pixel
#define GAME_AREA_MEMORY_SIZE   (GAME_RES_WIDTH * GAME_RES_HEIGHT * (BPP/ 8))
#define CALCULATE_FPS_EVERY_X_FRAMES 60
#define TARGET_MICROSECONDS_PER_FRAME 16667
#define SIMD

// this approach sucks, change it later...
#define SpriteAssets "assets\\Hero_Suit0_Down_Standing.bmp"
#define character_sprite_down_standing 0
#define character_sprite_down_walk_one 1
#define character_sprite_down_walk_two 2

#define character_sprite_left_standing 3
#define character_sprite_left_walk_one 4
#define character_sprite_left_walk_two 5

#define character_sprite_right_standing 6
#define character_sprite_right_walk_one 7
#define character_sprite_right_walk_two 8

#define character_sprite_up_standing 9
#define character_sprite_up_walk_one 10
#define character_sprite_up_walk_two 11

#define character_direction_down 0

#define character_direction_left 3

#define character_direction_right 6

#define character_direction_up 9

#define character_animation_standing 0

#define character_animation_cycle_one 1

#define character_animation_cycle_two 2

#define log_severity_none 0

#define log_severity_info 1

#define log_severity_warning 2

#define log_severity_error 3

#define log_severity_debug 4

#define MAX_NUMBER_GAME_SOUND_EFFECTS 4

typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);


typedef struct game_info

        {
            MONITORINFO MonitorInfo;
            long monitor_height;
            long monitor_width;
        } game_info;

typedef struct game_performance_info
{
    uint64_t TotalFramesRendered;
    uint64_t TickFrequency; // stores our cpu ticksp
    uint32_t RawFramesPerSecondAverage;
    uint32_t CookedFramesPerSecondAverage;
    float AvgFpsRaw;
    float AvgFpsCooked;
    BOOL DebugModeOn;
    LONG MinimumTimerResolution;
    LONG MaximumTimerResolution;
    LONG CurrentTimerResolution;
    DWORD HandleCount;
    PROCESS_MEMORY_COUNTERS_EX MemoryInfo;
    SYSTEM_INFO SystemInfo;
    int64_t  CurrentSystemTime;
    int64_t  PreviousSystemTime;
    float CpuPercentage;
    int MaxGameResScaleFactor;
    int CurrentGameResScaleFactor;
} game_performance_info;

typedef struct GAME_BIT_MAP
{
    BITMAPINFO BitMapInfo;
    void* memory_canvas;
} GAME_BIT_MAP;

typedef struct PIXEL24
{
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
} PIXEL24;

typedef struct PIXEL32
{
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
    uint8_t Padding;
} PIXEL32;

typedef struct Player
{
    char name[12];
    GAME_BIT_MAP PlayerSprite[12];
    int32_t ScreenPosX;
    int32_t ScreenPosY;
    int32_t HP;
    int32_t strength;
    int32_t mp;
    uint8_t Direction;
    uint8_t SpriteIndex;
    uint8_t PixelPosition;
} Player;

typedef struct GameCoordinate {
    int32_t X;
    int32_t Y;
} GameCoordinate;

typedef struct GameLogSeverity {
    int32_t LogLevel;
} GameLogSeverity;

#ifdef SIMD
void base_screen(uint32_t *pixel_color);
#else
void base_screen();
#endif

typedef enum game_states
{
    GAME_INTRO_STATE = 0,
    GAME_MAIN_MENU_STATE = 1,
    GAME_YESORNOEXITMENU_STATE = 2,
    GAME_OVERWORLD_STATE = 3,
    GAME_BATTLE_STATE = 3,
    GAME_PAUSE_STATE = 4,
    GAME_OPTIONS_STATE = 5,
    GAME_NAME_CHARACTER_STATE = 6
} game_states;

HRESULT InitializeSoundEngine(void);

typedef struct GAME_SOUND {
    WAVEFORMATEX WaveFormat;
    XAUDIO2_BUFFER Buffer;
} GAME_SOUND ;

DWORD LoadWavFileFromDisk(_In_ char* WavFileName, _Inout_ GAME_SOUND* game_sound);
void PlayGameSound(_In_ GAME_SOUND* GameSound);

typedef struct game_registry_info {
    DWORD LOG_LEVEL;
    DWORD GAME_SOUND_EFFECTS_VOLUME_LEVEL;
    DWORD GAME_MUSIC_VOLUME_LEVEL;
    DWORD CURRENT_GAME_RES_SCALE_FACTOR;
} game_registry_info;

#endif //UNTITLED7_PROJECTDECLARATIONS_H
