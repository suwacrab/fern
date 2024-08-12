#include <fern.h>
#include <fern_common.h>

static void warn_cgb_reg(const std::string& name, int data) {
	std::printf("warning: CGB register written to (%02Xh) (%s)\n",
		data,name.c_str()
	);
}

namespace fern {
	CMem::CMem() {
		reset();
	}
	CMem::~CMem() {
		delete m_mapper;
	}

	auto CMem::reset() -> void {
		delete m_mapper;
		m_mapper = nullptr;
		m_io = {};
		m_io.m_LCDC = 0x91;
		m_io.m_IF = 0x01;
		m_io.m_DIV = 0xAB;

		m_io.m_BGP = 0xFC;
		m_io.m_OBP[0] = 0xFF;
		m_io.m_OBP[1] = 0xFF;

		m_rambankCount = 0;
		m_rombankCount = 0;
	}

	auto CMem::mapper_setupNone() -> void {
		m_mapper = new CMapperNone();
		m_mapper->assign_emu(m_emu);
	}
	auto CMem::mapper_setupMBC1(bool use_ram, bool use_battery) -> void {
		std::printf("created mapper\n");
		m_mapper = new CMapperMBC1(use_ram, use_battery);
		m_mapper->assign_emu(m_emu);
	}
	auto CMem::mapper_setupMBC5(bool use_ram, bool use_battery, bool use_rumble) -> void {
		std::printf("created mapper\n");
		m_mapper = new CMapperMBC5(use_ram, use_battery, use_rumble);
		m_mapper->assign_emu(m_emu);
	}

	auto CMem::interrupt_match(int mask) -> bool {
		return (m_io.m_IF & mask) && (m_io.m_IE & mask);
	}
	auto CMem::interrupt_clear(int mask) -> void {
		m_io.m_IF &= (0xFF ^ mask);
	}
	auto CMem::rombank_current() -> int {
		if(!m_mapper) {
			std::puts("error: called rombank_current() with invalid mapper");
			std::exit(-1);
			return -1;
		}
		return m_mapper->rom_bank();
	}

