cmake_minimum_required(VERSION 3.27)
project(untitled7 C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled7 main.c
        projectdeclarations.c
        projectdeclarations.h
        menu.c
        menu.h
        GameOpeningSplashScreen.c
        GameOpeningSplashScreen.h
        GameMainMenuScreen.c
        GameMainMenuScreen.h
        GameYesOrNoExitMenu.c
        GameYesOrNoExitMenu.h
        GameOptionsScreen.c
        GameOptionsScreen.h
        GameCharacterNamingScreen.c
        GameCharacterNamingScreen.h
        GameOverworldScreen.c
        GameOverworldScreen.h
        stb_vorbis.c
        stb_vorbis.h
)

target_link_libraries(untitled7 PRIVATE
        winmm
        ole32
        uuid
)

