/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "charset.h"
#include "scumm.h"
#include "nut_renderer.h"

CharsetRenderer::CharsetRenderer(Scumm *vm) {

	_nextLeft = 0;
	_nextTop = 0;

	_top = 0;
	_left = 0;
	_startLeft = 0;
	_right = 0;

	_color = 0;

	_dropShadow = false;
	_center = false;
	_hasMask = false;
	_ignoreCharsetMask = false;
	_blitAlso = false;
	_firstChar = false;
	_disableOffsX = false;

	_vm = vm;
	_curId = 0;
}

void CharsetRendererCommon::setCurID(byte id) {
	_vm->checkRange(_vm->_maxCharsets - 1, 0, id, "Printing with bad charset %d");

	_curId = id;

	_fontPtr = _vm->getResourceAddress(rtCharset, id);
	if (_fontPtr == 0)
		error("CharsetRendererCommon::setCurID: charset %d not found!\n", id);

	if (_vm->_version == 4)
		_fontPtr += 17;
	else
		_fontPtr += 29;
}

void CharsetRendererV3::setCurID(byte id) {
	_vm->checkRange(_vm->_maxCharsets - 1, 0, id, "Printing with bad charset %d");

	_curId = id;

	_fontPtr = _vm->getResourceAddress(rtCharset, id);
	if (_fontPtr == 0)
		error("CharsetRendererCommon::setCurID: charset %d not found!\n", id);

	_nbChars = _fontPtr[4];
	_fontPtr += 6;
	_widthTable = _fontPtr;
	_fontPtr += _nbChars;
}

// do spacing for variable width old-style font
int CharsetRendererClassic::getCharWidth(byte chr) {
	if (chr >= 0x80 && _vm->_CJKMode)
		return 6;
	int spacing = 0;

	int offs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
	if (offs) {
		spacing = _fontPtr[offs] + (signed char)_fontPtr[offs + 2];
	}
	
	return spacing;
}

int CharsetRenderer::getStringWidth(int arg, const byte *text) {
	int pos = 0;
	int width = 1;
	byte chr;
	int oldID = getCurID(); 

	while ((chr = text[pos++]) != 0) {
		if (chr == 0xD)
			break;
		if (chr == '@')
			continue;
		if (chr == 254 || chr == 255) {
			chr = text[pos++];
			if (chr == 3)	// 'WAIT'
				break;
			if (chr == 8) { // 'Verb on next line'
				if (arg == 1)
					break;
				while (text[pos++] == ' ')
					;
				continue;
			}
			if (chr == 10 || chr == 21 || chr == 12 || chr == 13) {
				pos += 2;
				continue;
			}
			if (chr == 9 || chr == 1 || chr == 2) // 'Newline'
				break;
			if (chr == 14) {
				int set = text[pos] | (text[pos + 1] << 8);
				pos += 2;
				setCurID(set);
				continue;
			}
		}
		width += getCharWidth(chr);
	}

	setCurID(oldID);

	return width;
}

void CharsetRenderer::addLinebreaks(int a, byte *str, int pos, int maxwidth) {
	int lastspace = -1;
	int curw = 1;
	byte chr;
	int oldID = getCurID(); 

	while ((chr = str[pos++]) != 0) {
		if (chr == '@')
			continue;
		if (chr == 254)
			chr = 255;
		if (chr == 255) {
			chr = str[pos++];
			if (chr == 3) // 'Wait'
				break;
			if (chr == 8) { // 'Verb on next line'
				if (a == 1) {
					curw = 1;
				} else {
					while (str[pos] == ' ')
						str[pos++] = '@';
				}
				continue;
			}
			if (chr == 10 || chr == 21 || chr == 12 || chr == 13) {
				pos += 2;
				continue;
			}
			if (chr == 1) { // 'Newline'
				curw = 1;
				continue;
			}
			if (chr == 2) // 'Don't terminate with \n'
				break;
			if (chr == 14) {
				int set = str[pos] | (str[pos + 1] << 8);
				pos += 2;
				setCurID(set);
				continue;
			}
		}

		if (chr == ' ')
			lastspace = pos - 1;

		curw += getCharWidth(chr);
		if (lastspace == -1)
			continue;
		if (curw > maxwidth) {
			str[lastspace] = 0xD;
			curw = 1;
			pos = lastspace + 1;
			lastspace = -1;
		}
	}

	setCurID(oldID);
}

