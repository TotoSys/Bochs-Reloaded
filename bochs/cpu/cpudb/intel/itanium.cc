/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 2025 TotoSys
//          Written by TotoSys
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

#include "bochs.h"
#include "cpu/cpu.h"
#include "gui/siminterface.h"
#include "param_names.h"
#include "itanium.h"

#define LOG_THIS cpu->

#if BX_SUPPORT_X86_64 && BX_SUPPORT_IA64

itanium_t::itanium_t(BX_CPU_C *cpu):
    bx_cpuid_t(cpu)
{
  if (! BX_SUPPORT_X86_64)
    BX_PANIC(("You must enable x86-64 for Intel Itanium configuration"));

  if (! BX_SUPPORT_IA64)
    BX_PANIC(("You must enable IA-64 for Intel Itanium configuration"));

  // Enable standard x86 features that Itanium supported in x86 compatibility mode
  // Note: Itanium had limited x86 support - only basic features
  enable_cpu_extension(BX_ISA_X87);
  enable_cpu_extension(BX_ISA_486);
  enable_cpu_extension(BX_ISA_PENTIUM);
  enable_cpu_extension(BX_ISA_P6);
  enable_cpu_extension(BX_ISA_MMX);
  enable_cpu_extension(BX_ISA_SYSENTER_SYSEXIT);
  enable_cpu_extension(BX_ISA_DEBUG_EXTENSIONS);
  enable_cpu_extension(BX_ISA_VME);
  enable_cpu_extension(BX_ISA_PSE);
  enable_cpu_extension(BX_ISA_PAE);
  enable_cpu_extension(BX_ISA_PGE);
#if BX_PHY_ADDRESS_LONG
  enable_cpu_extension(BX_ISA_PSE36);
#endif
  enable_cpu_extension(BX_ISA_MTRR);
  enable_cpu_extension(BX_ISA_PAT);
  enable_cpu_extension(BX_ISA_XAPIC);
  enable_cpu_extension(BX_ISA_CMPXCHG16B);
  enable_cpu_extension(BX_ISA_CLFLUSH);
  
  // Itanium specific extensions
  enable_cpu_extension(BX_ISA_JMPE);  // Enable JMPE instruction support
}

void itanium_t::get_cpuid_leaf(Bit32u function, Bit32u subfunction, cpuid_function_t *leaf) const
{
  static const char* brand_string = "Intel(R) Itanium(R) Processor";

  static bool cpuid_limit_winnt = SIM->get_param_bool(BXPN_CPUID_LIMIT_WINNT)->get();
  if (cpuid_limit_winnt)
    if (function > 2 && function < 0x80000000) function = 2;

  switch(function) {
  case 0x80000000:
    get_ext_cpuid_leaf_0(leaf);
    return;
  case 0x80000001:
    get_ext_cpuid_leaf_1(leaf);
    return;
  case 0x80000002:
  case 0x80000003:
  case 0x80000004:
    get_ext_cpuid_brand_string_leaf(brand_string, function, leaf);
    return;
  case 0x80000005:
    get_reserved_leaf(leaf);
    return;
  case 0x80000006:
    get_ext_cpuid_leaf_6(leaf);
    return;
  case 0x80000007:
    get_ext_cpuid_leaf_7(leaf);
    return;
  case 0x80000008:
    get_ext_cpuid_leaf_8(leaf);
    return;
  case 0x00000000:
    get_std_cpuid_leaf_0(leaf);
    return;
  case 0x00000001:
    get_std_cpuid_leaf_1(leaf);
    return;
  case 0x00000002:
    get_std_cpuid_leaf_2(leaf);
    return;
  case 0x00000003:
    get_reserved_leaf(leaf);
    return;
  case 0x00000004:
    get_std_cpuid_leaf_4(subfunction, leaf);
    return;
  case 0x00000005:
    get_std_cpuid_leaf_5(leaf);
    return;
  case 0x00000006:
    get_std_cpuid_leaf_6(leaf);
    return;
  case 0x00000007:
  case 0x00000008:
  case 0x00000009:
  case 0x0000000A:
  case 0x0000000B:
  case 0x0000000C:
  default:
    get_reserved_leaf(leaf);
    return;
  }
}

// leaf 0x00000000 //
void itanium_t::get_std_cpuid_leaf_0(cpuid_function_t *leaf) const
{
  // EAX: highest std function understood by CPUID
  // EBX: vendor ID string
  // EDX: vendor ID string
  // ECX: vendor ID string
  get_leaf_0(0x6, "GenuineIntel", leaf);
}

// leaf 0x00000001 //
void itanium_t::get_std_cpuid_leaf_1(cpuid_function_t *leaf) const
{
  // EAX:       CPU Version Information
  //   [3:0]   Stepping ID = 4
  //   [7:4]   Model: starts at 1 = 0
  //   [11:8]  Family: 4=486, 5=Pentium, 6=PPro, 7=Itanium, ... = 7
  //   [13:12] Type: 0=OEM, 1=overdrive, 2=dual cpu, 3=reserved = 0
  //   [19:16] Extended Model = 0
  //   [27:20] Extended Family = 2
  leaf->eax = 0x20000704;

  // EBX:
  //   [7:0]   Brand ID
  //   [15:8]  CLFLUSH cache line size (value*8 = cache line size in bytes)
  //   [23:16] Number of logical processors in one physical processor
  //   [31:24] Local Apic ID
  leaf->ebx = 0;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_CLFLUSH)) {
    leaf->ebx |= (CACHE_LINE_SIZE / 8) << 8;
  }
  unsigned n_logical_processors = ncores*nthreads;
  leaf->ebx |= (n_logical_processors << 16);
