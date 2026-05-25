//
// Created by Chine on 03/04/2026.
//

//
// menu.c — definitions for all menu items and menus.
// This file must be compiled and linked once.
// All other .c files should only #include "menu.h".
//

#include "menu.h"

// -------------------------------------------------------
// TITLE SCREEN
// -------------------------------------------------------

MenuItem g_mi_ResumeGame = {"RESUME GAME", g_mi_ResumeGameAction, IsActive,   32, 48};
MenuItem g_mi_StartGame  = {"START GAME",  g_mi_StartGameAction,  IsInactive, 32, 48};
MenuItem g_mi_SaveGame   = {"SAVE GAME",   g_mi_SaveGameAction,   IsActive,   32, 60};
MenuItem g_mi_CloseGame  = {"CLOSE GAME",  g_mi_CloseGameAction,  IsActive,   32, 72};
MenuItem g_mi_OptionGame = {"OPTION GAME", g_mi_OptionGameAction, IsActive,   32, 84};

MenuItem* g_mi_TitleScreenItems[] = {
    &g_mi_ResumeGame, &g_mi_StartGame, &g_mi_SaveGame,
    &g_mi_CloseGame, &g_mi_OptionGame
};

Menu MainGameMenu = {
    "GAME B TITLE SCREEN", 0,
    _countof(g_mi_TitleScreenItems),
    g_mi_TitleScreenItems
};

// -------------------------------------------------------
// EXIT SCREEN
// -------------------------------------------------------

MenuItem g_mi_ExitYes = {"YES", g_mi_ExitGameAction,     IsActive, 32, 32};
MenuItem g_mi_ExitNo  = {"NO",  g_mi_DontExitGameAction, IsActive, 32, 48};

MenuItem* g_mi_ExitYesOrNoScreenItems[] = { &g_mi_ExitYes, &g_mi_ExitNo };

Menu g_ExitYesOrNoMenu = {
    "ARE YOU SURE YOU WANT TO LEAVE?", 0,
    _countof(g_mi_ExitYesOrNoScreenItems),
    g_mi_ExitYesOrNoScreenItems
};

// -------------------------------------------------------
// OPTIONS SCREEN
// -------------------------------------------------------

MenuItem g_mi_OptionsSoundLevel = {"SOUND EFFECTS VOLUME", g_mi_OptionsSoundLevelAction, IsActive, 32, 48};
MenuItem g_mi_MusicSoundLevel   = {"MUSIC SOUND VOLUME",   g_mi_MusicSoundLevelAction,   IsActive, 32, 60};
MenuItem g_mi_ScreenResolution  = {"RESOLUTION",           g_mi_ScreenResolutionAction,  IsActive, 32, 72};
MenuItem g_mi_OptionsBack       = {"BACK",                 g_mi_OptionsBackAction,        IsActive, 32, 84};

MenuItem* g_mi_OptionsScreenItems[] = {
    &g_mi_OptionsSoundLevel, &g_mi_MusicSoundLevel,
    &g_mi_ScreenResolution,  &g_mi_OptionsBack
};

Menu g_mi_OptionsMenu = {
    "OPTIONS", 0,
    _countof(g_mi_OptionsScreenItems),
    g_mi_OptionsScreenItems
};

// -------------------------------------------------------
// CHARACTER NAMING SCREEN — uppercase A-M  (y=90)
// -------------------------------------------------------

MenuItem g_mi_CharacterNamingScreenCharA = {"A", g_mi_CharacterNameAddChar, IsActive, 108, 90};
MenuItem g_mi_CharacterNamingScreenCharB = {"B", g_mi_CharacterNameAddChar, IsActive, 122, 90};
MenuItem g_mi_CharacterNamingScreenCharC = {"C", g_mi_CharacterNameAddChar, IsActive, 136, 90};
MenuItem g_mi_CharacterNamingScreenCharD = {"D", g_mi_CharacterNameAddChar, IsActive, 150, 90};
MenuItem g_mi_CharacterNamingScreenCharE = {"E", g_mi_CharacterNameAddChar, IsActive, 164, 90};
MenuItem g_mi_CharacterNamingScreenCharF = {"F", g_mi_CharacterNameAddChar, IsActive, 178, 90};
MenuItem g_mi_CharacterNamingScreenCharG = {"G", g_mi_CharacterNameAddChar, IsActive, 192, 90};
MenuItem g_mi_CharacterNamingScreenCharH = {"H", g_mi_CharacterNameAddChar, IsActive, 206, 90};
MenuItem g_mi_CharacterNamingScreenCharI = {"I", g_mi_CharacterNameAddChar, IsActive, 220, 90};
MenuItem g_mi_CharacterNamingScreenCharJ = {"J", g_mi_CharacterNameAddChar, IsActive, 234, 90};
MenuItem g_mi_CharacterNamingScreenCharK = {"K", g_mi_CharacterNameAddChar, IsActive, 248, 90};
MenuItem g_mi_CharacterNamingScreenCharL = {"L", g_mi_CharacterNameAddChar, IsActive, 262, 90};
MenuItem g_mi_CharacterNamingScreenCharM = {"M", g_mi_CharacterNameAddChar, IsActive, 276, 90};

