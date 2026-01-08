#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
//#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif


#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 800
#define CHIP_SCALE 10
#define CHIP_W     (64 * CHIP_SCALE)
#define CHIP_H     (32 * CHIP_SCALE)
#define MENU_HEIGHT 40
#define UI_OFFSET_X 10
#define UI_OFFSET_Y (10 + MENU_HEIGHT)
#define PANEL_SPACING 10
#define SIDE_PANEL_WIDTH 230
#define MAX_ROMS 64


Sound Wav;

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
	memset(chip8.stack,0,sizeof(chip8.stack));
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

void drawDisplay(void)
{
	int xOffset = UI_OFFSET_X + 4;
	int yOffset = UI_OFFSET_Y + 4;

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip8.display[x + y * 64]) {
                DrawRectangle(
                    xOffset + x * CHIP_SCALE,
                    yOffset + y * CHIP_SCALE,
                    CHIP_SCALE,
                    CHIP_SCALE,
                    WHITE
                );
            }
        }
    }
}

void updateTimers()
{

    static double lastTick = 0.0;
    double now = GetTime();

    if (now - lastTick >= (1.0 / 60.0)) {

        if (chip8.delay_timer > 0) {
            chip8.delay_timer--;
        }

        if (chip8.sound_timer > 0) {
            if (chip8.sound_timer == 1) {
                PlaySound(Wav);
            }
            chip8.sound_timer--;
        }

        lastTick += (1.0 / 60.0);
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
				case 0x00E0:{ // CLS
					memset(chip8.display,0,64*32);
					chip8.drawFlag = 1;
					break;}
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
            case 0x4:{
				uint16_t sum=chip8.V[X]+chip8.V[Y];
				chip8.V[0xF]=(sum>0xFF)?1:0;
				chip8.V[X]=sum&0xFF;
                break;
			}
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
    case 0xC000:{
		int randValue = GetRandomValue(0, 255);
		chip8.V[X]=randValue & kk;
        break;
	}
    case 0xD000:
		{
			uint8_t x = chip8.V[X] % 64;
			uint8_t y = chip8.V[Y] % 32;
			uint8_t height = n;
			chip8.V[0xF] = 0;
			for (int row = 0; row < height; row++) {
				uint16_t spriteAddr = chip8.I + row;
				if (spriteAddr >= sizeof(chip8.memory)) break;
				uint8_t spriteByte = chip8.memory[spriteAddr];
				for (int col = 0; col < 8; col++) {
					if ((spriteByte & (0x80 >> col)) == 0) continue;
					uint8_t pixelX = (x + col) % 64;
					uint8_t pixelY = (y + row) % 32;
					uint16_t displayIndex = pixelX + pixelY * 64;
					if (chip8.display[displayIndex]) {
						chip8.V[0xF] = 1;
					}
					chip8.display[displayIndex] ^= 1;
				}
			}
			chip8.drawFlag = 1;
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
            case 0x0A:{

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
			}
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
				chip8.I += X + 1;
                break;
            case 0x65:
				for(int index=0;index<=X;index++){
					chip8.V[index]=chip8.memory[chip8.I+index];
				}
				chip8.I += X + 1;
                break;
        }

	break;
	default:
		printf("Unknown opcode: 0x%X\n", opcode);
		break;
	}
}
static bool romDropdownOpen = false;
static int romIndex = -1;

Rectangle menuBar = { 0, 0, WINDOW_WIDTH, MENU_HEIGHT };
Rectangle romDrop = { UI_OFFSET_X, 5, 220, MENU_HEIGHT - 10 };
Rectangle chip8Panel = {
	UI_OFFSET_X,
	UI_OFFSET_Y,
	CHIP_W,
	CHIP_H
};
Rectangle stackPanel = {
	UI_OFFSET_X + CHIP_W + PANEL_SPACING,
	UI_OFFSET_Y,
	SIDE_PANEL_WIDTH,
	CHIP_H
};
Rectangle disasmPanel = {
	UI_OFFSET_X,
	UI_OFFSET_Y + CHIP_H + PANEL_SPACING,
	WINDOW_WIDTH - (2 * UI_OFFSET_X),
	WINDOW_HEIGHT - (UI_OFFSET_Y + CHIP_H + PANEL_SPACING + UI_OFFSET_X)
};

void DrawPanel(Rectangle r, const char *title)
{
    DrawRectangleRec(r, (Color){30, 30, 30, 255});
    DrawRectangleLinesEx(r, 1, (Color){120, 120, 120, 255});
    DrawText(title, r.x + 6, r.y + 6, 16, RAYWHITE);
}

