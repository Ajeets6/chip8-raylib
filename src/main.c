

#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <stdio.h>
#include <stdint.h>


struct Chip8{
	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t I;
	uint16_t stack[16];
	uint16_t pc;
	bool display[64*32];




};


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
