//
// Created by Chine on 25/05/2026.
//

#include "GameOverworldScreen.h"
#include "projectdeclarations.h"

void DrawGameOverWorldScreen(void)
{
    char Text[] = "TESTANDO AQUI";
    char Text2[] = "DESENHANDO FONTES PERSONALIZADAS COM C!";
    static int LocalFrameCounter = 0;

#ifdef SIMD
    uint32_t color = 0xFF9900FF;
    Load32BppOverworldIntoBackBuffer(&G_game_overworld_info.OverWorldBackGroundSprite,0,0);
    //base_screen(&color);
#else
    base_screen();
#endif
    PIXEL32 FontColor;
    FontColor.Blue = 0x00;
    FontColor.Green = 0x00;
    FontColor.Red = 0x00;

    if (game_performance.DebugModeOn )
    {
        int DrawYIndex = 0;
        int DrawXIndex = 0;
        int CameraTileX = g_CameraPosition.X / 16;
        int CameraTileY = g_CameraPosition.Y / 16;

        for (int row = DrawYIndex; row < G_game_overworld_info.TileMap.TileMapHeight; row++)
        {
            DrawXIndex = 0;
            for (int column = DrawXIndex; column < G_game_overworld_info.TileMap.TileMapWidth; column++)
            {
                int index = (row + CameraTileY) * G_game_overworld_info.TileMap.TileMapWidth + (column + CameraTileX);
                char TileValue[8];

                int screenX = (DrawXIndex) * 16;
                int screenY = (DrawYIndex) * 16;

                _itoa_s(G_game_overworld_info.TileMap.Map[index], TileValue, sizeof(TileValue), 10);

                BlitStringIntoBuffer(
                    &g_Game_Font,
                    screenX,
                    screenY,
                    TileValue,
                    FontColor
                );
                DrawXIndex++;
            }
            DrawYIndex++;
        }
    }


    BlitStringIntoBuffer (&g_Game_Font, 50, 50, Text, FontColor);
    BlitStringIntoBuffer (&g_Game_Font, 50, 75, Text2, FontColor);
    Load32BppIntoBackBuffer(&g_Player.PlayerSprite[g_Player.Direction + g_Player.SpriteIndex], g_Player.ScreenPosX, g_Player.ScreenPosY);

    if (LocalFrameCounter == 30) {
        PlayGameMusic(&gMusicOverworld01);
    }
    LocalFrameCounter++;

}

static BOOL IsTilePassable(int PixelX, int PixelY)
{
    int Col   = (PixelX + g_CameraPosition.X) / 16;
    int Row   = (PixelY + g_CameraPosition.Y) / 16;
    int Index = Row * G_game_overworld_info.TileMap.TileMapWidth + Col;

    for (uint8_t i = 0; i < _countof(gPassableTiles); i++)
    {
        if (G_game_overworld_info.TileMap.Map[Index] == gPassableTiles[i])
            return TRUE;
    }
    return FALSE;
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
        PIXEL32 FontColor;
        FontColor.Blue = 0x00;
        FontColor.Green = 0x00;
        FontColor.Red = 0x00;

    }


    if (DownKeyIsDown)
    {

        BOOL CanMoveToDesiredTile =
            IsTilePassable(g_Player.ScreenPosX,      g_Player.ScreenPosY + 16) &&
            IsTilePassable(g_Player.ScreenPosX + 15, g_Player.ScreenPosY + 16);

        if (CanMoveToDesiredTile)
        {
            if (g_Player.ScreenPosY < GAME_RES_HEIGHT - 16)
            {
                g_Player.ScreenPosY  += 1;
                g_Player.PixelPosition += 1;
                g_Player.Direction = character_direction_down;
            }
            if (g_Player.ScreenPosY == GAME_RES_HEIGHT - 16 &&
                g_CameraPosition.Y < G_game_overworld_info.OverWorldBackGroundSprite.BitMapInfo.bmiHeader.biHeight)
            {
                g_Player.PixelPosition += 1;
                g_CameraPosition.Y++;
                g_Player.Direction = character_direction_down;
            }
        }
    }

    if (UpKeyIsDown)
    {

        BOOL CanMoveToDesiredTile =
            IsTilePassable(g_Player.ScreenPosX,      g_Player.ScreenPosY - 1) &&
            IsTilePassable(g_Player.ScreenPosX + 15, g_Player.ScreenPosY - 1);

        if (CanMoveToDesiredTile)
        {
            if (g_Player.ScreenPosY > 0)
            {
                g_Player.ScreenPosY  -= 1;
                g_Player.PixelPosition += 1;
                g_Player.Direction = character_direction_up;
            }
            if (g_Player.ScreenPosY == 0 && g_CameraPosition.Y > 0)
            {
                g_Player.PixelPosition += 1;
                g_CameraPosition.Y--;
                g_Player.Direction = character_direction_up;
            }
        }
    }

    if (LeftKeyIsDown)
    {

        BOOL CanMoveToDesiredTile =
            IsTilePassable(g_Player.ScreenPosX - 1, g_Player.ScreenPosY) &&
            IsTilePassable(g_Player.ScreenPosX - 1, g_Player.ScreenPosY + 15);

        if (CanMoveToDesiredTile)
        {
            if (g_Player.ScreenPosX < 16 && g_CameraPosition.X > 0)
            {
                g_Player.PixelPosition += 1;
                g_CameraPosition.X--;
                g_Player.Direction = character_direction_left;
            }
            if (g_Player.ScreenPosX > 0)
            {
                g_Player.ScreenPosX  -= 1;
                g_Player.PixelPosition += 1;
                g_Player.Direction = character_direction_left;
            }
        }
    }

    if (RightKeyIsDown)
    {

        BOOL CanMoveToDesiredTile =
            IsTilePassable(g_Player.ScreenPosX + 16, g_Player.ScreenPosY) &&
            IsTilePassable(g_Player.ScreenPosX + 16, g_Player.ScreenPosY + 15);

        if (CanMoveToDesiredTile)
        {
            if (g_Player.ScreenPosX >= GAME_RES_WIDTH - 16 &&
                g_CameraPosition.X < G_game_overworld_info.OverWorldBackGroundSprite.BitMapInfo.bmiHeader.biWidth)
            {
                g_Player.PixelPosition += 1;
                g_CameraPosition.X++;
                g_Player.Direction = character_direction_right;
            }
            if (g_Player.ScreenPosX < GAME_RES_WIDTH - 16)
            {
                g_Player.ScreenPosX  += 1;
                g_Player.PixelPosition += 1;
                g_Player.Direction = character_direction_right;
            }
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