	auto CMem::read(size_t addr) -> uint32_t {
		addr &= 0xFFFF;
		auto addr_hi = addr >> 8;
		auto addr_lo = addr & 0xFF;

		// bit 15 clear: ROM read
		// bit 15 set: RAM read
		if((addr>>15) == 0) {
			return m_mapper->read_rom(addr);
		} else {
			// $8000-$9FFF : VRAM
			if(addr_hi >= 0x80 && addr_hi <= 0x9F) {
				return m_vram.at(addr & 0x1FFF);
			}
			// $A000-$BFFF : SRAM
			else if(addr_hi >= 0xA0 && addr_hi <= 0xBF) {
				return m_mapper->read_sram(addr & 0x1FFF);
			} 
			// $C000-$DFFF : WRAM
			else if(addr_hi >= 0xC0 && addr_hi <= 0xDF) {
				return read_wram(addr & 0x1FFF);
			}
			// $E000-$FDFF : HRAM
			else if(addr_hi >= 0xE0 && addr_hi <= 0xFD) {
				return m_wram.at(addr & 0x1FFF);
			}
			// $FE00-$FE9F : OAM
			else if(addr_hi == 0xFE) {
				if(addr_lo >= 0xA0 || !oam_accessible()) {
					return 0xFF;
				}
				return m_oam.at(addr_lo);
			}
			// $FF00-$FFFF : HRAM
			else if(addr_hi == 0xFF) {
				return read_hram(addr_lo);
			} 
			// unknown
			else {
				std::printf("unimplemented: RAM read (%04zXh)\n",addr);
				std::exit(-1);
			}
		}
	}
	auto CMem::read_wram(int addr) -> int {
		addr &= 0x1FFF;

		if(addr >= 0x1000 && emu()->cgb_enabled()) {
			int wrambank = m_io.m_SVBK;
			if(wrambank == 0) wrambank = 1;
			addr &= 0xFFF;
			addr += KBSIZE(4) * wrambank;
		}
		return m_wram.at(addr);
	}
	auto CMem::read_hram(int addr) -> uint32_t {
		addr &= 0xFF;
		if(addr == 0xFF) {
			return m_io.m_IE;
		} else if(addr>>7) {
			return m_hram[addr & 0x7F];
		} else {
			switch(addr) {
				// joypad -------------------------------@/
				case 0x00: {
					int paddata = 0xCF;
					if(m_io.m_joypmode == fern::JOYPMode::button) {
						if(emu()->button_held(EmuButton::a)) paddata ^= 0x1;
						if(emu()->button_held(EmuButton::b)) paddata ^= 0x2;
						if(emu()->button_held(EmuButton::select)) paddata ^= 0x4;
						if(emu()->button_held(EmuButton::start)) paddata ^= 0x8;
					} else {
						if(emu()->button_held(EmuButton::right)) paddata ^= 0x1;
						if(emu()->button_held(EmuButton::left)) paddata ^= 0x2;
						if(emu()->button_held(EmuButton::up)) paddata ^= 0x4;
						if(emu()->button_held(EmuButton::down)) paddata ^= 0x8;
					}
					//std::printf("pad: %1Xh (mode: %d)\n",paddata,m_io.m_joypmode);
					return paddata;
				}
				// serial -------------------------------@/
				case 0x01: return 0;
				case 0x02: return m_io.m_SC;
				// timer --------------------------------@/
				case 0x04: return m_io.m_DIV;
				// misc ---------------------------------@/
				case 0x0F: return m_io.m_IF;
				// sound --------------------------------@/
				case 0x10: return m_io.m_NR10;
				case 0x11: return m_io.m_NR10 & (0b11 << 6);
				case 0x12: return m_io.m_NR12;
				case 0x13: return m_io.m_NR13;
				case 0x14: return m_io.m_NR14 & (1<<6);
				case 0x15: return 0x00; // ponkotsutank reads it...
				case 0x1A: return m_io.m_NR30;
				case 0x20: return m_io.m_NR41;
				case 0x21: return m_io.m_NR42;
				case 0x22: return m_io.m_NR43;
				case 0x24: return m_io.m_NR50;
				case 0x26: return m_io.m_NR52;
				case 0x19: return m_io.m_NR24 & (1<<6);
				case 0x1E: return m_io.m_NR34 & (1<<6);
				case 0x23: return m_io.m_NR44 & (1<<6);
				// video --------------------------------@/
				case 0x40: return m_io.m_LCDC;
				case 0x41: return m_io.m_STAT;
				case 0x44: return m_io.m_LY;
				case 0x47: return m_io.m_BGP;
				case 0x4A: return m_io.m_WY;
				case 0x4B: return m_io.m_WX;
				case 0x4D: return m_io.m_KEY1;
				case 0x4F: return m_io.m_VBK;
				// misc ---------------------------------@/
				case 0x70: return m_io.m_SVBK;
				// unknown ------------------------------@/
				default: {
					std::printf("unimplemented: IO read (%02Xh)\n",addr);
					std::exit(-1);
					return 0;
				}
			}
		}
	}
	
