#pragma once

#define GAME_NAME  "GameB"
// in this case we are picking these values because they are divisible by 16 because thats how many pixels our tilex will have
#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define BPP 32 // defines our bits per pixel
#define GAME_AREA_MEMORY_SIZE   (GAME_RES_WIDTH * GAME_RES_HEIGHT * (BPP/ 8))


void* memory_canvas;

typedef struct GAME_BIT_MAP
{
	BITMAPINFO BitMapInfo;
	void* memory_canvas;
} GAME_BIT_MAP;
// in order to begin drawing our pixels we will need to define what a pixel is...
//this type def is how the og 1992 devs did it but we will use a macro from a library to replace the unsigned chars into uint8_t
typedef struct PIXEL_32
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
} PIXEL_32;



LRESULT CALLBACK MainWindowProcedure(HWND, UINT, WPARAM, LPARAM);


DWORD create_main_window();


BOOL game_running_check (void);

void ProcessPlayerInput(void);
void RenderFrameGraphics(void);