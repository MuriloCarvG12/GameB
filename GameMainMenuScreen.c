//
// Created by Chine on 25/05/2026.
//

#include "GameMainMenuScreen.h"
#include "projectdeclarations.h"


void GameMainMenuScreen(void)
{
    static int FramesPassed = 0;
            uint32_t colorBlack = 0xFF111111;
            PIXEL32 FontColor;

            base_screen(&colorBlack);

            if (FramesPassed <= 60)
            {
                FontColor.Blue    = 0x11;
                FontColor.Green   = 0x11;
                FontColor.Red     = 0x11;
                FontColor.Padding = 0xFF;
            }
            else if (FramesPassed <= 120)
            {
                FontColor.Blue    = 0x55;
                FontColor.Green   = 0x55;
                FontColor.Red     = 0x55;
                FontColor.Padding = 0xFF;
            }
            else if (FramesPassed <= 200)
            {
                FontColor.Blue    = 0x99;
                FontColor.Green   = 0x99;
                FontColor.Red     = 0x99;
                FontColor.Padding = 0xFF;
            }
            else
            {
                FontColor.Blue    = 0xFF;
                FontColor.Green   = 0xFF;
                FontColor.Red     = 0xFF;
                FontColor.Padding = 0xFF;
            }

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

            FramesPassed++;
}

void ProcessGameMainMenuScreenInput(void)
{
    short UpKeyIsDown = GetAsyncKeyState('W');
    short DownKeyIsDown = GetAsyncKeyState('S');
    short EnterKeyIsDown = GetAsyncKeyState(VK_RETURN);

    static short EnterKeyWasDown;
    static short UpKeyWasDown;
    static short DownKeyWasDown;

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