	auto CMem::write(size_t addr,int data) -> void {
		data &= 0xFF;
		addr &= 0xFFFF;
		auto addr_hi = addr >> 8;
		auto addr_lo = addr & 0xFF;

		// bit 15 clear: ROM access
		// bit 15 set: RAM access
		if((addr>>15) == 0) {
			m_mapper->write_rom(addr,data);
		} else {
			// VRAM
			if(addr_hi >= 0x80 && addr_hi <= 0x9F) {
				write_vram(addr & 0x1FFF,data);
			}
			// SRAM
			else if(addr_hi >= 0xA0 && addr_hi <= 0xBF) {
				m_mapper->write_sram(addr & 0x1FFF,data);
			} 
			// WRAM
			else if(addr_hi >= 0xC0 && addr_hi <= 0xDF) {
				write_wram(addr & 0x1FFF,data);
			}
			// echo RAM
			else if(addr_hi >= 0xE0 && addr_hi <= 0xFD) {
				write_wram(addr & 0x1FFF,data);
			}
			// OAM
			else if(addr_hi == 0xFE) {
				if(addr_lo < 0xA0) {
					if(!oam_accessible()) {
						std::puts("attempt to write to OAM while inaccessible");
						emu()->cpu.print_status();
					} else {
						m_oam[addr_lo] = data;
					}
				} else {
					/*std::printf(
						"warning: write to prohibited $FExx region [@%04Xh][%02Xh]\n",
						addr,data
					);*/
				}
			}
			// HRAM
			else if(addr_hi == 0xFF) {
				write_hram(addr_lo,data);
			} 
			// unknown...
			else {
				std::printf("unimplemented: RAM write (%04zXh)\n",addr);
				std::exit(-1);
			}
		}	
	}
	auto CMem::write_wram(int addr,int data) -> void {
		data &= 0xFF;
		addr &= 0x1FFF;

		if(addr >= 0x1000 && emu()->cgb_enabled()) {
			int wrambank = m_io.m_SVBK;
			if(wrambank == 0) wrambank = 1;
			addr &= 0xFFF;
			addr += KBSIZE(4) * wrambank;
		}
		m_wram[addr] = data;
	}
	auto CMem::write_vram(int addr,int data) -> void {
		addr &= 0x1FFF;

		if(!vram_accessible()) return;
		addr += KBSIZE(8) * m_io.m_VBK;
		m_vram[addr] = data;
		//	std::printf("attempt to write to vram in mode 3 (%04Xh)\n",addr);
		//	emu()->cpu.print_status();
		//	std::exit(-1);
	}
	auto CMem::write_hram(int addr,int data) -> void {
		addr &= 0xFF;
		data &= 0xFF;

		if(addr == 0xFF) {
			m_io.m_IE = data & 0b11111;
		} else if(addr>>7) {
			m_hram[addr & 0x7F] = data;
		} else {
			switch(addr) {
				// joypad -------------------------------@/
				case 0x00: { // JOYP
					int seldata = data & 0x30;
					if(seldata == 0x30) {
						//std::printf("warning: double-write to joyp\n");
						//emu()->cpu.print_status();
						break;
					}
					
					// directionals
					if(data & BIT(4)) {
						m_io.m_joypmode = fern::JOYPMode::button;
					} 
					// buttons
					else if(data & BIT(5)) {
						m_io.m_joypmode = fern::JOYPMode::direction;
					}
					break;
				}
				case 0x01: { // SB
					//std::printf("warning: unimplemented link cable write ($%02X)\n",data);
					m_io.m_SB = data;
					break;
				}
				case 0x02: { // SC
					m_io.m_SC = data;
					break;
				}
				case 0x04: { // DIV
					m_io.m_DIV = 0;
					break;
				}
				case 0x05: { // TIMA
					m_io.m_TIMA = data;
					break;
				}
				case 0x06: { // TMA
					m_io.m_TMA = data;
					break;
				}
				case 0x07: { // TAC
					m_io.m_TAC = data;
					break;
				}
				case 0x0F: { // IF
					m_io.m_IF = data & 0b11111;
					break;
				}
				// sound --------------------------------@/
				case 0x10: { // NR10
					m_io.m_NR10 = data;
					break;
				}
				case 0x11: { // NR11
					m_io.m_NR11 = data;
					break;
				}
				case 0x12: { // NR12
					m_io.m_NR12 = data;
					break;
				}
				case 0x13: { // NR13
					m_io.m_NR13 = data;
					break;
				}
				case 0x14: { // NR14
					constexpr int mask = 0b11000111;
					m_io.m_NR14 = data & mask;
					break;
				}
				case 0x15: break;

				case 0x16: { // NR21
					m_io.m_NR21 = data;
					break;
				}
				case 0x17: { // NR22
					m_io.m_NR22 = data;
					break;
				}
				case 0x18: { // NR23
					m_io.m_NR23 = data;
					break;
				}
				case 0x19: { // NR24
					constexpr int mask = 0b11000111;
					m_io.m_NR24 = data & mask;
					break;
				}

				case 0x1A: { // NR30
					m_io.m_NR30 = data & 0x80;
					break;
				}
				case 0x1B: { // NR31
					m_io.m_NR31 = data;
					break;
				}
				case 0x1C: { // NR32
					m_io.m_NR32 = data & (0b11<<5);
					break;
				}
				case 0x1D: { // NR33
					m_io.m_NR33 = data;
					break;
				}
				case 0x1E: { // NR34
					constexpr int mask = 0b11000111;
					m_io.m_NR34 = data & mask;
					break;
				}
				case 0x1F: break;

				case 0x20: { // NR41
					m_io.m_NR41 = data & 63;
					break;
				}
				case 0x21: { // NR42
					m_io.m_NR42 = data;
					break;
				}
				case 0x22: { // NR43
					m_io.m_NR43 = data;
					break;
				}
				case 0x23: { // NR44
					m_io.m_NR44 = data & 0xC0;
					break;
				}
			
				case 0x24: { // NR50
					m_io.m_NR50 = data;
					break;
				}
				case 0x25: { // NR51
					m_io.m_NR51 = data;
					break;
				}
				case 0x26: { // NR52
					m_io.m_NR52 &= 0x7F;
					m_io.m_NR52 |= (data & 0x80);
					break;
				}

				case 0x30: case 0x31: case 0x32: case 0x33:
				case 0x34: case 0x35: case 0x36: case 0x37:
				case 0x38: case 0x39: case 0x3A: case 0x3B:
				case 0x3C: case 0x3D: case 0x3E:
				case 0x3f: { // wave RAM
					int wave_idx = addr & 0xF;
					m_io.m_waveram[wave_idx] = data & 0xF;
					break;
				}
				// video --------------------------------@/
				case 0x40: { // LCDC
					if(!m_io.ppu_enabled() && (data & 0x80)) {
						emu()->cpu.m_dotclock = 0;
						emu()->cpu.m_lycCooldown = true;
						emu()->renderer.present();
					}
				//	std::printf("LCDC write ($%02X)\n",data);
					/*if(data == 0) {
						emu()->cpu.print_status();
						emu()->debug_set(true);
					}*/	
					m_io.m_LCDC = data;
					break;
				}
				case 0x41: { // STAT
					m_io.m_STAT &= 0b111;
					m_io.m_STAT |= (data & 0b01111000);
					break;
				}
				case 0x42: { // SCY
					m_io.m_SCY = data;
					break;
				}
				case 0x43: { // SCX
					m_io.m_SCX = data;
					break;
				}
				case 0x44: { // LY
					emu()->cpu.m_lycCooldown = true;
					emu()->cpu.m_dotclock = 0;
					m_io.m_LY = data;
					break;
				}
				case 0x45: { // LYC
					m_io.m_LYC = data;
					break;
				}
				case 0x46: { // OAM DMA
					// source addr: $XX00
					// output addr: $FE00
					// length: $A0
					int src_addr = (data<<8);
					if(!emu()->cpu.speed_doubled()) {
						for(int i=0; i<0xA0; i++) {
							m_oam[i] = read(src_addr + i);
							emu()->cpu.clock_tick(1);
						}
					} else {
						for(int i=0; i<0xA0; i += 2) {
							m_oam[i] = read(src_addr + i);
							m_oam[i+1] = read(src_addr + i + 1);
							emu()->cpu.clock_tick(1);
						}
					}
					break;
				}
				case 0x47: { // BGP
					m_io.m_BGP = data;
					break;
				}
				case 0x48: { // OBP0
					m_io.m_OBP[0] = data;
					break;
				}
				case 0x49: { // OBP1
					m_io.m_OBP[1] = data;
					break;
				}
				case 0x4A: { // WY
					m_io.m_WY = data;
					break;
				}
				case 0x4B: { // WX
					m_io.m_WX = data;
					break;
				}
				
				case 0x4D: { // KEY1
					if(emu()->cgb_enabled()) {
						m_io.m_KEY1 = (m_io.m_KEY1 & BIT(7)) | (data & 1);
					} else {
						warn_cgb_reg("VBK",data);
					}
					break;
				}
				case 0x4F: { // VBK
					if(emu()->cgb_enabled()) {
						m_io.m_VBK = data & 1;
					} else {
						warn_cgb_reg("VBK",data);
					}
					break;
				}
				
				// CGB? ---------------------------------@/
				case 0x51: { // HDMA1
					if(emu()->cgb_enabled()) {
						m_io.m_HDMA1 = data;
					} else {
						warn_cgb_reg("HDMA1",data);
					}
					break;
				}
				case 0x52: { // HDMA2
					if(emu()->cgb_enabled()) {
						m_io.m_HDMA2 = data;
					} else {
						warn_cgb_reg("HDMA2",data);
					}
					break;
				}
				case 0x53: { // HDMA3
					if(emu()->cgb_enabled()) {
						m_io.m_HDMA3 = data & 0x1F;
					} else {
						warn_cgb_reg("HDMA3",data);
					}
					break;
				}
				case 0x54: { // HDMA4
					if(emu()->cgb_enabled()) {
						m_io.m_HDMA4 = data & 0xF0;
					} else {
						warn_cgb_reg("HDMA4",data);
					}
					break;
				}
				case 0x55: { // HDMA5
					if(emu()->cgb_enabled()) {
						auto& cpu = emu()->cpu;
						const bool is_general = (data & RFlagHDMA::hblank) == 0;
						const int block_count = (data & 0x7F) + 1;
						const int wait_time = cpu.speed_doubled() ? 16 : 8;

						if(is_general) {
							auto addr_src = m_io.hdma_getSource();
							auto addr_out = m_io.hdma_getOutput();

							for(int y=0; y<block_count; y++) {
								for(int x=0; x<0x10; x++) {
									write_vram(addr_out,read(addr_src));
									addr_out++;
									addr_src++;
								}
								cpu.clock_tick(wait_time);
							}
							m_io.m_HDMA5 = 0xFF;
						} else {
							std::printf("hblank dma unimplemented...\n");
							cpu.print_status();
							std::exit(-1);
						}
					} else {
						warn_cgb_reg("HDMA5",data);
					}
					break;
				}
				
				case 0x68: { // BGPI
					if(emu()->cgb_enabled()) {
						m_io.m_BGPI = data & (0x3F | RFlagPalet::autoincr);
					} else {
						warn_cgb_reg("BGPI",data);
					}
					break;
				}
				case 0x69: { // BGPD
					if(emu()->cgb_enabled()) {
						int index = m_io.bgpal_index();
						
						m_paletBG[index] = data;
						
						index += (m_io.m_BGPI & RFlagPalet::autoincr) ? 1 : 0;
						index &= 0x3F;
						m_io.m_BGPI = (m_io.m_BGPI & RFlagPalet::autoincr) | index;
					} else {
						warn_cgb_reg("BGPD",data);
					}
					break;
				}
				case 0x6A: { // OBPI
					if(emu()->cgb_enabled()) {
						m_io.m_OBPI = data & (0x3F | RFlagPalet::autoincr);
					} else {
						warn_cgb_reg("OBPI",data);
					}
					break;
				}
				case 0x6B: { // OBPD
					if(emu()->cgb_enabled()) {
						int index = m_io.objpal_index();
						
						m_paletObj[index] = data;
						
						index += (m_io.m_OBPI & RFlagPalet::autoincr) ? 1 : 0;
						index &= 0x3F;
						m_io.m_OBPI = (m_io.m_OBPI & RFlagPalet::autoincr) | index;
					} else {
						warn_cgb_reg("OBPD",data);
					}
					break;
				}
				case 0x70: { // SVBK
					if(emu()->cgb_enabled()) {
						m_io.m_SVBK = data & 0b111;
					} else {
						warn_cgb_reg("SVBK",data);
						emu()->cpu.print_status();
					}
					break;
				}
						   
				// unknown ------------------------------@/
				/*case 0x6C:
				case 0x6D:
				case 0x6E:
				case 0x6F: break;

				case 0x71:
				case 0x72:
				case 0x73:
				case 0x74:
				case 0x75:
				case 0x76:
				case 0x77: break;
				*/

				case 0x78:
				case 0x79:
				case 0x7A:
				case 0x7B:
				case 0x7C:
				case 0x7D:
				case 0x7E:
				case 0x7F: break;

				default: {
					std::printf("unimplemented: IO write (%02Xh)\n",addr);
					emu()->cpu.print_status();
					std::exit(-1);
					break;
				}
			}
		}
	}

