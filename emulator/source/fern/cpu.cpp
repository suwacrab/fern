#include <fern.h>

#define INSTRFN_NAME(name) instrfn_##name
#define INSTRFN(name) void INSTRFN_NAME(name)(fern::CCPU* cpu,fern::CEmulator* emu)
#define INSTRFN_PFX(name) void INSTRFN_NAME(name)(fern::CCPU* cpu,fern::CEmulator* emu, int register_id, int opcode_mode)

// bitwise AND
static INSTRFN(and_a_imm8);
static INSTRFN_PFX(andxor);
static INSTRFN_PFX(orcp);

// arithemetic
static INSTRFN(inc_b);
static INSTRFN(inc_c);
static INSTRFN(inc_d);
static INSTRFN(inc_e);
static INSTRFN(inc_h);
static INSTRFN(inc_l);
static INSTRFN(inc_a);

static INSTRFN(dec_b);
static INSTRFN(dec_c);
static INSTRFN(dec_d);
static INSTRFN(dec_e);
static INSTRFN(dec_h);
static INSTRFN(dec_l);

static INSTRFN(inc_bc);
static INSTRFN(inc_de);
static INSTRFN(inc_hl);
static INSTRFN(inc_sp);

static INSTRFN(dec_bc);
static INSTRFN(dec_de);
static INSTRFN(dec_hl);
static INSTRFN(dec_sp);

static INSTRFN(inc_hld);
static INSTRFN(dec_hld);

static INSTRFN(add_hl_bc);
static INSTRFN(add_hl_de);
static INSTRFN(add_hl_hl);
static INSTRFN(add_hl_sp);

static INSTRFN(add_a_imm8);
static INSTRFN_PFX(addadc);
static INSTRFN_PFX(subsbc);

// ld
static INSTRFN(ld_a_imm8);
static INSTRFN(ld_b_imm8);
static INSTRFN(ld_c_imm8);
static INSTRFN(ld_d_imm8);
static INSTRFN(ld_e_imm8);
static INSTRFN(ld_h_imm8);
static INSTRFN(ld_l_imm8);

static INSTRFN_PFX(ldbldc);
static INSTRFN_PFX(lddlde);
static INSTRFN_PFX(ldhldl);
static INSTRFN_PFX(ldhllda);

static INSTRFN(ld_bc_imm16);
static INSTRFN(ld_de_imm16);
static INSTRFN(ld_hl_imm16);
static INSTRFN(ld_sp_imm16);

static INSTRFN(ld_a_bc);
static INSTRFN(ld_a_de);
static INSTRFN(ld_bc_a);
static INSTRFN(ld_de_a);

static INSTRFN(ld_hli_a);
static INSTRFN(ld_hld_a);
static INSTRFN(ld_a_hli);
static INSTRFN(ld_a_hld);

static INSTRFN(ld_hl_imm8);

static INSTRFN(ld_a16_a);
static INSTRFN(ld_a_a16);

static INSTRFN(ldh_a8_a);
static INSTRFN(ldh_a_a8);
static INSTRFN(ldh_c_a);
static INSTRFN(ldh_a_c);

static INSTRFN(cp_imm8);

// jumps & calls
static INSTRFN(jp_imm16);
static INSTRFN(jp_hl);

static INSTRFN(jr);
static INSTRFN(jr_nz);
static INSTRFN(jr_z);
static INSTRFN(jr_nc);
static INSTRFN(jr_c);

static INSTRFN(call);
static INSTRFN(call_nz);
static INSTRFN(call_z);
static INSTRFN(call_nc);
static INSTRFN(call_c);

static INSTRFN(ret);
static INSTRFN(reti);
static INSTRFN(ret_nz);
static INSTRFN(ret_z);
static INSTRFN(ret_nc);
static INSTRFN(ret_c);

static INSTRFN(rst_00);
static INSTRFN(rst_10);
static INSTRFN(rst_20);
static INSTRFN(rst_30);

static INSTRFN(rst_08);
static INSTRFN(rst_18);
static INSTRFN(rst_28);
static INSTRFN(rst_38);

// stack
static INSTRFN(pop_bc);
static INSTRFN(pop_de);
static INSTRFN(pop_hl);
static INSTRFN(pop_af);

static INSTRFN(push_bc);
static INSTRFN(push_de);
static INSTRFN(push_hl);
static INSTRFN(push_af);

// CB prefix
static INSTRFN(prefix);

// misc
static INSTRFN(di);
static INSTRFN(ei);
static INSTRFN(halt);
static INSTRFN(nop);
static INSTRFN(cpl);

static INSTRFN(invalid);
static INSTRFN(unimplemented);
static INSTRFN_PFX(unimplemented_pfx);