#if BX_SUPPORT_APIC
  leaf->ebx |= ((cpu->get_apic_id() & 0xff) << 24);
#endif

  // ECX: Extended Feature Flags
  leaf->ecx = 0;
  // Itanium typically had very limited x86 feature support in compatibility mode
  
  // EDX: Standard Feature Flags
  leaf->edx = 0;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_X87))
    leaf->edx |= BX_CPUID_STD_X87;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_VME))
    leaf->edx |= BX_CPUID_STD_VME;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_DEBUG_EXTENSIONS))
    leaf->edx |= BX_CPUID_STD_DEBUG_EXTENSIONS;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_PSE))
    leaf->edx |= BX_CPUID_STD_PSE;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_PAE))
    leaf->edx |= BX_CPUID_STD_PAE;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_PGE))
    leaf->edx |= BX_CPUID_STD_PGE;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_MMX))
    leaf->edx |= BX_CPUID_STD_MMX;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_PAT))
    leaf->edx |= BX_CPUID_STD_PAT;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_SYSENTER_SYSEXIT))
    leaf->edx |= BX_CPUID_STD_SYSENTER_SYSEXIT;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_MTRR))
    leaf->edx |= BX_CPUID_STD_MTRR;
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_CLFLUSH))
    leaf->edx |= BX_CPUID_STD_CLFLUSH;
  
  // IA-64 capability bit (bit 30)
  leaf->edx |= (1 << 30); // IA-64 capability bit
  
  // JMPE capability bit (bit 31)
  if (BX_CPUID_SUPPORT_ISA_EXTENSION(BX_ISA_JMPE))
    leaf->edx |= (1 << 31);
}

// leaf 0x00000002 //
void itanium_t::get_std_cpuid_leaf_2(cpuid_function_t *leaf) const
{
  // CPUID function 0x00000002 - Cache and TLB Descriptors for Itanium
  leaf->eax = 0x00000000;  // No cache descriptors defined for Itanium
  leaf->ebx = 0;
  leaf->ecx = 0;
  leaf->edx = 0;
}

// leaf 0x00000004 //
void itanium_t::get_std_cpuid_leaf_4(Bit32u subfunction, cpuid_function_t *leaf) const
{
  // CPUID function 0x00000004 - Deterministic Cache Parameters
  leaf->eax = 0;
  leaf->ebx = 0;
  leaf->ecx = 0;
  leaf->edx = 0;
}

// leaf 0x00000005 //
void itanium_t::get_std_cpuid_leaf_5(cpuid_function_t *leaf) const
{
  // CPUID function 0x00000005 - MONITOR/MWAIT Leaf
  // Not supported on Itanium
  leaf->eax = 0;
  leaf->ebx = 0;
  leaf->ecx = 0;
  leaf->edx = 0;
}

// leaf 0x00000006 //
void itanium_t::get_std_cpuid_leaf_6(cpuid_function_t *leaf) const
{
  // CPUID function 0x00000006 - Thermal and Power Management Leaf
  // Not supported on Itanium
  leaf->eax = 0;
  leaf->ebx = 0;
  leaf->ecx = 0;
  leaf->edx = 0;
}

// leaf 0x80000000 //
void itanium_t::get_ext_cpuid_leaf_0(cpuid_function_t *leaf) const
{
  // EAX: highest extended function understood by CPUID
  leaf->eax = 0x80000008;
  
  // EBX, ECX, EDX: reserved
  leaf->ebx = 0;
  leaf->ecx = 0;
  leaf->edx = 0;
}

// leaf 0x80000001 //
void itanium_t::get_ext_cpuid_leaf_1(cpuid_function_t *leaf) const
{
  // EAX: reserved
  leaf->eax = 0;
  
  // EBX: reserved
  leaf->ebx = 0;
  
  // ECX:
  leaf->ecx = 0;

  // EDX:
  // Itanium supports 64-bit mode and NX bit
  leaf->edx = BX_CPUID_STD2_NX;
}

// leaf 0x80000006 //
void itanium_t::get_ext_cpuid_leaf_6(cpuid_function_t *leaf) const
{
  // CPUID function 0x80000006 - L2 Cache and TLB Identifiers
  leaf->eax = 0;
  leaf->ebx = 0;
  leaf->ecx = 0x04008040;  // L2 Cache: 4MB, 64-byte line size, 16-way associative
  leaf->edx = 0;
}

// leaf 0x80000007 //
void itanium_t::get_ext_cpuid_leaf_7(cpuid_function_t *leaf) const
{
  // CPUID function 0x80000007 - Advanced Power Management
  leaf->eax = 0;
  leaf->ebx = 0;
  leaf->ecx = 0;
  leaf->edx = 0x00000100; // bit 8 - invariant TSC
}

// leaf 0x80000008 //
void itanium_t::get_ext_cpuid_leaf_8(cpuid_function_t *leaf) const
{
  // CPUID function 0x80000008 - Address Size and Physical Core Count
  leaf->eax = 0x00003040; // bit 0-7: physical address bits (48-bit physical address)
                         // bit 8-15: virtual address bits (64-bit virtual address)
  leaf->ebx = 0;
  leaf->ecx = 0;
  leaf->edx = 0;
}

void itanium_t::dump_cpuid(void) const
{
  bx_cpuid_t::dump_cpuid(0x6, 0x8);
}

bx_cpuid_t *create_itanium_cpuid(BX_CPU_C *cpu) { return new itanium_t(cpu); }

#endif