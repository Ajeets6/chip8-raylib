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
	uint8_t  drawFlag;
	uint16_t keypad;

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

void UpdateChip8Keys(void)
{
    chip8.keypad = 0;

    if (IsKeyDown(KEY_X)) chip8.keypad |= (1 << 0x0);
    if (IsKeyDown(KEY_ONE)) chip8.keypad |= (1 << 0x1);
    if (IsKeyDown(KEY_TWO)) chip8.keypad |= (1 << 0x2);
    if (IsKeyDown(KEY_THREE)) chip8.keypad |= (1 << 0x3);
    if (IsKeyDown(KEY_Q)) chip8.keypad |= (1 << 0x4);
    if (IsKeyDown(KEY_W)) chip8.keypad |= (1 << 0x5);
    if (IsKeyDown(KEY_E)) chip8.keypad |= (1 << 0x6);
    if (IsKeyDown(KEY_A)) chip8.keypad |= (1 << 0x7);
    if (IsKeyDown(KEY_S)) chip8.keypad |= (1 << 0x8);
    if (IsKeyDown(KEY_D)) chip8.keypad |= (1 << 0x9);
    if (IsKeyDown(KEY_Z)) chip8.keypad |= (1 << 0xA);
    if (IsKeyDown(KEY_C)) chip8.keypad |= (1 << 0xB);
    if (IsKeyDown(KEY_FOUR)) chip8.keypad |= (1 << 0xC);
    if (IsKeyDown(KEY_R)) chip8.keypad |= (1 << 0xD);
    if (IsKeyDown(KEY_F)) chip8.keypad |= (1 << 0xE);
    if (IsKeyDown(KEY_V)) chip8.keypad |= (1 << 0xF);
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

void updateTimers(Sound Wav){
	if(chip8.delay_timer>0){
		chip8.delay_timer--;
	}
	if(chip8.sound_timer>0){
		if(chip8.sound_timer==1){
			PlaySound(Wav); // Beep sound
		}
		chip8.sound_timer--;
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
					chip8.drawFlag = 1;
					break;
				case 0x00EE: // RET
					chip8.sp--;
					chip8.pc=chip8.stack[chip8.sp];
					break;
				default:
					printf("Unknown opcode: 0x%X\n", opcode);
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
		chip8.V[X]+=kk;
        break;
    case 0x8000:
        switch (opcode & 0xF) {
            case 0x0:
				chip8.V[X]=chip8.V[Y];
                break;
            case 0x1:
				chip8.V[X]|=chip8.V[Y];
                break;
            case 0x2:
				chip8.V[X] &= chip8.V[Y];
                break;
            case 0x3:
				chip8.V[X] ^=chip8.V[Y];
                break;
            case 0x4:
				uint16_t sum=chip8.V[X]+chip8.V[Y];
				chip8.V[0xF]=(sum>0xFF)?1:0;
				chip8.V[X]=sum&0xFF;
                break;
            case 0x5:
				chip8.V[0xF]=(chip8.V[X]>chip8.V[Y])?1:0;
				chip8.V[X]-=chip8.V[Y];
                break;
            case 0x6:
				if(chip8.V[X]&0x1){
					chip8.V[0xF]=1;
				}else{
					chip8.V[0xF]=0;
				}
				chip8.V[X]>>=1;

                break;
            case 0x7:
				chip8.V[0xF]=(chip8.V[Y]>chip8.V[X])?1:0;
				chip8.V[X]=chip8.V[Y]-chip8.V[X];
                break;
            case 0xE:
				if((chip8.V[X] & 0x80) !=0){
					chip8.V[0xF]=1;
				}else{
					chip8.V[0xF]=0;
				}
				chip8.V[X]<<=1;
                break;
			default:
				printf("Unknown opcode: 0x%X\n", opcode);
				break;
        }

        break;
    case 0x9000:
		if(chip8.V[X]!=chip8.V[Y]){
			chip8.pc+=2;
		}
        break;
    case 0xA000:
		chip8.I=nnn;
        break;
    case 0xB000:
		chip8.pc=nnn+chip8.V[0];
        break;
    case 0xC000:
		int randValue = GetRandomValue(0, 255);
		chip8.V[X]=randValue & kk;
        break;
    case 0xD000:
		{
			uint8_t x=chip8.V[X]%64;
			uint8_t y=chip8.V[Y]%32;
			uint8_t width=8;
			uint8_t height=n;
			chip8.V[0xF]=0;
			for(int row=0;row<height;row++){
				uint8_t spriteByte=chip8.memory[chip8.I+row];
				for(int col=0;col<width;col++){
					if((spriteByte & (0x80>>col))!=0){
						uint16_t displayIndex=(x+col+(y+row)*64);
						if(chip8.display[displayIndex]){
							chip8.V[0xF]=1;
						}
						chip8.display[displayIndex]^=1;
					}
				}
			}
			chip8.drawFlag=1;
		}

        break;
    case 0xE000:
        switch (opcode & 0xFF) {
            case 0x9E:
				if (chip8.keypad & (1 << chip8.V[X])) {
                chip8.pc += 2;
            	}
                break;

            case 0xA1:
				if (!(chip8.keypad & (1 << chip8.V[X]))) {
				chip8.pc += 2;
			}
                break;
			default:
				printf("Unknown opcode: 0x%X\n", opcode);
				break;
        }

        break;
    case 0xF000:
        switch (opcode & 0xFF) {
            case 0x07:
				chip8.V[X]=chip8.delay_timer;
                break;
            case 0x0A:

				bool keyPressed = false;

				for (uint8_t k = 0; k < 16; k++) {
					if (chip8.keypad & (1 << k)) {
						chip8.V[X] = k;
						keyPressed = true;
						break;
					}
				}

				if (!keyPressed) {
					chip8.pc -= 2;   // stall
				}
				break;
            case 0x15:
				chip8.delay_timer=chip8.V[X];
                break;
            case 0x18:
				chip8.sound_timer=chip8.V[X];
                break;
            case 0x1E:
				chip8.I+=chip8.V[X];
                break;
            case 0x29:
				chip8.I=chip8.V[X]*5;
                break;
            case 0x33:
				chip8.memory[chip8.I]=chip8.V[X]/100;
				chip8.memory[chip8.I+1]=(chip8.V[X]/10)%10;
				chip8.memory[chip8.I+2]=chip8.V[X]%10;
                break;
            case 0x55:
				for(int index=0;index<=X;index++){
					chip8.memory[chip8.I+index]=chip8.V[index];
				}
                break;
            case 0x65:
				for(int index=0;index<=X;index++){
					chip8.V[index]=chip8.memory[chip8.I+index];
				}
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
	//SetExitKey(0);
	InitWindow(1280, 800, "Chip8");
	SearchAndSetResourceDir("resources");
	InitAudioDevice();
    Sound Wav = LoadSound("BEEP2.wav");
	initalize();
	if(loadROM("roms\\PONG")){
		return 1;
	}
while (!WindowShouldClose())
{
    // Run CPU cycles
    for (int i = 0; i < 10; i++) {
        execute();
    }

    BeginDrawing();
    ClearBackground(BLACK);   // ALWAYS clear ONCE per frame
	updateTimers(Wav);
	UpdateChip8Keys();
    drawDisplay();            // ALWAYS draw framebuffers
    EndDrawing();
}
	UnloadSound(Wav);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}
