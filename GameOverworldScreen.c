//
// Created by Chine on 25/05/2026.
//

#include "GameOverworldScreen.h"
#include "projectdeclarations.h"

void DrawGameOverWorldScreen(void)
{
    char Text[] = "TESTANDO AQUI";
    char Text2[] = "DESENHANDO FONTES PERSONALIZADAS COM C!";

#ifdef SIMD
    uint32_t color = 0xFF9900FF;
    Load32BppOverworldIntoBackBuffer(&g_OverWorld01Sprite,0,0);
    //base_screen(&color);
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
}

void ProcessGameOverWorldScreenInput(void)
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
        if (g_Player.ScreenPosY == GAME_RES_HEIGHT - 16 && g_CameraPosition.Y < g_OverWorld01Sprite.BitMapInfo.bmiHeader.biHeight) {
            g_Player.PixelPosition += 1;
            g_CameraPosition.Y++;
        }
    }

    if(LeftKeyIsDown)
    {
        if (g_Player.ScreenPosX < 16 && g_CameraPosition.X > 0) {
            g_Player.PixelPosition += 1;
            g_CameraPosition.X--;
        }
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
        if (g_CameraPosition.X < g_OverWorld01Sprite.BitMapInfo.bmiHeader.biWidth && g_Player.ScreenPosX == GAME_RES_WIDTH - 16) {
            g_Player.PixelPosition += 1;
            g_CameraPosition.X++;
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
        if (g_Player.ScreenPosY <= 16 && g_CameraPosition.Y > 0) {
            g_Player.PixelPosition += 1;
            g_CameraPosition.Y--;
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

