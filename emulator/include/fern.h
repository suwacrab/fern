#ifndef FERN_H
#define FERN_H

#include <array>
#include <cstdint>

#include <deque>
#include <vector>
#include <stack>
#include <string>

#include <blob.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

namespace fern {
	class CCPU;
	class CEmulator;
	class CEmulatorComponent;
	class CRenderer;

	constexpr int SCREEN_X = 160;
	constexpr int SCREEN_Y = 144;
	constexpr int SCREENVRAM_X = 8 * 32;
	constexpr int SCREENVRAM_Y = 8 * 24;

	constexpr int KBSIZE(int n) { return 1024 * n; }
	constexpr int MBSIZE(int n) { return KBSIZE(1024) * n; }

	namespace RFlagMapAttrib {
		constexpr auto bank(int attr) -> int { return (attr>>3)&1; }
		constexpr auto flipX(int attr) -> int { return (attr>>5)&1; }
		constexpr auto flipY(int attr) -> int { return (attr>>6)&1; }
	};
	namespace RFlagPalet {
		enum {
			autoincr = 0x80
		};
	}
	namespace RFlagHDMA {
		enum {
			general = 0,
			hblank = 0x80
		};
	}
	namespace RFlagLCDC {
		enum {
			bgon = 0x01,
			objon = 0x02,
			obj16 = 0x04,
			map9C00 = 0x08,
			chr8000 = 0x10,
			winon = 0x20,
			win9C00 = 0x40,
			lcdon = 0x80
		};
	}
	namespace RFlagSTAT {
		enum {
			mode0int = 0x08,
			mode1int = 0x10,
			mode2int = 0x20,
			lycint = 0x40	
		};
	}
	namespace RFlagIF {
		enum {
			vblank = 0x01,
			stat = 0x02,
			timer = 0x04
		};
	}

	namespace EmuButton {
		enum {
			up,
			down,
			left,
			right,
			a,b,start,select,
			num_keys
		};
	}
	namespace RegisterName {
		enum {
			B,C,
			D,E,
			H,L,
			HLData,A
		};
		constexpr auto is_hldata(int id) -> bool {
			return id == HLData;
		}
	}
	namespace JOYPMode {
		enum {
			button,direction
		};
	}

	class CEmulatorComponent {
		protected:
			CEmulator* m_emu;
		public:
			constexpr auto emu() { return m_emu; }
			auto assign_emu(CEmulator* emu) {
				m_emu = emu;
			}
	};

	// renderer -----------------------------------------@/
	struct CColor {
		short a,r,g,b;
		CColor() : r(0),g(0),b(0) {}
		CColor(int alpha) : a(alpha),r(0),g(0),b(0) {}
		CColor(int p_r, int p_g, int p_b) : a(255),r(p_r),g(p_g),b(p_b) {}

		static auto from_rgb15(int clrdat) -> CColor;
	};
	class CScreen {
		private:
			std::vector<CColor> m_bmp;
			int m_width, m_height;
		public:
			CScreen(int width, int height);

			auto render_toSurface(SDL_Surface* surface) -> void;

			auto clear(CColor color) -> void;
			auto dot_set(int x, int y, CColor color) -> void;
			constexpr auto dot_access(int x, int y) -> CColor& {
				return m_bmp[x + y * width()];
			}

			auto in_range(int x, int y) -> bool;
			constexpr auto width() const -> int { return m_width; }
			constexpr auto height() const -> int { return m_height; }
			constexpr auto dimensions() const -> int { return width() * height(); }
	};
	class CRenderer : public CEmulatorComponent {
		private:
			SDL_Window* m_window;
			SDL_Window* m_windowVRAM;
			SDL_Renderer* m_renderer;
			CScreen m_screen;
			CScreen m_screenVRAM;
			int m_timeLastFrame;
			int m_savetimer;
			bool m_vsyncEnabled;
		public:
			CRenderer();
			~CRenderer();

			auto window_create(bool vsync) -> void;

			auto render_vramwindow() -> void;
			auto present() -> void;
			auto draw_line(int draw_y) -> void;
			auto draw_lineDMG(int draw_y) -> void;
			auto draw_lineCGB(int draw_y) -> void;