// uppercase N-Z  (y=104)
MenuItem g_mi_CharacterNamingScreenCharN = {"N", g_mi_CharacterNameAddChar, IsActive, 108, 104};
MenuItem g_mi_CharacterNamingScreenCharO = {"O", g_mi_CharacterNameAddChar, IsActive, 122, 104};
MenuItem g_mi_CharacterNamingScreenCharP = {"P", g_mi_CharacterNameAddChar, IsActive, 136, 104};
MenuItem g_mi_CharacterNamingScreenCharQ = {"Q", g_mi_CharacterNameAddChar, IsActive, 150, 104};
MenuItem g_mi_CharacterNamingScreenCharR = {"R", g_mi_CharacterNameAddChar, IsActive, 164, 104};
MenuItem g_mi_CharacterNamingScreenCharS = {"S", g_mi_CharacterNameAddChar, IsActive, 178, 104};
MenuItem g_mi_CharacterNamingScreenCharT = {"T", g_mi_CharacterNameAddChar, IsActive, 192, 104};
MenuItem g_mi_CharacterNamingScreenCharU = {"U", g_mi_CharacterNameAddChar, IsActive, 206, 104};
MenuItem g_mi_CharacterNamingScreenCharV = {"V", g_mi_CharacterNameAddChar, IsActive, 220, 104};
MenuItem g_mi_CharacterNamingScreenCharW = {"W", g_mi_CharacterNameAddChar, IsActive, 234, 104};
MenuItem g_mi_CharacterNamingScreenCharX = {"X", g_mi_CharacterNameAddChar, IsActive, 248, 104};
MenuItem g_mi_CharacterNamingScreenCharY = {"Y", g_mi_CharacterNameAddChar, IsActive, 262, 104};
MenuItem g_mi_CharacterNamingScreenCharZ = {"Z", g_mi_CharacterNameAddChar, IsActive, 276, 104};

// lowercase a-m  (y=118)
MenuItem g_mi_CharacterNamingScreenChara = {"a", g_mi_CharacterNameAddChar, IsActive, 108, 118};
MenuItem g_mi_CharacterNamingScreenCharb = {"b", g_mi_CharacterNameAddChar, IsActive, 122, 118};
MenuItem g_mi_CharacterNamingScreenCharc = {"c", g_mi_CharacterNameAddChar, IsActive, 136, 118};
MenuItem g_mi_CharacterNamingScreenChard = {"d", g_mi_CharacterNameAddChar, IsActive, 150, 118};
MenuItem g_mi_CharacterNamingScreenChare = {"e", g_mi_CharacterNameAddChar, IsActive, 164, 118};
MenuItem g_mi_CharacterNamingScreenCharf = {"f", g_mi_CharacterNameAddChar, IsActive, 178, 118};
MenuItem g_mi_CharacterNamingScreenCharg = {"g", g_mi_CharacterNameAddChar, IsActive, 192, 118};
MenuItem g_mi_CharacterNamingScreenCharh = {"h", g_mi_CharacterNameAddChar, IsActive, 206, 118};
MenuItem g_mi_CharacterNamingScreenChari = {"i", g_mi_CharacterNameAddChar, IsActive, 220, 118};
MenuItem g_mi_CharacterNamingScreenCharj = {"j", g_mi_CharacterNameAddChar, IsActive, 234, 118};
MenuItem g_mi_CharacterNamingScreenChark = {"k", g_mi_CharacterNameAddChar, IsActive, 248, 118};
MenuItem g_mi_CharacterNamingScreenCharl = {"l", g_mi_CharacterNameAddChar, IsActive, 262, 118};
MenuItem g_mi_CharacterNamingScreenCharm = {"m", g_mi_CharacterNameAddChar, IsActive, 276, 118};

