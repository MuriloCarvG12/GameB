//
// Created by Chine on 25/05/2026.
//

#include "GameOptionsScreen.h"
#include "projectdeclarations.h"

void DrawOptionsScreen(void)
{
    uint32_t colorBlack = 0xFF111111;
    PIXEL32 FontColor;
    FontColor.Blue = 0xFF;
    FontColor.Green = 0xFF;
    FontColor.Red = 0xFF;

    base_screen(&colorBlack);

    char MenuItemMessage[40] = "";
    strncpy(MenuItemMessage, g_mi_OptionsMenu.MenuText, strlen(g_mi_OptionsMenu.MenuText));
    BlitStringIntoBuffer (&g_Game_Font, 50, 36, MenuItemMessage, FontColor);

    for (uint8_t MenuItemIterator = 0; MenuItemIterator < g_mi_OptionsMenu.ItemCount; MenuItemIterator++)
    {
        if (g_mi_OptionsMenu.Items[MenuItemIterator]->ItemIsActive == TRUE) {
            if (g_mi_OptionsMenu.SelectedItem == MenuItemIterator) {
                BlitStringIntoBuffer (&g_Game_Font, 35,g_mi_OptionsMenu.Items[MenuItemIterator]->Y , ">", FontColor);
            }
            char CurrentMenuItemString[40] = "";
            strncpy(CurrentMenuItemString,  g_mi_OptionsMenu.Items[MenuItemIterator]->ItemTitle, strlen(g_mi_OptionsMenu.Items[MenuItemIterator]->ItemTitle));
            BlitStringIntoBuffer (&g_Game_Font, 50, g_mi_OptionsMenu.Items[MenuItemIterator]->Y, CurrentMenuItemString, FontColor);
        }

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

    const int SoundEffectYOffset = g_mi_OptionsMenu.Items[0]->Y;
    int SoundEffectXOffset = 165;
    for (uint8_t SoundEffectIndicatorIterator = 0; SoundEffectIndicatorIterator < 10; SoundEffectIndicatorIterator++)
    {
        float SoundEffectIndicatorValue = (float)SoundEffectIndicatorIterator / 10.0f;



        SoundEffectXOffset = SoundEffectXOffset + 6;
        if (SoundEffectIndicatorValue < G_Current_Game_SoundEffect_Volume)
        {
           BlitStringIntoBuffer (&g_Game_Font, SoundEffectXOffset, SoundEffectYOffset, "\xf2" , ActiveFontColor);
        }
        else
        {
            BlitStringIntoBuffer (&g_Game_Font, SoundEffectXOffset, SoundEffectYOffset, "\xf2" , InactiveFontColor);
        }
    }

    const int MusicYOffset = g_mi_OptionsMenu.Items[1]->Y;
    int MusicXOffset = strlen(g_mi_OptionsMenu.Items[1]->ItemTitle) * (8) + 8;
    for (uint8_t SoundMusicIndicatorIterator = 0; SoundMusicIndicatorIterator < 10; SoundMusicIndicatorIterator++)
    {
        float SoundMusicIndicatorIteratorValue = (float)SoundMusicIndicatorIterator / 10.0f;

        MusicXOffset = MusicXOffset + 6;
        if (SoundMusicIndicatorIteratorValue < G_Current_Game_Music_Volume)
        {
            BlitStringIntoBuffer (&g_Game_Font, MusicXOffset,  MusicYOffset, "\xf2" , ActiveFontColor);
        }
        else
        {
            BlitStringIntoBuffer (&g_Game_Font, MusicXOffset,  MusicYOffset, "\xf2" , InactiveFontColor);
        }
    }

    const int ResolutionYOffset = g_mi_OptionsMenu.Items[2]->Y;
    int ResolutionXOffset = g_mi_OptionsMenu.Items[2]->X + 75;
    for (uint8_t ResolutionIndicatorIterator = 0; ResolutionIndicatorIterator < game_performance.MaxGameResScaleFactor; ResolutionIndicatorIterator++)
    {
        ResolutionXOffset += 6;
        if (ResolutionIndicatorIterator < game_performance.CurrentGameResScaleFactor)
        {
            BlitStringIntoBuffer (&g_Game_Font, ResolutionXOffset,  ResolutionYOffset, "\xf2" , ActiveFontColor);
        }
        else
        {
            BlitStringIntoBuffer (&g_Game_Font, ResolutionXOffset,  ResolutionYOffset, "\xf2" , InactiveFontColor);
        }
    }

}

void ProcessOptionsMenuInput(void)
{
    short UpKeyIsDown = GetAsyncKeyState('W');
    short DownKeyIsDown = GetAsyncKeyState('S');
    short EnterKeyIsDown = GetAsyncKeyState(VK_RETURN);

    static short EnterKeyWasDown;
    static short UpKeyWasDown;
    static short DownKeyWasDown;

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
}