	// general mapper -----------------------------------@/
	auto CMapper::error_unimpl(const std::string& message) -> void {
		std::printf("mapper '%s': error: unimplemented (%s)\n",
			name().c_str(),message.c_str()
		);
		std::exit(-1);
	}

	// none mapper --------------------------------------@/
	auto CMapperNone::read_rom(size_t addr) -> uint32_t {
		return m_emu->mem.m_rombanks[0].data[addr & 0x7FFF];
	};

	auto CMapperNone::read_sram(size_t addr) -> uint32_t {
		//error_unimpl("true SRAM read");
		return 0xFF;
	}
	auto CMapperNone::write_sram(size_t addr, int data) -> void {
		//error_unimpl("true SRAM write");
	}
	auto CMapperNone::write_rom(size_t addr, int data) -> void {
	}

	// MBC1 ---------------------------------------------@/
	CMapperMBC1::CMapperMBC1(bool use_ram, bool use_battery) {
		m_banknum = 1;
		m_banknum_hi = 0;
		m_ramEnabled = 0;
		m_rambankmode = false;

		m_useram = use_ram;
		m_usebattery = use_battery;
	}
	
	auto CMapperMBC1::read_rom(size_t addr) -> uint32_t {
		if((addr>>14) == 0) {
			return m_emu->mem.m_rombanks[0].data[addr];
		}
		addr &= 0x3FFF;
		return m_emu->mem.m_rombanks[rombank_get()].data[addr];
	};