			constexpr auto vsync_set(bool enable) -> void { m_vsyncEnabled = enable; }
			constexpr auto vsync_enabled() const -> bool { return m_vsyncEnabled; }
	};

	// mapper -------------------------------------------@/
	class CMapper : public CEmulatorComponent {
		private:
		public:
			auto error_unimpl(const std::string& msg) -> void;
			virtual auto name() const -> std::string = 0;
			virtual auto read_rom(size_t addr) -> uint32_t = 0;
			virtual auto read_sram(size_t addr) -> uint32_t = 0;
			virtual auto write_sram(size_t addr, int data) -> void = 0;
			virtual auto write_rom(size_t addr, int data) -> void = 0;
			virtual auto sram_serialize() -> Blob = 0;
			virtual ~CMapper() {}
			
			virtual auto rom_bank() const -> int = 0;
	};
	class CMapperNone : public CMapper {
		private:
		public:
			auto name() const -> std::string { return "none"; };
			auto read_rom(size_t addr) -> uint32_t;
			auto read_sram(size_t addr) -> uint32_t;
			auto write_sram(size_t addr, int data) -> void;
			auto write_rom(size_t addr, int data) -> void;
			auto sram_serialize() -> Blob { return Blob(); }
			CMapperNone() {}
			~CMapperNone() {}

			auto rom_bank() const -> int { return 0; }
	};
	class CMapperMBC1 : public CMapper {
		private:
			int m_banknum;
			int m_banknum_hi;
			bool m_ramEnabled;
			bool m_useram;
			bool m_usebattery;
			bool m_rambankmode;
		public:
			auto name() const -> std::string { return "MBC1"; };
			auto read_rom(size_t addr) -> uint32_t;
			auto read_sram(size_t addr) -> uint32_t;
			auto write_sram(size_t addr, int data) -> void;
			auto write_rom(size_t addr, int data) -> void;
			auto sram_serialize() -> Blob;
			
			CMapperMBC1(bool use_ram, bool use_battery);
			~CMapperMBC1() {}

			auto rombank_get() const -> int {
				if(m_rambankmode) {
					return m_banknum;
				} else {
			   		return m_banknum | (m_banknum_hi<<5);
				}
			}
			auto rom_bank() const -> int {
				return rombank_get();
			}
	};
	class CMapperMBC3 : public CMapper {
		private:
			int m_rambanknum;
			int m_rombanknum;
			
			bool m_sramIsRTC;
			bool m_ramEnabled;
			bool m_useram;
			bool m_usebattery;
			bool m_rambankmode;
			bool m_usetimer;

			int m_rtcReg;
			int m_rtcDay;
			int m_rtcSec,m_rtcMin,m_rtcHour;
			int m_rtcLatchReady;
		public:
			auto name() const -> std::string { return "MBC3"; };
			auto read_rom(size_t addr) -> uint32_t;
			auto read_sram(size_t addr) -> uint32_t;
			auto write_sram(size_t addr, int data) -> void;
			auto write_rom(size_t addr, int data) -> void;
			auto sram_serialize() -> Blob;
			
			CMapperMBC3(bool use_ram, bool use_battery, bool use_timer);
			~CMapperMBC3() {}

			auto rombank_get() const -> int {
				return m_rombanknum;
			}
			auto rom_bank() const -> int {
				return rombank_get();
			}
	};
	class CMapperMBC5 : public CMapper {
		private:
			int m_rambanknum;
			int m_rombanknum;
			int m_rombanknum_hi;
			
			bool m_ramEnabled;
			bool m_useram;
			bool m_usebattery;
			bool m_rambankmode;
			bool m_userumble;
		public:
			auto name() const -> std::string { return "MBC5"; };
			auto read_rom(size_t addr) -> uint32_t;
			auto read_sram(size_t addr) -> uint32_t;
			auto write_sram(size_t addr, int data) -> void;
			auto write_rom(size_t addr, int data) -> void;
			auto sram_serialize() -> Blob;
			
			CMapperMBC5(bool use_ram, bool use_battery, bool use_rumble);
			~CMapperMBC5() {}

			auto rombank_get() const -> int {
				return m_rombanknum | (m_rombanknum_hi<<8);
			}
			auto rom_bank() const -> int {
				return rombank_get();
			}
	};

