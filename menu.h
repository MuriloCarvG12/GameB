//
// Created by Chine on 03/04/2026.
//

#ifndef UNTITLED7_MENU_H
#define UNTITLED7_MENU_H
#include <stdint.h>
#include <stdlib.h>

typedef struct MenuItem {
    char* ItemTitle;
    void (*action)(void);
    uint16_t X;
    uint16_t Y;
} MenuItem;

typedef struct Menu {
    char* MenuText;
    uint8_t SelectedItem;
    uint8_t ItemCount;
    MenuItem** Items;
} Menu;

void g_mi_ResumeGameAction(void){};
void g_mi_StartGameAction(void){};
void g_mi_SaveGameAction(void){};
void g_mi_OptionGameAction(void){};
void g_mi_CloseGameAction(void){};

MenuItem g_mi_ResumeGame = {"RESUME GAME", g_mi_ResumeGameAction, 32, 32};
MenuItem g_mi_StartGame  = {"START GAME",  g_mi_StartGameAction,  32, 48};
MenuItem g_mi_SaveGame   = {"SAVE GAME",   g_mi_SaveGameAction,   32, 64};
MenuItem g_mi_CloseGame  = {"CLOSE GAME",  g_mi_CloseGameAction,  32, 80};
MenuItem g_mi_OptionGame = {"OPTION GAME", g_mi_OptionGameAction, 32, 96};

MenuItem* g_mi_TitleScreenItems[] = { &g_mi_ResumeGame, &g_mi_StartGame, &g_mi_SaveGame, &g_mi_CloseGame, &g_mi_OptionGame };

Menu MainGameMenu = {"GAME B TITLE SCREEN", 0 , _countof(g_mi_TitleScreenItems), g_mi_TitleScreenItems};

#endif //UNTITLED7_MENU_H