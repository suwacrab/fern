#include <fern.h>
#include <vector>
#include <iostream>
#include <SDL2/SDL.h>

namespace fern {
	CEmulator::CEmulator() {
		m_quitflag = false;
		m_cgbmode = false;

		m_debugEnable = true;
		m_debugSkipping = false;
		m_debugSkipAddr = 0;

		cpu.assign_emu(this);
		mem.assign_emu(this);
		renderer.assign_emu(this);

		if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
		{
			std::printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		}

		renderer.window_create();
	}

	auto CEmulator::process_message() -> void {
		SDL_Event eve;
		while(SDL_PollEvent(&eve)) {
			switch(eve.type) {
				case SDL_QUIT: {
					quit();
					break;
				}
				case SDL_KEYDOWN: {
					auto key = eve.key.keysym.sym;
					if(key == SDLK_g) {
						debug_set(true);
					} else if(key == SDLK_r) {
						debug_set(false);
					}
					break;
				}
			}
		}
		
		// get keyboard state
		const auto keystate = SDL_GetKeyboardState(NULL);
		m_joypad_state.at(EmuButton::up) = keystate[SDL_SCANCODE_UP];
		m_joypad_state.at(EmuButton::down) = keystate[SDL_SCANCODE_DOWN];
		m_joypad_state.at(EmuButton::left) = keystate[SDL_SCANCODE_LEFT];
		m_joypad_state.at(EmuButton::right) = keystate[SDL_SCANCODE_RIGHT];
		m_joypad_state.at(EmuButton::b) = keystate[SDL_SCANCODE_A];
		m_joypad_state.at(EmuButton::a) = keystate[SDL_SCANCODE_S];
		m_joypad_state.at(EmuButton::start) = keystate[SDL_SCANCODE_B];
		m_joypad_state.at(EmuButton::select) = keystate[SDL_SCANCODE_V];
	}
	auto CEmulator::button_held(int btn) -> bool {
		if(btn < 0) return false;
		if(btn >= EmuButton::num_keys) return false;
		return m_joypad_state.at(btn);
	}

	auto CEmulator::boot() -> void {
		std::puts("booting rom...");
		std::printf("mapper: %s\n",mem.m_mapper->name().c_str());

		cpu.step();

		while(!did_quit()) {
			if(m_debugSkipping) {
				if(cpu.m_PC == m_debugSkipAddr) {
					debug_set(true);
					m_debugSkipping = false;
				}
			}
			// debug process
			if(m_debugEnable) {
				std::string cmdname;
				std::printf("enter command (r=run,w=where,s=step,g=run til,p=peek,q=quit): ");
				std::cin >> cmdname;
				if(cmdname == "p") {
					int peek_addr = 0;
					std::printf("where to? (hex): ");
					std::scanf("%x",&peek_addr);
					std::printf("read $%04X: $%04X\n",peek_addr,mem.read(peek_addr));
				}
				else if(cmdname == "q") {
					quit();
				}
				else if(cmdname == "w") {
					cpu.print_status();
				}
				else if(cmdname == "r") {
					m_debugEnable = false;
				}
				else if(cmdname == "s") {
					cpu.step();
					cpu.print_status();
				}
				else if(cmdname == "ss") {
					int to_step = 0;
					std::printf("how many lines? (int): ");
					std::scanf("%d",&to_step);
					for(int i=0; i<to_step; i++) cpu.step();

					cpu.print_status();
				}
				else if(cmdname == "g") {
					int to_addr = 0;
					std::printf("where to? (hex): ");
					std::scanf("%x",&to_addr);
					m_debugSkipAddr = to_addr;
					m_debugSkipping = true;
					debug_set(false);
				}
				else {
					std::printf("unknown command %s\n",cmdname.c_str());
				}
			} 
			// regular process
			else {
				cpu.step();
			}
			process_message();
		}
	}
	auto CEmulator::load_romfile(const std::string& filename) -> void {
		cpu.reset();
		//mem.reset();
		//std::puts("unimplemented");
		//std::exit(-1);
		
		auto file = std::fopen(filename.c_str(),"rb");
		if(!file) {
			std::printf("error: unable to open file '%s'\n",filename.c_str());
			std::exit(-1);
		}

		std::fseek(file,0,SEEK_END);
		std::vector<uint8_t> rom_vec;
		rom_vec.resize(std::ftell(file));
		std::rewind(file);

		// write rom to new vector ----------------------@/
		for(size_t i=0; i<rom_vec.size(); i++) {
			uint8_t chr = 0;
			std::fread(&chr,1,sizeof(char),file);
			rom_vec[i] = chr;
		}
		std::fclose(file);

		// get mapper -----------------------------------@/
		uint8_t hdr_carttype = rom_vec.at(0x0147);

		switch(hdr_carttype) {
			case 0: { mem.mapper_setupNone(); break; }
			// MBC1
			case 1: { mem.mapper_setupMBC1(false,false); break; }
			default: {
				std::printf("error: unknown mapper %02Xh\n",hdr_carttype);
				std::exit(-1);
			}
		}

		// write ROM banks to memory
		const size_t banksize = 16 * 1024;
		const size_t rom_size = (32*1024) * (1 << rom_vec.at(0x148));
		const size_t num_banks = rom_size / banksize;

		for(size_t b=0; b<num_banks; b++) {
			for(size_t i=0; i<banksize; i++) {
				mem.m_rombanks[b].data.at(i) = rom_vec.at(i + (b*banksize));
			}
		}
	}
}

