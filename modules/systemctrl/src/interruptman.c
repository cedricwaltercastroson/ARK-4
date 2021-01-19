/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <globals.h>
#include <macros.h>
#include "module2.h"

extern ARKConfig* ark_config;

// Interrupt Manager Patch
SceModule2* patchInterruptMan(void)
{
	// Find Module
	SceModule2* mod = (SceModule2 *)sceKernelFindModuleByName("sceInterruptManager");
	
	// Fetch Text Address
	u32 addr = mod->text_addr;
	u32 topaddr = mod->text_addr + mod->text_size;
	
	if (IS_PSP(ark_config->exec_mode)){
	    // disable writing invalid address to reset vector
	    _sw(NOP, mod->text_addr + 0x00000DEC);
	    _sw(NOP, mod->text_addr + 0x00000DEC+4);

	    // disable crash code
	    _sw(0x408F7000, mod->text_addr + 0x00000E98); // mct0 $t7, $EPC
	    _sw(NOP, mod->text_addr + 0x00000E98+4);
	}
	else{
	    for (; addr<topaddr; addr+=4){
		    u32 data = _lw(addr);
		    // Override Endless Loop of breaking Death with EPC = t7
		    if (data == 0x0003FF8D){
			    _sw(0x408F7000, addr-4);
			    _sw(NOP, addr);
		    }
		    // Prevent Hardware Register Writing
		    else if ((data & 0x0000FFFF) == 0xBC00){
			    _sw(NOP, addr+4);
			    _sw(NOP, addr+8);
		    }
	    }
	}
	// Flush Cache
	flushCache();
	return mod;
}

