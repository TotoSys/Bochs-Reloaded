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
#define LOG_THIS BX_CPU_THIS_PTR

// The JMPE instruction transitions from IA-32 (x86) to IA-64 (Itanium) mode

// JMPE with 16-bit relative offset
void BX_CPP_AttrRegparmN(1) BX_CPU_C::JMPE_Jw(bxInstruction_c *i)
{
  BX_DEBUG(("JMPE_Jw: Switching to Itanium mode"));
  
  if (i->as32L()) {
    Bit32u new_eip = EIP + (Bit32s) i->Iw();
    branch_near32(new_eip);
  }
  else {
    Bit16u new_ip = IP + i->Iw();
    branch_near16(new_ip);
  }

  // Transition to IA-64 mode
  enter_ia64_mode();
  
  BX_NEXT_TRACE(i);
}

// JMPE instruction - Jump to IA-64 mode
void BX_CPP_AttrRegparmN(1) BX_CPU_C::JMPE_Jd(bxInstruction_c *i)
{
  // This instruction is used to transition from x86 to IA-64 mode
  // In a real Itanium processor, this would switch execution to IA-64 code
  // For our emulation, we just report that the instruction executed
  BX_INFO(("JMPE instruction: Transition to IA-64 mode requested"));

#if BX_DEBUGGER
  BX_CPU_THIS_PTR show_flag |= Flag_ia64_mode;
#endif

  // In a full implementation, we would:
  // 1. Save current x86 state
  // 2. Switch to IA-64 execution mode
  // 3. Begin executing IA-64 instructions at the target address

  // For now, we just end the simulation as we don't fully support IA-64 execution
  BX_PANIC(("JMPE: Full IA-64 mode not implemented in this version of Bochs"));
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::JMPE_Ev(bxInstruction_c *i)
{
  // This is the register/memory operand version of JMPE
  // It works the same as the immediate version
  BX_INFO(("JMPE instruction: Transition to IA-64 mode requested"));

#if BX_DEBUGGER
  BX_CPU_THIS_PTR show_flag |= Flag_ia64_mode;
#endif

  // In a full implementation, we would:
  // 1. Save current x86 state
  // 2. Switch to IA-64 execution mode
  // 3. Begin executing IA-64 instructions at the target address from register/memory

  // For now, we just end the simulation as we don't fully support IA-64 execution
  BX_PANIC(("JMPE: Full IA-64 mode not implemented in this version of Bochs"));
}

// IA-64 Register access functions

// General register (GR) access
Bit64u BX_CPP_AttrRegparmN(1) BX_CPU_C::ia64_read_gr(unsigned reg)
{
  // r0 is hardwired to zero
  if (reg == 0)
    return 0;
  
  // Check register number boundary
  if (reg >= BX_IA64_GR_REGS) {
    BX_ERROR(("ia64_read_gr: register number out of range: %d", reg));
    return 0;
  }
  
  return BX_CPU_THIS_PTR ia64_gr[reg];
}

void BX_CPP_AttrRegparmN(2) BX_CPU_C::ia64_write_gr(unsigned reg, Bit64u val)
{
  // r0 is read-only and always zero
  if (reg == 0)
    return;
  
  // Check register number boundary
  if (reg >= BX_IA64_GR_REGS) {
    BX_ERROR(("ia64_write_gr: register number out of range: %d", reg));
    return;
  }
  
  BX_CPU_THIS_PTR ia64_gr[reg] = val;
}

// Floating-point register (FR) access
Bit64u BX_CPP_AttrRegparmN(1) BX_CPU_C::ia64_read_fr(unsigned reg)
{
  // f0 is hardwired to 0.0
  if (reg == 0)
    return 0;
  
  // f1 is hardwired to 1.0
  if (reg == 1)
    return BX_CONST64(0x3FF0000000000000); // IEEE 754 double-precision 1.0
  
  // Check register number boundary
  if (reg >= BX_IA64_FR_REGS) {
    BX_ERROR(("ia64_read_fr: register number out of range: %d", reg));
    return 0;
  }
  
  return BX_CPU_THIS_PTR ia64_fr[reg];
}

void BX_CPP_AttrRegparmN(2) BX_CPU_C::ia64_write_fr(unsigned reg, Bit64u val)
{
  // f0 and f1 are read-only
  if (reg <= 1)
    return;
  
  // Check register number boundary
  if (reg >= BX_IA64_FR_REGS) {
    BX_ERROR(("ia64_write_fr: register number out of range: %d", reg));
    return;
  }
  
  BX_CPU_THIS_PTR ia64_fr[reg] = val;
}

// Predicate register (PR) access
bool BX_CPP_AttrRegparmN(1) BX_CPU_C::ia64_read_pr(unsigned reg)
{
  // p0 is hardwired to 1 (true)
  if (reg == 0)
    return true;
  
  // Check register number boundary
  if (reg >= BX_IA64_PR_REGS) {
    BX_ERROR(("ia64_read_pr: register number out of range: %d", reg));
    return false;
  }
  
  // Calculate which 64-bit block and bit position
  unsigned block = reg / 64;
  unsigned bit = reg % 64;
  
  return (BX_CPU_THIS_PTR ia64_pr[block] >> bit) & 1;
}

void BX_CPP_AttrRegparmN(2) BX_CPU_C::ia64_write_pr(unsigned reg, bool val)
{
  // p0 is read-only and always true
  if (reg == 0)
    return;
  
  // Check register number boundary
  if (reg >= BX_IA64_PR_REGS) {
    BX_ERROR(("ia64_write_pr: register number out of range: %d", reg));
    return;
  }
  
  // Calculate which 64-bit block and bit position
  unsigned block = reg / 64;
  unsigned bit = reg % 64;
  
  if (val)
    BX_CPU_THIS_PTR ia64_pr[block] |= (BX_CONST64(1) << bit);
  else
    BX_CPU_THIS_PTR ia64_pr[block] &= ~(BX_CONST64(1) << bit);
}

// Branch register (BR) access
Bit64u BX_CPP_AttrRegparmN(1) BX_CPU_C::ia64_read_br(unsigned reg)
{
  // Check register number boundary
  if (reg >= BX_IA64_BR_REGS) {
    BX_ERROR(("ia64_read_br: register number out of range: %d", reg));
    return 0;
  }
  
  return BX_CPU_THIS_PTR ia64_br[reg];
}

void BX_CPP_AttrRegparmN(2) BX_CPU_C::ia64_write_br(unsigned reg, Bit64u val)
{
  // Check register number boundary
  if (reg >= BX_IA64_BR_REGS) {
    BX_ERROR(("ia64_write_br: register number out of range: %d", reg));
    return;
  }
  
  BX_CPU_THIS_PTR ia64_br[reg] = val;
}

// Application register (AR) access
Bit64u BX_CPP_AttrRegparmN(1) BX_CPU_C::ia64_read_ar(unsigned reg)
{
  // Check register number boundary
  if (reg >= BX_IA64_AR_REGS) {
    BX_ERROR(("ia64_read_ar: register number out of range: %d", reg));
    return 0;
  }
  
  return BX_CPU_THIS_PTR ia64_ar[reg];
}

void BX_CPP_AttrRegparmN(2) BX_CPU_C::ia64_write_ar(unsigned reg, Bit64u val)
{
  // Check register number boundary
  if (reg >= BX_IA64_AR_REGS) {
    BX_ERROR(("ia64_write_ar: register number out of range: %d", reg));
    return;
  }
  
  BX_CPU_THIS_PTR ia64_ar[reg] = val;
}

// Mode control functions
void BX_CPU_C::enter_ia64_mode(void)
{
  BX_INFO(("CPU transitioning to Itanium (IA-64) mode"));
  BX_CPU_THIS_PTR ia64_mode = true;
  // Additional Itanium mode initialization would go here
}

void BX_CPU_C::leave_ia64_mode(void)
{
  BX_INFO(("CPU transitioning from Itanium (IA-64) mode to x86 mode"));
  BX_CPU_THIS_PTR ia64_mode = false;
}

bool BX_CPU_C::is_ia64_mode(void)
{
  return BX_CPU_THIS_PTR ia64_mode;
}