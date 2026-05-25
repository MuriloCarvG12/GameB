//
// Created by Chine on 03/04/2026.
//

#ifndef UNTITLED7_MENU_H
#define UNTITLED7_MENU_H
#include <stdint.h>
#include <stdlib.h>

#include "projectdeclarations.h"

#define IsActive   TRUE
#define IsInactive FALSE

typedef struct MenuItem {
    char* ItemTitle;
    void (*action)(void);
    boolean ItemIsActive;
    uint16_t X;
    uint16_t Y;
} MenuItem;

typedef struct Menu {
    char* MenuText;
    uint8_t SelectedItem;
    uint8_t ItemCount;
    MenuItem** Items;
} Menu;

// -------------------------------------------------------
// ACTION FUNCTION DECLARATIONS
// -------------------------------------------------------

// Title screen
void g_mi_ResumeGameAction(void);
void g_mi_StartGameAction(void);
void g_mi_SaveGameAction(void);
void g_mi_OptionGameAction(void);
void g_mi_CloseGameAction(void);

// Exit screen
void g_mi_ExitGameAction(void);
void g_mi_DontExitGameAction(void);

// Options screen
void g_mi_OptionsSoundLevelAction(void);
void g_mi_MusicSoundLevelAction(void);
void g_mi_ScreenResolutionAction(void);
void g_mi_OptionsBackAction(void);

// Character naming screen
void g_mi_CharacterNameAddChar(void);
void g_mi_CharacterNameBack(void);
void g_mi_CharacterNameConfirmName(void);

// -------------------------------------------------------
// MENU ITEM DECLARATIONS  (defined in menu.c)
// -------------------------------------------------------

// Title screen items
extern MenuItem g_mi_ResumeGame;
extern MenuItem g_mi_StartGame;
extern MenuItem g_mi_SaveGame;
extern MenuItem g_mi_CloseGame;
extern MenuItem g_mi_OptionGame;
extern MenuItem* g_mi_TitleScreenItems[];
extern Menu MainGameMenu;

// Exit screen items
extern MenuItem g_mi_ExitYes;
extern MenuItem g_mi_ExitNo;
extern MenuItem* g_mi_ExitYesOrNoScreenItems[];
extern Menu g_ExitYesOrNoMenu;

// Options screen items
extern MenuItem g_mi_OptionsSoundLevel;
extern MenuItem g_mi_MusicSoundLevel;
extern MenuItem g_mi_ScreenResolution;
extern MenuItem g_mi_OptionsBack;
extern MenuItem* g_mi_OptionsScreenItems[];
extern Menu g_mi_OptionsMenu;

// Character naming screen items — uppercase A-M
extern MenuItem g_mi_CharacterNamingScreenCharA;
extern MenuItem g_mi_CharacterNamingScreenCharB;
extern MenuItem g_mi_CharacterNamingScreenCharC;
extern MenuItem g_mi_CharacterNamingScreenCharD;
extern MenuItem g_mi_CharacterNamingScreenCharE;
extern MenuItem g_mi_CharacterNamingScreenCharF;
extern MenuItem g_mi_CharacterNamingScreenCharG;
extern MenuItem g_mi_CharacterNamingScreenCharH;
extern MenuItem g_mi_CharacterNamingScreenCharI;
extern MenuItem g_mi_CharacterNamingScreenCharJ;
extern MenuItem g_mi_CharacterNamingScreenCharK;
extern MenuItem g_mi_CharacterNamingScreenCharL;
extern MenuItem g_mi_CharacterNamingScreenCharM;

// Character naming screen items — uppercase N-Z
extern MenuItem g_mi_CharacterNamingScreenCharN;
extern MenuItem g_mi_CharacterNamingScreenCharO;
extern MenuItem g_mi_CharacterNamingScreenCharP;
extern MenuItem g_mi_CharacterNamingScreenCharQ;
extern MenuItem g_mi_CharacterNamingScreenCharR;
extern MenuItem g_mi_CharacterNamingScreenCharS;
extern MenuItem g_mi_CharacterNamingScreenCharT;
extern MenuItem g_mi_CharacterNamingScreenCharU;
extern MenuItem g_mi_CharacterNamingScreenCharV;
extern MenuItem g_mi_CharacterNamingScreenCharW;
extern MenuItem g_mi_CharacterNamingScreenCharX;
extern MenuItem g_mi_CharacterNamingScreenCharY;
extern MenuItem g_mi_CharacterNamingScreenCharZ;

// Character naming screen items — lowercase a-m
extern MenuItem g_mi_CharacterNamingScreenChara;
extern MenuItem g_mi_CharacterNamingScreenCharb;
extern MenuItem g_mi_CharacterNamingScreenCharc;
extern MenuItem g_mi_CharacterNamingScreenChard;
extern MenuItem g_mi_CharacterNamingScreenChare;
extern MenuItem g_mi_CharacterNamingScreenCharf;
extern MenuItem g_mi_CharacterNamingScreenCharg;
extern MenuItem g_mi_CharacterNamingScreenCharh;
extern MenuItem g_mi_CharacterNamingScreenChari;
extern MenuItem g_mi_CharacterNamingScreenCharj;
extern MenuItem g_mi_CharacterNamingScreenChark;
extern MenuItem g_mi_CharacterNamingScreenCharl;
extern MenuItem g_mi_CharacterNamingScreenCharm;

// Character naming screen items — lowercase n-z
extern MenuItem g_mi_CharacterNamingScreenCharn;
extern MenuItem g_mi_CharacterNamingScreenCharo;
extern MenuItem g_mi_CharacterNamingScreenCharp;
extern MenuItem g_mi_CharacterNamingScreenCharq;
extern MenuItem g_mi_CharacterNamingScreenCharr;
extern MenuItem g_mi_CharacterNamingScreenChars;
extern MenuItem g_mi_CharacterNamingScreenChart;
extern MenuItem g_mi_CharacterNamingScreenCharu;
extern MenuItem g_mi_CharacterNamingScreenCharv;
extern MenuItem g_mi_CharacterNamingScreenCharw;
extern MenuItem g_mi_CharacterNamingScreenCharx;
extern MenuItem g_mi_CharacterNamingScreenChary;
extern MenuItem g_mi_CharacterNamingScreenCharz;

// Character naming screen — buttons and menu
extern MenuItem g_mi_CharacterNamingScreenBackButton;
extern MenuItem g_mi_CharacterNamingScreenConfirmNameButton;
extern MenuItem* g_mi_CharacterNamingScreenItems[];
extern Menu g_mi_CharacterNamingScreenMenu;


#endif //UNTITLED7_MENU_H