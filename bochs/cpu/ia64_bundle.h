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

#ifndef BX_IA64_BUNDLE_H
#define BX_IA64_BUNDLE_H

// IA-64 bundle structure (128 bits/16 bytes)
typedef struct {
  Bit32u slot0;  // Instruction slot 0
  Bit32u slot1;  // Instruction slot 1
  Bit32u slot2;  // Instruction slot 2
  Bit32u template_field;  // 4-bit template field and other fields
} bx_ia64_bundle_t;

// IA-64 instruction unit types
enum {
  IA64_UNIT_ALU = 0,
  IA64_UNIT_MEMORY = 1,
  IA64_UNIT_FLOAT = 2,
  IA64_UNIT_BRANCH = 3,
  IA64_UNIT_EXTENDED = 4
};

// IA-64 templates determine which execution unit handles each slot
typedef struct {
  Bit8u slot0_type;
  Bit8u slot1_type;
  Bit8u slot2_type;
  bool stop_bit;  // Stop bit after this bundle
} bx_ia64_template_t;

// Template information for all 32 possible template values
extern bx_ia64_template_t ia64_templates[32];

// Fetch and decode an IA-64 bundle
void fetch_ia64_bundle(bx_ia64_bundle_t *bundle, bx_address eip);
void execute_ia64_bundle(bx_ia64_bundle_t *bundle);

#endif // BX_IA64_BUNDLE_H