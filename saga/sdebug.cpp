/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*

 Description:   
 
    Scripting module simple thread debugging support

 Notes: 
*/

#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "actor_mod.h"
#include "console_mod.h"
#include "text_mod.h"
#include "scene_mod.h"
#include "font_mod.h"

/*
 * Begin module component
\*--------------------------------------------------------------------------*/
#include "script.h"
#include "sthread.h"

namespace Saga {

#define SD_DISPLAY_LEN 128

#define SD_ADDTXT( x ) strncat( disp_buf, x, SD_DISPLAY_LEN );

int SDEBUG_PrintInstr(R_SCRIPT_THREAD * thread)
{

	R_TEXTLIST_ENTRY tl_e;

	const byte *start_p;
	const byte *read_p;

	char tmp_buf[80] = { 0 };
	static char disp_buf[SD_DISPLAY_LEN] = { 0 };

	int in_char;
	/*int op_offset; */

	int n_switch;
	int i;

	R_SCENE_INFO si;

	SCENE_GetInfo(&si);

	disp_buf[0] = 0;

	if (ScriptModule.dbg_txtentry != NULL) {

		TEXT_DeleteEntry(si.text_list, ScriptModule.dbg_txtentry);
		ScriptModule.dbg_txtentry = NULL;
	}

	tl_e.color = 1;
	tl_e.effect_color = 0;
	tl_e.text_x = 2;
	tl_e.text_y = 20;
	tl_e.font_id = SMALL_FONT_ID;
	tl_e.flags = FONT_OUTLINE;
	tl_e.string = disp_buf;
	tl_e.display = 1;

	read_p = ScriptModule.current_script->bytecode->bytecode_p +
	    thread->i_offset;

	start_p = read_p;

	in_char = ys_read_u8(read_p, &read_p);

	sprintf(tmp_buf, "%04lX | %02X | ", thread->i_offset, in_char);
	strncat(disp_buf, tmp_buf, SD_DISPLAY_LEN);

	switch (in_char) {

		/* Align */
	case 0x01:

		SD_ADDTXT("ALGN |");
		break;

		/* Push nothing */
	case 0x02:

		SD_ADDTXT("PSHN |");
		break;

		/* Pop nothing */
	case 0x03:

		SD_ADDTXT("POPN |");
		break;

		/* Push false (0) */
	case 0x04:

		SD_ADDTXT("PSHF |");
		break;

		/* Push true (1) */
	case 0x05:

		SD_ADDTXT("PSHT |");
		break;

		/* Push word (dialogue string index) */
	case 0x08:
		{
			int param;

			SD_ADDTXT("PSHD | ");

			param = ys_read_u16_le(read_p, &read_p);
			sprintf(tmp_buf, "%02X", param);
			SD_ADDTXT(tmp_buf);
/*
				if(( param >= 0 ) && ( param < diag_list->n_dialogue )) {
					printf(" ; \"%.*s\"", R_SCRIPT_STRINGLIMIT, diag_list->str[param] );
				}
				else {
					printf(" ; Invalid dialogue string.\n" );
				}
*/
		}
		break;

		/* Push word */
	case 0x06:
		{
			int param;

			SD_ADDTXT("PUSH | ");
			param = ys_read_u16_le(read_p, &read_p);
			sprintf(tmp_buf, "%04X", param);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Test flag */
	case 0x0B:
		{
			int param1;
			int param2;

			SD_ADDTXT("TSTF | ");
			param1 = *read_p++;
			param2 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %04X", param1, param2);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Get word */
	case 0x0C:
		{
			int param1;
			int param2;

			SD_ADDTXT("GETW | ");
			param1 = *read_p++;
			param2 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %04X", param1, param2);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Modify flag */
	case 0x0F:
		{
			int param1;
			int param2;

			SD_ADDTXT("MODF | ");
			param1 = *read_p++;
			param2 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %04X", param1, param2);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Put word */
	case 0x10:
		{
			int param1;
			int param2;

			SD_ADDTXT("PUTW | ");
			param1 = *read_p++;
			param2 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %04X", param1, param2);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Modify flag and pop */
	case 0x13:
		{
			int param1;
			int param2;

			SD_ADDTXT("MDFP | ");
			param1 = *read_p++;
			param2 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %04X", param1, param2);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Put word and pop */
	case 0x14:
		{
			int param1;
			int param2;

			SD_ADDTXT("PTWP | ");
			param1 = *read_p++;
			param2 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %04X", param1, param2);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Call subscript ? */
	case 0x17:
		{
			int param1;
			int param2;
			int param3;

			SD_ADDTXT("GOSB | ");
			param1 = *read_p++;
			param2 = *read_p++;
			param3 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %02X %04X", param1, param2,
			    param3);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Call function */
	case 0x19:
	case 0x18:
		{
			int func_num;
			int param;

			SD_ADDTXT("CALL | ");

			func_num = *read_p++;
			sprintf(tmp_buf, "%02X ", func_num);
			SD_ADDTXT(tmp_buf);

			param = ys_read_u16_le(read_p, &read_p);
			sprintf(tmp_buf, "%04X ", param);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Begin subscript */
	case 0x1A:
		{
			int param;

			SD_ADDTXT("ENTR | ");
			param = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%04X ", param);
			SD_ADDTXT(tmp_buf);
/*
				for( i = 0 ; i < script_list->n_scripts ; i++ ) {
					if( op_offset == script_list->scripts[i].offset ) {
						printf("; Entrypoint \"%s\".", script_list->scriptl_p +
							script_list->scripts[i].name_offset );
								
						break;
					}
				}
*/
		}
		break;

	case 0x1B:
		SD_ADDTXT("??? ");
		break;

		/* End subscript */
	case 0x1C:
		SD_ADDTXT("EXIT |");
		break;

		/* Unconditional jump */
	case 0x1D:
		{
			int param1;

			SD_ADDTXT("JMP  | ");
			param1 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%04X", param1);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Jump if nonzero + POP */
	case 0x1E:
		{
			int param1;

			SD_ADDTXT("JNZP | ");
			param1 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%04X", param1);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Jump if zero + POP */
	case 0x1F:
		{
			int param1;

			SD_ADDTXT("JZP  | ");
			param1 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%04X", param1);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Jump if nonzero */
	case 0x20:
		{
			int param1;

			SD_ADDTXT("JNZ  | ");
			param1 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%04X", param1);
			SD_ADDTXT(tmp_buf);
		}
		break;

		/* Jump if zero */
	case 0x21:
		{
			int param1;

			SD_ADDTXT("JZ   | ");
			param1 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%04X", param1);
			SD_ADDTXT(tmp_buf);

		}
		break;

		/* Switch */
	case 0x22:
		{
			int switch_num;
			int switch_jmp;
			int default_jmp;

			SD_ADDTXT("SWCH | ");
			n_switch = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X\n", n_switch);
			SD_ADDTXT(tmp_buf);

			for (i = 0; i < n_switch; i++) {
				switch_num = ys_read_u16_le(read_p, &read_p);
				switch_jmp = ys_read_u16_le(read_p, &read_p);

				/*
				 * printf( R_TAB "CASE %04X, %04X\n", switch_num, switch_jmp );
				 */
			}

			default_jmp = ys_read_u16_le(read_p, &read_p);

			/*
			 * printf( R_TAB "DEF %04X", default_jmp );
			 */
		}
		break;

		/* Random branch */
	case 0x24:
		{
			int n_switch2;
			int switch_num;
			int switch_jmp;

			SD_ADDTXT("RJMP | ");

			/* Ignored? */
			ys_read_u16_le(read_p, &read_p);

			n_switch2 = ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%04X", n_switch2);
			SD_ADDTXT(tmp_buf);

			for (i = 0; i < n_switch2; i++) {
				/*printf("\n"); */
				switch_num = ys_read_u16_le(read_p, &read_p);
				switch_jmp = ys_read_u16_le(read_p, &read_p);
				/*
				 * printf( R_TAB "WEIGHT %04X, %04X", switch_num, switch_jmp );
				 */
			}
		}
		break;

	case 0x25:
		SD_ADDTXT("NEG  |");
		break;

	case 0x26:
		SD_ADDTXT("TSTZ |");
		break;

	case 0x27:
		SD_ADDTXT("NOT  |");
		break;

	case 0x28:
		SD_ADDTXT("??? ");
		read_p++;
		ys_read_u16_le(read_p, &read_p);
		break;

	case 0x29:
		SD_ADDTXT("??? ");
		read_p++;
		ys_read_u16_le(read_p, &read_p);
		break;

	case 0x2A:
		SD_ADDTXT("??? ");
		read_p++;
		ys_read_u16_le(read_p, &read_p);
		break;

	case 0x2B:
		SD_ADDTXT("??? ");
		read_p++;
		ys_read_u16_le(read_p, &read_p);
		break;

		/* Addition */
	case 0x2C:
		SD_ADDTXT("ADD  |");
		break;

		/* Subtraction */
	case 0x2D:
		SD_ADDTXT("SUB  |");
		break;

		/* Integer multiplication */
	case 0x2E:
		SD_ADDTXT("MULT |");
		break;

		/* Integer division */
	case 0x2F:
		SD_ADDTXT("DIV |");
		break;

		/* Modulus */
	case 0x30:
		SD_ADDTXT("MOD  |");
		break;

		/* Test equality */
	case 0x33:
		SD_ADDTXT("EQU  |");
		break;

		/* Test inequality */
	case 0x34:
		SD_ADDTXT("NEQU |");
		break;

		/* Test Greater-than */
	case 0x35:
		SD_ADDTXT("GRT  |");
		break;

		/* Test Less-than */
	case 0x36:
		SD_ADDTXT("LST  |");
		break;

		/* Test Greater-than or Equal to */
	case 0x37:
		SD_ADDTXT("GRTE |");
		break;

		/* Test Less-than or Equal to */
	case 0x38:
		SD_ADDTXT("LSTE |");
		break;

	case 0x3F:
		SD_ADDTXT("SHR  |");
		break;

	case 0x40:
		SD_ADDTXT("SHL  |");
		break;

	case 0x41:
		SD_ADDTXT("AND  |");
		break;

	case 0x42:
		SD_ADDTXT("OR   |");
		break;

	case 0x43:
		SD_ADDTXT("XOR  |");
		break;

	case 0x44:
		SD_ADDTXT("LAND |");
		break;

	case 0x45:
		SD_ADDTXT("LOR  |");
		break;

	case 0x46:
		SD_ADDTXT("LXOR |");
		break;

	case 0x53:
		{
			int n_voices;
			int param1;
			int param2;

			SD_ADDTXT("DLGP | ");

			n_voices = *read_p++;
			param1 = ys_read_u16_le(read_p, &read_p);
			param2 = *read_p++;

			/* ignored ? */
			ys_read_u16_le(read_p, &read_p);

			sprintf(tmp_buf, "%02X %04X %02X", n_voices, param1,
			    param2);
			SD_ADDTXT(tmp_buf);
		}
		break;

	case 0x54:
		SD_ADDTXT("DLGS |");
		break;

	case 0x55:
		SD_ADDTXT("DLGX |");
		break;

	case 0x56:
		{
			int param1;
			int param2;
			int param3;

			SD_ADDTXT("DLGO | ");
			param1 = *read_p++;
			param2 = *read_p++;

			sprintf(tmp_buf, "%02X %02X ", param1, param2);
			SD_ADDTXT(tmp_buf);

			if (param2 > 0) {
				param3 = ys_read_u16_le(read_p, &read_p);

				sprintf(tmp_buf, "%04X", param3);
				SD_ADDTXT(tmp_buf);
			}
		}
		break;

	case 0x57:
		{
			int param1;
			int param2;
			int param3;

			SD_ADDTXT("JMPS | ");

			param1 = ys_read_u16_le(read_p, &read_p);
			param2 = ys_read_u16_le(read_p, &read_p);
			param3 = *read_p++;

			sprintf(tmp_buf, "%04X %04X %02X", param1, param2,
			    param3);
			SD_ADDTXT(tmp_buf);
		}
		break;

	default:
		sprintf(tmp_buf, "Invalid opcode.\n");
		SD_ADDTXT(tmp_buf);
		break;

	}			/* end switch( in_char ) */

	ScriptModule.dbg_txtentry = TEXT_AddEntry(si.text_list, &tl_e);
	TEXT_SetDisplay(ScriptModule.dbg_txtentry, 1);

	return R_SUCCESS;
}

} // End of namespace Saga