namespace fern {
	CCPU::CCPU() {
		// setup instruction table ----------------------@/
		opcode_clear();
		
		// setup unimplemented instrs -------------------@/
		const int bad_instrs[] = { 
			0xD3,0xDB,0xDD,0xE3,0xE4,0xEB,0xEC,0xED,
			0xF4,0xFC,0xFD
		};
		for(auto instr : bad_instrs) {
			opcode_set(instr,CCPUInstr(INSTRFN_NAME(invalid),"invalid"));
		}

		// setup all other instructios ------------------@/
		// bitwise
		opcode_set(0xE6,CCPUInstr(INSTRFN_NAME(and_a_imm8),"and a,imm8"));
		opcode_setPrefix(0xA,CCPUInstrPfx(INSTRFN_NAME(andxor),"and a,xx/xor a,xx"));
		opcode_setPrefix(0xB,CCPUInstrPfx(INSTRFN_NAME(orcp),"or a,xx/cp a,xx"));
		
		// ldh
		opcode_set(0xE0,CCPUInstr(INSTRFN_NAME(ldh_a8_a),"ldh [a8],a"));
		opcode_set(0xF0,CCPUInstr(INSTRFN_NAME(ldh_a_a8),"ldh a,[a8]"));
		opcode_set(0xE2,CCPUInstr(INSTRFN_NAME(ldh_c_a),"ldh [c],a"));
		opcode_set(0xF2,CCPUInstr(INSTRFN_NAME(ldh_a_c),"ldh a,[c]"));
	
		// arith
		opcode_set(0x04,CCPUInstr(INSTRFN_NAME(inc_b),"inc b"));
		opcode_set(0x0C,CCPUInstr(INSTRFN_NAME(inc_c),"inc c"));
		opcode_set(0x14,CCPUInstr(INSTRFN_NAME(inc_d),"inc d"));
		opcode_set(0x1C,CCPUInstr(INSTRFN_NAME(inc_e),"inc e"));
		opcode_set(0x24,CCPUInstr(INSTRFN_NAME(inc_h),"inc h"));
		opcode_set(0x2C,CCPUInstr(INSTRFN_NAME(inc_l),"inc l"));
		opcode_set(0x3C,CCPUInstr(INSTRFN_NAME(inc_a),"inc a"));

		opcode_set(0x05,CCPUInstr(INSTRFN_NAME(dec_b),"dec b"));
		opcode_set(0x0D,CCPUInstr(INSTRFN_NAME(dec_c),"dec c"));
		opcode_set(0x15,CCPUInstr(INSTRFN_NAME(dec_d),"dec d"));
		opcode_set(0x1D,CCPUInstr(INSTRFN_NAME(dec_e),"dec e"));
		opcode_set(0x25,CCPUInstr(INSTRFN_NAME(dec_h),"dec h"));
		opcode_set(0x2D,CCPUInstr(INSTRFN_NAME(dec_l),"dec l"));
		
		opcode_set(0x03,CCPUInstr(INSTRFN_NAME(inc_bc),"inc bc"));
		opcode_set(0x13,CCPUInstr(INSTRFN_NAME(inc_de),"inc de"));
		opcode_set(0x23,CCPUInstr(INSTRFN_NAME(inc_hl),"inc hl"));
		opcode_set(0x33,CCPUInstr(INSTRFN_NAME(inc_sp),"inc sp"));

		opcode_set(0x0B,CCPUInstr(INSTRFN_NAME(dec_bc),"dec bc"));
		opcode_set(0x1B,CCPUInstr(INSTRFN_NAME(dec_de),"dec de"));
		opcode_set(0x2B,CCPUInstr(INSTRFN_NAME(dec_hl),"dec hl"));
		opcode_set(0x3B,CCPUInstr(INSTRFN_NAME(dec_sp),"dec sp"));
		
		opcode_set(0x34,CCPUInstr(INSTRFN_NAME(inc_hld),"inc [hl]"));
		opcode_set(0x35,CCPUInstr(INSTRFN_NAME(dec_hld),"dec [hl]"));

		opcode_set(0x09,CCPUInstr(INSTRFN_NAME(add_hl_bc),"add hl,bc"));
		opcode_set(0x19,CCPUInstr(INSTRFN_NAME(add_hl_de),"add hl,de"));
		opcode_set(0x29,CCPUInstr(INSTRFN_NAME(add_hl_hl),"add hl,hl"));
		opcode_set(0x39,CCPUInstr(INSTRFN_NAME(add_hl_sp),"add hl,sp"));

		opcode_set(0xC6,CCPUInstr(INSTRFN_NAME(add_a_imm8),"add a,imm8"));
		opcode_setPrefix(0x8,CCPUInstrPfx(INSTRFN_NAME(addadc),"add a,xx/adc a,xx"));
		opcode_setPrefix(0x9,CCPUInstrPfx(INSTRFN_NAME(subsbc),"sub a,xx/sbc a,xx"));

		// loads
		opcode_set(0x06,CCPUInstr(INSTRFN_NAME(ld_b_imm8),"ld b, imm8"));
		opcode_set(0x0E,CCPUInstr(INSTRFN_NAME(ld_c_imm8),"ld c, imm8"));
		opcode_set(0x16,CCPUInstr(INSTRFN_NAME(ld_d_imm8),"ld d, imm8"));
		opcode_set(0x1E,CCPUInstr(INSTRFN_NAME(ld_e_imm8),"ld e, imm8"));
		opcode_set(0x26,CCPUInstr(INSTRFN_NAME(ld_h_imm8),"ld h, imm8"));
		opcode_set(0x2E,CCPUInstr(INSTRFN_NAME(ld_l_imm8),"ld l, imm8"));
		opcode_set(0x3E,CCPUInstr(INSTRFN_NAME(ld_a_imm8),"ld a, imm8"));
		
		opcode_setPrefix(0x4,CCPUInstrPfx(INSTRFN_NAME(ldbldc),"ld b,xx/lda c,xx"));
		opcode_setPrefix(0x5,CCPUInstrPfx(INSTRFN_NAME(lddlde),"ld d,xx/lda e,xx"));
		opcode_setPrefix(0x6,CCPUInstrPfx(INSTRFN_NAME(ldhldl),"ld h,xx/lda l,xx"));
		opcode_setPrefix(0x7,CCPUInstrPfx(INSTRFN_NAME(ldhllda),"ld [hl],xx/lda a,xx"));
		
		opcode_set(0x01,CCPUInstr(INSTRFN_NAME(ld_bc_imm16),"ld bc, imm16"));
		opcode_set(0x11,CCPUInstr(INSTRFN_NAME(ld_de_imm16),"ld de, imm16"));
		opcode_set(0x21,CCPUInstr(INSTRFN_NAME(ld_hl_imm16),"ld hl, imm16"));
		opcode_set(0x31,CCPUInstr(INSTRFN_NAME(ld_sp_imm16),"ld sp, imm16"));

		opcode_set(0x0A,CCPUInstr(INSTRFN_NAME(ld_a_bc),"ld a,[bc]"));
		opcode_set(0x1A,CCPUInstr(INSTRFN_NAME(ld_a_de),"ld a,[de]"));
		opcode_set(0x02,CCPUInstr(INSTRFN_NAME(ld_bc_a),"ld [bc], a"));
		opcode_set(0x12,CCPUInstr(INSTRFN_NAME(ld_de_a),"ld [de], a"));

		opcode_set(0x22,CCPUInstr(INSTRFN_NAME(ld_hli_a),"ld [hl+],a"));
		opcode_set(0x2A,CCPUInstr(INSTRFN_NAME(ld_a_hli),"ld a,[hl+]"));
		opcode_set(0x32,CCPUInstr(INSTRFN_NAME(ld_hld_a),"ld [hl-],a"));
		opcode_set(0x3A,CCPUInstr(INSTRFN_NAME(ld_a_hld),"ld a,[hl-]"));

		opcode_set(0xEA,CCPUInstr(INSTRFN_NAME(ld_a16_a),"ld [a16], a"));
		opcode_set(0xFA,CCPUInstr(INSTRFN_NAME(ld_a_a16),"ld a,[a16]"));
		
		opcode_set(0x36,CCPUInstr(INSTRFN_NAME(ld_hl_imm8),"ld [hl],imm8"));
		
		opcode_set(0xFE,CCPUInstr(INSTRFN_NAME(cp_imm8),"cp a, imm8"));

		// jumps & calls
		opcode_set(0xC3,CCPUInstr(INSTRFN_NAME(jp_imm16),"jp imm16"));
		opcode_set(0xE9,CCPUInstr(INSTRFN_NAME(jp_hl),"jp hl"));

		opcode_set(0x18,CCPUInstr(INSTRFN_NAME(jr),"jr imm8"));
		opcode_set(0x20,CCPUInstr(INSTRFN_NAME(jr_nz),"jr nz,imm8"));
		opcode_set(0x28,CCPUInstr(INSTRFN_NAME(jr_z),"jr z,imm8"));
		opcode_set(0x30,CCPUInstr(INSTRFN_NAME(jr_nc),"jr nc,imm8"));
		opcode_set(0x38,CCPUInstr(INSTRFN_NAME(jr_c),"jr c,imm8"));

		opcode_set(0xCD,CCPUInstr(INSTRFN_NAME(call),"call imm16"));
		opcode_set(0xC4,CCPUInstr(INSTRFN_NAME(call_nz),"call nz,imm16"));
		opcode_set(0xCC,CCPUInstr(INSTRFN_NAME(call_z),"call z,imm16"));
		opcode_set(0xD4,CCPUInstr(INSTRFN_NAME(call_nc),"call nc,imm16"));
		opcode_set(0xDC,CCPUInstr(INSTRFN_NAME(call_c),"call c,imm16"));

		opcode_set(0xC9,CCPUInstr(INSTRFN_NAME(ret),"ret"));
		opcode_set(0xD9,CCPUInstr(INSTRFN_NAME(reti),"reti"));
		opcode_set(0xC0,CCPUInstr(INSTRFN_NAME(ret_nz),"ret nz"));
		opcode_set(0xC8,CCPUInstr(INSTRFN_NAME(ret_z),"ret z"));
		opcode_set(0xD0,CCPUInstr(INSTRFN_NAME(ret_nc),"ret nc"));
		opcode_set(0xD8,CCPUInstr(INSTRFN_NAME(ret_c),"ret c"));
		
		opcode_set(0xC7,CCPUInstr(INSTRFN_NAME(rst_00),"rst $00"));
		opcode_set(0xD7,CCPUInstr(INSTRFN_NAME(rst_10),"rst $10"));
		opcode_set(0xE7,CCPUInstr(INSTRFN_NAME(rst_20),"rst $20"));
		opcode_set(0xF7,CCPUInstr(INSTRFN_NAME(rst_30),"rst $30"));

		opcode_set(0xCF,CCPUInstr(INSTRFN_NAME(rst_08),"rst $08"));
		opcode_set(0xDF,CCPUInstr(INSTRFN_NAME(rst_18),"rst $18"));
		opcode_set(0xEF,CCPUInstr(INSTRFN_NAME(rst_28),"rst $28"));
		opcode_set(0xFF,CCPUInstr(INSTRFN_NAME(rst_38),"rst $38"));

		// stack
		opcode_set(0xC1,CCPUInstr(INSTRFN_NAME(pop_bc),"pop bc"));
		opcode_set(0xD1,CCPUInstr(INSTRFN_NAME(pop_de),"pop de"));
		opcode_set(0xE1,CCPUInstr(INSTRFN_NAME(pop_hl),"pop hl"));
		opcode_set(0xF1,CCPUInstr(INSTRFN_NAME(pop_af),"pop af"));

		opcode_set(0xC5,CCPUInstr(INSTRFN_NAME(push_bc),"push bc"));
		opcode_set(0xD5,CCPUInstr(INSTRFN_NAME(push_de),"push de"));
		opcode_set(0xE5,CCPUInstr(INSTRFN_NAME(push_hl),"push hl"));
		opcode_set(0xF5,CCPUInstr(INSTRFN_NAME(push_af),"push af"));

		// prefix
		opcode_set(0xCB,CCPUInstr(INSTRFN_NAME(prefix),"$CB ($xx)"));

		// misc
		opcode_set(0x00,CCPUInstr(INSTRFN_NAME(nop),"nop"));
		opcode_set(0x2F,CCPUInstr(INSTRFN_NAME(cpl),"cpl"));
		opcode_set(0xF3,CCPUInstr(INSTRFN_NAME(di),"di"));
		opcode_set(0xFB,CCPUInstr(INSTRFN_NAME(ei),"ei"));
		opcode_set(0x76,CCPUInstr(INSTRFN_NAME(halt),"halt"));
		
		// reset cpu state ------------------------------@/
		reset();
	}

