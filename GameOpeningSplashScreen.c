//
// Created by Chine on 25/05/2026.
//

#include "GameOpeningSplashScreen.h"
#include "projectdeclarations.h"


void DrawGameOpeningSplashScreen(void)
{
    uint32_t colorBlack = 0xFF111111;
    char GameNameString[] = "--GAME STUDIO--";
    char GameNameStudio[] = "--PRESENTS--";

    PIXEL32 FontColor;
    FontColor.Blue = 0x11;
    FontColor.Green = 0x11;
    FontColor.Red = 0x11;
    FontColor.Padding = 0xFF;

    static int FramesPassed = 0;

    if (FramesPassed == 60) {
        PlayGameSound(&gIntroEffect);
    }
    if (FramesPassed >= 60 && FramesPassed <= 120)
    {
        FontColor.Blue = 0xFF;
        FontColor.Green = 0xFF;
        FontColor.Red = 0xFF;
        FontColor.Padding = 0xFF;
    }
    else if (FramesPassed >= 120 && FramesPassed <= 180)
    {
        FontColor.Blue = 0xD1;
        FontColor.Green = 0xD1;
        FontColor.Red = 0xD1;
        FontColor.Padding = 0xFF;
    }
    else if (FramesPassed >= 180 && FramesPassed <= 240)
    {
        FontColor.Blue = 0x94;
        FontColor.Green = 0x94;
        FontColor.Red = 0x94;
        FontColor.Padding = 0xFF;
    }
    else
    {
        FontColor.Blue = 0x11;
        FontColor.Green = 0x11;
        FontColor.Red = 0x11;
        FontColor.Padding = 0xFF;
    }

    base_screen(&colorBlack);

    BlitStringIntoBuffer (&g_Game_Font, (GAME_RES_WIDTH /2)- 8 * 6, 54, GameNameString, FontColor);
    BlitStringIntoBuffer (&g_Game_Font, (GAME_RES_WIDTH /2)- 8 * 5, 78, GameNameStudio, FontColor);

    FramesPassed++;

    if (FramesPassed >= 300) {
        FramesPassed = 0;
        g_CurrentGameState = GAME_MAIN_MENU_STATE;
    }
}