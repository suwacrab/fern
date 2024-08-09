#include <fern.h>
#include <vector>
#include <SDL2/SDL.h>

namespace fern {
	CEmulator::CEmulator() {
		m_quitflag = false;
		m_cgbmode = false;
		m_debugEnable = true;
		cpu.assign_emu(this);
		mem.assign_emu(this);
		renderer.assign_emu(this);

		if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
		{
			std::printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		}

		renderer.window_create();
	}

	auto CEmulator::boot() -> void {
		std::puts("booting rom...");
		std::printf("mapper: %s\n",mem.m_mapper->name().c_str());

		cpu.step();

		while(!did_quit()) {
			// debug process
			if(m_debugEnable) {
				char cmdname = 0;
				std::printf("enter command (r=run,w=where, s=step, g=run til addr, q=quit): ");
				std::scanf(" %c",&cmdname);
				switch(cmdname) {
					case 'q': {
						quit();
						break;
					}
					case 'w': {
						cpu.print_status();
						break;
					}
					case 'r': {
						m_debugEnable = false;
						break;
					}
					case 's': {
						cpu.step();
						cpu.print_status();
						break;
					}
					case 'g': {
						int to_addr = 0;
						std::printf("where to? (hex): ");
						std::scanf("%x",&to_addr);
						//int iter_count = 0;
						while(cpu.m_PC != to_addr) {
							cpu.step();
							//iter_count += 1;
							renderer.process_message();
							if(did_quit()) break;
							/*if((iter_count%5000)==0) {
								cpu.print_status();
							}*/
						}
						break;
					}
					default: {
						std::printf("unknown command %d\n",cmdname);
						std::exit(-1);
					}
				}	
			} 
			// regular process
			else {
				cpu.step();
			}
			renderer.process_message();
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

