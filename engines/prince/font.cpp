/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/archive.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/str.h"

#include "graphics/surface.h"

#include "prince/font.h"

namespace Prince {

Font::Font() {
}

Font::~Font() {
	delete [] _fontData;
}

bool Font::loadFromStream(Common::SeekableReadStream &stream) {
	stream.seek(0);
	_fontData = new byte[stream.size()];
	stream.read(_fontData, stream.size());
	return true;
}

int Font::getFontHeight() const {
	return _fontData[5];
}

int Font::getMaxCharWidth() const {
	return 0;
}

Font::ChrData Font::getChrData(byte chr) const {
	chr -= 32;
	uint16 chrOffset = 4*chr+6;

	ChrData chrData;
	chrData._width = _fontData[chrOffset+2];
	chrData._height = _fontData[chrOffset+3];
	chrData._pixels = _fontData + READ_LE_UINT16(_fontData + chrOffset);

	return chrData;
}

int Font::getCharWidth(byte chr) const {
	return getChrData(chr)._width;
}

void Font::drawChar(Graphics::Surface *dst, byte chr, int posX, int posY, uint32 color) const {
	const ChrData chrData = getChrData(chr);

	for (int y = 0; y < chrData._height; ++y) {
		for (int x = 0; x < chrData._width; ++x) {
			byte d = chrData._pixels[x + (chrData._width * y)];
			if (d == 0) d = 255;
			else if (d == 1) d = 0;
			else if (d == 2) d = color;
			else if (d == 3) d = 0;
			if (d != 255) {
				*(byte*)dst->getBasePtr(posX + x, posY + y) = d;
			}
		}
	}
}

}

/* vim: set tabstop=4 expandtab!: */
