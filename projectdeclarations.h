//
// Created by Murilo on 31/08/2025.
//

#ifndef UNTITLED7_PROJECTDECLARATIONS_H
#define UNTITLED7_PROJECTDECLARATIONS_H

#include <windows.h>
#include <stdint.h>
#include <emmintrin.h>
#include <psapi.h>


#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define BPP 32 // defines our bits per pixel
#define GAME_AREA_MEMORY_SIZE   (GAME_RES_WIDTH * GAME_RES_HEIGHT * (BPP/ 8))
#define CALCULATE_FPS_EVERY_X_FRAMES 60
#define TARGET_MICROSECONDS_PER_FRAME 16667
#define SIMD

// this approach sucks, change it later...
#define SpriteAssets "assets\\Hero_Suit0_Down_Standing.bmpx"
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
} game_performance_info;

typedef struct GAME_BIT_MAP
{
    BITMAPINFO BitMapInfo;
    void* memory_canvas;
} GAME_BIT_MAP;

typedef struct PIXEL32
{
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
    uint8_t Alpha;
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

#ifdef SIMD
void base_screen(uint32_t *pixel_color);
#else
void base_screen();
#endif


#endif //UNTITLED7_PROJECTDECLARATIONS_H
