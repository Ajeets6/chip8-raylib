#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <stdio.h>
#include <stdint.h>
#include <string.h>


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

	memset(chip8.display,0,64*32);
	memset(chip8.memory,0,4096);
	memset(chip8.stack,0,16);
	memset(chip8.V,0,16);

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

int loadROM(const char* filename){
	FILE* rom=fopen(filename,"rb");
	if(rom==NULL){
		printf("Failed to open ROM\n");
		return 1;
	}
	fseek(rom,0,SEEK_END);
	long romSize=ftell(rom);
	rewind(rom);

	size_t result=fread(&chip8.memory[0x200],1,romSize,rom);
	if(result!=romSize){
		printf("Failed to read ROM\n");
		return 1;
	}
	fclose(rom);
	return 0;
}

void drawDisplay(){
	for(int y=0;y<32;y++){
		for(int x=0;x<64;x++){
			if(chip8.display[x+y*64]){
				DrawRectangle(x*10,y*10,10,10,WHITE);
			}
		}
	}
}

void execute(){
	uint16_t opcode = chip8.memory[chip8.pc] << 8 | chip8.memory[chip8.pc + 1];
	chip8.pc += 2;


	uint8_t  X   = (opcode >> 8) & 0x0F;
	uint8_t  Y   = (opcode >> 4) & 0x0F;
	uint8_t  n   =  opcode       & 0x0F;
	uint8_t  kk  =  opcode       & 0xFF;
	uint16_t nnn =  opcode       & 0x0FFF;


	switch (opcode & 0xF000) {
    case 0x0000:
        switch(opcode & 0x00FF){
				case 0x00E0: // CLS
					memset(chip8.display,0,64*32);
					break;
				case 0x00EE: // RET
					chip8.sp--;
					chip8.pc=chip8.stack[chip8.sp];
					break;
			}
			break;
    case 0x1000:
		chip8.pc=nnn;
        break;
    case 0x2000:
		chip8.sp++;
		chip8.stack[chip8.sp-1]=chip8.pc;
		chip8.pc=nnn;
        break;
    case 0x3000:
		if(chip8.V[X]==kk){
			chip8.pc+=2;
		}
        break;
    case 0x4000:
		if(chip8.V[X]!=kk){
			chip8.pc+=2;
		}
        break;
    case 0x5000:
		if(chip8.V[X]==chip8.V[Y]){
			chip8.pc+=2;
		}
        break;
    case 0x6000:
		chip8.V[X]=kk;
        break;
    case 0x7000:
		chip8.V[X]=chip8.V[X]+kk;
        break;
    case 0x8000:
        switch (opcode & 0xF) {
            case 0x0:
                break;
            case 0x1:
                break;
            case 0x2:
                break;
            case 0x3:
                break;
            case 0x4:
                break;
            case 0x5:
                break;
            case 0x6:
                break;
            case 0x7:
                break;
            case 0xE:
                break;
        }

        break;
    case 0x9000:
        break;
    case 0xA000:
        break;
    case 0xB000:
        break;
    case 0xC000:
        break;
    case 0xD000:
        break;
    case 0xE000:
        switch (opcode & 0xFF) {
            case 0x9E:
                break;
            case 0xA1:
                break;
        }

        break;
    case 0xF000:
        switch (opcode & 0xFF) {
            case 0x07:
                break;
            case 0x0A:
                break;
            case 0x15:
                break;
            case 0x18:
                break;
            case 0x1E:
                break;
            case 0x29:
                break;
            case 0x33:
                break;
            case 0x55:
                break;
            case 0x65:
                break;
        }

	break;
	default:
		printf("Unknown opcode: 0x%X\n", opcode);
		break;
	}
}
int main ()
{

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	SetExitKey(0);
	InitWindow(1280, 800, "Chip8");
	SearchAndSetResourceDir("resources");
	initalize();
	if(loadROM("IBMLogo.ch8")){
		return 1;
	}
	while (!WindowShouldClose())
	{

		BeginDrawing();
		ClearBackground(BLACK);


		EndDrawing();
	}


	CloseWindow();
	return 0;
}
