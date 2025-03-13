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

#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#include "cpu.h"
#include "ia64.h"
#include "ia64_instructions.h"
#define LOG_THIS BX_CPU_THIS_PTR

// Decode IA-64 A-type instruction (ALU)
void decode_ia64_a_instruction(Bit32u slot, ia64_a_inst_t *inst)
{
  // Extract fields from the 41-bit instruction
  inst->op = (slot >> 0) & 0xF;    // Bits 0-3
  inst->r1 = (slot >> 4) & 0x7F;   // Bits 4-10
  inst->ve = (slot >> 11) & 0x1;   // Bit 11
  inst->x4 = (slot >> 12) & 0x3;   // Bits 12-13
  inst->s  = (slot >> 14) & 0x1;   // Bit 14
  inst->r2 = (slot >> 15) & 0x7F;  // Bits 15-21
  inst->r3 = (slot >> 22) & 0x7F;  // Bits 22-28
  inst->x2 = (slot >> 29) & 0xF;   // Bits 29-32
  inst->qp = (slot >> 33) & 0x3F;  // Bits 33-38 (higher bits will be 0 due to 32-bit limitation)
  
  BX_DEBUG(("IA-64 A-type: op=%x r1=%d r2=%d r3=%d qp=%d", inst->op, inst->r1, inst->r2, inst->r3, inst->qp));
}

// Decode IA-64 I-type instruction (ALU with immediate)
void decode_ia64_i_instruction(Bit32u slot, ia64_i_inst_t *inst)
{
  // Extract fields from the 41-bit instruction
  inst->op  = (slot >> 0) & 0xF;    // Bits 0-3
  inst->r1  = (slot >> 4) & 0x7F;   // Bits 4-10
  inst->ve  = (slot >> 11) & 0x1;   // Bit 11
  inst->x2  = (slot >> 12) & 0x3;   // Bits 12-13
  inst->s   = (slot >> 14) & 0x1;   // Bit 14
  inst->imm = (slot >> 15) & 0x3FFF; // Bits 15-28
  inst->qp  = (slot >> 33) & 0x3F;  // Bits 33-38
  
  BX_DEBUG(("IA-64 I-type: op=%x r1=%d imm=0x%x qp=%d", inst->op, inst->r1, inst->imm, inst->qp));
}

// Decode IA-64 M-type instruction (Memory)
void decode_ia64_m_instruction(Bit32u slot, ia64_m_inst_t *inst)
{
  // Extract fields from the 41-bit instruction
  inst->op   = (slot >> 0) & 0xF;    // Bits 0-3
  inst->r1   = (slot >> 4) & 0x7F;   // Bits 4-10
  inst->hint = (slot >> 11) & 0x3;   // Bits 11-12
  inst->x4   = (slot >> 13) & 0x7;   // Bits 13-15
  inst->r2   = (slot >> 16) & 0x7F;  // Bits 16-22
  inst->r3   = (slot >> 23) & 0x7F;  // Bits 23-29
  inst->x2   = (slot >> 30) & 0xF;   // Bits 30-33
  inst->qp   = (slot >> 34) & 0x3F;  // Bits 34-39
  
  BX_DEBUG(("IA-64 M-type: op=%x r1=%d r2=%d r3=%d qp=%d", inst->op, inst->r1, inst->r2, inst->r3, inst->qp));
}

// Decode IA-64 B-type instruction (Branch)
void decode_ia64_b_instruction(Bit32u slot, ia64_b_inst_t *inst)
{
  // Extract fields from the 41-bit instruction
  inst->op  = (slot >> 0) & 0xF;    // Bits 0-3
  inst->b2  = (slot >> 4) & 0xF;    // Bits 4-7
  inst->p   = (slot >> 8) & 0x1;    // Bit 8
  inst->d2  = (slot >> 9) & 0x3FF;  // Bits 9-18
  inst->wh  = (slot >> 19) & 0x7;   // Bits 19-21
  inst->d   = (slot >> 22) & 0x3F;  // Bits 22-27
  inst->qp  = (slot >> 28) & 0x3F;  // Bits 28-33
  
  BX_DEBUG(("IA-64 B-type: op=%x p=%d d=0x%x%x wh=%d qp=%d", inst->op, inst->p, inst->d, inst->d2, inst->wh, inst->qp));
}

