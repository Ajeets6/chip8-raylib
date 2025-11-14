

#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <stdio.h>
#include <stdint.h>
#include<string.h>


struct Chip8{
	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t I;
	uint16_t stack[16];
	uint16_t pc;
	bool display[64*32];
	uint16_t sp;
	uint8_t delay_timer;
	uint8_t sound_timer;

};
struct Chip8 chip8;
void initalize(){

	chip8.pc=0x200;
	chip8.I=0;
	chip8.sp=0;
	chip8.delay_timer=0;
	chip8.sound_timer=0;

	memset(chip8.display,0,4096);
	memset(chip8.memory,0,4096);
	memset(chip8.stack,0,16);



uint8_t fonts[80] ={
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
for(int i=0;i<80;i++){
	chip8.memory[i]=fonts[i];
	}
}

int main ()
{

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	SetExitKey(0);


	InitWindow(1280, 800, "Chip8");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");



	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// draw some text using the default font
		DrawText("Hello Raylib", 200,200,20,WHITE);


		EndDrawing();
	}


	CloseWindow();
	return 0;
}