	auto CCPU::reset() -> void {
		m_should_enableIME = false;
		m_regIME = false;

		m_regF = 0;
		m_regB = 0x00;
		m_regC = 0x13;
		m_regD = 0x00;
		m_regE = 0xD8;
		hl_set(0x014D);
		flag_setZero(true);

		m_PC = 0x0100;
		m_SP = 0xFFFE;

		m_dotclock = 0;
		m_curopcode_ptr = nullptr;
	}

	auto CCPU::opcode_clear() -> void {
		for(int i=0; i<16; i++) {
			opcode_setPrefix(i,CCPUInstrPfx(INSTRFN_NAME(unimplemented_pfx),"unimplemented"));
		}
		for(int i=0; i<256; i++) {
			opcode_set(i,CCPUInstr(INSTRFN_NAME(unimplemented),"unimplemented"));
		}
	}
	auto CCPU::opcode_set(std::size_t index,CCPUInstr instr) -> void {
		m_opcodetable.at(index) = instr;
	}
	auto CCPU::opcode_setPrefix(std::size_t index,CCPUInstrPfx instr) -> void {
		m_opcodetable_pfx.at(index) = instr;
	}

	auto CCPU::print_status() -> void {
		if(!m_curopcode_ptr) return;
		auto &mem = emu()->mem;
		const auto rombank = mem.rombank_current();
		const int ly = mem.m_io.m_LY;
		std::printf("last opcode: %s\n",m_curopcode_ptr->name.c_str());
		std::printf("CPU: %04Xh[+%4Xh]\n",m_PC,m_SP);
		std::printf("ROM: %02Xh\n",rombank);
		std::printf("\tAF:  %04Xh LY:   %3d\n",reg_af(),ly);
		std::printf("\tBC:  %04Xh LCDC: %02Xh\n",reg_bc(),mem.m_io.m_LCDC);
		std::printf("\tDE:  %04Xh IE:   %d\n",reg_de(),mem.m_io.m_IE);
		std::printf("\tHL:  %04Xh IF:   %d\n",reg_hl(),mem.m_io.m_IF);
		std::printf("\tSTAT:%4Xh IME:  %d\n",mem.m_io.m_STAT,m_regIME);
	}
	