// German Zak font (should work for US version too).
#ifdef __PALM_OS__
static byte *germanCharsetDataV2;
static byte *frenchCharsetDataV2;
#else
static byte germanCharsetDataV2[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x03, 0x06, 0x0c, 0x18, 0x3e, 0x03, 0x00, 
	0x80, 0xc0, 0x60, 0x30, 0x18, 0x7c, 0xc0, 0x00, 
	0x00, 0x03, 0x3e, 0x18, 0x0c, 0x06, 0x03, 0x01, 
	0x00, 0xc0, 0x7c, 0x18, 0x30, 0x60, 0xc0, 0x80, 
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
	0x03, 0x03, 0x03, 0x07, 0x07, 0x0f, 0x1f, 0x7f, 
	0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 
	0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 
	0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0x0c, 0x18, 
	0x00, 0x00, 0x00, 0x80, 0xc0, 0x60, 0x30, 0x18, 
	0x18, 0x30, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00, 
	0x18, 0x0c, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x1f, 0x1f, 0x18, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0xf8, 0xf8, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xf8, 0xf8, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0x07, 0x0c, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0xc0, 0xe0, 0x30, 0x18, 0x18, 
	0x18, 0x18, 0x30, 0xe0, 0xc0, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x0c, 0x07, 0x03, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x18, 0x3c, 0x66, 0xc3, 0xc3, 0x66, 0x3c, 0x18, 
	0x18, 0x3c, 0x7e, 0xff, 0xff, 0x7e, 0x3c, 0x18, 
	0x18, 0x66, 0xc3, 0xdb, 0xdb, 0xc3, 0x66, 0x18, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00, 
	0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x66, 0x66, 0xff, 0x66, 0xff, 0x66, 0x66, 0x00, 
	0x18, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x18, 0x00, 
	0x62, 0x66, 0x0c, 0x18, 0x30, 0x66, 0x46, 0x00, 
	0x3c, 0x66, 0x3c, 0x38, 0x67, 0x66, 0x3f, 0x00, 
	0x30, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00, 
	0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00, 
	0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30, 
	0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 
	0x00, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00, 
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7e, 0x00, 
	0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00, 
	0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00, 
	0x06, 0x0e, 0x1e, 0x66, 0x7f, 0x06, 0x06, 0x00, 
	0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00, 
	0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00, 
	0x7e, 0x66, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x00, 
	0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00, 
	0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00, 
	0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30, 
	0x0e, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0e, 0x00, 
	0x7c, 0x82, 0xba, 0xa2, 0xa2, 0xba, 0x82, 0x7c, 
	0x70, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x70, 0x00, 
	0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x18, 0x3c, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 
	0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00, 
	0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00, 
	0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00, 
	0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7e, 0x00, 
	0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00, 
	0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00, 
	0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 
	0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 
	0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0x6c, 0x38, 0x00, 
	0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00, 
	0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00, 
	0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00, 
	0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00, 
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00, 
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x0e, 0x00, 
	0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00, 
	0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00, 
	0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 
	0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00, 
	0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00, 
	0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00, 
	0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00, 
	0x66, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00, 
	0x66, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00, 
	0x66, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xdb, 0xdb, 0x00, 
	0x00, 0x10, 0x30, 0x7f, 0x7f, 0x30, 0x10, 0x00, 
	0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00, 
	0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x7c, 0x00, 
	0x00, 0x00, 0x3c, 0x60, 0x60, 0x60, 0x3c, 0x00, 
	0x00, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3e, 0x00, 
	0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 
	0x00, 0x0e, 0x18, 0x3e, 0x18, 0x18, 0x18, 0x00, 
	0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x7c, 
	0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x00, 
	0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00, 
	0x00, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x3c, 
	0x00, 0x60, 0x60, 0x6c, 0x78, 0x6c, 0x66, 0x00, 
	0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 
	0x00, 0x00, 0x66, 0x7f, 0x7f, 0x6b, 0x63, 0x00, 
	0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00, 
	0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 
	0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06, 
	0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00, 
	0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00, 
	0x00, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x0e, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 
	0x00, 0x00, 0x63, 0x6b, 0x7f, 0x3e, 0x36, 0x00, 
	0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x0c, 0x78, 
	0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00, 
	0x66, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x66, 0x18, 0x3c, 0x66, 0x7e, 0x66, 0x66, 0x00, 
	0x42, 0x3c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x1c, 0x36, 0x36, 0x7c, 0x66, 0x66, 0x7c, 0x40, 
	0x08, 0x0c, 0x0e, 0xff, 0xff, 0x0e, 0x0c, 0x08, 
};

