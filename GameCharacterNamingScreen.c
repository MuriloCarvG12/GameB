//
// Created by Chine on 25/05/2026.
//

#include "GameCharacterNamingScreen.h"
#include "projectdeclarations.h"

void DrawCharacterNamingScreen()
{
    PIXEL32 FontColor;
    FontColor.Blue = 0xFF;
    FontColor.Green = 0xFF;
    FontColor.Red = 0xFF;
    FontColor.Padding = 0xFF;

    uint32_t colorBlack = 0xFF111111;;

    base_screen(&colorBlack);

    char MenuItemMessage[40] = "";
    strncpy(MenuItemMessage, g_mi_CharacterNamingScreenMenu.MenuText, strlen(g_mi_CharacterNamingScreenMenu.MenuText));
    BlitStringIntoBuffer (&g_Game_Font, (GAME_RES_WIDTH /2)- 8 * 10, 36, MenuItemMessage, FontColor);


    for (int i = 0; i < _countof(g_Player.name); i++)
    {
        int currentDistance = 8 * i;

        if (g_Player.name[i] == '\0') {
            BlitStringIntoBuffer (&g_Game_Font, 150 + currentDistance, 60, "_", FontColor);
        }
        else {
            char singleChar[2] = { g_Player.name[i], '\0' };
            BlitStringIntoBuffer(&g_Game_Font, 150 + currentDistance, 60, singleChar, FontColor);
        }

    }

    for (uint8_t CharacterIterator = 0; CharacterIterator < g_mi_CharacterNamingScreenMenu.ItemCount; CharacterIterator++)
    {
        BlitStringIntoBuffer (&g_Game_Font,
                                g_mi_CharacterNamingScreenMenu.Items[CharacterIterator]->X,
                                g_mi_CharacterNamingScreenMenu.Items[CharacterIterator]->Y,
                                g_mi_CharacterNamingScreenMenu.Items[CharacterIterator]->ItemTitle ,
                                FontColor);

        if (g_mi_CharacterNamingScreenMenu.SelectedItem == CharacterIterator) {
            BlitStringIntoBuffer (&g_Game_Font,g_mi_CharacterNamingScreenMenu.Items[CharacterIterator]->X - 8 ,g_mi_CharacterNamingScreenMenu.Items[CharacterIterator]->Y , ">", FontColor);
        }
    }
}

void ProcessCharacterNamingMenuInput(void)
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

    // 52 for the back button and 53 for the continue button
    if (UpKeyIsDown && !UpKeyWasDown)
    {
        if (g_mi_CharacterNamingScreenMenu.SelectedItem == 53) {
            g_mi_CharacterNamingScreenMenu.SelectedItem = 46; PlayGameSound(&gMenuNavigate);
        }
        else if (g_mi_CharacterNamingScreenMenu.SelectedItem - 13 >= 0)
        {g_mi_CharacterNamingScreenMenu.SelectedItem -= 13; PlayGameSound(&gMenuNavigate);}

    }

    if (DownKeyIsDown && !DownKeyWasDown)
    {
        if (g_mi_CharacterNamingScreenMenu.SelectedItem + 13 < 53)
        {g_mi_CharacterNamingScreenMenu.SelectedItem += 13; PlayGameSound(&gMenuNavigate);}
        else if (g_mi_CharacterNamingScreenMenu.SelectedItem == 39 || g_mi_CharacterNamingScreenMenu.SelectedItem == 40) {
            g_mi_CharacterNamingScreenMenu.SelectedItem = 52; PlayGameSound(&gMenuNavigate);
        }
        else if (g_mi_CharacterNamingScreenMenu.SelectedItem > 45) {
            g_mi_CharacterNamingScreenMenu.SelectedItem = 53; PlayGameSound(&gMenuNavigate);
        }
    }

    if (RightKeyIsDown && !RightKeyWasDown)
    {
        if (g_mi_CharacterNamingScreenMenu.SelectedItem < 53) {g_mi_CharacterNamingScreenMenu.SelectedItem++; PlayGameSound(&gMenuNavigate);}
    }

    if (LeftKeyIsDown && !LeftKeyWasDown)
    {
        if (g_mi_CharacterNamingScreenMenu.SelectedItem > 0) {g_mi_CharacterNamingScreenMenu.SelectedItem--; PlayGameSound(&gMenuNavigate);}
    }

    if (EnterKeyIsDown && !EnterKeyWasDown)
    {
        g_mi_CharacterNamingScreenMenu.Items[g_mi_CharacterNamingScreenMenu.SelectedItem]->action();
        PlayGameSound(&gMenuNavigate);
    }

    UpKeyWasDown = UpKeyIsDown;
    DownKeyWasDown = DownKeyIsDown;
    RightKeyWasDown = RightKeyIsDown;
    LeftKeyWasDown = LeftKeyIsDown;
    EnterKeyWasDown = EnterKeyIsDown;
}