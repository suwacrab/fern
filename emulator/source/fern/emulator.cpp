#include <fern.h>
#include <vector>
#include <iostream>

#include <SDL2/SDL.h>

namespace fern {
	CEmulator::CEmulator(const CEmuInitFlags* flags) {
		CEmuInitFlags default_flags;
		if(!flags) flags = &default_flags;

		m_quitflag = false;
		m_cgbEnabled = true;

		m_debugEnable = flags->debug;
		m_debugSkipping = false;
		m_debugSkipAddr = 0;
		m_verboseEnable = flags->verbose;

		cpu.assign_emu(this);
		mem.assign_emu(this);
		renderer.assign_emu(this);

		if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
		{
			std::printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
			std::exit(-1);
		}

		std::atexit(SDL_Quit);
		renderer.window_create(flags->vsync);
	//	renderer.window_create(true);
	}

	auto CEmulator::savedata_getFilename() -> std::string {
		return m_romfilename + ".fsv";
	}
	auto CEmulator::savedata_sync() -> void {
		auto savedat = mem.m_mapper->sram_serialize();
		if(savedat.size() == 0) return;
		
		Blob blob_header;
		Blob blob_file;

		// write header ---------------------------------@/
		blob_header.write_str("FSV");
		blob_header.write_u32(savedat.size());

		// write to file --------------------------------@/
		blob_file.write_blob(blob_header);
		blob_file.write_blob(savedat);

		blob_file.write_file(savedata_getFilename(),true);
	}

	auto CEmulator::process_message() -> void {
		SDL_Event eve;
		while(SDL_PollEvent(&eve)) {
			switch(eve.type) {
				case SDL_WINDOWEVENT: {
					if(eve.window.event == SDL_WINDOWEVENT_CLOSE) {
						quit();
					}
					break;
				}
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
				std::printf("enter command (type h for help): ");
				std::cin >> cmdname;
				if(cmdname == "h") {
					std::puts(
						"\t[h]elp   - display this help\n"
						"\t[w]here  - print what current emulator status\n"
						"\t[r]un    - continue running normally\n"
						"\t[s]tep   - step 1 instruction\n"
						"\t[ss]tep  - step multiple instructions\n"
						"\t[g]o     - run intil specified address\n"
						"\t[p]eek   - peek (aka. read) specified address\n"
						"\t[q]uit   - stop emulation"
					);
				}
				else if(cmdname == "p") {
					int peek_addr = 0;
					std::printf("where to? (hex): ");
					std::scanf("%x",&peek_addr);
					std::printf("read $%04X: $%04X\n",peek_addr,mem.read(peek_addr));
				}
				else if(cmdname == "q") {
					quit();
				}
				else if(cmdname == "w") {
					cpu.print_status(true);
				}
				else if(cmdname == "r") {
					m_debugEnable = false;
				}
				else if(cmdname == "s" || cmdname == "ss") {
					int to_step = 1;
					if(cmdname == "ss") {
						std::printf("how many lines? (int): ");
						std::scanf("%d",&to_step);
					}
					for(int i=0; i<to_step; i++) cpu.step();

					cpu.print_status(true);
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

		savedata_sync();
	}
	auto CEmulator::load_romfile(const std::string& filename) -> void {
		cpu.reset();
		
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

		bool sram_used = false;
		switch(hdr_carttype) {
			case 0: { mem.mapper_setupNone(); break; }
			// MBC1
			case 0x01: { mem.mapper_setupMBC1(false,false); break; }
			case 0x03: { mem.mapper_setupMBC1(true,true); sram_used = true; break; }
			// MBC3
			case 0x13: { mem.mapper_setupMBC3(true,true,true); sram_used = true; break; }
			// MBC5
			case 0x1B: { mem.mapper_setupMBC5(true,true,false); sram_used = true; break; }
			// unknown
			default: {
				std::printf("error: ROM has unknown mapper %02Xh\n",hdr_carttype);
				std::puts("this ROM may either not be supported, or it's not a proper ROM.");
				std::exit(-1);
			}
		}

		// setup cgb flags ------------------------------@/
		int cgb_flag = rom_vec.at(0x143);
		if( (cgb_flag == 0x80) || (cgb_flag == 0xC0) ) {
			m_cgbEnabled = true;
			std::puts("CGB mode!");
		} else if(cgb_flag == 0x00) {
			m_cgbEnabled = false;
		} else {
			std::printf("warning: ROM has unknown CGB flag. ($%02X) emulation may not work properly...\n",
				cgb_flag
			);
			m_cgbEnabled = false;
		}

		// setup banks ----------------------------------@/
		if(sram_used) {
			int bankcount = 0;
			int size_id = rom_vec.at(0x149);
			switch(size_id) {
				// no RAM
				case 0:
				case 1: { bankcount = 0; break; }
				// 1 banks x 8kib
				case 2: { bankcount = 1; break; }
				// 4 banks x 8kib
				case 3: { bankcount = 4; break; }
				// 16 banks x 8kib
				case 4: { bankcount = 16; break; }
				// 8 banks x 8kib
				case 5: { bankcount = 8; break; }
				// unknown
				default: {
					std::printf("error: ROM has unknown RAM size %d\n",
						size_id
					);
					std::exit(-1);
					break;
				}

			}
			mem.m_rambankCount = bankcount;
			std::printf("RAM size: %d KB\n",bankcount * 8);
		}

		// write ROM banks to memory
		const size_t banksize = KBSIZE(16);
		const size_t rom_size = KBSIZE(32) * (1 << rom_vec.at(0x148));
		const size_t num_banks = rom_size / banksize;

		mem.m_rombankCount = num_banks;

		for(size_t b=0; b<num_banks; b++) {
			for(size_t i=0; i<banksize; i++) {
				mem.m_rombanks[b].data.at(i) = rom_vec.at(i + (b*banksize));
			}
		}

		m_romfilename = filename;

		// load save data, if any
		auto fsvfile = std::fopen(savedata_getFilename().c_str(),"rb");
		if(fsvfile) {
			const char fsv_magic[4] = { 'F','S','V',0 };
			for(int i=0; i<4; i++) {
				char chr = 0xFF;
				std::fread(&chr,1,sizeof(char),fsvfile);
				if(chr != fsv_magic[i]) {
					std::printf("error: corrupt .fsv savefile\n");
					std::exit(-1);
				}
			}

			uint32_t savesize = 0;
			std::fread(&savesize,1,sizeof(savesize),fsvfile);

			for(int i=0; i<savesize; i++) {
				uint8_t chr = 0;
				std::fread(&chr,1,sizeof(char),fsvfile);
				mem.m_sram[i] = chr;
			}

			std::fclose(fsvfile);
		}

		// setup CGB stuff
		if(cgb_enabled()) {
			cpu.m_regA = 0x11;
		}
	}
}