// French Zak font (should work for US version too).
static byte frenchCharsetDataV2[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x03, 0x06, 0x0c, 0x18, 0x3e, 0x03, 0x00, 
	0x80, 0xc0, 0x60, 0x30, 0x18, 0x7c, 0xc0, 0x00, 
	0x00, 0x03, 0x3e, 0x18, 0x0c, 0x06, 0x03, 0x01, 
	0x00, 0xc0, 0x7c, 0x18, 0x30, 0x60, 0xc0, 0x80, 
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
	0x03, 0x03, 0x03, 0x07, 0x07, 0x0f, 0x1f, 0x7f, 
	0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 
	0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 
	0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0x0c, 0x18, 
	0x00, 0x00, 0x00, 0x80, 0xc0, 0x60, 0x30, 0x18, 
	0x18, 0x30, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00, 
	0x18, 0x0c, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x1f, 0x1f, 0x18, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0xf8, 0xf8, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xf8, 0xf8, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0x07, 0x0c, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0xc0, 0xe0, 0x30, 0x18, 0x18, 
	0x18, 0x18, 0x30, 0xe0, 0xc0, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x0c, 0x07, 0x03, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x18, 0x3c, 0x66, 0xc3, 0xc3, 0x66, 0x3c, 0x18, 
	0x18, 0x3c, 0x7e, 0xff, 0xff, 0x7e, 0x3c, 0x18, 
	0x18, 0x66, 0xc3, 0xdb, 0xdb, 0xc3, 0x66, 0x18, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00, 
	0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x66, 0x66, 0xff, 0x66, 0xff, 0x66, 0x66, 0x00, 
	0x18, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x18, 0x00, 
	0x62, 0x66, 0x0c, 0x18, 0x30, 0x66, 0x46, 0x00, 
	0x3c, 0x66, 0x3c, 0x38, 0x67, 0x66, 0x3f, 0x00, 
	0x30, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00, 
	0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00, 
	0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30, 
	0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 
	0x00, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00, 
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7e, 0x00, 
	0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00, 
	0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00, 
	0x06, 0x0e, 0x1e, 0x66, 0x7f, 0x06, 0x06, 0x00, 
	0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00, 
	0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00, 
	0x7e, 0x66, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x00, 
	0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00, 
	0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00, 
	0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30, 
	0x10, 0x08, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00, 
	0x18, 0x24, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00, 
	0x00, 0x00, 0x3c, 0x60, 0x60, 0x3c, 0x18, 0x38, 
	0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x18, 0x3c, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 
	0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00, 
	0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00, 
	0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00, 
	0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7e, 0x00, 
	0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00, 
	0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00, 
	0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 
	0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 
	0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0x6c, 0x38, 0x00, 
	0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00, 
	0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00, 
	0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00, 
	0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00, 
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00, 
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x0e, 0x00, 
	0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00, 
	0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00, 
	0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 
	0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00, 
	0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00, 
	0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00, 
	0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00, 
	0x08, 0x10, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 
	0x10, 0x08, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 
	0x18, 0x24, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xdb, 0xdb, 0x00, 
	0x00, 0x6c, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00, 
	0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00, 
	0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x7c, 0x00, 
	0x00, 0x00, 0x3c, 0x60, 0x60, 0x60, 0x3c, 0x00, 
	0x00, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3e, 0x00, 
	0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 
	0x00, 0x0e, 0x18, 0x3e, 0x18, 0x18, 0x18, 0x00, 
	0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x7c, 
	0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x00, 
	0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00, 
	0x00, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x3c, 
	0x00, 0x60, 0x60, 0x6c, 0x78, 0x6c, 0x66, 0x00, 
	0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 
	0x00, 0x00, 0x66, 0x7f, 0x7f, 0x6b, 0x63, 0x00, 
	0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00, 
	0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00, 
	0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 
	0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06, 
	0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00, 
	0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00, 
	0x00, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x0e, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 
	0x00, 0x00, 0x63, 0x6b, 0x7f, 0x3e, 0x36, 0x00, 
	0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x0c, 0x78, 
	0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00, 
	0x18, 0x24, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00, 
	0x18, 0x24, 0x00, 0x3c, 0x66, 0x66, 0x3c, 0x00, 
	0x10, 0x08, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x00, 
	0x18, 0x24, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x00, 
	0x08, 0x0c, 0x0e, 0xff, 0xff, 0x0e, 0x0c, 0x08, 
};
#endif