// Decode IA-64 F-type instruction (Floating-point)
void decode_ia64_f_instruction(Bit32u slot, ia64_f_inst_t *inst)
{
  // Extract fields from the 41-bit instruction
  inst->op  = (slot >> 0) & 0xF;    // Bits 0-3
  inst->f1  = (slot >> 4) & 0x7F;   // Bits 4-10
  inst->sf  = (slot >> 11) & 0x1;   // Bit 11
  inst->x4  = (slot >> 12) & 0x7;   // Bits 12-14
  inst->f2  = (slot >> 15) & 0x7F;  // Bits 15-21
  inst->f3  = (slot >> 22) & 0x7F;  // Bits 22-28
  inst->x2  = (slot >> 29) & 0xF;   // Bits 29-32
  inst->qp  = (slot >> 33) & 0x3F;  // Bits 33-38
  
  BX_DEBUG(("IA-64 F-type: op=%x f1=%d f2=%d f3=%d sf=%d qp=%d", 
           inst->op, inst->f1, inst->f2, inst->f3, inst->sf, inst->qp));
}

// Implementation of basic A-type instruction execution (integer ALU)
void execute_ia64_a_instruction(ia64_a_inst_t *inst)
{
  // Check predicate register - if false (0), instruction is nullified
  if (inst->qp != 0 && !BX_CPU_THIS_PTR ia64_read_pr(inst->qp))
    return;
  
  Bit64u result = 0;
  Bit64u src1 = BX_CPU_THIS_PTR ia64_read_gr(inst->r1);
  Bit64u src2 = BX_CPU_THIS_PTR ia64_read_gr(inst->r2);
  
  // Execute based on opcode
  switch (inst->op) {
    case 0x0: // ADD
      result = src1 + src2;
      break;
      
    case 0x1: // SUB
      result = src1 - src2;
      break;
      
    case 0x2: // AND
      result = src1 & src2;
      break;
      
    case 0x3: // OR
      result = src1 | src2;
      break;
      
    case 0x4: // XOR
      result = src1 ^ src2;
      break;
      
    case 0x5: // SHLADD (Shift left and add)
      {
        unsigned shift = inst->x4;
        if (shift > 0)
          result = (src1 << shift) + src2;
        else
          result = src1 + src2;
      }
      break;
      
    default:
      BX_ERROR(("Unsupported IA-64 A-type opcode: %x", inst->op));
      return;
  }
  
  // Write result to destination register
  BX_CPU_THIS_PTR ia64_write_gr(inst->r3, result);
}

// Implementation of basic I-type instruction execution (integer ALU with immediate)
void execute_ia64_i_instruction(ia64_i_inst_t *inst)
{
  // Check predicate register - if false (0), instruction is nullified
  if (inst->qp != 0 && !BX_CPU_THIS_PTR ia64_read_pr(inst->qp))
    return;
  
  Bit64u result = 0;
  Bit64u src1 = BX_CPU_THIS_PTR ia64_read_gr(inst->r1);
  Bit64u imm = inst->imm;
  
  // Sign extend the immediate value if needed
  if (inst->s && (imm & 0x2000))
    imm |= BX_CONST64(0xFFFFFFFFFFFFC000); // Sign extend from 14 bits
  
  // Execute based on opcode
  switch (inst->op) {
    case 0x0: // ADDI (Add immediate)
      result = src1 + imm;
      break;
      
    case 0x1: // SUBI (Subtract immediate)
      result = src1 - imm;
      break;
      
    case 0x2: // ANDI (And immediate)
      result = src1 & imm;
      break;
      
    case 0x3: // ORI (Or immediate)
      result = src1 | imm;
      break;
      
    case 0x4: // XORI (Xor immediate)
      result = src1 ^ imm;
      break;
      
    case 0x5: // SHLADDUI (Shift left and add unsigned immediate)
      {
        unsigned shift = inst->x2;
        result = (imm << shift) + src1;
      }
      break;
      
    default:
      BX_ERROR(("Unsupported IA-64 I-type opcode: %x", inst->op));
      return;
  }
  
  // Destination register is typically r1
  BX_CPU_THIS_PTR ia64_write_gr(inst->r1, result);
}

