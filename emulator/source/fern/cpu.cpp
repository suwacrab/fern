#include <fern.h>
#include <fern_common.h>

#define INSTRFN_NAME(name) fernOpcodes :: op_##name
#define fern_opcodefn(name) void op_##name (fern::CCPU* cpu,fern::CEmulator* emu)
#define fern_opcodepfxfn(name) void op_##name (fern::CCPU* cpu,fern::CEmulator* emu, int register_id, int opcode_mode)

namespace fernOpcodes {
	// bitwise ops
	fern_opcodefn(and_a_imm8) {
		cpu->flag_syncAnd(cpu->m_regA,cpu->read_pc(1));
		cpu->m_regA &= cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(or_a_imm8) {
		cpu->m_regA |= cpu->read_pc(1);
		cpu->flag_setZero(cpu->m_regA == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(false);

		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(xor_a_imm8) {
		cpu->m_regA ^= cpu->read_pc(1);
		cpu->flag_setZero(cpu->m_regA == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(false);

		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}

	fern_opcodepfxfn(andxor) {
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
	fern_opcodepfxfn(orcp) {
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

	// TODO: check rotate operation.
	fern_opcodefn(rlca) {
		int hibit = cpu->m_regA >> 7;
		cpu->m_regA <<= 1;
		cpu->m_regA |= hibit;

		cpu->flag_setZero(false);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(hibit);
	
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(rla) {
		int carry = cpu->flag_carry();
		int hibit = cpu->m_regA >> 7;
		cpu->m_regA <<= 1;
		cpu->m_regA |= carry;

		cpu->flag_setZero(false);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(hibit);
	
		cpu->pc_increment(1);
		cpu->clock_tick(1);	
	}
	fern_opcodefn(rrca) {
		int lobit = cpu->m_regA & 1;
		cpu->m_regA >>= 1;
		cpu->m_regA |= lobit << 7;

		cpu->flag_setZero(false);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(lobit);
	
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(rra) {
		int carry = cpu->flag_carry();
		int lobit = cpu->m_regA & 1;
		cpu->m_regA >>= 1;
		cpu->m_regA |= carry << 7;

		cpu->flag_setZero(false);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry(false);
		cpu->flag_setCarry(lobit);
	
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}

	// arithemetic
	fern_opcodefn(inc_b) {
		cpu->flag_syncCompareInc(cpu->m_regB);
		cpu->m_regB += 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(inc_c) {
		cpu->flag_syncCompareInc(cpu->m_regC);
		cpu->m_regC += 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(inc_d) {
		cpu->flag_syncCompareInc(cpu->m_regD);
		cpu->m_regD += 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(inc_e) {
		cpu->flag_syncCompareInc(cpu->m_regE);
		cpu->m_regE += 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(inc_h) {
		cpu->flag_syncCompareInc(cpu->m_regH);
		cpu->m_regH += 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(inc_l) {
		cpu->flag_syncCompareInc(cpu->m_regL);
		cpu->m_regL += 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(inc_a) {
		cpu->flag_syncCompareInc(cpu->m_regA);
		cpu->m_regA += 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}

	fern_opcodefn(dec_b) {
		cpu->flag_syncCompareDec(cpu->m_regB);
		cpu->m_regB -= 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(dec_c) {
		cpu->flag_syncCompareDec(cpu->m_regC);
		cpu->m_regC -= 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(dec_d) {
		cpu->flag_syncCompareDec(cpu->m_regD);
		cpu->m_regD -= 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(dec_e) {
		cpu->flag_syncCompareDec(cpu->m_regE);
		cpu->m_regE -= 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(dec_h) {
		cpu->flag_syncCompareDec(cpu->m_regH);
		cpu->m_regH -= 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(dec_l) {
		cpu->flag_syncCompareDec(cpu->m_regL);
		cpu->m_regL -= 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(dec_a) {
		cpu->flag_syncCompareDec(cpu->m_regA);
		cpu->m_regA -= 1;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}

	fern_opcodefn(inc_bc) {
		cpu->bc_set(cpu->reg_bc() + 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(inc_de) {
		cpu->de_set(cpu->reg_de() + 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(inc_hl) {
		cpu->hl_set(cpu->reg_hl() + 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(inc_sp) {
		cpu->sp_set(cpu->m_SP + 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}

	fern_opcodefn(dec_bc) {
		cpu->bc_set(cpu->reg_bc() - 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(dec_de) {
		cpu->de_set(cpu->reg_de() - 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(dec_hl) {
		cpu->hl_set(cpu->reg_hl() - 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(dec_sp) {
		cpu->sp_set(cpu->m_SP - 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}

	fern_opcodefn(inc_hld) {
		int data = emu->mem.read(cpu->reg_hl());
		cpu->flag_syncCompareInc(data);
		emu->mem.write(cpu->reg_hl(),data + 1);
		cpu->pc_increment(1);
		cpu->clock_tick(3);
	}
	fern_opcodefn(dec_hld) {
		int data = emu->mem.read(cpu->reg_hl());
		cpu->flag_syncCompareDec(data);
		emu->mem.write(cpu->reg_hl(),data - 1);
		cpu->pc_increment(1);
		cpu->clock_tick(3);
	}

	fern_opcodefn(add_hl_bc) {
		cpu->flag_syncAdd16(cpu->reg_hl(),cpu->reg_bc());
		cpu->hl_set(cpu->reg_hl() + cpu->reg_bc());
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(add_hl_de) {
		cpu->flag_syncAdd16(cpu->reg_hl(),cpu->reg_de());
		cpu->hl_set(cpu->reg_hl() + cpu->reg_de());
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(add_hl_hl) {
		cpu->flag_syncAdd16(cpu->reg_hl(),cpu->reg_hl());
		cpu->hl_set(cpu->reg_hl() + cpu->reg_hl());
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(add_hl_sp) {
		cpu->flag_syncAdd16(cpu->reg_hl(),cpu->m_SP);
		cpu->hl_set(cpu->reg_hl() + cpu->m_SP);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}

	fern_opcodefn(add_a_imm8) {
		int opB = cpu->read_pc(1);
		int res_nyb = (cpu->m_regA & 0xF) + (opB & 0xF);
		int result = static_cast<int>(cpu->m_regA) + opB;
		cpu->m_regA = result;
		cpu->flag_setZero((result & 0xFF) == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		cpu->flag_setCarry(result > 255);
		
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(adc_a_imm8) {
		int opB = cpu->read_pc(1) + cpu->flag_carry();
		int res_nyb = (cpu->m_regA & 0xF) + (opB & 0xF);
		int result = static_cast<int>(cpu->m_regA) + opB;
		cpu->m_regA = result;
		cpu->flag_setZero((result & 0xFF) == 0);
		cpu->flag_setSubtract(false);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		cpu->flag_setCarry(result > 255);
		
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(sub_a_imm8) {
		int opB = cpu->read_pc(1);
		int res_nyb = (cpu->m_regA & 0xF) - (opB & 0xF);
		int result = static_cast<int>(cpu->m_regA) - opB;
		cpu->flag_setCarry(opB > cpu->m_regA);
		cpu->m_regA = result;
		cpu->flag_setZero((result & 0xFF) == 0);
		cpu->flag_setSubtract(true);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(sbc_a_imm8) {
		int opB = cpu->read_pc(1) + cpu->flag_carry();
		int res_nyb = (cpu->m_regA & 0xF) - (opB & 0xF);
		int result = static_cast<int>(cpu->m_regA) - opB;
		cpu->flag_setCarry(opB > cpu->m_regA);
		cpu->m_regA = result;
		cpu->flag_setZero((result & 0xFF) == 0);
		cpu->flag_setSubtract(true);
		cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodepfxfn(addadc) {
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

		if(!opcode_mode) { // add
			int res_nyb = (cpu->m_regA & 0xF) + (cur_reg() & 0xF);
			int result = static_cast<int>(cpu->m_regA) + cur_reg();
			cpu->m_regA = result;
			cpu->flag_setZero((result & 0xFF) == 0);
			cpu->flag_setSubtract(false);
			cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
			cpu->flag_setCarry(result > 255);
		} else { // adc
			int op2 = cur_reg() + cpu->flag_carry();
			int res_nyb = (cpu->m_regA & 0xF) + (op2 & 0xF);
			int result = static_cast<int>(cpu->m_regA) + op2;
			cpu->m_regA = result;
			cpu->flag_setZero((result & 0xFF) == 0);
			cpu->flag_setSubtract(false);
			cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
			cpu->flag_setCarry(result > 255);
		}

		cpu->pc_increment(pc_offset);
		cpu->clock_tick(clock_ticks);
	}
	fern_opcodepfxfn(subsbc) {
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
			cpu->flag_setZero((result & 0xFF) == 0);
			cpu->flag_setSubtract(true);
			cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		} else {
			int op2 = cur_reg() + cpu->flag_carry();
			int res_nyb = (cpu->m_regA & 0xF) - (op2 & 0xF);
			int result = static_cast<int>(cpu->m_regA) - op2;
			cpu->flag_setCarry(op2 > cpu->m_regA);
			cpu->m_regA = result;
			cpu->flag_setZero((result & 0xFF) == 0);
			cpu->flag_setSubtract(true);
			cpu->flag_setHalfcarry((res_nyb & 0x10) == 0x10);
		}

		cpu->pc_increment(pc_offset);
		cpu->clock_tick(clock_ticks);
	}

	// loads
	fern_opcodefn(ld_b_imm8) {
		cpu->m_regB = cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_c_imm8) {
		cpu->m_regC = cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_d_imm8) {
		cpu->m_regD = cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_e_imm8) {
		cpu->m_regE = cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_h_imm8) {
		cpu->m_regH = cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_l_imm8) {
		cpu->m_regL = cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_a_imm8) {
		cpu->m_regA = cpu->read_pc(1);
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}

	fern_opcodepfxfn(ldbldc) {
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

		/*if(register_id == 0) {
			std::puts("debug break!");
			emu->debug_set(true);
		}*/
		cpu->pc_increment(pc_offset);
		cpu->clock_tick(clock_ticks);
	}
	fern_opcodepfxfn(lddlde) {
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
	fern_opcodepfxfn(ldhldl) {
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
	fern_opcodepfxfn(ldhllda) {
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

	fern_opcodefn(ld_bc_imm16) {
		cpu->m_regC = cpu->read_pc(1);
		cpu->m_regB = cpu->read_pc(2);
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}
	fern_opcodefn(ld_de_imm16) {
		cpu->m_regE = cpu->read_pc(1);
		cpu->m_regD = cpu->read_pc(2);
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}
	fern_opcodefn(ld_hl_imm16) {
		cpu->m_regL = cpu->read_pc(1);
		cpu->m_regH = cpu->read_pc(2);
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}
	fern_opcodefn(ld_sp_imm16) {
		cpu->m_SP = cpu->read_pc(1) | (cpu->read_pc(2)<<8);
		cpu->pc_increment(3);
		cpu->clock_tick(3);
	}

	// TODO: correct flags
	fern_opcodefn(ld_a16_sp) {
		int addr = cpu->read_pc16(1);
		emu->mem.write(addr+0,cpu->m_SP & 0xFF);
		emu->mem.write(addr+1,(cpu->m_SP>>8) & 0xFF);
		cpu->pc_increment(3);
		cpu->clock_tick(5);
	}
	fern_opcodefn(ld_hl_spimm8) {
		int offset = static_cast<int8_t>(cpu->read_pc(1));
		int result = (cpu->m_SP + offset);
		int lo = (cpu->m_SP>>8)&1;
		cpu->hl_set(result);
		cpu->flag_setZero(false);
		cpu->flag_setSubtract(false);
		cpu->flag_setCarry(lo != ((result>>8)&1));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	}

	fern_opcodefn(ld_sp_hl) {
		cpu->m_SP = cpu->reg_hl();
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}

	fern_opcodefn(ld_a_bc) {
		cpu->m_regA = emu->mem.read(cpu->reg_bc());
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_a_de) {
		cpu->m_regA = emu->mem.read(cpu->reg_de());
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_bc_a) {
		emu->mem.write(cpu->reg_bc(),cpu->m_regA);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_de_a) {
		emu->mem.write(cpu->reg_de(),cpu->m_regA);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}

	fern_opcodefn(ld_a_hli) {
		cpu->m_regA = emu->mem.read(cpu->reg_hl());
		cpu->hl_set(cpu->reg_hl() + 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_a_hld) {
		cpu->m_regA = emu->mem.read(cpu->reg_hl());
		cpu->hl_set(cpu->reg_hl() - 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_hli_a) {
		emu->mem.write(cpu->reg_hl(),cpu->m_regA);
		cpu->hl_set(cpu->reg_hl() + 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ld_hld_a) {
		emu->mem.write(cpu->reg_hl(),cpu->m_regA);
		cpu->hl_set(cpu->reg_hl() - 1);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}

	fern_opcodefn(ld_hl_imm8) {
		emu->mem.write(cpu->reg_hl(),cpu->read_pc(1));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	}

	fern_opcodefn(ld_a16_a) {
		int addr = cpu->read_pc(1) | (cpu->read_pc(2)<<8);
		emu->mem.write(addr,cpu->m_regA);
		cpu->pc_increment(3);
		cpu->clock_tick(4);
	}
	fern_opcodefn(ld_a_a16) {
		int addr = cpu->read_pc(1) | (cpu->read_pc(2)<<8);
		cpu->m_regA = emu->mem.read(addr);
		cpu->pc_increment(3);
		cpu->clock_tick(4);
	}

	// ldh
	fern_opcodefn(ldh_a8_a) {
		emu->mem.write(0xFF00 + cpu->read_pc(1),cpu->m_regA);
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	}
	fern_opcodefn(ldh_a_a8) {
		cpu->m_regA = emu->mem.read(0xFF00 + cpu->read_pc(1));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	}
	fern_opcodefn(ldh_c_a) {
		emu->mem.write(0xFF00 + cpu->m_regC, cpu->m_regA);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}
	fern_opcodefn(ldh_a_c) {
		cpu->m_regA = emu->mem.read(0xFF00 + cpu->m_regC);
		cpu->pc_increment(1);
		cpu->clock_tick(2);
	}

	// cp
	fern_opcodefn(cp_imm8) {
		cpu->flag_syncCompare(cpu->m_regA,cpu->read_pc(1));
		cpu->pc_increment(2);
		cpu->clock_tick(2);
	}

	// jumps
	fern_opcodefn(jp_imm16) {
		auto addr_lo = cpu->emu()->mem.read(cpu->m_PC+1);
		auto addr_hi = cpu->emu()->mem.read(cpu->m_PC+2);
		cpu->pc_set(addr_lo | (addr_hi<<8));
		cpu->clock_tick(4);
	}
	fern_opcodefn(jp_hl) {
		cpu->pc_set(cpu->reg_hl());
		cpu->clock_tick(1);
	}

	fern_opcodefn(jr) {
		cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
		cpu->pc_increment(2);
		cpu->clock_tick(3);
	}
	fern_opcodefn(jr_nz) {
		if(!cpu->flag_zero()) {
			cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
			cpu->pc_increment(2);
			cpu->clock_tick(3);
		} else {
			cpu->pc_increment(2);
			cpu->clock_tick(2);
		}
	}
	fern_opcodefn(jr_z) {
		if(cpu->flag_zero()) {
			cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
			cpu->pc_increment(2);
			cpu->clock_tick(3);
		} else {
			cpu->pc_increment(2);
			cpu->clock_tick(2);
		}
	}
	fern_opcodefn(jr_nc) {
		if(!cpu->flag_carry()) {
			cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
			cpu->pc_increment(2);
			cpu->clock_tick(3);
		} else {
			cpu->pc_increment(2);
			cpu->clock_tick(2);
		}
	}
	fern_opcodefn(jr_c) {
		if(cpu->flag_carry()) {
			cpu->jump_rel(static_cast<int8_t>(cpu->read_pc(1)));
			cpu->pc_increment(2);
			cpu->clock_tick(3);
		} else {
			cpu->pc_increment(2);
			cpu->clock_tick(2);
		}
	}

	fern_opcodefn(jp_nz) {
		if(!cpu->flag_zero()) {
			cpu->pc_set(cpu->read_pc16(1));
			cpu->clock_tick(4);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}
	fern_opcodefn(jp_z) {
		if(cpu->flag_zero()) {
			cpu->pc_set(cpu->read_pc16(1));
			cpu->clock_tick(4);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}
	fern_opcodefn(jp_nc) {
		if(!cpu->flag_carry()) {
			cpu->pc_set(cpu->read_pc16(1));
			cpu->clock_tick(4);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}
	fern_opcodefn(jp_c) {
		if(cpu->flag_carry()) {
			cpu->pc_set(cpu->read_pc16(1));
			cpu->clock_tick(4);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}

	fern_opcodefn(call) {
		cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
		cpu->clock_tick(6);
	}
	fern_opcodefn(call_nz) {
		if(!cpu->flag_zero()) {
			cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
			cpu->clock_tick(6);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}
	fern_opcodefn(call_z) {
		if(cpu->flag_zero()) {
			cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
			cpu->clock_tick(6);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}
	fern_opcodefn(call_nc) {
		if(!cpu->flag_carry()) {
			cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
			cpu->clock_tick(6);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}
	fern_opcodefn(call_c) {
		if(cpu->flag_carry()) {
			cpu->call(cpu->read_pc16(1),cpu->m_PC+3);
			cpu->clock_tick(6);
		} else {
			cpu->pc_increment(3);
			cpu->clock_tick(3);
		}
	}

	fern_opcodefn(ret) {
		cpu->calreturn();
		cpu->clock_tick(4);
	}
	fern_opcodefn(reti) {
		cpu->calreturn(true);
		cpu->clock_tick(4);
	}
	fern_opcodefn(ret_nz) {
		if(!cpu->flag_zero()) {
			cpu->calreturn();
			cpu->clock_tick(5);
		} else {
			cpu->pc_increment(1);
			cpu->clock_tick(2);
		}
	}
	fern_opcodefn(ret_z) {
		if(cpu->flag_zero()) {
			cpu->calreturn();
			cpu->clock_tick(5);
		} else {
			cpu->pc_increment(1);
			cpu->clock_tick(2);
		}
	}
	fern_opcodefn(ret_nc) {
		if(!cpu->flag_carry()) {
			cpu->calreturn();
			cpu->clock_tick(5);
		} else {
			cpu->pc_increment(1);
			cpu->clock_tick(2);
		}
	}
	fern_opcodefn(ret_c) {
		if(cpu->flag_carry()) {
			cpu->calreturn();
			cpu->clock_tick(5);
		} else {
			cpu->pc_increment(1);
			cpu->clock_tick(2);
		}
	}

	fern_opcodefn(rst_00) {
		cpu->call(0x00,cpu->m_PC+1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(rst_10) {
		cpu->call(0x10,cpu->m_PC+1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(rst_20) {
		cpu->call(0x20,cpu->m_PC+1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(rst_30) {
		cpu->call(0x30,cpu->m_PC+1);
		cpu->clock_tick(4);
	}

	fern_opcodefn(rst_08) {
		cpu->call(0x08,cpu->m_PC+1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(rst_18) {
		cpu->call(0x18,cpu->m_PC+1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(rst_28) {
		cpu->call(0x28,cpu->m_PC+1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(rst_38) {
		cpu->call(0x38,cpu->m_PC+1);
		cpu->clock_tick(4);
	}

	// stack
	fern_opcodefn(pop_bc) {
		cpu->bc_set(cpu->stack_pop16());
		cpu->pc_increment(1);
		cpu->clock_tick(3);
	}
	fern_opcodefn(pop_de) {
		cpu->de_set(cpu->stack_pop16());
		cpu->pc_increment(1);
		cpu->clock_tick(3);
	}
	fern_opcodefn(pop_hl) {
		cpu->hl_set(cpu->stack_pop16());
		cpu->pc_increment(1);
		cpu->clock_tick(3);
	}
	fern_opcodefn(pop_af) {
		cpu->m_regF = cpu->stack_pop8() & 0xF0;
		cpu->m_regA = cpu->stack_pop8();
		cpu->pc_increment(1);
		cpu->clock_tick(3);
	}

	fern_opcodefn(push_bc) {
		cpu->stack_push16(cpu->reg_bc());
		cpu->pc_increment(1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(push_de) {
		cpu->stack_push16(cpu->reg_de());
		cpu->pc_increment(1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(push_hl) {
		cpu->stack_push16(cpu->reg_hl());
		cpu->pc_increment(1);
		cpu->clock_tick(4);
	}
	fern_opcodefn(push_af) {
		cpu->stack_push16(cpu->reg_af());
		cpu->pc_increment(1);
		cpu->clock_tick(4);
	}

	// CB prefix
	fern_opcodefn(prefix) {
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
			case 0x00: { // RLC/RRC
				int result = 0;
				bool carry = false;
				if(oper_mode == 0) {
					result = (cur_reg()<<1) | (cur_reg()>>7);
					carry = cur_reg() >> 7;
				} else {
					result = (cur_reg()>>1) | (cur_reg()<<7);
					carry = cur_reg()&1;
				}
				
				cpu->flag_setZero(result == 0);
				cpu->flag_setSubtract(false);
				cpu->flag_setHalfcarry(false);
				cpu->flag_setCarry(carry);
				reg_write(result);

				clock_ticks = uses_hl ? 4 : 2;
				writeback_hldat = uses_hl;
				break;
			}
			case 0x01: { // RL/RR
				int result = 0;
				bool carry = false;
				if(oper_mode == 0) {
					result = (cur_reg()<<1) | cpu->flag_carry();
					carry = cur_reg() >> 7;
				} else {
					result = (cur_reg()>>1) | (cpu->flag_carry()<<7);
					carry = cur_reg()&1;
				}
				
				cpu->flag_setZero(result == 0);
				cpu->flag_setSubtract(false);
				cpu->flag_setHalfcarry(false);
				cpu->flag_setCarry(carry);
				reg_write(result);

				clock_ticks = uses_hl ? 4 : 2;
				writeback_hldat = uses_hl;
				break;
			}
			case 0x02: { // SLA/SRA
				int result = 0;
				bool carry = false;
				if(oper_mode == 0) {
					result = cur_reg() << 1;
					carry = cur_reg() >> 7;
				} else {
					result = (cur_reg()>>7) | (cur_reg()&0x80);
					carry = cur_reg()&1;
				}
				
				cpu->flag_setZero(result == 0);
				cpu->flag_setSubtract(false);
				cpu->flag_setHalfcarry(false);
				cpu->flag_setCarry(carry);
				reg_write(result);

				clock_ticks = uses_hl ? 4 : 2;
				writeback_hldat = uses_hl;
				break;
			}
			case 0x3: { // SWAP/SRL
				if(oper_mode == 0) {
					int lo = cur_reg() & 0xF;
					int hi = cur_reg() >> 4;
					reg_write((lo<<4) | hi);
					cpu->flag_setZero(cur_reg() == 0);
					cpu->flag_setSubtract(false);
					cpu->flag_setHalfcarry(false);
					cpu->flag_setCarry(false);
					
					clock_ticks = uses_hl ? 4 : 2;
					writeback_hldat = uses_hl;
				} else {
					int lo = cur_reg() & 1;
					reg_write(cur_reg() >> 1);
					cpu->flag_setZero(cur_reg() == 0);
					cpu->flag_setSubtract(false);
					cpu->flag_setHalfcarry(false);
					cpu->flag_setCarry(lo);
					
					clock_ticks = uses_hl ? 4 : 2;
					writeback_hldat = uses_hl;
				}
				break;
			}
			// BIT --------------------------------------@/
			case 0x4:
			case 0x5:
			case 0x6:
			case 0x7: {
				int bit_idx = (prefix_operand - 0x40) >> 3;
				bool flag = (cur_reg() >> bit_idx)&1;
				cpu->flag_setZero(flag == 0);
				cpu->flag_setSubtract(false);
				cpu->flag_setHalfcarry(true);

				clock_ticks = uses_hl ? 3 : 2;
				break;
			}
			// RES --------------------------------------@/
			case 0x8:
			case 0x9:
			case 0xA:
			case 0xB: {
				int bit_idx = (prefix_operand - 0x80) >> 3;
				reg_write(cur_reg() & (0xFF ^ (1<<bit_idx)));

				writeback_hldat = uses_hl;
				clock_ticks = uses_hl ? 4 : 2;
				break;
			}

			// SET --------------------------------------@/
			case 0xC:
			case 0xD:
			case 0xE:
			case 0xF: {
				int bit_idx = (prefix_operand - 0xC0) >> 3;
				reg_write(cur_reg() | (1<<bit_idx));

				writeback_hldat = uses_hl;
				clock_ticks = uses_hl ? 4 : 2;
				break;
			}

			default: {
				std::printf("unimplemented prefix op (%02Xh)\n",oper_id);
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
	fern_opcodefn(invalid) {
		std::puts("invalid instruction");
		cpu->print_status();
		std::exit(-1);
	}
	fern_opcodefn(unimplemented) {
		std::printf("unimplemented instruction (%02Xh)\n",cpu->m_curopcode);
		cpu->print_status();
		std::exit(-1);
	}
	fern_opcodepfxfn(unimplemented_pfx) {
		std::printf("unimplemented instruction prefix (%02Xh)\n",cpu->m_curopcode);
		cpu->print_status();
		std::exit(-1);
	}

	fern_opcodefn(nop) {
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(cpl) {
		cpu->m_regA = ~cpu->m_regA;
		cpu->flag_setSubtract(true);
		cpu->flag_setHalfcarry(true);
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(scf) {
		cpu->flag_setCarry(true);
		cpu->flag_setSubtract(true);
		cpu->flag_setHalfcarry(true);
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(ccf) {
		cpu->flag_setSubtract(true);
		cpu->flag_setHalfcarry(true);
		cpu->flag_setCarry(!cpu->flag_carry());
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(daa) {
		int orig_reg = cpu->m_regA;
		const int nyb_lo = orig_reg & 0xf;

		// set correction
		int correction = cpu->flag_carry() ? 0x60 : 0;
		if(cpu->flag_halfcarry() || (!cpu->flag_subtract() && (nyb_lo > 9)) )
			correction |= 0x06;
		if(cpu->flag_carry() || (!cpu->flag_subtract() && (orig_reg > 0x99)))
			correction |= 0x60;

		// subtract if needed
		if(cpu->flag_subtract()) {
			orig_reg = static_cast<uint8_t>(orig_reg - correction);
		} else {
			orig_reg = static_cast<uint8_t>(orig_reg + correction);
		}

		if( (correction<<2) & 0x100 ) {
			cpu->flag_setCarry(true);
		}

		cpu->m_regA = orig_reg;
		cpu->flag_setZero(cpu->m_regA == 0);
		cpu->flag_setHalfcarry(false);
		
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(di) {
		cpu->m_regIME = false;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(ei) {
		cpu->m_should_enableIME = true;
		cpu->pc_increment(1);
		cpu->clock_tick(1);
	}
	fern_opcodefn(halt) {
		cpu->pc_increment(1);

		if(!cpu->m_regIME) {
			std::puts("error: halt while IME unset?");
			cpu->print_status();
			std::exit(-1);
		} else {
			cpu->halt_waitStart();
			while(cpu->halt_isWaiting()) {
				cpu->clock_tick(1);
			}
		}
	}
};

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
		opcode_set(0xF6,CCPUInstr(INSTRFN_NAME(or_a_imm8),"or a,imm8"));
		opcode_setPrefix(0xA,CCPUInstrPfx(INSTRFN_NAME(andxor),"and a,xx/xor a,xx"));
		opcode_setPrefix(0xB,CCPUInstrPfx(INSTRFN_NAME(orcp),"or a,xx/cp a,xx"));

		opcode_set(0xEE,CCPUInstr(INSTRFN_NAME(xor_a_imm8),"xor a,imm8"));
		
		opcode_set(0x07,CCPUInstr(INSTRFN_NAME(rlca),"rlca"));
		opcode_set(0x17,CCPUInstr(INSTRFN_NAME(rla),"rla"));
		opcode_set(0x0F,CCPUInstr(INSTRFN_NAME(rrca),"rrca"));
		opcode_set(0x1F,CCPUInstr(INSTRFN_NAME(rra),"rra"));
		
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
		opcode_set(0x3D,CCPUInstr(INSTRFN_NAME(dec_a),"dec a"));
		
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
		opcode_set(0xCE,CCPUInstr(INSTRFN_NAME(adc_a_imm8),"adc a,imm8"));
		opcode_set(0xD6,CCPUInstr(INSTRFN_NAME(sub_a_imm8),"sub a,imm8"));
		opcode_set(0xDE,CCPUInstr(INSTRFN_NAME(sbc_a_imm8),"sbc a,imm8"));
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
		
		opcode_set(0x08,CCPUInstr(INSTRFN_NAME(ld_a16_sp),"ld [a16], sp"));
		opcode_set(0xf8,CCPUInstr(INSTRFN_NAME(ld_hl_spimm8),"ld hl, sp+imm8"));
		opcode_set(0xf9,CCPUInstr(INSTRFN_NAME(ld_sp_hl),"ld sp, hl"));

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
		opcode_set(0x18,CCPUInstr(INSTRFN_NAME(jr),"jr imm8"));
		opcode_set(0x20,CCPUInstr(INSTRFN_NAME(jr_nz),"jr nz,imm8"));
		opcode_set(0x28,CCPUInstr(INSTRFN_NAME(jr_z),"jr z,imm8"));
		opcode_set(0x30,CCPUInstr(INSTRFN_NAME(jr_nc),"jr nc,imm8"));
		opcode_set(0x38,CCPUInstr(INSTRFN_NAME(jr_c),"jr c,imm8"));

		opcode_set(0xC3,CCPUInstr(INSTRFN_NAME(jp_imm16),"jp imm16"));
		opcode_set(0xE9,CCPUInstr(INSTRFN_NAME(jp_hl),"jp hl"));
		opcode_set(0xC2,CCPUInstr(INSTRFN_NAME(jp_nz),"jp nz,imm16"));
		opcode_set(0xCA,CCPUInstr(INSTRFN_NAME(jp_z),"jp z,imm16"));
		opcode_set(0xD2,CCPUInstr(INSTRFN_NAME(jp_nc),"jp nc,imm16"));
		opcode_set(0xDA,CCPUInstr(INSTRFN_NAME(jp_c),"jp c,imm16"));

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
		opcode_set(0x37,CCPUInstr(INSTRFN_NAME(scf),"scf"));
		opcode_set(0x3F,CCPUInstr(INSTRFN_NAME(ccf),"ccf"));
		opcode_set(0x27,CCPUInstr(INSTRFN_NAME(daa),"daa"));
		opcode_set(0xF3,CCPUInstr(INSTRFN_NAME(di),"di"));
		opcode_set(0xFB,CCPUInstr(INSTRFN_NAME(ei),"ei"));
		opcode_set(0x76,CCPUInstr(INSTRFN_NAME(halt),"halt"));
		
		// reset cpu state ------------------------------@/
		reset();
	}

	auto CCPU::reset() -> void {
		m_should_enableIME = false;
		m_regIME = false;
		m_haltwaiting = false;

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
		m_clockWaiting = false;
		m_clockWaitBuffer = std::stack<int>();

		m_lycCooldown = false;

		m_timerctrDiv = 0;
		m_timerctrMain = 0;

		m_curopcode_ptr = nullptr;

		// setup instruction history
		m_instrhistory = std::deque<CInstrHistoryData>();
		std::vector<int> opcodedata = { 0xFF };
		for(int i=0; i<4; i++) {
			m_instrhistory.push_back(CInstrHistoryData());
		}
	}

	// instruction history ------------------------------@/
	auto CCPU::instrhistory_get(int index) -> CInstrHistoryData {
		return m_instrhistory.at(m_instrhistory.size()-1-index);
	}
	auto CCPU::instrhistory_push(int bank, int pc, const std::vector<int>& opcodedata) -> void {
		auto instr = fern::CInstrHistoryData(bank,pc,opcodedata);
		m_instrhistory.push_back(instr);
		m_instrhistory.pop_front();
	}
	auto CCPU::instrhistory_pushCurrent() -> void {
		// get rom bank
		int bank = emu()->mem.rombank_current();
		int pc = m_PC;
		std::vector<int> opcode_data = { 0xFF };
		instrhistory_push(bank,pc,opcode_data);
	}

	// opcode setting -----------------------------------@/
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

	auto CCPU::print_status(bool instr_history) -> void {
		if(!m_curopcode_ptr) return;
		auto &mem = emu()->mem;
		const auto rombank = mem.rombank_current();
		const int ly = mem.m_io.m_LY;
		std::printf("last opcode: %s\n",m_curopcode_ptr->name.c_str());
		std::printf("CPU: %04Xh[+%4Xh]\n",m_PC,m_SP);
		std::printf("ROM: %02Xh\n",rombank);
		std::printf("\tAF:   $%04X LY:   %3d\n",reg_af(),ly);
		std::printf("\tBC:   $%04X LCDC: $%02X\n",reg_bc(),mem.m_io.m_LCDC);
		std::printf("\tDE:   $%04X IE:   %d\n",reg_de(),mem.m_io.m_IE);
		std::printf("\tHL:   $%04X IF:   %d\n",reg_hl(),mem.m_io.m_IF);
		std::printf("\tSTAT: $%04X IME:  %d\n",mem.m_io.m_STAT,m_regIME);
		std::printf("\tDC:    %4d DIV:   $%02X\n",m_dotclock,mem.m_io.m_DIV);

		if(instr_history) {
			for(int i=0; i<m_instrhistory.size(); i++) {
				const auto hisdata = instrhistory_get(i);
				std::printf("\thistory[-%d]: pc=$%02X:%04X\n",i,hisdata.bank,hisdata.pc);
			}
		}
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
		flag_setZero((result & 0xFF) == 0);
		flag_setSubtract(true);
		flag_setHalfcarry((res_4b & 0x10) == 0x10);
		flag_setCarry(opB > opA);
	}
	auto CCPU::flag_syncCompareInc(int operand) -> void {
		auto result = (operand + 1) & 0xFF;
		auto res_4b = (operand & 0xF) + (1 & 0xF);
		flag_setZero(result == 0);
		flag_setSubtract(false);
		flag_setHalfcarry((res_4b & 0x10) == 0x10);
	}
	auto CCPU::flag_syncCompareDec(int operand) -> void {
		auto result = (operand - 1) & 0xFF;
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
			m_PC = stack_pop16();
			m_should_enableIME = true;
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

		// push opcode
		instrhistory_pushCurrent();

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
			std::printf("unknown opcode: $%02X\n",opcode_num);
			print_status();
			std::exit(-1);
		}
	}

	auto CCPU::clock_tick(const int cyclecnt) -> void {
		// if not already waiting, wait, and buffer additional clock ticks
		if(!m_clockWaiting) {
			m_clockWaiting = true;
			m_clockWaitBuffer.push(cyclecnt);
			
			while(!m_clockWaitBuffer.empty()) {
				const auto wait_cycles = m_clockWaitBuffer.top();
				m_clockWaitBuffer.pop();

				auto &mem = emu()->mem;
				// 4 dots per cycle
				if(mem.m_io.ppu_enabled()) {
					m_dotclock += (wait_cycles*4);
				}

				// set current mode
				// mode 2: OAM scan (0-79?) (no OAM)
				// mode 3: OAM draw (no OAM/VRAM)
				// mode 1: vblank (all accessible)
				// mode 0: hblank (all accessible)
				bool do_drawline = false;
				auto old_scanline = mem.m_io.m_LY;
				int old_mode = mem.m_io.stat_getMode();
				bool did_vblStart = false;
				bool do_flipscreen = false;

				if(m_dotclock >= 456) {
					mem.m_io.m_LY += 1;
					if(mem.m_io.m_LY > 153) {
						mem.m_io.m_LY = 0;
					}
					did_vblStart = (mem.m_io.m_LY == 144);
					//did_vblStart = (mem.m_io.m_LY == 0);
					do_flipscreen = did_vblStart;

					do_drawline = true;
					m_dotclock -= 456;
					m_lycCooldown = true;
				} else {
					if(m_dotclock < 80) {
						mem.m_io.stat_setMode(2);
					} else if(m_dotclock >= 80 && m_dotclock < (80+172)) {
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

				// set flags based on mode changes ------@/
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
				}

				if((mem.m_io.m_STAT & RFlagSTAT::lycint) && mem.m_io.stat_lycSame() && m_lycCooldown) {
					if(mem.m_io.ppu_enabled()) {
						mem.m_io.m_IF |= RFlagIF::stat;
						m_lycCooldown = false;
					}
				}

				if(did_vblStart) {
					mem.m_io.m_IF |= RFlagIF::vblank;
					do_flipscreen = true;
				}

				// tick timers --------------------------@/
				m_timerctrDiv += wait_cycles;
				if(m_timerctrDiv >= 64) {
					m_timerctrDiv -= 64;
					mem.m_io.m_DIV += 1;
				}
				if(mem.m_io.m_TAC & BIT(2)) {
					m_timerctrMain += wait_cycles;
				}
				const std::array<int,4> timer_limts = { 256,4,16,64 };
				int maintimer_limit = timer_limts[mem.m_io.m_TAC & 0b11];
				if(m_timerctrMain >= maintimer_limit) {
					m_timerctrMain -= maintimer_limit;
					mem.m_io.m_TIMA += 1;
					if(mem.m_io.m_TIMA == 0) {
						mem.m_io.m_IF |= RFlagIF::timer;
						mem.m_io.m_TIMA = mem.m_io.m_TMA;
					}
				}

				// deal with interrupts, if enabled. ----@/
				if(m_regIME) {
					if((mem.m_io.m_IF & mem.m_io.m_IE) != 0) {
						m_haltwaiting = false;	
					}

					// vblank interrupt
					if(mem.interrupt_match(BIT(0))) {
						mem.interrupt_clear(BIT(0));
						m_regIME = false;
						stack_push16(m_PC);
						m_PC = 0x40;
						clock_tick(5);
					}
					// LCD interrupt
					else if(mem.interrupt_match(BIT(1))) {
						mem.interrupt_clear(BIT(1));
						m_regIME = false;
						stack_push16(m_PC);
						m_PC = 0x48;
						clock_tick(5);
					}
					// timer interrupt
					else if(mem.interrupt_match(BIT(2))) {
						mem.interrupt_clear(BIT(2));
						m_regIME = false;
						stack_push16(m_PC);
						m_PC = 0x50;
						clock_tick(5);
					}
					// serial interrupt
					else if(mem.interrupt_match(0x08)) {
						std::puts("unimplemented interrupt (serial)");
						std::exit(-1);
					}
					// joypad interrupt
					else if(mem.interrupt_match(0x10)) {
						std::puts("unimplemented interrupt (joypad)");
						std::exit(-1);
					}
				}

				if(do_drawline) {
					if(!mem.m_io.ppu_enabled()) {
						std::puts("CCPU::clock_tick(): bad drawline?");
						std::exit(-1);
					}
					emu()->renderer.draw_line(old_scanline);
				}
				if(do_flipscreen) {
					emu()->renderer.present();
				}
			}

			m_clockWaiting = false;
		}
		// otherwise, wait til current fn's done	
		else {
			m_clockWaitBuffer.push(cyclecnt);
		}
	}
}