	auto CCPU::flag_syncAnd(int opA,int opB) -> void {
		auto result = (opA & opB);
		flag_setZero(result == 0);
		flag_setSubtract(false);
		flag_setHalfcarry(true);
		flag_setCarry(false);
	}
	auto CCPU::flag_syncAdd16(uint32_t opA, uint32_t opB) -> void {
		auto result = (opA + opB);
		auto res_4b = (opA & 0xFFF) + (opB & 0xFFF);
		flag_setSubtract(false);
		flag_setHalfcarry((res_4b & 0x1000) == 0x1000);
		flag_setCarry(result > 0xFFFF);
	}
	auto CCPU::flag_syncCompare(int opA,int opB) -> void {
		auto result = (opA - opB);
		auto res_4b = (opA & 0xF) - (opB & 0xF);
		flag_setZero(result == 0);
		flag_setSubtract(true);
		flag_setHalfcarry((res_4b & 0x10) == 0x10);
		flag_setCarry(opB > opA);
	}
	auto CCPU::flag_syncCompareInc(int operand) -> void {
		auto result = operand + 1;
		auto res_4b = (operand & 0xF) + (1 & 0xF);
		flag_setZero(result == 0);
		flag_setSubtract(false);
		flag_setHalfcarry((res_4b & 0x10) == 0x10);
	}
	auto CCPU::flag_syncCompareDec(int operand) -> void {
		auto result = operand - 1;
		auto res_4b = (operand & 0xF) - (1 & 0xF);
		flag_setZero(result == 0);
		flag_setSubtract(true);
		flag_setHalfcarry((res_4b & 0x10) == 0x10);
	}
	
	// memory access ------------------------------------@/
	auto CCPU::read_pc(int offset) -> uint32_t {
		return m_emu->mem.read(m_PC + offset);
	}
	auto CCPU::read_pc16(int offset) -> uint32_t {
		return read_pc(offset) | (read_pc(offset+1)<<8);
	}
	auto CCPU::read_sp(int offset) -> uint32_t {
		return m_emu->mem.read(m_SP + offset);
	}
	auto CCPU::read_sp16(int offset) -> uint32_t {
		return read_sp(offset) | (read_sp(offset+1)<<8);
	}
	
	auto CCPU::stack_pop8() -> int {
		int data = read_sp(0);
		m_SP += 1;
		return data;
	}
	auto CCPU::stack_pop16() -> int {
		int data = read_sp16(0);
		m_SP += 2;
		return data;
	}
	auto CCPU::stack_push8(int data) -> void {
		m_SP -= 1;
		emu()->mem.write(m_SP,data & 0xFF);
	}
	auto CCPU::stack_push16(int data) -> void {
		data &= 0xFFFF;
		stack_push8(data>>8);
		stack_push8(data);
	}

	// jump-related -------------------------------------@/
	auto CCPU::jump_rel(int offset) -> void {
		m_PC += offset;
	}
	auto CCPU::call(int addr, int retaddr) -> void {
		retaddr &= 0xFFFF;
		stack_push16(retaddr);
		m_PC = addr;
	}
	auto CCPU::calreturn(bool enable_intr) -> void {
		if(enable_intr) {
			std::puts("reti not implemented!");
			print_status();
			std::exit(-1);
		} else {
			m_PC = stack_pop16();
		}
	}

	auto CCPU::step() -> void {
		if(m_should_enableIME) {
			m_should_enableIME = false;
			m_regIME = true;
			execute_opcode();
		} else {
			execute_opcode();
		}
	}

	auto CCPU::execute_opcode() -> void {
		auto opcode_num = m_emu->mem.read(m_PC);
		int opcode_pfx = opcode_num >> 4;
		int opcode_mode = (opcode_num>>3) & 1;

		// regular opcode
		if(m_opcodetable[opcode_num].fn != INSTRFN_NAME(unimplemented)) {
			auto &opcode = m_opcodetable[opcode_num];
			m_curopcode = opcode_num;
			m_curopcode_ptr = &opcode;
			opcode.fn(this,m_emu);
		}
		// prefixed opcode
		 else if(m_opcodetable_pfx[opcode_pfx].fn != INSTRFN_NAME(unimplemented_pfx)) {
			auto &opcode = m_opcodetable_pfx[opcode_pfx];
			m_curopcode = opcode_num;
			m_curopcode_ptr = &opcode;
			// call function with register ID & mode
			opcode.fn(this,m_emu,opcode_num & 7,opcode_mode);
		} else {
			std::printf("bad opcode...");
			print_status();
			std::exit(-1);
		}
	}