// Implementation of basic M-type instruction execution (memory)
void execute_ia64_m_instruction(ia64_m_inst_t *inst)
{
  // Check predicate register - if false (0), instruction is nullified
  if (inst->qp != 0 && !BX_CPU_THIS_PTR ia64_read_pr(inst->qp))
    return;
  
  Bit64u base_addr = BX_CPU_THIS_PTR ia64_read_gr(inst->r2);
  Bit64u offset = BX_CPU_THIS_PTR ia64_read_gr(inst->r1);
  Bit64u addr = base_addr + offset;
  
  // Execute based on opcode
  switch (inst->op) {
    case 0x0: // LD1 (Load 1 byte)
      {
        Bit8u val = read_virtual_byte(BX_SEG_REG_DS, addr);
        BX_CPU_THIS_PTR ia64_write_gr(inst->r3, (Bit64s)(Bit8s)val);
      }
      break;
      
    case 0x1: // LD2 (Load 2 bytes)
      {
        Bit16u val = read_virtual_word(BX_SEG_REG_DS, addr);
        BX_CPU_THIS_PTR ia64_write_gr(inst->r3, (Bit64s)(Bit16s)val);
      }
      break;
      
    case 0x2: // LD4 (Load 4 bytes)
      {
        Bit32u val = read_virtual_dword(BX_SEG_REG_DS, addr);
        BX_CPU_THIS_PTR ia64_write_gr(inst->r3, (Bit64s)(Bit32s)val);
      }
      break;
      
    case 0x3: // LD8 (Load 8 bytes)
      {
        Bit64u val = read_virtual_qword(BX_SEG_REG_DS, addr);
        BX_CPU_THIS_PTR ia64_write_gr(inst->r3, val);
      }
      break;
      
    case 0x4: // ST1 (Store 1 byte)
      {
        Bit64u val = BX_CPU_THIS_PTR ia64_read_gr(inst->r3);
        write_virtual_byte(BX_SEG_REG_DS, addr, val & 0xFF);
      }
      break;
      
    case 0x5: // ST2 (Store 2 bytes)
      {
        Bit64u val = BX_CPU_THIS_PTR ia64_read_gr(inst->r3);
        write_virtual_word(BX_SEG_REG_DS, addr, val & 0xFFFF);
      }
      break;
      
    case 0x6: // ST4 (Store 4 bytes)
      {
        Bit64u val = BX_CPU_THIS_PTR ia64_read_gr(inst->r3);
        write_virtual_dword(BX_SEG_REG_DS, addr, val & 0xFFFFFFFF);
      }
      break;
      
    case 0x7: // ST8 (Store 8 bytes)
      {
        Bit64u val = BX_CPU_THIS_PTR ia64_read_gr(inst->r3);
        write_virtual_qword(BX_SEG_REG_DS, addr, val);
      }
      break;
      
    default:
      BX_ERROR(("Unsupported IA-64 M-type opcode: %x", inst->op));
      return;
  }
}