void CharsetRendererV2::setCurID(byte id) {

	switch (_vm->_language) {
	case DE_DEU:
		_fontPtr = germanCharsetDataV2;
		break;
	case FR_FRA:
		_fontPtr = frenchCharsetDataV2;
		break;
	default:
		// For now we default to the german font
		_fontPtr = germanCharsetDataV2;
		break;
	}

#if 0
	// Decompress weird encoding in which the Zak executable contains the font.
	// I leave the code around in case we need to use it again (e.g. we might
	// have to include different fonts for french/spanish/russian/... version
	// of MM / Zak
	//
	int count = 0, len;
	byte b;
	const byte *data = frenchCharsetDataV2;
	const int size = sizeof(frenchCharsetDataV2);
	for (int offset = 0; offset < size; offset++) {
		if (data[offset+1] == 0x00 && data[offset+2] == 0xB2 &&
			data[offset+5] == 0x00 && data[offset+6] == 0xB0) {
			b = data[offset+3];
			len = data[offset+4];
			while (len--) {
				printf("0x%02x, ", b);
				count++;
				if (count % 8 == 0)
					printf("\n");
			}
			offset += 6;
		} else {
			printf("0x%02x, ", data[offset]);
			count++;
			if (count % 8 == 0)
				printf("\n");
		}
	}
	printf("\n");
	_system->quit();
#endif
}

int CharsetRendererV3::getCharWidth(byte chr) {
	int spacing = 0;

	spacing = *(_widthTable + chr);

	return spacing;
}

void CharsetRendererV3::setColor(byte color)
{
	_color = color;
	if (_vm->_features & GF_16COLOR) {
		_dropShadow = ((_color & 0xF0) != 0);
		_color &= 0x0f;
	} else
		_dropShadow = false;
}

void CharsetRendererV3::printChar(int chr) {
	// Indy3 / Zak256 / Loom
	VirtScreen *vs;
	byte *char_ptr, *dest_ptr, *mask_ptr;
	bool useMask;
	int w, h;
	int drawTop;

	_vm->checkRange(_vm->_maxCharsets - 1, 0, _curId, "Printing with bad charset %d");

	if ((vs = _vm->findVirtScreen(_top)) == NULL)
		return;

	if (chr == '@')
		return;

	_vm->_charsetColorMap[1] = _color;

	if (_firstChar) {
		_str.left = _left;
		_str.top = _top;
		_str.right = _left;
		_str.bottom = _top;
		_firstChar = false;
	}

	w = h = 8;
	if (_dropShadow) {
		w++;
		h++;
	}
	
	drawTop = _top - vs->topline;
	char_ptr = _fontPtr + chr * 8;
	dest_ptr = vs->screenPtr + vs->xstart + drawTop * _vm->_screenWidth + _left;
	mask_ptr = _vm->getMaskBuffer(_left, drawTop, 0);
	useMask = (vs->number == 0 && !_ignoreCharsetMask);

	_vm->updateDirtyRect(vs->number, _left, _left + w, drawTop, drawTop + h, 0);
	if (vs->number == 0)
		_hasMask = true;

	drawBits1(vs, dest_ptr, char_ptr, mask_ptr, drawTop, 8, 8);

	if (_str.left > _left)
		_str.left = _left;

	_left += getCharWidth(chr);

	if (_str.right < _left) {
		_str.right = _left;
		if (_dropShadow)
			_str.right++;
	}

	if (_str.bottom < _top + h)
		_str.bottom = _top + h;
}

