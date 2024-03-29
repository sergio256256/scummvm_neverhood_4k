/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NEVERHOOD_MICROTILES_H
#define NEVERHOOD_MICROTILES_H

#include "common/scummsys.h"
#include "common/list.h"
#include "common/util.h"
#include "common/rect.h"
#include "neverhood.h"

namespace Neverhood {

typedef uint32 BoundingBox;

const BoundingBox FullBoundingBox  = 0x00001F1F;
const BoundingBox EmptyBoundingBox = 0x00000000;
const int TileSize = UPSCALE_X(32);

typedef Common::List<Common::Rect> RectangleList;

class MicroTileArray {
public:
	MicroTileArray(int16 width, int16 height);
	~MicroTileArray();
	void addRect(Common::Rect r);
	void clear();
	RectangleList *getRectangles();
protected:
	BoundingBox *_tiles;
	int16 _tilesW, _tilesH;
	byte TileX0(const BoundingBox &boundingBox);
	byte TileY0(const BoundingBox &boundingBox);
	byte TileX1(const BoundingBox &boundingBox);
	byte TileY1(const BoundingBox &boundingBox);
	bool isBoundingBoxEmpty(const BoundingBox &boundingBox);
	bool isBoundingBoxFull(const BoundingBox &boundingBox);
	void setBoundingBox(BoundingBox &boundingBox, byte x0, byte y0, byte x1, byte y1);
	void updateBoundingBox(BoundingBox &boundingBox, byte x0, byte y0, byte x1, byte y1);
};

} // namespace Neverhood

#endif // NEVERHOOD_MICROTILES_H
