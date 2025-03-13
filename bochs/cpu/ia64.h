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

#ifndef BX_IA64_SUPPORT_H
#define BX_IA64_SUPPORT_H

#define BX_IA64_BUNDLE_SIZE 16  // Itanium instructions are grouped into 16-byte bundles

// Itanium register access macros will go here
#define IA64_REG_READ(regnum) (BX_CPU_THIS_PTR ia64_regs[regnum])
#define IA64_REG_WRITE(regnum, val) BX_CPU_THIS_PTR ia64_regs[regnum] = (val)

// Itanium mode control
void BX_CPU_C::enter_ia64_mode(void);
void BX_CPU_C::leave_ia64_mode(void);
bool BX_CPU_C::is_ia64_mode(void);

// JMPE instruction implementation declarations
void BX_CPP_AttrRegparmN(1) BX_CPU_C::JMPE_Jw(bxInstruction_c *i);
void BX_CPP_AttrRegparmN(1) BX_CPU_C::JMPE_Jd(bxInstruction_c *i);
void BX_CPP_AttrRegparmN(1) BX_CPU_C::JMPE_Ev(bxInstruction_c *i)

#endif // BX_IA64_SUPPORT_H