void CharsetRendererClassic::printChar(int chr) {
	int width, height, origWidth, origHeight;
	int offsX, offsY;
	VirtScreen *vs;
	const byte *charPtr;
	int is2byte = (chr >= 0x80 && _vm->_CJKMode) ? 1 : 0;

	_vm->checkRange(_vm->_maxCharsets - 1, 1, _curId, "Printing with bad charset %d");
	
	if ((vs = _vm->findVirtScreen(_top)) == NULL && (vs = _vm->findVirtScreen(_top + getFontHeight())) == NULL)
		return;

	if (chr == '@')
		return;

	_vm->_charsetColorMap[1] = _color;
	
	if (is2byte) {
		_dropShadow = true;
		charPtr = g_scumm->get2byteCharPtr(chr);
		width = g_scumm->_2byteWidth;
		height = g_scumm->_2byteHeight;
		offsX = offsY = 0;
	} else {
		uint32 charOffs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
		assert(charOffs < 0x10000);
		if (!charOffs)
			return;
		charPtr = _fontPtr + charOffs;
		
		width = charPtr[0];
		height = charPtr[1];

		if (_disableOffsX) {
			offsX = 0;
		} else {
			offsX = (signed char)charPtr[2];
		}
	
		offsY = (signed char)charPtr[3];

		charPtr += 4;	// Skip over char header
	}
	origWidth = width;
	origHeight = height;
	
	if (_dropShadow) {
		width++;
		height++;
	}
	if (_firstChar) {
		_str.left = 0;
		_str.top = 0;
		_str.right = 0;
		_str.bottom = 0;
	}

	_top += offsY;
	_left += offsX;

	if (_left + origWidth > _right + 1 || _left < 0) {
		_left += origWidth;
		_top -= offsY;
		return;
	}

	_disableOffsX = false;

	if (_firstChar) {
		_str.left = _left;
		_str.top = _top;
		_str.right = _left;
		_str.bottom = _top;
		_firstChar = false;
	}

	if (_left < _str.left)
		_str.left = _left;

	if (_top < _str.top)
		_str.top = _top;

	int drawTop = _top - vs->topline;

	_vm->updateDirtyRect(vs->number, _left, _left + width, drawTop, drawTop + height + offsY, 0);

	if (vs->number != 0)
		_blitAlso = false;
	if (vs->number == 0 && !_ignoreCharsetMask)
		_hasMask = true;


	byte *mask = _vm->getMaskBuffer(_left, drawTop, 0);
	byte *dst = vs->screenPtr + vs->xstart + drawTop * _vm->_screenWidth + _left;

	byte *back = dst;
	if (_blitAlso) {
		dst = _vm->getResourceAddress(rtBuffer, vs->number + 5)
			+ vs->xstart + drawTop * _vm->_screenWidth + _left;
	}

	if (is2byte) {
		drawBits1(vs, dst, charPtr, mask, drawTop, origWidth, origHeight);
	} else {
		byte bpp = *_fontPtr;
		drawBitsN(vs, dst, charPtr, mask, bpp, drawTop, origWidth, origHeight);
	}

	if (_blitAlso) {
		int h = height;
		do {
			memcpy(back, dst, width);
			back += _vm->_screenWidth;
			dst += _vm->_screenWidth;
		} while (--h);
	}
	
	_left += origWidth;

	if (_str.right < _left) {
		_str.right = _left;
		if (_dropShadow)
			_str.right++;
	}

	if (_str.bottom < _top + height)
		_str.bottom = _top + height;

	_top -= offsY;
}

void CharsetRendererClassic::drawBitsN(VirtScreen *vs, byte *dst, const byte *src, byte *mask, byte bpp, int drawTop, int width, int height) {
	byte maskmask;
	int y, x;
	int maskpos;
	int color;
	byte numbits, bits;
	bool useMask = (vs->number == 0 && !_ignoreCharsetMask);

	assert(bpp == 1 || bpp == 2 || bpp == 4 || bpp == 8); 
	bits = *src++;
	numbits = 8;

	for (y = 0; y < height && y + drawTop < vs->height; y++) {
		maskmask = revBitMask[_left & 7];
		maskpos = 0;

		for (x = 0; x < width; x++) {
			color = (bits >> (8 - bpp)) & 0xFF;
			
			if (color && y + drawTop >= 0) {
				*dst = _vm->_charsetColorMap[color];
				if (useMask) {
					mask[maskpos] |= maskmask;
				}
			}
			dst++;
			bits <<= bpp;
			numbits -= bpp;
			if (numbits == 0) {
				bits = *src++;
				numbits = 8;
			}
			maskmask >>= 1;
			if (maskmask == 0) {
				maskmask = 0x80;
				maskpos++;
			}
		}
		dst += _vm->_screenWidth - width;
		mask += _vm->gdi._numStrips;
	}
}