// Implementation of basic B-type instruction execution (branch)
void execute_ia64_b_instruction(ia64_b_inst_t *inst)
{
  // Check predicate register - if false (0), instruction is nullified
  if (inst->qp != 0 && !BX_CPU_THIS_PTR ia64_read_pr(inst->qp))
    return;
  
  // Calculate branch target
  // Combine d and d2 fields to form a 16-bit displacement
  Bit16s displacement = ((inst->d & 0x3F) << 10) | (inst->d2 & 0x3FF);
  
  // Sign extend the displacement (if highest bit is set)
  if (displacement & 0x8000) {
    displacement |= 0xFFFF0000;
  }
  
  // Target is calculated by multiplying by 16 (size of a bundle)
  Bit64u target = RIP + (displacement * 16);
  
  // Execute based on opcode
  switch (inst->op) {
    case 0x0: // BR (Unconditional branch)
      BX_INFO(("IA-64 Unconditional branch to 0x" FMT_ADDRX64, target));
      RIP = target;
      break;
      
    case 0x1: // BR.COND (Conditional branch)
      {
        bool condition = false;
        // wh field determines the condition
        switch (inst->wh) {
          case 0: // Equal to zero
            condition = (BX_CPU_THIS_PTR ia64_read_gr(inst->d & 0x7F) == 0);
            break;
          case 1: // Not equal to zero
            condition = (BX_CPU_THIS_PTR ia64_read_gr(inst->d & 0x7F) != 0);
            break;
          default:
            BX_ERROR(("Unsupported IA-64 condition code: %x", inst->wh));
            return;
        }
        
        if (condition) {
          BX_INFO(("IA-64 Conditional branch taken to 0x" FMT_ADDRX64, target));
          RIP = target;
        } else {
          BX_INFO(("IA-64 Conditional branch not taken"));
        }
      }
      break;
      
    case 0x2: // BR.CALL (Call)
      {
        // Save return address in a branch register (b0)
        BX_CPU_THIS_PTR ia64_write_br(0, RIP + 16); // Next bundle
        BX_INFO(("IA-64 Call to 0x" FMT_ADDRX64 ", return addr 0x" FMT_ADDRX64, 
                target, RIP + 16));
        RIP = target;
      }
      break;
      
    case 0x3: // BR.RET (Return)
      {
        // Get return address from branch register (b0)
        Bit64u ret_addr = BX_CPU_THIS_PTR ia64_read_br(0);
        BX_INFO(("IA-64 Return to 0x" FMT_ADDRX64, ret_addr));
        RIP = ret_addr;
      }
      break;
      
    case 0x4: // BR.IA32 (Return to IA-32/x86 mode)
      {
        // Special branch type that switches back to x86 mode
        Bit64u x86_target = BX_CPU_THIS_PTR ia64_read_gr(inst->d & 0x7F);
        BX_INFO(("IA-64 Returning to x86 mode at address 0x" FMT_ADDRX64, x86_target));
        
        // Switch back to x86 mode
        BX_CPU_THIS_PTR leave_ia64_mode();
        
        // Set x86 EIP to the target address
        RIP = x86_target & 0xFFFFFFFF; // 32-bit EIP
      }
      break;
      
    default:
      BX_ERROR(("Unsupported IA-64 B-type opcode: %x", inst->op));
      return;
  }
}

// Implementation of basic F-type instruction execution (floating-point)
void execute_ia64_f_instruction(ia64_f_inst_t *inst)
{
  // Check predicate register - if false (0), instruction is nullified
  if (inst->qp != 0 && !BX_CPU_THIS_PTR ia64_read_pr(inst->qp))
    return;
  
  Bit64u src1 = BX_CPU_THIS_PTR ia64_read_fr(inst->f1);
  Bit64u src2 = BX_CPU_THIS_PTR ia64_read_fr(inst->f2);
  Bit64u result = 0;
  
  // Execute based on opcode
  switch (inst->op) {
    case 0x0: // FADD (Floating-point Add)
      {
        float64 a = float64_from_raw(src1);
        float64 b = float64_from_raw(src2);
        float_status_t status;
        result = float64_to_raw(float64_add(a, b, status));
      }
      break;
      
    case 0x1: // FSUB (Floating-point Subtract)
      {
        float64 a = float64_from_raw(src1);
        float64 b = float64_from_raw(src2);
        float_status_t status;
        result = float64_to_raw(float64_sub(a, b, status));
      }
      break;
      
    case 0x2: // FMUL (Floating-point Multiply)
      {
        float64 a = float64_from_raw(src1);
        float64 b = float64_from_raw(src2);
        float_status_t status;
        result = float64_to_raw(float64_mul(a, b, status));
      }
      break;
      
    case 0x3: // FDIV (Floating-point Divide)
      {
        float64 a = float64_from_raw(src1);
        float64 b = float64_from_raw(src2);
        float_status_t status;
        result = float64_to_raw(float64_div(a, b, status));
      }
      break;
      
    default:
      BX_ERROR(("Unsupported IA-64 F-type opcode: %x", inst->op));
      return;
  }
  
  BX_CPU_THIS_PTR ia64_write_fr(inst->f3, result);
}