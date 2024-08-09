#include <fern.h>

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
			// $C000-$CFFF : WRAM bank0
			if(addr_hi >= 0xC0 && addr_hi <= 0xCF) {
				return m_wram.at(addr & 0x0FFF);
			}
			// $D000-$DFFF : WRAM bank1-7
			else if(addr_hi >= 0xC0 && addr_hi <= 0xDF) {
				return m_wram.at(4*1024 + (addr & 0x0FFF));
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
					return 0x3F; // no buttons pressed
				}
				// video --------------------------------@/
				case 0x41: return m_io.m_STAT;
				case 0x44: return m_io.m_LY;
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
				m_wram[addr & 0x1FFF] = data;
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
	auto CMem::write_vram(int addr,int data) -> void {
		addr &= 0x1FFF;

		if(vram_accessible()) {
			m_vram[addr] = data;
		} else {
			std::printf("attempt to write to vram in mode 3 (%04Xh)\n",addr);
			emu()->cpu.print_status();
			std::exit(-1);
		}
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
					std::printf("warning: write to JOYP register (%02Xh)\n",data);
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
				// video --------------------------------@/
				case 0x40: { // LCDC
					if(!m_io.ppu_enabled() && (data & 0x80)) {
						emu()->cpu.m_dotclock = 0;
						emu()->renderer.present();
					}
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
				case 0x45: { // LYC
					m_io.m_LYC = data;
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
				
				// CGB? ---------------------------------@/
				case 0x68: { // BGPI
					if(emu()->is_cgb()) {
						std::puts("CGB palettes unimplemented");
						std::exit(-1);
						//m_io.m_BGPI = data & 0b111;
					} else {
						warn_cgb_reg("BGPI",data);
					}
					break;
				}
				case 0x69: { // BGPD
					if(emu()->is_cgb()) {
						std::puts("CGB palettes unimplemented");
						std::exit(-1);
						//m_io.m_BGPD = data & 0b111;
					} else {
						warn_cgb_reg("BGPD",data);
					}
					break;
				}
				case 0x6A: { // OBPI
					if(emu()->is_cgb()) {
						std::puts("CGB palettes unimplemented");
						std::exit(-1);
						//m_io.m_OBPI = data & 0b111;
					} else {
						warn_cgb_reg("OBPI",data);
					}
					break;
				}
				case 0x6B: { // OBPD
					if(emu()->is_cgb()) {
						std::puts("CGB palettes unimplemented");
						std::exit(-1);
						//m_io.m_OBPD = data & 0b111;
					} else {
						warn_cgb_reg("OBPD",data);
					}
					break;
				}
				case 0x70: { // SVBK
					if(emu()->is_cgb()) {
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
		if(!m_usebattery) { return 0; }
		addr &= 0x1FFF;

		std::printf("unimplemented (true SRAM read)\n");
		std::exit(-1);
	}
	
	auto CMapperMBC1::write_sram(size_t addr, int data) -> void {
		if(!m_useram) { return; }
		if(!m_usebattery) { return; }
		addr &= 0x1FFF;

		std::printf("unimplemented (true SRAM write)\n");
		std::exit(-1);
	}
	auto CMapperMBC1::write_rom(size_t addr, int data) -> void {
		addr &= 0x7FFF;
		int reg_num = addr >> 13;
		switch(reg_num) {
			case 0: {
				if(data == 0xA) {
					m_ramEnabled = true;
				} else {
					m_ramEnabled = false;
				}
				break;
			}
			case 1: {
				// deal with MBC1 bank issue (setting 0 == setting 1)
				if(data == 0) {
					m_banknum = 1;
				} else {
					m_banknum = data & 0b11111;
				}
				break;
			}
			case 2: {
				m_banknum_hi = data & 0b11;
				break;
			}
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
};