void CharsetRendererCommon::drawBits1(VirtScreen *vs, byte *dst, const byte *src, byte *mask, int drawTop, int width, int height) {
	byte maskmask;
	int y, x;
	int maskpos;
	byte bits = 0;
	bool useMask = (vs->number == 0 && !_ignoreCharsetMask);

	for (y = 0; y < height && y + drawTop < vs->height; y++) {
		maskmask = revBitMask[_left & 7];
		maskpos = 0;

		for (x = 0; x < width; x++) {
			if ((x % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask[x % 8]) && y + drawTop >= 0) {
				if (_dropShadow) {
					*(dst + 1) = 0;
					*(dst + _vm->_screenWidth) = 0;
					*(dst + _vm->_screenWidth + 1) = 0;
				}					
				*dst = _vm->_charsetColorMap[1];
				if (useMask) {
					mask[maskpos] |= maskmask;
					if (_dropShadow) {
						mask[maskpos + _vm->gdi._numStrips] |= maskmask;
						if (maskmask == 1) {
							mask[maskpos + 1] |= 0x80;
							mask[maskpos + _vm->gdi._numStrips + 1] |= 0x80;
						} else {
							mask[maskpos] |= (maskmask >> 1);
							mask[maskpos + _vm->gdi._numStrips] |= (maskmask >> 1);
						}
					}
				}
			}
			dst++;
			maskmask >>= 1;
			if (maskmask == 0) {
				maskmask = 0x80;
				maskpos++;
			}
		}

		dst += _vm->_screenWidth - width;
		mask += _vm->gdi._numStrips;
	}
}

CharsetRendererNut::CharsetRendererNut(Scumm *vm)
	 : CharsetRenderer(vm) {
	_current = 0;

	for (int i = 0; i < 5; i++) {
		char fontname[256];
		sprintf(fontname, "font%d.nut", i);
		_fr[i] = new NutRenderer(_vm);
		if (!(_fr[i]->loadFont(fontname, _vm->getGameDataPath()))) {
			delete _fr[i];
			_fr[i] = NULL;
		}
	}
}

CharsetRendererNut::~CharsetRendererNut() {
	for (int i = 0; i < 5; i++)
		delete _fr[i];
}

void CharsetRendererNut::setCurID(byte id) {
	assert(id < 5);
	_curId = id;
	_current = _fr[id];
	assert(_current);
}

int CharsetRendererNut::getCharWidth(byte chr) {
	assert(_current);
	return _current->getCharWidth(chr);
}

int CharsetRendererNut::getFontHeight() {
	// FIXME / TODO: how to implement this properly???
	assert(_current);
	return _current->getCharHeight('|');
}

void CharsetRendererNut::printChar(int chr) {
	assert(_current);
	if (chr == '@')
		return;

	if (_firstChar) {
		_str.left = _left;
		_str.top = _top;
		_str.right = _left;
		_str.bottom = _top;
		_firstChar = false;
	}

	int width = _current->getCharWidth(chr);
	int height = _current->getCharHeight(chr);

	if (chr >= 256 && _vm->_CJKMode)
		width = 16;

	_hasMask = true;
	_current->drawShadowChar(chr, _left, _top, _color, !_ignoreCharsetMask);
	_vm->updateDirtyRect(0, _left, _left + width, _top, _top + height, 0);

	_left += width;
	if (_left > _str.right)
		_str.right = _left;

	if (_top + height > _str.bottom)
		_str.bottom = _top + height;
}

#ifdef __PALM_OS__
#include "scumm_globals.h" // init globals
void Charset_initGlobals()		{	
	GSETPTR(germanCharsetDataV2, GBVARS_GERMANCHARSETDATAV2_INDEX, byte, GBVARS_SCUMM)
	GSETPTR(frenchCharsetDataV2, GBVARS_FRENCHCHARSETDATAV2_INDEX, byte, GBVARS_SCUMM)
}
void Charset_releaseGlobals()	{
	GRELEASEPTR(GBVARS_GERMANCHARSETDATAV2_INDEX, GBVARS_SCUMM)
	GRELEASEPTR(GBVARS_FRENCHCHARSETDATAV2_INDEX, GBVARS_SCUMM)
}
#endif
