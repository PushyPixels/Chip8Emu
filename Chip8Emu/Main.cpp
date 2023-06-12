#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Chip8Emu.h"

// Override base class with your custom functionality
class RenderingEngine : public olc::PixelGameEngine
{
public:
	char* filename;
	Chip8Emu emu;

	RenderingEngine()
	{
		// Name your application
		sAppName = "Chip8Emu";
	}

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		emu.Start();
		LoadGame();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		UpdateInput();
		emu.Update();

		if (emu.drawFlag)
		{
			for (int i = 0; i < 64 * 32; i++)
			{
				if (emu.gfx[i])
				{
					Draw(i % 64, i / 64, olc::Pixel(255, 255, 255));
				}
				else
				{
					Draw(i % 64, i / 64, olc::Pixel(0, 0, 0));
				}
			}

			emu.drawFlag = false;
		}

		return true;
	}

	void LoadGame()
	{
		std::ifstream romfile;
		if (filename)
		{
			romfile.open(filename, std::ios::binary);
			if (!romfile)
			{
				std::cout << "Failed to open the file, falling back to Invaders.ch8" << std::endl;
				romfile.open("Invaders.ch8", std::ios::binary);
			}
		}
		else
		{
			std::cout << "No file provided, falling back to Invaders.ch8" << std::endl;
			romfile.open("Invaders.ch8", std::ios::binary);
		}

		char c;
		int i = 0;
		while (romfile.get(c))
		{
			//std::cout << std::hex << (uint16_t)c << std::dec << std::endl;
			emu.memory[i + 512] = c;
			i++;
		}
	}

	void UpdateInput()
	{
		// Input handling
		if (GetKey(olc::Key::K0).bPressed)
		{
			emu.key[0] = 1;
		}
		if (GetKey(olc::Key::K1).bPressed)
		{
			emu.key[1] = 1;
		}
		if (GetKey(olc::Key::K2).bPressed)
		{
			emu.key[2] = 1;
		}
		if (GetKey(olc::Key::K3).bPressed)
		{
			emu.key[3] = 1;
		}
		if (GetKey(olc::Key::K4).bPressed)
		{
			emu.key[4] = 1;
		}
		if (GetKey(olc::Key::K5).bPressed)
		{
			emu.key[5] = 1;
		}
		if (GetKey(olc::Key::K6).bPressed)
		{
			emu.key[6] = 1;
		}
		if (GetKey(olc::Key::K7).bPressed)
		{
			emu.key[7] = 1;
		}
		if (GetKey(olc::Key::K8).bPressed)
		{
			emu.key[8] = 1;
		}
		if (GetKey(olc::Key::K9).bPressed)
		{
			emu.key[9] = 1;
		}
		if (GetKey(olc::Key::A).bPressed)
		{
			emu.key[0xA] = 1;
		}
		if (GetKey(olc::Key::B).bPressed)
		{
			emu.key[0xB] = 1;
		}
		if (GetKey(olc::Key::C).bPressed)
		{
			emu.key[0xC] = 1;
		}
		if (GetKey(olc::Key::D).bPressed)
		{
			emu.key[0xD] = 1;
		}
		if (GetKey(olc::Key::E).bPressed)
		{
			emu.key[0xE] = 1;
		}
		if (GetKey(olc::Key::F).bPressed)
		{
			emu.key[0xF] = 1;
		}

		// Up
		if (GetKey(olc::Key::K0).bReleased)
		{
			emu.key[0] = 0;
		}
		if (GetKey(olc::Key::K1).bReleased)
		{
			emu.key[1] = 0;
		}
		if (GetKey(olc::Key::K2).bReleased)
		{
			emu.key[2] = 0;
		}
		if (GetKey(olc::Key::K3).bReleased)
		{
			emu.key[3] = 0;
		}
		if (GetKey(olc::Key::K4).bReleased)
		{
			emu.key[4] = 0;
		}
		if (GetKey(olc::Key::K5).bReleased)
		{
			emu.key[5] = 0;
		}
		if (GetKey(olc::Key::K6).bReleased)
		{
			emu.key[6] = 0;
		}
		if (GetKey(olc::Key::K7).bReleased)
		{
			emu.key[7] = 0;
		}
		if (GetKey(olc::Key::K8).bReleased)
		{
			emu.key[8] = 0;
		}
		if (GetKey(olc::Key::K9).bReleased)
		{
			emu.key[9] = 0;
		}
		if (GetKey(olc::Key::A).bReleased)
		{
			emu.key[0xA] = 0;
		}
		if (GetKey(olc::Key::B).bReleased)
		{
			emu.key[0xB] = 0;
		}
		if (GetKey(olc::Key::C).bReleased)
		{
			emu.key[0xC] = 0;
		}
		if (GetKey(olc::Key::D).bReleased)
		{
			emu.key[0xD] = 0;
		}
		if (GetKey(olc::Key::E).bReleased)
		{
			emu.key[0xE] = 0;
		}
		if (GetKey(olc::Key::F).bReleased)
		{
			emu.key[0xF] = 0;
		}
	}
};

int main(int argc, char* argv[])
{
	RenderingEngine engine;

	if (argc < 2)
	{
		//std::cout << "No file provided." << std::endl;
	}
	else
	{
		engine.filename = argv[1];
		std::cout << "Loading: " << engine.filename << std::endl;
	}

	if (engine.Construct(64, 32, 4, 4, false, false))
		engine.Start();
	return 0;
}
