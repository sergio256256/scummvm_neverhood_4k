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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "titanic/string.h"

namespace Titanic {

CString CString::left(uint count) const {
	return (count > size()) ? CString() : CString(c_str(), c_str() + count);
}

CString CString::right(uint count) const {
	int strSize = size();
	return (count > strSize) ? CString() : 
		CString(c_str() + strSize - count, c_str() + strSize);
}

CString CString::mid(uint start, uint count) const {
	if (start >= size())
		return CString();
	else
		return CString(c_str() + start, MIN(count, size() - start));
}

CString CString::mid(uint start) const {
	uint strSize = size();
	assert(start <= strSize);
	return mid(start, strSize - start);
}

int CString::indexOf(char c) const {
	const char *charP = strchr(c_str(), c);
	return charP ? charP - c_str() : -1;
}

int CString::lastIndexOf(char c) const {
	const char *charP = strrchr(c_str(), c);
	return charP ? charP - c_str() : -1;
}

FileType CString::fileTypeSuffix() const {
	CString ext = right(1);
	if (ext == "0" || ext == "4")
		return FILETYPE_IMAGE;
	else if (ext == "1")
		return FILETYPE_WAV;
	else if (ext == "2" || ext == "3")
		return FILETYPE_MOVIE;
	
	ext = right(3);
	if (ext == "tga" || ext == "jpg")
		return FILETYPE_IMAGE;
	else if (ext == "wav")
		return FILETYPE_WAV;
	else if (ext == "avi" || ext == "mov")
		return FILETYPE_MOVIE;
	else if (ext == "dlg")
		return FILETYPE_DLG;
	else
		return FILETYPE_UNKNOWN;
}

ImageType CString::imageTypeSuffix() const {
	CString ext = right(1);
	if (ext == "0")
		return IMAGETYPE_TARGA;
	else if (ext == "4")
		return IMAGETYPE_JPEG;

	ext = right(3);
	if (ext == "tga")
		return IMAGETYPE_TARGA;
	else if (ext == "jpg")
		return IMAGETYPE_JPEG;
	else
		return IMAGETYPE_UNKNOWN;
}

} // End of namespace Titanic