	// memory -------------------------------------------@/
	struct CRomBank {
		std::array<uint8_t,KBSIZE(16)> data;
	};
	struct CMemIO {
		bool m_joypmode;
		
		int m_IF;
		int m_IE;
		int m_SB,m_SC;

		uint8_t m_DIV,m_TIMA,m_TMA,m_TAC;

		// 10h
		int m_NR10,m_NR11,m_NR12,m_NR13,m_NR14;
		int m_NR21,m_NR22,m_NR23,m_NR24;
		int m_NR30,m_NR31,m_NR32,m_NR33,m_NR34;
		int m_NR41,m_NR42,m_NR43,m_NR44;

		int m_NR50;
		int m_NR51;
		int m_NR52;

		int m_waveram[0x10];

		// 40h
		int m_LCDC;
		int m_STAT;
		int m_SCY,m_SCX;

		int m_LY;
		int m_LYC;
		int m_BGP;
		int m_OBP[2];
		int m_WY,m_WX;

		int m_VBK;
		int m_SVBK;

		int m_KEY1;

		// 50h
		int m_HDMA1,m_HDMA2,m_HDMA3,m_HDMA4;
		int m_HDMA5;
		
		// 60h
		int m_BGPI,m_BGPD;
		int m_OBPI,m_OBPD;

		auto ppu_enabled() const -> bool {
			return (m_LCDC & 0x80) != 0;
		}

		auto stat_getMode() const -> int {
			return m_STAT & 0b11;
		}
		auto stat_setMode(int mode) -> void {
			m_STAT &= (0xFF ^ 0b11);
			m_STAT |= (mode & 0b11);
		}
		auto stat_setLYC(bool flag) -> void {
			m_STAT &= (0xFF ^ 0x04);
			if(flag) m_STAT |= 0x04;
		}
		auto stat_lycSame() const -> bool {
			return (m_STAT & 0x04) != 0;
		}
		constexpr auto hdma_getSource() const -> int {
			return m_HDMA2 | (m_HDMA1<<8);
		}
		constexpr auto hdma_getOutput() const -> int {
			return m_HDMA4 | (m_HDMA3<<8);
		}
		constexpr auto palreg_index(const int& reg) const -> int {
			return reg & 0x3F;
		}
		constexpr auto bgpal_index() const -> int { return palreg_index(m_BGPI); }
		constexpr auto objpal_index() const -> int { return palreg_index(m_OBPI); }
	};
	class CMem : public CEmulatorComponent {
		public:
			std::array<uint8_t,2 * KBSIZE(8)> m_vram; // 2x8kib
			std::array<uint8_t,16 * KBSIZE(8)> m_sram; // 16*8kib
			std::array<uint8_t,8 * KBSIZE(4)> m_wram; // 8x4kib
			std::array<uint8_t,160> m_oam; // 4*40b
			std::array<uint8_t,128> m_hram; // 128b
			std::array<uint8_t,64> m_paletObj;
			std::array<uint8_t,64> m_paletBG;
			CMemIO m_io;
			std::array<CRomBank,256> m_rombanks;

			int m_rambankCount;
			int m_rombankCount;

			CMapper* m_mapper;

			CMem();
			~CMem();

			auto reset() -> void;

			auto mapper_setupNone() -> void;
			auto mapper_setupMBC1(bool use_ram, bool use_battery) -> void;
			auto mapper_setupMBC3(bool use_ram, bool use_battery, bool use_timer) -> void;
			auto mapper_setupMBC5(bool use_ram, bool use_battery, bool use_rumble) -> void;

			auto interrupt_match(int mask) -> bool;
			auto interrupt_clear(int mask) -> void;
			auto rombank_current() -> int;
			auto oam_accessible() -> bool {
				return ((m_io.stat_getMode()&1) == 0)
					|| (!m_io.ppu_enabled())
					|| (m_io.m_LY >= fern::SCREEN_Y);
			}
			auto vram_accessible() -> bool {
				return (m_io.stat_getMode() != 3)
					|| (!m_io.ppu_enabled())
					|| (m_io.m_LY >= fern::SCREEN_Y);
			}
			auto addr_inRange(size_t addr, size_t min,size_t max) -> bool {
				return (addr >= min) && (addr < max);
			}
			auto read(size_t addr) -> uint32_t;
			auto read_hram(int addr) -> uint32_t;
			auto read_wram(int addr) -> int;
			auto write(size_t addr, int data) -> void;
			auto write_wram(int addr,int data) -> void;
			auto write_hram(int addr,int data) -> void;
			auto write_vram(int addr,int data) -> void;