	auto CCPU::clock_tick(int cyclecnt) -> void {
		auto &mem = emu()->mem;
		// 4 dots per cycle
		if(mem.m_io.ppu_enabled()) {
			m_dotclock += (cyclecnt*4);
		}

		// set current mode
		// mode 2: OAM scan (0-79?) (no OAM)
		// mode 3: OAM draw (no OAM/VRAM)
		// mode 0: hblank (all accessible)
		bool frame_ended = false;
		bool do_drawline = false;
		auto old_scanline = mem.m_io.m_LY;
		int old_mode = mem.m_io.stat_getMode();

		if(m_dotclock >= 456) {
			mem.m_io.m_LY += 1;
			if(mem.m_io.m_LY > 153) {
				mem.m_io.m_LY = 0;
				//std::puts("newline!");
				//print_status();
				frame_ended = true;
			}

			do_drawline = true;
			m_dotclock -= 456;
		} else {
			if(m_dotclock < 80) {
				mem.m_io.stat_setMode(2);
			} else if(m_dotclock >= 80 || m_dotclock < (80+172)) {
				mem.m_io.stat_setMode(3);
			} else {
				mem.m_io.stat_setMode(0);
			}
		}

		// additional mode checking
		if(mem.m_io.m_LY >= 144) {
			mem.m_io.stat_setMode(1);
		}

		if(!mem.m_io.ppu_enabled()) {
			mem.m_io.m_LY = 0;
			m_dotclock = 0;
		}

		mem.stat_lycSync();

		if(mem.m_io.ppu_enabled()) {
			const int cur_mode = mem.m_io.stat_getMode();
			if(cur_mode != old_mode) {
				bool do_setflag = false;
				if((cur_mode == 0) && (mem.m_io.m_STAT&0x08)) {
					do_setflag = true;
				}
				if((cur_mode == 1) && (mem.m_io.m_STAT&0x10)) {
					do_setflag = true;
				}
				if((cur_mode == 2) && (mem.m_io.m_STAT&0x20)) {
					do_setflag = true;
				}
				mem.m_io.m_IF |= do_setflag ? 0x2 : 0;
			}
			if((mem.m_io.m_STAT & 0x40) && mem.m_io.stat_lycSame()) {
				mem.m_io.m_IF |= 0x2;
			}
		}

		// deal with interrupts, if enabled.
		if(m_regIME) {
			// LCD interrupt
			if(mem.interrupt_match(0x2)) {
				mem.interrupt_clear(0x2);
				m_regIME = false;
				stack_push16(m_PC);
				m_PC = 0x48;
			}
		}

		if(do_drawline) {
			if(!mem.m_io.ppu_enabled()) {
				std::puts("fuck off");
				std::exit(-1);
			}
			emu()->renderer.draw_line(old_scanline);
		}
		if(frame_ended) {
			emu()->renderer.present();
		}

		
	}
}

// bitwise ops
static INSTRFN(and_a_imm8) {
	cpu->flag_syncAnd(cpu->m_regA,cpu->read_pc(1));
	cpu->m_regA &= cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}

static INSTRFN_PFX(andxor) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};

	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		auto result = (cpu->m_regA & cur_reg());
		cpu->m_regA = result;
		cpu->flag_setZero(result == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(true);
		cpu->flag_setCarry(false);
	} else {
		auto result = (cpu->m_regA ^ cur_reg());
		cpu->m_regA = result;
		cpu->flag_setZero(result == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(false);
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}
static INSTRFN_PFX(orcp) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};

	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		auto result = (cpu->m_regA | cur_reg());
		cpu->m_regA = result;
		cpu->flag_setZero(result == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(false);
	} else {
		cpu->flag_syncCompare(cpu->m_regA,cur_reg());
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}

