#pragma once
#include <stack>
#include <string>

class Chip8Emu
{
public:
	int debugFlag = 0;
	int cyclesPerUpdate = 1;
	bool cycleUntilDraw = 1; // currently unimplemented
	int cyclesPerTimerDecrement = 10;
	bool gfx[64 * 32];
	bool key[16]; // Keypad input state
	unsigned char memory[4096];
	bool drawFlag;

private:
	// Internals
	unsigned short opcode;
	unsigned char V[16]; // Registers
	unsigned short I; // Index register
	unsigned short pc; // Program counter

	unsigned char delay_timer;
	unsigned char sound_timer;

	unsigned int currentCycle;

	std::stack<unsigned short> stack; // may need to limit to 16 elements?

	// Graphics
	unsigned char chip8_fontset[5 * 16] =
	{
		0b11110000,
		0b10010000,
		0b10010000,
		0b10010000,
		0b11110000,
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

public:
	void Start();
	void Initialize();
	void Update();
	void Cycle();
	void Execute();
	void Log(unsigned int opcode, std::string string);
	void Log(unsigned int opcode, std::ostringstream& stringStream);
};