			auto stat_lycSync() -> void {
				m_io.stat_setLYC(m_io.m_LY == m_io.m_LYC);
			}
	};

	// CPU ----------------------------------------------@/
	typedef void (*CCPUInstrFn)(CCPU*,CEmulator*);
	typedef void (*CCPUInstrPfxFn)(CCPU*,CEmulator*,int,int);
	struct CCPUInstrBase {
		std::string name;
		CCPUInstrBase() {}
	};
	struct CCPUInstr : public CCPUInstrBase {
		CCPUInstrFn fn;
		CCPUInstr() {}
		CCPUInstr(CCPUInstrFn fn_ptr,std::string instr_name) {
			fn = fn_ptr;
			name = instr_name;
		}
	};
	struct CCPUInstrPfx : public CCPUInstrBase {
		CCPUInstrPfxFn fn;
		CCPUInstrPfx() {}
		CCPUInstrPfx(CCPUInstrPfxFn fn_ptr,std::string instr_name) {
			fn = fn_ptr;
			name = instr_name;
		}
	};

	struct CInstrHistoryData {
		int pc;
		int bank;
		std::vector<int> opcodedata;

		CInstrHistoryData() 
			: pc(0),bank(0),opcodedata({0})
			{}
		CInstrHistoryData(int mem_bank, int pcaddr, const std::vector<int>& opdata) {
			pc = pcaddr;
			bank = mem_bank;
			opcodedata = opdata;
		}

		constexpr int opcode_num() const { return opcodedata.at(0); }
	};
	
	struct CCycle {
		int num;
		CCycle(int n) : num(n << 8) {}
		auto full_cycles() -> int { return num >> 8; }
	};

	class CCPU : public CEmulatorComponent {
		private:
			std::array<CCPUInstr,0x100> m_opcodetable;
			std::array<CCPUInstrPfx,0x10> m_opcodetable_pfx;
		public:
			bool m_speedDoubled;
			bool m_should_enableIME;
			bool m_regIME;
			bool m_haltwaiting;
			uint8_t m_regA,m_regF;
			uint8_t m_regB,m_regC;
			uint8_t m_regD,m_regE;
			uint8_t m_regH,m_regL;
			uint16_t m_SP;
			uint16_t m_PC;
			uint8_t m_curopcode;
			CCPUInstrBase* m_curopcode_ptr;

			bool m_lycCooldown;
			int m_dotclock;
			int m_dotclockLimit;
			int m_dotclockMode;
			bool m_clockWaiting;
			std::stack<int> m_clockWaitBuffer;

			int m_timerctrDiv;
			int m_timerctrMain;

			std::deque<CInstrHistoryData> m_instrhistory;

			CCPU();

			constexpr auto pc_set(std::size_t addr) { m_PC = addr; }
			constexpr auto pc_increment(std::size_t offset) { m_PC += offset; }

			constexpr auto speed_doubled() -> bool { return m_speedDoubled; }

			auto dotclock_reset() -> void;

			auto instrhistory_get(int index) -> CInstrHistoryData;
			auto instrhistory_push(int bank, int pc, const std::vector<int>& opcodedata) -> void;
			auto instrhistory_pushCurrent() -> void;

			auto flag_syncAnd(int opA,int opB) -> void;
			auto flag_syncAdd16(uint32_t opA,uint32_t opB) -> void;
			auto flag_syncCompare(int opA,int opB) -> void;
			auto flag_syncCompareAdd(int opA, int opB) -> void;
			auto flag_syncCompareInc(int operand) -> void;
			auto flag_syncCompareDec(int operand) -> void;
			
			auto read_pc(int offset) -> uint32_t;
			auto read_pc16(int offset) -> uint32_t;
			auto read_sp(int offset) -> uint32_t;
			auto read_sp16(int offset) -> uint32_t;
			auto stack_pop8() -> int;
			auto stack_pop16() -> int;
			auto stack_push8(int data) -> void;
			auto stack_push16(int data) -> void;
			
