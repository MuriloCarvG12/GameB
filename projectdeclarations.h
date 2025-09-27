//
// Created by Murilo on 31/08/2025.
//

#ifndef UNTITLED7_PROJECTDECLARATIONS_H
#define UNTITLED7_PROJECTDECLARATIONS_H

#include <windows.h>
#include <stdint.h>


#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define BPP 32 // defines our bits per pixel
#define GAME_AREA_MEMORY_SIZE   (GAME_RES_WIDTH * GAME_RES_HEIGHT * (BPP/ 8))
#define CALCULATE_FPS_EVERY_X_FRAMES 60
#define TARGET_MICROSECONDS_PER_FRAME 16667
#define SIMD


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
    int32_t WorldPosX;
    int32_t WorldPosY;
    int32_t HP;
    int32_t strength;
    int32_t mp;
} Player;

#ifdef SIMD
void base_screen(uint32_t *pixel_color);
#else
void base_screen();
#endif


#endif //UNTITLED7_PROJECTDECLARATIONS_H