	auto CMapperMBC1::read_sram(size_t addr) -> uint32_t {
		if(!m_useram) { return 0; }
		//if(!m_usebattery) { return 0; }
		
		addr &= 0x1FFF;
		return emu()->mem.m_sram[addr];
		//error_unimpl("true SRAM read");
	}
	
	// TODO: make this good, check for banking, etc.
	auto CMapperMBC1::write_sram(size_t addr, int data) -> void {
		if(!m_useram) { return; }
		//if(!m_usebattery) { return; }
		addr &= 0x1FFF;

		emu()->mem.m_sram[addr] = data;
		//error_unimpl("true SRAM write");
	}
	auto CMapperMBC1::write_rom(size_t addr, int data) -> void {
		addr &= 0x7FFF;
		int reg_num = addr >> 13;
		switch(reg_num) {
			// RAM enable -------------------------------@/
			case 0: {
				if(data == 0xA) {
					std::puts("mbc: RAM enabled");
					m_ramEnabled = true;
				} else {
					m_ramEnabled = false;
				}
				break;
			}
			// ROM bank number --------------------------@/
			case 1: {
				// deal with MBC1 bank issue (setting 0 == setting 1)
				if(data == 0) {
					m_banknum = 1;
				} else {
					m_banknum = data & 0b11111;
				}
				break;
			}
			// RAM bank number/upper ROM bits -----------@/
			case 2: {
				std::printf("mbc: RAM bank switch (%d)\n",data & 0b11);
				m_banknum_hi = data & 0b11;
				break;
			}
			// RAM bank mode ----------------------------@/
			case 3: {
				m_rambankmode = data & 1;
				break;
			}
			default: {
				std::printf("unsupported MBC1 reg %d\n",reg_num);
				std::exit(-1);
				break;
			}
		}
		//std::printf("MBC1 write: [%04Xh](%02Xh)\n",addr,data);
		//emu()->cpu.print_status();
	}