// lowercase n-z  (y=132)
MenuItem g_mi_CharacterNamingScreenCharn = {"n", g_mi_CharacterNameAddChar, IsActive, 108, 132};
MenuItem g_mi_CharacterNamingScreenCharo = {"o", g_mi_CharacterNameAddChar, IsActive, 122, 132};
MenuItem g_mi_CharacterNamingScreenCharp = {"p", g_mi_CharacterNameAddChar, IsActive, 136, 132};
MenuItem g_mi_CharacterNamingScreenCharq = {"q", g_mi_CharacterNameAddChar, IsActive, 150, 132};
MenuItem g_mi_CharacterNamingScreenCharr = {"r", g_mi_CharacterNameAddChar, IsActive, 164, 132};
MenuItem g_mi_CharacterNamingScreenChars = {"s", g_mi_CharacterNameAddChar, IsActive, 178, 132};
MenuItem g_mi_CharacterNamingScreenChart = {"t", g_mi_CharacterNameAddChar, IsActive, 192, 132};
MenuItem g_mi_CharacterNamingScreenCharu = {"u", g_mi_CharacterNameAddChar, IsActive, 206, 132};
MenuItem g_mi_CharacterNamingScreenCharv = {"v", g_mi_CharacterNameAddChar, IsActive, 220, 132};
MenuItem g_mi_CharacterNamingScreenCharw = {"w", g_mi_CharacterNameAddChar, IsActive, 234, 132};
MenuItem g_mi_CharacterNamingScreenCharx = {"x", g_mi_CharacterNameAddChar, IsActive, 248, 132};
MenuItem g_mi_CharacterNamingScreenChary = {"y", g_mi_CharacterNameAddChar, IsActive, 262, 132};
MenuItem g_mi_CharacterNamingScreenCharz = {"z", g_mi_CharacterNameAddChar, IsActive, 276, 132};

// Buttons
MenuItem g_mi_CharacterNamingScreenBackButton        = {"BACK",         g_mi_CharacterNameBack,        IsActive, 108, 146};
MenuItem g_mi_CharacterNamingScreenConfirmNameButton = {"CONFIRM NAME", g_mi_CharacterNameConfirmName, IsActive, 212, 146};

// Item array
MenuItem* g_mi_CharacterNamingScreenItems[] = {
    &g_mi_CharacterNamingScreenCharA, &g_mi_CharacterNamingScreenCharB, &g_mi_CharacterNamingScreenCharC, &g_mi_CharacterNamingScreenCharD,
    &g_mi_CharacterNamingScreenCharE, &g_mi_CharacterNamingScreenCharF, &g_mi_CharacterNamingScreenCharG, &g_mi_CharacterNamingScreenCharH,
    &g_mi_CharacterNamingScreenCharI, &g_mi_CharacterNamingScreenCharJ, &g_mi_CharacterNamingScreenCharK, &g_mi_CharacterNamingScreenCharL,
    &g_mi_CharacterNamingScreenCharM, &g_mi_CharacterNamingScreenCharN, &g_mi_CharacterNamingScreenCharO, &g_mi_CharacterNamingScreenCharP,
    &g_mi_CharacterNamingScreenCharQ, &g_mi_CharacterNamingScreenCharR, &g_mi_CharacterNamingScreenCharS, &g_mi_CharacterNamingScreenCharT,
    &g_mi_CharacterNamingScreenCharU, &g_mi_CharacterNamingScreenCharV, &g_mi_CharacterNamingScreenCharW, &g_mi_CharacterNamingScreenCharX,
    &g_mi_CharacterNamingScreenCharY, &g_mi_CharacterNamingScreenCharZ, &g_mi_CharacterNamingScreenChara, &g_mi_CharacterNamingScreenCharb,
    &g_mi_CharacterNamingScreenCharc, &g_mi_CharacterNamingScreenChard, &g_mi_CharacterNamingScreenChare, &g_mi_CharacterNamingScreenCharf,
    &g_mi_CharacterNamingScreenCharg, &g_mi_CharacterNamingScreenCharh, &g_mi_CharacterNamingScreenChari, &g_mi_CharacterNamingScreenCharj,
    &g_mi_CharacterNamingScreenChark, &g_mi_CharacterNamingScreenCharl, &g_mi_CharacterNamingScreenCharm, &g_mi_CharacterNamingScreenCharn,
    &g_mi_CharacterNamingScreenCharo, &g_mi_CharacterNamingScreenCharp, &g_mi_CharacterNamingScreenCharq, &g_mi_CharacterNamingScreenCharr,
    &g_mi_CharacterNamingScreenChars, &g_mi_CharacterNamingScreenChart, &g_mi_CharacterNamingScreenCharu, &g_mi_CharacterNamingScreenCharv,
    &g_mi_CharacterNamingScreenCharw, &g_mi_CharacterNamingScreenCharx, &g_mi_CharacterNamingScreenChary, &g_mi_CharacterNamingScreenCharz,
    &g_mi_CharacterNamingScreenBackButton, &g_mi_CharacterNamingScreenConfirmNameButton
};

Menu g_mi_CharacterNamingScreenMenu = {
    "What will be your name, hero?", 0,
    _countof(g_mi_CharacterNamingScreenItems),
    g_mi_CharacterNamingScreenItems
};