// arithemetic
static INSTRFN(inc_b) {
	cpu->flag_syncCompareInc(cpu->m_regB);
	cpu->m_regB += 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(inc_c) {
	cpu->flag_syncCompareInc(cpu->m_regC);
	cpu->m_regC += 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(inc_d) {
	cpu->flag_syncCompareInc(cpu->m_regD);
	cpu->m_regD += 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(inc_e) {
	cpu->flag_syncCompareInc(cpu->m_regE);
	cpu->m_regE += 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(inc_h) {
	cpu->flag_syncCompareInc(cpu->m_regH);
	cpu->m_regH += 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(inc_l) {
	cpu->flag_syncCompareInc(cpu->m_regL);
	cpu->m_regL += 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(inc_a) {
	cpu->flag_syncCompareInc(cpu->m_regA);
	cpu->m_regA += 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}

static INSTRFN(dec_b) {
	cpu->flag_syncCompareDec(cpu->m_regB);
	cpu->m_regB -= 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(dec_c) {
	cpu->flag_syncCompareDec(cpu->m_regC);
	cpu->m_regC -= 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(dec_d) {
	cpu->flag_syncCompareDec(cpu->m_regD);
	cpu->m_regD -= 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(dec_e) {
	cpu->flag_syncCompareDec(cpu->m_regE);
	cpu->m_regE -= 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(dec_h) {
	cpu->flag_syncCompareDec(cpu->m_regH);
	cpu->m_regH -= 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(dec_l) {
	cpu->flag_syncCompareDec(cpu->m_regL);
	cpu->m_regL -= 1;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}

static INSTRFN(inc_bc) {
	cpu->bc_set(cpu->reg_bc() + 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(inc_de) {
	cpu->de_set(cpu->reg_de() + 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(inc_hl) {
	cpu->hl_set(cpu->reg_hl() + 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(inc_sp) {
	cpu->sp_set(cpu->m_SP + 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}

static INSTRFN(dec_bc) {
	cpu->bc_set(cpu->reg_bc() - 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(dec_de) {
	cpu->de_set(cpu->reg_de() - 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(dec_hl) {
	cpu->hl_set(cpu->reg_hl() - 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(dec_sp) {
	cpu->sp_set(cpu->m_SP - 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}

static INSTRFN(inc_hld) {
	int data = emu->mem.read(cpu->reg_hl());
	cpu->flag_syncCompareInc(data);
	emu->mem.write(cpu->reg_hl(),data + 1);
	cpu->pc_increment(3);
	cpu->clock_tick(1);
}
static INSTRFN(dec_hld) {
	int data = emu->mem.read(cpu->reg_hl());
	cpu->flag_syncCompareDec(data);
	emu->mem.write(cpu->reg_hl(),data - 1);
	cpu->pc_increment(3);
	cpu->clock_tick(1);
}

static INSTRFN(add_hl_bc) {
	cpu->flag_syncAdd16(cpu->reg_hl(),cpu->reg_bc());
	cpu->hl_set(cpu->reg_hl() + cpu->reg_bc());
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(add_hl_de) {
	cpu->flag_syncAdd16(cpu->reg_hl(),cpu->reg_de());
	cpu->hl_set(cpu->reg_hl() + cpu->reg_de());
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(add_hl_hl) {
	cpu->flag_syncAdd16(cpu->reg_hl(),cpu->reg_hl());
	cpu->hl_set(cpu->reg_hl() + cpu->reg_hl());
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(add_hl_sp) {
	cpu->flag_syncAdd16(cpu->reg_hl(),cpu->m_SP);
	cpu->hl_set(cpu->reg_hl() + cpu->m_SP);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}

static INSTRFN(add_a_imm8) {
	int opB = cpu->read_pc(1);
	int res_nyb = (cpu->m_regA & 0xF) + (opB & 0xF);
	int result = static_cast<int>(cpu->m_regA) + opB;
	cpu->m_regA = result;
	cpu->flag_setZero(result == 0);
	cpu->flag_setSubtract(false);
	cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
	cpu->flag_setCarry(result > 255);
	
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}
static INSTRFN_PFX(addadc) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};

	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		int res_nyb = (cpu->m_regA & 0xF) + (cur_reg() & 0xF);
		int result = static_cast<int>(cpu->m_regA) + cur_reg();
		cpu->m_regA = result;
		cpu->flag_setZero(result == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		cpu->flag_setCarry(result > 255);
	} else {
		int op2 = cur_reg() + cpu->flag_carry();
		int res_nyb = (cpu->m_regA & 0xF) + (op2 & 0xF);
		int result = static_cast<int>(cpu->m_regA) + op2;
		cpu->m_regA = result;
		cpu->flag_setZero(result == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		cpu->flag_setCarry(result > 255);
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}
static INSTRFN_PFX(subsbc) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};

	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		int res_nyb = (cpu->m_regA & 0xF) - (cur_reg() & 0xF);
		int result = static_cast<int>(cpu->m_regA) - cur_reg();
		cpu->flag_setCarry(cur_reg() > cpu->m_regA);
		cpu->m_regA = result;
		cpu->flag_setZero(result == 0);
		cpu->flag_setSubtract(true);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
	} else {
		std::puts("sbc not implemented");
		cpu->print_status();
		std::exit(-1);

		int op2 = cur_reg() + cpu->flag_carry();
		int res_nyb = (cpu->m_regA & 0xF) + (op2 & 0xF);
		int result = static_cast<int>(cpu->m_regA) + op2;
		cpu->m_regA = result;
		cpu->flag_setZero(result == 0);
		cpu->flag_setSubtract(true);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		cpu->flag_setCarry(result > 255);
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}

// loads
static INSTRFN(ld_b_imm8) {
	cpu->m_regB = cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}
static INSTRFN(ld_c_imm8) {
	cpu->m_regC = cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}
static INSTRFN(ld_d_imm8) {
	cpu->m_regD = cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}
static INSTRFN(ld_e_imm8) {
	cpu->m_regE = cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}
static INSTRFN(ld_h_imm8) {
	cpu->m_regH = cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}
static INSTRFN(ld_l_imm8) {
	cpu->m_regL = cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}
static INSTRFN(ld_a_imm8) {
	cpu->m_regA = cpu->read_pc(1);
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}

static INSTRFN_PFX(ldbldc) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};
	
	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		cpu->m_regB = cur_reg();
	} else {
		cpu->m_regC = cur_reg();
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}
static INSTRFN_PFX(lddlde) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};
	
	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		cpu->m_regD = cur_reg();
	} else {
		cpu->m_regE = cur_reg();
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}
static INSTRFN_PFX(ldhldl) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};
	
	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		cpu->m_regH = cur_reg();
	} else {
		cpu->m_regL = cur_reg();
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}
static INSTRFN_PFX(ldhllda) {
	auto uses_hl = fern::RegisterName::is_hldata(register_id);
	if(uses_hl && opcode_mode == 0) {
		std::puts("unimplemented: HALT");
		cpu->print_status();
		std::exit(-1);
	}
	
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[register_id];
	};
	
	int clock_ticks = uses_hl ? 2 : 1;
	int pc_offset = 1;

	if(!opcode_mode) {
		emu->mem.write(cpu->reg_hl(),cur_reg());
		clock_ticks = 2;
	} else {
		cpu->m_regA = cur_reg();
	}

	cpu->pc_increment(pc_offset);
	cpu->clock_tick(clock_ticks);
}

static INSTRFN(ld_bc_imm16) {
	cpu->m_regC = cpu->read_pc(1);
	cpu->m_regB = cpu->read_pc(2);
	cpu->pc_increment(3);
	cpu->clock_tick(3);
}
static INSTRFN(ld_de_imm16) {
	cpu->m_regE = cpu->read_pc(1);
	cpu->m_regD = cpu->read_pc(2);
	cpu->pc_increment(3);
	cpu->clock_tick(3);
}
static INSTRFN(ld_hl_imm16) {
	cpu->m_regL = cpu->read_pc(1);
	cpu->m_regH = cpu->read_pc(2);
	cpu->pc_increment(3);
	cpu->clock_tick(3);
}
static INSTRFN(ld_sp_imm16) {
	cpu->m_SP = cpu->read_pc(1) | (cpu->read_pc(2)<<8);
	cpu->pc_increment(3);
	cpu->clock_tick(3);
}

static INSTRFN(ld_a_bc) {
	cpu->m_regA = emu->mem.read(cpu->reg_bc());
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(ld_a_de) {
	cpu->m_regA = emu->mem.read(cpu->reg_de());
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(ld_bc_a) {
	emu->mem.write(cpu->reg_bc(),cpu->m_regA);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(ld_de_a) {
	emu->mem.write(cpu->reg_de(),cpu->m_regA);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}

static INSTRFN(ld_a_hli) {
	cpu->m_regA = emu->mem.read(cpu->reg_hl());
	cpu->hl_set(cpu->reg_hl() + 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(ld_a_hld) {
	cpu->m_regA = emu->mem.read(cpu->reg_hl());
	cpu->hl_set(cpu->reg_hl() - 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(ld_hli_a) {
	emu->mem.write(cpu->reg_hl(),cpu->m_regA);
	cpu->hl_set(cpu->reg_hl() + 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(ld_hld_a) {
	emu->mem.write(cpu->reg_hl(),cpu->m_regA);
	cpu->hl_set(cpu->reg_hl() - 1);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}

static INSTRFN(ld_hl_imm8) {
	emu->mem.write(cpu->reg_hl(),cpu->read_pc(1));
	cpu->pc_increment(2);
	cpu->clock_tick(3);
}

static INSTRFN(ld_a16_a) {
	int addr = cpu->read_pc(1) | (cpu->read_pc(2)<<8);
	emu->mem.write(addr,cpu->m_regA);
	cpu->pc_increment(3);
	cpu->clock_tick(4);
}
static INSTRFN(ld_a_a16) {
	int addr = cpu->read_pc(1) | (cpu->read_pc(2)<<8);
	cpu->m_regA = emu->mem.read(addr);
	cpu->pc_increment(3);
	cpu->clock_tick(4);
}

// ldh
static INSTRFN(ldh_a8_a) {
	emu->mem.write(0xFF00 + cpu->read_pc(1),cpu->m_regA);
	cpu->pc_increment(2);
	cpu->clock_tick(3);
}
static INSTRFN(ldh_a_a8) {
	cpu->m_regA = emu->mem.read(0xFF00 + cpu->read_pc(1));
	cpu->pc_increment(2);
	cpu->clock_tick(3);
}
static INSTRFN(ldh_c_a) {
	emu->mem.write(0xFF00 + cpu->m_regC, cpu->m_regA);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}
static INSTRFN(ldh_a_c) {
	cpu->m_regA = emu->mem.read(0xFF00 + cpu->m_regC);
	cpu->pc_increment(1);
	cpu->clock_tick(2);
}

// cp
static INSTRFN(cp_imm8) {
	cpu->flag_syncCompare(cpu->m_regA,cpu->read_pc(1));
	cpu->pc_increment(2);
	cpu->clock_tick(2);
}

// jumps
static INSTRFN(jp_imm16) {
	auto addr_lo = cpu->emu()->mem.read(cpu->m_PC+1);
	auto addr_hi = cpu->emu()->mem.read(cpu->m_PC+2);
	cpu->pc_set(addr_lo | (addr_hi<<8));
	cpu->clock_tick(4);
}
static INSTRFN(jp_hl) {
	cpu->pc_set(cpu->reg_hl());
	cpu->clock_tick(1);
}

static INSTRFN(jr) {
	cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
	cpu->pc_increment(2);
	cpu->clock_tick(3);
}
static INSTRFN(jr_nz) {
	if(!cpu->flag_zero()) {
		cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	} else {
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
}
static INSTRFN(jr_z) {
	if(cpu->flag_zero()) {
		cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	} else {
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
}
static INSTRFN(jr_nc) {
	if(!cpu->flag_carry()) {
		cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	} else {
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
}
static INSTRFN(jr_c) {
	if(cpu->flag_carry()) {
		cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	} else {
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
}

static INSTRFN(call) {
	cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
	cpu->clock_tick(6);
}
static INSTRFN(call_nz) {
	if(!cpu->flag_zero()) {
		cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
		cpu->clock_tick(6);
	} else {
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}
}
static INSTRFN(call_z) {
	if(cpu->flag_zero()) {
		cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
		cpu->clock_tick(6);
	} else {
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}
}
static INSTRFN(call_nc) {
	if(!cpu->flag_carry()) {
		cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
		cpu->clock_tick(6);
	} else {
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}
}
static INSTRFN(call_c) {
	if(cpu->flag_carry()) {
		cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
		cpu->clock_tick(6);
	} else {
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}
}

static INSTRFN(ret) {
	cpu->calreturn();
	cpu->clock_tick(4);
}
static INSTRFN(reti) {
	cpu->calreturn(true);
	cpu->clock_tick(4);
}
static INSTRFN(ret_nz) {
	if(!cpu->flag_zero()) {
		cpu->calreturn();
		cpu->clock_tick(5);
	} else {
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
}
static INSTRFN(ret_z) {
	if(cpu->flag_zero()) {
		cpu->calreturn();
		cpu->clock_tick(5);
	} else {
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
}
static INSTRFN(ret_nc) {
	if(!cpu->flag_carry()) {
		cpu->calreturn();
		cpu->clock_tick(5);
	} else {
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
}
static INSTRFN(ret_c) {
	if(cpu->flag_carry()) {
		cpu->calreturn();
		cpu->clock_tick(5);
	} else {
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
}

static INSTRFN(rst_00) {
	cpu->call(0x00,cpu->m_PC+1);
	cpu->clock_tick(4);
}
static INSTRFN(rst_10) {
	cpu->call(0x10,cpu->m_PC+1);
	cpu->clock_tick(4);
}
static INSTRFN(rst_20) {
	cpu->call(0x20,cpu->m_PC+1);
	cpu->clock_tick(4);
}
static INSTRFN(rst_30) {
	cpu->call(0x30,cpu->m_PC+1);
	cpu->clock_tick(4);
}

static INSTRFN(rst_08) {
	cpu->call(0x08,cpu->m_PC+1);
	cpu->clock_tick(4);
}
static INSTRFN(rst_18) {
	cpu->call(0x18,cpu->m_PC+1);
	cpu->clock_tick(4);
}
static INSTRFN(rst_28) {
	cpu->call(0x28,cpu->m_PC+1);
	cpu->clock_tick(4);
}
static INSTRFN(rst_38) {
	cpu->call(0x38,cpu->m_PC+1);
	cpu->clock_tick(4);
}

// stack
static INSTRFN(pop_bc) {
	cpu->bc_set(cpu->stack_pop16());
	cpu->pc_increment(1);
	cpu->clock_tick(3);
}
static INSTRFN(pop_de) {
	cpu->de_set(cpu->stack_pop16());
	cpu->pc_increment(1);
	cpu->clock_tick(3);
}
static INSTRFN(pop_hl) {
	cpu->hl_set(cpu->stack_pop16());
	cpu->pc_increment(1);
	cpu->clock_tick(3);
}
static INSTRFN(pop_af) {
	cpu->m_regF = cpu->stack_pop8();
	cpu->m_regA = cpu->stack_pop8();
	cpu->pc_increment(1);
	cpu->clock_tick(3);
}

static INSTRFN(push_bc) {
	cpu->stack_push16(cpu->reg_bc());
	cpu->pc_increment(1);
	cpu->clock_tick(4);
}
static INSTRFN(push_de) {
	cpu->stack_push16(cpu->reg_de());
	cpu->pc_increment(1);
	cpu->clock_tick(4);
}
static INSTRFN(push_hl) {
	cpu->stack_push16(cpu->reg_hl());
	cpu->pc_increment(1);
	cpu->clock_tick(4);
}
static INSTRFN(push_af) {
	cpu->stack_push16(cpu->reg_af());
	cpu->pc_increment(1);
	cpu->clock_tick(4);
}

// CB prefix
static INSTRFN(prefix) {
	int prefix_operand = cpu->read_pc(1);
	int reg_id = prefix_operand & 7;
	int oper_id = prefix_operand >> 4;
	int oper_mode = (prefix_operand >> 3) & 1;
	
	auto uses_hl = fern::RegisterName::is_hldata(reg_id);
	uint8_t cur_hldat = 0;
	if(uses_hl) cur_hldat = emu->mem.read(cpu->reg_hl());
	
	uint8_t* reg_ptrs[8] = {
		&cpu->m_regB,&cpu->m_regC,
		&cpu->m_regD,&cpu->m_regE,
		&cpu->m_regH,&cpu->m_regL,
		&cur_hldat,&cpu->m_regA,
	};

	auto cur_reg = [&]() {
		return *reg_ptrs[reg_id];
	};
	auto reg_write = [&](uint8_t data) {
		*reg_ptrs[reg_id] = data;
	};

	int clock_ticks = -1;
	bool writeback_hldat = false;

	// operation fetching
	switch(oper_id) {
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7: { // BIT
			int bit_idx = (prefix_operand - 0x40) >> 3;
			bool flag = (cur_reg() >> bit_idx)&1;
			cpu->flag_setZero(!flag);
			cpu->flag_setSubtract(false);
			cpu->flag_setHalfcarry(true);

			clock_ticks = uses_hl ? 3 : 2;
			break;
		}
		case 0x3: { // SWAP/SRL
			if(oper_mode == 0) {
				int lo = cur_reg() & 0xF;
				int hi = cur_reg() >> 4;
				reg_write((lo<<4) | hi);
				cpu->flag_setZero(true);
				cpu->flag_setSubtract(false);
				cpu->flag_setHalfcarry(false);
				cpu->flag_setCarry(false);
				
				clock_ticks = uses_hl ? 4 : 2;
				writeback_hldat = uses_hl;
			} else {
				std::puts("unimplemented: SRL");
				std::exit(-1);
			}
			break;
		}
		default: {
			std::printf("unimplemented prefix op (%02Xh)\n",prefix_operand);
			cpu->print_status();
			std::exit(-1);
		}
	}

	// write back to HL pointer, if needed
	if(writeback_hldat) {
		emu->mem.write(cpu->reg_hl(),cur_hldat);
	}

	if(clock_ticks == -1) {
		std::puts("prefix opcode error: incorrect clockincrement!");
		cpu->print_status();
		std::exit(-1);
	}

	cpu->pc_increment(2);
	cpu->clock_tick(clock_ticks);
}

// misc
static INSTRFN(invalid) {
	std::puts("invalid instruction");
	cpu->print_status();
	std::exit(-1);
}
static INSTRFN(unimplemented) {
	std::printf("unimplemented instruction (%02Xh)\n",cpu->m_curopcode);
	cpu->print_status();
	std::exit(-1);
}
static INSTRFN_PFX(unimplemented_pfx) {
	std::printf("unimplemented instruction prefix (%02Xh)\n",cpu->m_curopcode);
	cpu->print_status();
	std::exit(-1);
}

static INSTRFN(nop) {
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(cpl) {
	cpu->m_regA ^= 0xFF;
	cpu->flag_setSubtract(false);
	cpu->flag_setHalfcarry(false);
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(di) {
	cpu->m_regIME = false;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(ei) {
	cpu->m_should_enableIME = true;
	cpu->pc_increment(1);
	cpu->clock_tick(1);
}
static INSTRFN(halt) {
	//cpu->pc_increment(1);
	//while(1) {
		cpu->clock_tick(1);
	//	emu->renderer.process_message();
	//}
}