	auto CMapperMBC1::sram_serialize() -> Blob {
		Blob datablob;

		if(m_usebattery) {
			for(int i=0; i<KBSIZE(8); i++) {
				datablob.write_u8(emu()->mem.m_sram[i]);
			}
		}

		return datablob;
	}

	// MBC5 ---------------------------------------------@/
	CMapperMBC5::CMapperMBC5(bool use_ram, bool use_battery, bool use_rumble) {
		m_rambanknum = 0;
		m_rombanknum = 0;
		m_rombanknum_hi = 0;

		m_ramEnabled = false;
		m_rambankmode = false;
		m_useram = use_ram;
		m_usebattery = use_battery;
		m_userumble = use_rumble;
	}
	
	auto CMapperMBC5::read_rom(size_t addr) -> uint32_t {
		if((addr>>14) == 0) {
			return m_emu->mem.m_rombanks[0].data[addr];
		}
		addr &= 0x3FFF;
		return m_emu->mem.m_rombanks[rombank_get()].data[addr];
	};

	auto CMapperMBC5::read_sram(size_t addr) -> uint32_t {
		if(!m_useram) { return 0; }
		//if(!m_usebattery) { return 0; }
		
		addr &= 0x1FFF;
		return emu()->mem.m_sram[addr];
		//error_unimpl("true SRAM read");
	}
	
