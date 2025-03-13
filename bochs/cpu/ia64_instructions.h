/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA B 02110-1301 USA
//
/////////////////////////////////////////////////////////////////////////

#ifndef BX_IA64_INSTRUCTIONS_H
#define BX_IA64_INSTRUCTIONS_H

// IA-64 instruction formats

// A-type instruction for integer ALU
typedef struct {
  unsigned qp   : 6;  // Predicate qualifier
  unsigned x2   : 4;  // Opcode extension
  unsigned r3   : 7;  // Source/destination register (r3)
  unsigned r2   : 7;  // Source register (r2)
  unsigned s    : 1;  // Size of value
  unsigned x4   : 2;  // Opcode extension
  unsigned ve   : 1;  // Vertical extension
  unsigned r1   : 7;  // Source register (r1)
  unsigned op   : 4;  // Opcode
} ia64_a_inst_t;

// I-type instruction for integer ALU with immediate
typedef struct {
  unsigned qp   : 6;  // Predicate qualifier
  unsigned imm  : 14; // Immediate value (14 bits)
  unsigned s    : 1;  // Size of value
  unsigned x2   : 2;  // Opcode extension
  unsigned ve   : 1;  // Vertical extension
  unsigned r1   : 7;  // Source register (r1)
  unsigned op   : 4;  // Opcode
} ia64_i_inst_t;

// M-type instruction for memory access
typedef struct {
  unsigned qp   : 6;  // Predicate qualifier
  unsigned x2   : 4;  // Opcode extension
  unsigned r3   : 7;  // Source/destination register (r3)
  unsigned r2   : 7;  // Base register (r2)
  unsigned x4   : 3;  // Opcode extension
  unsigned hint : 2;  // Memory hint
  unsigned r1   : 7;  // Offset register or immediate index
  unsigned op   : 4;  // Opcode
} ia64_m_inst_t;

// B-type instruction for branches
typedef struct {
  unsigned qp   : 6;  // Predicate qualifier
  unsigned d    : 6;  // Displacement (part 1)
  unsigned wh   : 3;  // Whether and hint
  unsigned d2   : 10; // Displacement (part 2)
  unsigned p    : 1;  // Prediction
  unsigned b2   : 4;  // Branch attributes
  unsigned op   : 4;  // Opcode
} ia64_b_inst_t;

// F-type instruction for floating-point
typedef struct {
  unsigned qp   : 6;  // Predicate qualifier
  unsigned x2   : 4;  // Opcode extension
  unsigned f3   : 7;  // Source/destination register (f3)
  unsigned f2   : 7;  // Source register (f2)
  unsigned x4   : 3;  // Opcode extension
  unsigned sf   : 1;  // Status field
  unsigned f1   : 7;  // Source register (f1)
  unsigned op   : 4;  // Opcode
} ia64_f_inst_t;

// Helper functions for decoding and executing IA-64 instructions
void decode_ia64_a_instruction(Bit32u slot, ia64_a_inst_t *inst);
void decode_ia64_i_instruction(Bit32u slot, ia64_i_inst_t *inst);
void decode_ia64_m_instruction(Bit32u slot, ia64_m_inst_t *inst);
void decode_ia64_b_instruction(Bit32u slot, ia64_b_inst_t *inst);
void decode_ia64_f_instruction(Bit32u slot, ia64_f_inst_t *inst);

// Functions to execute instructions based on type
void execute_ia64_a_instruction(ia64_a_inst_t *inst);
void execute_ia64_i_instruction(ia64_i_inst_t *inst);
void execute_ia64_m_instruction(ia64_m_inst_t *inst);
void execute_ia64_b_instruction(ia64_b_inst_t *inst);
void execute_ia64_f_instruction(ia64_f_inst_t *inst);

#endif // BX_IA64_INSTRUCTIONS_H