void DrawStackView(Rectangle r)
{
    BeginScissorMode(r.x, r.y, r.width, r.height);

    int y = r.y + 30;

    for (int i = 0; i < 16; i++) {
        Color col = (i == chip8.sp) ? YELLOW : RAYWHITE;
        DrawText(TextFormat("%02X: 0x%04X", i, chip8.V[i]),
                 r.x + 10, y, 14, col);
        y += 18;
    }

    EndScissorMode();
}

void DrawDisassembler(Rectangle r)
{
    BeginScissorMode(r.x, r.y, r.width, r.height);

    int y = r.y + 30;
    uint16_t pc = chip8.pc;

    for (int i = -5; i <= 10; i++) {
        uint16_t addr = pc + (i * 2);

        if (addr >= 4096 - 1) continue;

        uint16_t opcode =
            (chip8.memory[addr] << 8) | chip8.memory[addr + 1];

        Color col = (i == 0) ? YELLOW : RAYWHITE;

        DrawText(
            TextFormat("%04X  %04X", addr, opcode),
            r.x + 10, y, 16, col
        );

        y += 18;
    }

    EndScissorMode();
}

char romNames[MAX_ROMS][128];
int romCount = 0;

void LoadRomList(void)
{
	romCount = 0;

	FilePathList files = LoadDirectoryFiles("resources/roms");

	for (int i = 0; i < files.count && romCount < MAX_ROMS; i++) {
		if (IsFileExtension(files.paths[i], ".ch8;.rom")) {
			const char *name = GetFileName(files.paths[i]);
			strncpy(romNames[romCount], name, 127);
			romNames[romCount][127] = '\0';
			romCount++;
		}
	}

	UnloadDirectoryFiles(files);
}

void UpdateDrawFrame(void);

int main ()
{

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	//SetExitKey(0);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chip8");
	//SearchAndSetResourceDir("resources");
	InitAudioDevice();
    Wav = LoadSound("resources/BEEP2.wav");
	initalize();
	LoadRomList();
	 if(loadROM("resources/roms/PONG.ch8")){
        TraceLog(LOG_ERROR, "Failed to load ROM resources/roms/PONG");
        return 1;
    }

	float clock_speed=700.0f;
	int cycles_per_frame = (int)(clock_speed / 60.0f);
	bool pause = false;
	#if defined(PLATFORM_WEB)
    	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
	#else
	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
		UpdateDrawFrame();
	}
	#endif
	UnloadSound(Wav);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}

void UpdateDrawFrame(void)
{
	static bool pause = false;
	static float clock_speed = 700.0f;
	static double cyclesAccumulator = 0.0;
	if (IsKeyPressed(KEY_SPACE)) pause = !pause;

	UpdateChip8Keys();

	double frameTime = GetFrameTime();

	if (pause) {
		cyclesAccumulator = 0.0;
	} else {
		cyclesAccumulator += frameTime * clock_speed;
		int cyclesToRun = (int)cyclesAccumulator;
		const int maxCyclesPerFrame = 1200;
		if (cyclesToRun > maxCyclesPerFrame) cyclesToRun = maxCyclesPerFrame;
		cyclesAccumulator -= cyclesToRun;
		for (int i = 0; i < cyclesToRun; i++) {
			execute();
		}
		updateTimers();
	}

	BeginDrawing();
	ClearBackground(BLACK);

	DrawPanel(stackPanel,  "Stack");
	DrawPanel(chip8Panel,  "Display");
	DrawPanel(disasmPanel, "Disassembler");

	DrawStackView(stackPanel);
	DrawDisassembler(disasmPanel);
	drawDisplay();

	GuiPanel(menuBar, NULL);

	static char romListText[512] = "";
	romListText[0] = '\0';
	size_t used = 0;

	for (int i = 0; i < romCount; i++) {
		size_t nameLen = strlen(romNames[i]);
		if (used + nameLen + 1 >= sizeof(romListText)) break;
		memcpy(romListText + used, romNames[i], nameLen);
		used += nameLen;
		if (i < romCount - 1) {
			if (used + 1 >= sizeof(romListText)) break;
			romListText[used++] = ';';
		}
	}
	romListText[used] = '\0';

	bool dropdownAction = GuiDropdownBox(romDrop, romListText, &romIndex, romDropdownOpen);

	if (dropdownAction) {
		romDropdownOpen = !romDropdownOpen;

		if (!romDropdownOpen && romIndex >= 0 && romIndex < romCount) {
			char path[256];
			snprintf(path, sizeof(path), "resources/roms/%s", romNames[romIndex]);
			initalize();
			if (loadROM(path) != 0) {
				TraceLog(LOG_ERROR, TextFormat("Failed to load ROM %s", path));
			}
		}
	}

	EndDrawing();
}