	// TODO: make this good, check for banking, etc.
	auto CMapperMBC5::write_sram(size_t addr, int data) -> void {
		if(!m_useram) { return; }
		//if(!m_usebattery) { return; }
		addr &= 0x1FFF;

		emu()->mem.m_sram[addr] = data;
		//error_unimpl("true SRAM write");
	}
	auto CMapperMBC5::write_rom(size_t addr, int data) -> void {
		addr &= 0x7FFF;
		data &= 0xFF;

		int reg_num = addr >> 13;
		switch(reg_num) {
			// RAM enable -------------------------------@/
			case 0: {
				if(data == 0xA) {
					//std::puts("mbc: RAM enabled");
					m_ramEnabled = true;
				} else {
					m_ramEnabled = false;
				}
				break;
			}
			// ROM bank number --------------------------@/
			case 1: {
				m_rombanknum = data;
				break;
			}
			// ROM bank number --------------------------@/
			case 2: {
				m_rombanknum_hi = data & 1;
				break;
			}
			// RAM bank number --------------------------@/
			case 3: {
				std::printf("mbc: RAM bank switch (%d)\n",data & 0x0F);
				m_rambanknum = data & 0x0F;
				break;
			}
			default: {
				std::printf("unsupported MBC5 reg %d\n",reg_num);
				std::exit(-1);
				break;
			}
		}
	}

	auto CMapperMBC5::sram_serialize() -> Blob {
		Blob datablob;

		if(m_usebattery) {
			const int bank_count = emu()->mem.m_rambankCount;
			for(int i=0; i<KBSIZE(8) * bank_count; i++) {
				datablob.write_u8(emu()->mem.m_sram[i]);
			}
		}

		return datablob;
	}

};

