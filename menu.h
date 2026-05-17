//
// Created by Chine on 03/04/2026.
//

#ifndef UNTITLED7_MENU_H
#define UNTITLED7_MENU_H
#include <stdint.h>
#include <stdlib.h>

#define IsActive TRUE
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

// TITLE SCREEN
void g_mi_ResumeGameAction(void);
void g_mi_StartGameAction(void);
void g_mi_SaveGameAction(void);
void g_mi_OptionGameAction(void);
void g_mi_CloseGameAction(void);

MenuItem g_mi_ResumeGame = {"RESUME GAME", g_mi_ResumeGameAction, IsActive ,32, 48};
MenuItem g_mi_StartGame  = {"START GAME",  g_mi_StartGameAction, IsInactive ,32, 48};
MenuItem g_mi_SaveGame   = {"SAVE GAME",   g_mi_SaveGameAction, IsActive   ,32, 60};
MenuItem g_mi_CloseGame  = {"CLOSE GAME",  g_mi_CloseGameAction, IsActive , 32, 72};
MenuItem g_mi_OptionGame = {"OPTION GAME", g_mi_OptionGameAction, IsActive ,32, 84};


MenuItem* g_mi_TitleScreenItems[] = { &g_mi_ResumeGame, &g_mi_StartGame, &g_mi_SaveGame, &g_mi_CloseGame, &g_mi_OptionGame };

Menu MainGameMenu = {"GAME B TITLE SCREEN", 0 , _countof(g_mi_TitleScreenItems), g_mi_TitleScreenItems};

// EXIT SCREEN
void g_mi_ExitGameAction(void);
void g_mi_DontExitGameAction(void);

MenuItem g_mi_ExitYes = {"YES", g_mi_ExitGameAction, IsActive,32, 32};
MenuItem g_mi_ExitNo  = {"NO",  g_mi_DontExitGameAction, IsActive, 32, 48};

MenuItem* g_mi_ExitYesOrNoScreenItems[] = { &g_mi_ExitYes, &g_mi_ExitNo };

Menu g_ExitYesOrNoMenu = {"ARE YOU SURE YOU WANT TO LEAVE?", 0, _countof(g_mi_ExitYesOrNoScreenItems), g_mi_ExitYesOrNoScreenItems};

// OPTIONS SCREEN

void g_mi_OptionsSoundLevelAction(void);
void g_mi_MusicSoundLevelAction(void);
void g_mi_ScreenResolutionAction(void);
void g_mi_OptionsBackAction(void);

MenuItem g_mi_OptionsSoundLevel = {"SOUND EFFECTS VOLUME", g_mi_OptionsSoundLevelAction, IsActive, 32, 48};
MenuItem g_mi_MusicSoundLevel   = {"MUSIC SOUND VOLUME", g_mi_MusicSoundLevelAction, IsActive, 32 , 60};
MenuItem g_mi_ScreenResolution  = {"RESOLUTION", g_mi_ScreenResolutionAction, IsActive, 32, 72};
MenuItem g_mi_OptionsBack  = {"BACK", g_mi_OptionsBackAction, IsActive, 32, 84};

MenuItem* g_mi_OptionsScreenItems[] = { &g_mi_OptionsSoundLevel, &g_mi_MusicSoundLevel, &g_mi_ScreenResolution, &g_mi_OptionsBack};

Menu g_mi_OptionsMenu = {"OPTIONS", 0, _countof(g_mi_OptionsScreenItems), g_mi_OptionsScreenItems};


#endif //UNTITLED7_MENU_H