			auto jump_rel(int offset) -> void;
			auto call(int addr, int retaddr) -> void;
			auto calreturn(bool enable_intr = false) -> void;

			auto halt_waitStart() -> void { m_haltwaiting = true; }
			auto halt_isWaiting() -> bool { return m_haltwaiting; }
			auto clock_tick(int cyclecnt) -> void;

			auto opcode_clear() -> void;
			auto opcode_set(std::size_t index,CCPUInstr instr) -> void;
			auto opcode_setRaw(std::size_t index,CCPUInstr instr) -> void;
			auto opcode_setPrefix(std::size_t index,CCPUInstrPfx instr) -> void;

			auto print_status(bool instr_history = false) -> void;

			auto reset() -> void;
			auto step() -> void;
			auto execute_opcode() -> void;
			constexpr auto flag_setBit(int index, bool flag) -> void {
				m_regF &= (0xFF ^ (1<<index));
				m_regF |= flag<<index;
			}
			constexpr auto flag_setZero(bool flag) -> void {
				flag_setBit(7,flag);
			}
			constexpr auto flag_setSubtract(bool flag) -> void {
				flag_setBit(6,flag);
			}
			constexpr auto flag_setHalfcarry(bool flag) -> void {
				flag_setBit(5,flag);
			}
			constexpr auto flag_setCarry(bool flag) -> void {
				flag_setBit(4,flag);
			}
			constexpr auto flag_zero() -> bool { return (m_regF>>7)&1; }
			constexpr auto flag_subtract() -> bool { return (m_regF>>6)&1; }
			constexpr auto flag_halfcarry() -> bool { return (m_regF>>5)&1; }
			constexpr auto flag_carry() -> bool { return (m_regF>>4)&1; }
			constexpr auto reg_af() -> int { return (m_regA<<8) | m_regF; }
			constexpr auto reg_bc() -> int { return (m_regB<<8) | m_regC; }
			constexpr auto reg_de() -> int { return (m_regD<<8) | m_regE; }
			constexpr auto reg_hl() -> int { return (m_regH<<8) | m_regL; }
			auto hilo_split(int num, uint8_t& lo, uint8_t& hi) {
				lo = num & 0xFF;
				hi = (num>>8) & 0xFF;
			}
			constexpr auto bc_set(int num) -> void {
				hilo_split(num,m_regC,m_regB);
			}
			constexpr auto de_set(int num) -> void {
				hilo_split(num,m_regE,m_regD);
			}
			constexpr auto hl_set(int num) -> void {
				hilo_split(num,m_regL,m_regH);
			}
			constexpr auto sp_set(int num) -> void {
				m_SP = num;
			}

	};

	// emulator -----------------------------------------@/
	struct CEmuInitFlags {
		bool vsync;
		bool debug;
		bool verbose;

		CEmuInitFlags()
			: vsync(false),debug(false),verbose(false)
			{}
	};
	
	class CEmulator {
		private:
			bool m_quitflag;
			bool m_cgbEnabled;
			bool m_verboseEnable;
			bool m_debugEnable;
			bool m_debugSkipping;
			int m_debugSkipAddr;
			std::array<bool,EmuButton::num_keys> m_joypad_state;
			std::string m_romfilename;
		public:
			CCPU cpu;
			CMem mem;
			CRenderer renderer;

			CEmulator(const CEmuInitFlags* flags);

			auto savedata_sync() -> void;
			auto savedata_getFilename() -> std::string;

			auto process_message() -> void;
			auto button_held(int btn) -> bool;

			auto verbose_enabled() const -> bool { return m_verboseEnable; }
			auto cgb_enabled() const -> bool { return m_cgbEnabled; }
			auto debug_on() const -> bool { return m_debugEnable; }
			auto debug_set(bool enable) -> void { m_debugEnable = enable; }

			auto boot() -> void;
			auto load_romfile(const std::string& filename) -> void;
			auto quit() -> void { m_quitflag = true; }
			auto did_quit() -> bool { return m_quitflag; }
	};
};

#endif

