//
// Created by Chine on 25/05/2026.
//

#include "GameYesOrNoExitMenu.h"
#include "string.h"
#include "projectdeclarations.h"


void DrawYesOrNoExitMenu(void)
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
}

void ProcessYesOrNoExitMenuInput(void)
{
    short UpKeyIsDown = GetAsyncKeyState('W');
    short DownKeyIsDown = GetAsyncKeyState('S');
    short EnterKeyIsDown = GetAsyncKeyState(VK_RETURN);

    static short EnterKeyWasDown;
    static short LeftKeyWasDown;
    static short RightKeyWasDown;
    static short debug_key_was_down;
    static short UpKeyWasDown;
    static short DownKeyWasDown;

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