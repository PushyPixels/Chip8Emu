#include "Chip8Emu.h"

#include <array>
#include <stack>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <sstream>

// Use this for initialization
void Chip8Emu::Start()
{
	Initialize();
}

void Chip8Emu::Initialize()
{
	pc = 0x200;  // Program counter starts at 0x200
	opcode = 0;      // Reset current opcode	
	I = 0;      // Reset index register

	// Clear display
	std::fill(std::begin(gfx), std::end(gfx), 0);

	// Clear stack
	stack = {};

	// Clear registers V0-VF
	std::fill(std::begin(V), std::end(V), 0);

	// Clear memory
	std::fill(std::begin(memory), std::end(memory), 0);

	// Load fontset
	for (int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}

	// Reset timers
	delay_timer = 0;
	sound_timer = 0;
}

void Chip8Emu::Update()
{
	for (int i = 0; i < cyclesPerUpdate; i++)
	{
		// Run emulator cycle
		Cycle();

		currentCycle++;

		// Update timers
		if (delay_timer > 0)
		{
			//std::cout << (int)delay_timer << std::endl;

			if (currentCycle % cyclesPerTimerDecrement == 0)
			{
				delay_timer--;
			}
		}
		if (sound_timer > 0)
		{
			//if (!audioSource.isPlaying)
			//{
			//	audioSource.Play();
			//}

			if (currentCycle % cyclesPerTimerDecrement == 0)
			{
				sound_timer--;
			}

			if (sound_timer == 0) // Play sound when crossing to 0 (This may be wrong, check here: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.5)
			{
				//	audioSource.Stop();
			}
		}
	}
}

void Chip8Emu::Cycle()
{
	// Fetch opcode
	opcode = static_cast<unsigned short>(memory[pc] << 8 | memory[pc + 1]);
	//std::cout << "Current opcode: 0x" << std::hex << opcode << std::dec << std::endl;
		
	// Decode and execute opcode
	Execute();
}

