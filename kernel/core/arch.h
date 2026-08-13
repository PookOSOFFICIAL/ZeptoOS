#pragma once

#ifdef __x86_64__
#include "../x64/gdt.h"
#include "../x64/idt.h"
#include "../x64/elf.h"
#include "../x64/vmm.h"
#else
#include "../i386/gdt.h"
#include "../i386/idt.h"
#include "../i386/elf.h"
#include "../i386/vmm.h"
#endif