void Chip8Emu::Execute()
{
	unsigned char X;
	unsigned char Y;
	unsigned char N;
	unsigned short NN;
	unsigned short NNN;
	std::ostringstream stringStream;
	stringStream << std::hex << std::uppercase;

	switch (opcode & 0xF000)
	{
	case 0x0000:
		if (opcode == 0x00E0) // Clears screen
		{
			Log(opcode, "Clear screen");
			std::fill(std::begin(gfx), std::end(gfx), 0);
			drawFlag = true;
			pc += 2;
		}
		else if (opcode == 0x00EE) // Return from subroutine
		{
			Log(opcode, "Return from subroutine");
			pc = stack.top();
			stack.pop();
			pc += 2;
		}
		else
		{
			std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
		}
		break;

	case 0x1000: // 1NNN: Jump to NNN
		NNN = opcode & 0x0FFF;
		stringStream << "Jumped to " << NNN;
		Log(opcode, stringStream);
		pc = NNN;
		break;

	case 0x2000: // 2NNN: Call subroutine at NNN
		NNN = opcode & 0x0FFF;
		stringStream << "Called subroutine at " << NNN;
		Log(opcode, stringStream);
		stack.push(pc);
		pc = NNN;
		break;

	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN.
		X = (opcode & 0x0F00) >> 8;
		NN = opcode & 0x00FF;
		if (V[X] == NN)
		{
			stringStream << "Skipped next instruction (V" << static_cast<int>(X) << " == " << NN << ")";
			Log(opcode, stringStream);
			pc += 4;
		}
		else
		{
			stringStream << "Continue to next instruction (V" << static_cast<int>(X) << " != " << NN << ")";
			Log(opcode, stringStream);
			pc += 2;
		}
		break;

	case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN.
		X = (opcode & 0x0F00) >> 8;
		NN = opcode & 0x00FF;
		if (V[X] != NN)
		{
			stringStream << "Skipped next instruction (V" << static_cast<int>(X) << " != " << NN << ")";
			Log(opcode, stringStream);
			pc += 4;
		}
		else
		{
			stringStream << "Continue to next instruction (V" << static_cast<int>(X) << " == " << NN << ")";
			Log(opcode, stringStream);
			pc += 2;
		}
		break;

	case 0x5000: // 5XY0: Skips the next instruction if VX equals VY.
		X = (opcode & 0x0F00) >> 8;
		Y = (opcode & 0x00F0) >> 4;
		if (V[X] == V[Y])
		{
			stringStream << "Skipped next instruction (V" << static_cast<int>(X) << " == V" << static_cast<int>(Y) << ")";
			Log(opcode, stringStream);
			pc += 4;
		}
		else
		{
			stringStream << "Continue to next instruction (V" << static_cast<int>(X) << " != V" << static_cast<int>(Y) << ")";
			Log(opcode, stringStream);
			pc += 2;
		}
		break;

	case 0x6000: // 6XNN: Sets VX to NN.
		X = (opcode & 0x0F00) >> 8;
		NN = (opcode & 0x00FF);
		stringStream << "Set V" << static_cast<int>(X) << " to " << NN;
		Log(opcode, stringStream);
		V[X] = static_cast<unsigned char>(NN);
		pc += 2;
		break;

	case 0x7000: // 7XNN: Adds NN to VX.
		X = (opcode & 0x0F00) >> 8;
		NN = (opcode & 0x00FF);
		stringStream << "Added " << NN << " to V" << static_cast<int>(X);
		Log(opcode, stringStream);
		V[X] += static_cast<unsigned char>(NN);
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0: // 8XY0: Sets VX to the value of VY.
			X = (opcode & 0x0F00) >> 8;
			Y = (opcode & 0x00F0) >> 4;
			stringStream << "Set V" << static_cast<int>(X) << " to the value of V" << static_cast<int>(Y);
			Log(opcode, stringStream);
			V[X] = V[Y];
			pc += 2;
			break;

		case 0x1: // 8XY1: Sets VX to VX | VY.
			X = (opcode & 0x0F00) >> 8;
			Y = (opcode & 0x00F0) >> 4;
			stringStream << "Set V" << static_cast<int>(X) << " to V" << static_cast<int>(X) << " | V" << static_cast<int>(Y);
			Log(opcode, stringStream);
			V[X] |= V[Y];
			pc += 2;
			break;

		case 0x2: // 8XY2: Sets VX to VX & VY.
			X = (opcode & 0x0F00) >> 8;
			Y = (opcode & 0x00F0) >> 4;
			stringStream << "Set V" << static_cast<int>(X) << " to V" << static_cast<int>(X) << " & V" << static_cast<int>(Y);
			Log(opcode, stringStream);
			V[X] &= V[Y];
			pc += 2;
			break;

		case 0x3: // 8XY3: Sets VX to VX xor VY.
			X = (opcode & 0x0F00) >> 8;
			Y = (opcode & 0x00F0) >> 4;
			stringStream << "Set V" << static_cast<int>(X) << " to V" << static_cast<int>(X) << " xor V" << static_cast<int>(Y);
			Log(opcode, stringStream);
			V[X] ^= V[Y];
			pc += 2;
			break;

		case 0x4: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
		{
			X = (opcode & 0x0F00) >> 8;
			Y = (opcode & 0x00F0) >> 4;
			int temp = V[X] + V[Y];
			if (temp > 0xFF) // Carry if exeeds byte
			{
				stringStream << "Added V" << static_cast<int>(Y) << " to V" << static_cast<int>(X) << ". Carry, VF will be set to 1.";
				Log(opcode, stringStream);
				V[0xF] = 1;
			}
			else
			{
				stringStream << "Added V" << static_cast<int>(Y) << " to V" << static_cast<int>(X) << ". No carry, VF will be set to 0.";
				Log(opcode, stringStream);
				V[0xF] = 0;
			}
			V[X] += V[Y];
			pc += 2;
		}
		break;

		case 0x5: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		{
			X = (opcode & 0x0F00) >> 8;
			Y = (opcode & 0x00F0) >> 4;
			int temp = V[X] - V[Y];
			if (temp < 0) // Don't carry if borrow in subtract
			{
				stringStream << "Subtracted V" << static_cast<int>(Y) << " from V" << static_cast<int>(X) << ". Borrow, VF will be set to 0.";
				Log(opcode, stringStream);
				V[0xF] = 0;
			}
			else
			{
				stringStream << "Subtracted V" << static_cast<int>(Y) << " from V" << static_cast<int>(X) << ". No borrow, VF will be set to 1.";
				Log(opcode, stringStream);
				V[0xF] = 1;
			}
			V[X] -= V[Y];
			pc += 2;
		}
		break;

		case 0x6: // 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
			X = (opcode & 0x0F00) >> 8;
			stringStream << "Bit shifted V" << static_cast<int>(X) << " right by one. VF now contains least significant bit.";
			Log(opcode, stringStream);
			V[0xF] = (V[X] & 0x1);
			V[X] = (V[X] >> 1);
			pc += 2;
			break;

		case 0x7: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		{
			X = (opcode & 0x0F00) >> 8;
			Y = (opcode & 0x00F0) >> 4;
			int temp = V[Y] - V[X];
			if (temp < 0) // Don't carry if borrow in subtract
			{
				stringStream << "Set V" << static_cast<int>(X) << " to V" << static_cast<int>(Y) << " minus V" << X << " Borrow, VF will be set to 0.";
				Log(opcode, stringStream);
				V[0xF] = 0;
			}
			else
			{
				stringStream << "Set V" << static_cast<int>(X) << " to V" << static_cast<int>(Y) << " minus V" << X << " No borrow, VF will be set to 1.";
				Log(opcode, stringStream);
				V[0xF] = 1;
			}
			V[X] = V[Y] - V[X];
			pc += 2;
		}
		break;

		case 0xE: // 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift. // TODO: Questionable VF logic?
			X = (opcode & 0x0F00) >> 8;
			stringStream << "Bit shifted V" << static_cast<int>(X) << " left by one. VF now contains most significant bit.";
			Log(opcode, stringStream);
			V[0xF] = V[X] & 0x80;
			V[X] = V[X] << 1;
			pc += 2;
			break;

		default:
			std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
			break;
		}
		break;

	case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY.
		X = (opcode & 0x0F00) >> 8;
		Y = (opcode & 0x00F0) >> 4;
		if (V[X] != V[Y])
		{
			stringStream << "Skipped next instruction (V" << static_cast<int>(X) << " != V" << static_cast<int>(Y) << ")";
			Log(opcode, stringStream);
			pc += 4;
		}
		else
		{
			stringStream << "Continue to next instruction (V" << static_cast<int>(X) << " == V" << static_cast<int>(Y) << ")";
			Log(opcode, stringStream);
			pc += 2;
		}
		break;

	case 0xA000: // ANNN: Sets I to the address NNN
		NNN = opcode & 0x0FFF;
		stringStream << "Set I to the address " << NNN;
		Log(opcode, stringStream);
		I = NNN;
		pc += 2;
		break;

	case 0xB000: // BNNN: Jumps to the address NNN plus V0.
		NNN = opcode & 0x0FFF;
		stringStream << "Jump to the address " << NNN << " plus V0";
		Log(opcode, stringStream);
		pc = V[0] + NNN;
		break;

	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number and NN.
		Log(opcode, "Set VX to the result of a bitwise and operation on a random number and NN.");
		V[(opcode & 0x0F00) >> 8] = static_cast<unsigned char>((opcode & 0x00FF) & rand() % 255);
		pc += 2;
		break;

	case 0xD000: // DXYN: Draw sprite at location VX,VY on screen. Sprite is N lines high.
	{
		X = (opcode & 0x0F00) >> 8;
		Y = (opcode & 0x00F0) >> 4;
		N = opcode & 0x000F;
		unsigned short x = V[X];
		unsigned short y = V[Y];
		unsigned short sourcePixel;

		stringStream << "Draw sprite at location V" << static_cast<int>(X) << ",V" << static_cast<int>(Y) << " (" << std::dec << x << "," << y << std::hex << ") on screen. Sprite is " << static_cast<int>(N) << " lines high.";
		Log(opcode, stringStream);

		V[0xF] = 0;
		for (int yline = 0; yline < N; yline++)
		{
			sourcePixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((sourcePixel & (0x80 >> xline)) != 0)
				{
					int outputPixel = x + xline + ((y + yline) * 64);

					if (outputPixel >= 64 * 32)
					{
						std::cout << "CHIP8: Sprite draw location out of bounds!\n";
						break;
					}

					if(gfx[outputPixel])
					{
						V[0xF] = 1;
						gfx[outputPixel] = 0;
					}
					else
					{
						gfx[outputPixel] = 1;
					}
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x9E: // EX9E: Skips the next instruction if the key stored in VX is pressed.
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
			{
				Log(opcode, "Skipping the next instruction because the key stored in VX is pressed.");
				pc += 4;
			}
			else
			{
				Log(opcode, "Continuing to the next instruction because the key stored in VX is NOT pressed.");
				pc += 2;
			}
			break;

		case 0xA1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed.
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
			{
				Log(opcode, "Skipping the next instruction because the key stored in VX is NOT pressed.");
				pc += 4;
			}
			else
			{
				Log(opcode, "Continuing to the next instruction because the key stored in VX is pressed.");
				pc += 2;
			}
			break;

		default:
			std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
			break;
		}
		break;

	case 0xF000:
		int temp;
		switch (opcode & 0x00FF)
		{
		case 0x07: // FX07: Sets VX to the value of the delay timer.
			Log(opcode, "Set VX to the value of the delay timer");
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x0A: // FX0A: A key press is awaited, and then stored in VX.  STOPS EXECUTION (not including timers).
			Log(opcode, "Waiting for key press, will store in VX");
			for (unsigned char i = 0; i < 16; i++)
			{
				if (key[i] == 1)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					pc += 2;
					break;
				}
			}
			break;

		case 0x15: // FX15: Sets the delay timer to VX.
			Log(opcode, "Set the delay timer to VX");
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x18: // FX18: Sets the sound timer to VX.
			Log(opcode, "Set the sound timer to VX");
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x1E: // FX1E: Adds VX to I. VF is set to 1 if I+VX>0xFFF (Undocumented feature required by some games).
			temp = V[(opcode & 0x0F00) >> 8] + I;
			if (temp > 0xFFF) // Carry if exeeds I range
			{
				Log(opcode, "Added VX to I. VF is set to 1 because I+VX>0xFFF");
				V[0xF] = 1;
			}
			else
			{
				Log(opcode, "Added VX to I. VF is set to 0 because I+VX<=0xFFF");
				V[0xF] = 0;
			}
			I += static_cast<unsigned short>(V[(opcode & 0x0F00) >> 8]);
			pc += 2;
			break;

		case 0x29: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
			Log(opcode, "Set I to the location of the sprite for the character in VX");
			I = static_cast<unsigned short>(V[(opcode & 0x0F00) >> 8] * 5);
			pc += 2;
			break;

			// FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
			// the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation
			// of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
		case 0x33:
			Log(opcode, "Stored binary-coded decimal at I, I+1, and I+2");
			memory[I] =		V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = V[(opcode & 0x0F00) >> 8] / 10 % 10;
			memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
			pc += 2;
			break;

		case 0x55: // FX55: Stores V0 to VX in memory starting at address I.  I is unchanged.
			Log(opcode, "Stored V0 to VX in memory starting at address I.");
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
			{
				memory[I + i] = V[i];
			}
			pc += 2;
			break;

		case 0x65: // FX65: Fills V0 to VX with values from memory starting at address I.  I is unchanged.
			Log(opcode, "Filled V0 to VX with values from memory starting at address I.");
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
			{
				V[i] = memory[I + i];
			}
			pc += 2;
			break;

		default:
			std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
			break;
		}
		break;

	default:
		std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
		break;
	}
}

void Chip8Emu::Log(unsigned int opcode, std::string string)
{
	if(debugFlag)
	{
		std::cout << "PC:" << std::hex << std::uppercase << std::setfill('0') << std::setw(3) << pc << " Op:" << std::setw(4) << opcode << std::dec << ": " << string << std::endl;
	}
}

void Chip8Emu::Log(unsigned int opcode, std::ostringstream &stringStream)
{
	if (debugFlag)
	{
		std::cout << "PC:" << std::hex << std::uppercase << std::setfill('0') << std::setw(3) << pc << " Op:" << std::setw(4) << opcode << std::dec << ": " << stringStream.str() << std::endl;
	}
	stringStream.str("");
}


