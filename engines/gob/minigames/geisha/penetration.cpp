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

#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/decfile.h"
#include "gob/cmpfile.h"
#include "gob/anifile.h"
#include "gob/aniobject.h"

#include "gob/sound/sound.h"

#include "gob/minigames/geisha/penetration.h"
#include "gob/minigames/geisha/meter.h"
#include "gob/minigames/geisha/mouth.h"

namespace Gob {

namespace Geisha {

static const byte kPalette[48] = {
	0x16,  0x16,  0x16,
	0x12,  0x14,  0x16,
	0x34,  0x00,  0x25,
	0x1D,  0x1F,  0x22,
	0x24,  0x27,  0x2A,
	0x2C,  0x0D,  0x22,
	0x2B,  0x2E,  0x32,
	0x12,  0x09,  0x20,
	0x3D,  0x3F,  0x00,
	0x3F,  0x3F,  0x3F,
	0x00,  0x00,  0x00,
	0x15,  0x15,  0x3F,
	0x25,  0x22,  0x2F,
	0x1A,  0x14,  0x28,
	0x3F,  0x00,  0x00,
	0x15,  0x3F,  0x15
};

static const int kColorShield = 11;
static const int kColorHealth = 15;
static const int kColorBlack  = 10;
static const int kColorFloor  = 13;

enum Sprite {
	kSpriteFloorShield = 25,
	kSpriteExit        = 29,
	kSpriteFloor       = 30,
	kSpriteWall        = 31,
	kSpriteMouthBite   = 32,
	kSpriteMouthKiss   = 33
};

enum Animation {
	kAnimationMouthKiss = 33,
	kAnimationMouthBite = 34
};

static const int kMapTileWidth  = 24;
static const int kMapTileHeight = 24;

static const int kPlayAreaX      = 120;
static const int kPlayAreaY      =   7;
static const int kPlayAreaWidth  = 192;
static const int kPlayAreaHeight = 113;

static const int kPlayAreaBorderWidth  = kPlayAreaWidth  / 2;
static const int kPlayAreaBorderHeight = kPlayAreaHeight / 2;

const byte Penetration::kMaps[kModeCount][kFloorCount][kMapWidth * kMapHeight] = {
	{
		{ // Real mode, floor 0
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,
			50, 50,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50, 50,
			50,  0,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0,  0, 50,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50,  0,  0, 50, 50, 50, 50,  0, 54, 55,  0,  0, 50,  0, 50,
			50,  0, 50, 49,  0, 50,  0, 52, 53,  0, 50, 50, 50,  0,  0,  0, 50,
			50, 57,  0, 50,  0,  0,  0, 50, 50, 50,  0,  0, 56, 50, 54, 55, 50,
			50, 50,  0,  0, 50, 50, 50,  0,  0,  0,  0, 50,  0,  0, 50,  0, 50,
			50, 51, 50,  0, 54, 55,  0,  0, 50, 50, 50, 50, 52, 53, 50,  0, 50,
			50,  0, 50,  0,  0,  0,  0,  0, 54, 55,  0,  0,  0, 50,  0,  0, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0,  0, 50,
			50, 50,  0, 52, 53,  0,  0,  0,  0,  0,  0, 52, 53,  0,  0, 50, 50,
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0
		},
		{ // Real mode, floor 1
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,
			50,  0, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0, 50, 51, 52, 53,  0,  0, 52, 53,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50,  0, 50, 50,  0, 50,  0, 50,  0, 50, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 52, 53,  0,  0,  0,  0,  0, 52, 53,  0, 52, 53, 50,
			50, 57, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0, 50, 52, 53,  0,  0, 52, 53,  0,  0,  0,  0,  0, 54, 55, 50,
			50,  0, 50,  0, 50,  0, 50, 50,  0, 50, 50,  0, 50,  0, 50, 50, 50,
			50,  0, 50, 49,  0,  0, 52, 53,  0, 52, 53,  0,  0,  0, 50, 56, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0,  0,  0,  0,  0,  0,  0, 54, 55,  0,  0,  0,  0,  0,  0, 50,
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0
		},
		{ // Real mode, floor 2
			 0, 50, 50, 50, 50, 50, 50, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0,
			50, 52, 53,  0,  0,  0,  0, 50, 50, 50,  0,  0,  0,  0, 52, 53, 50,
			50,  0, 50, 50, 50,  0,  0,  0, 50,  0,  0,  0, 50, 50, 50,  0, 50,
			50,  0, 50, 52, 53, 50, 50, 52, 53,  0, 50, 50, 54, 55, 50,  0, 50,
			50,  0, 50,  0,  0,  0,  0, 50,  0, 50,  0,  0,  0,  0, 50,  0, 50,
			50,  0,  0,  0, 50,  0,  0,  0, 50,  0,  0,  0, 50,  0, 52, 53, 50,
			 0, 50,  0, 50, 50, 50,  0, 57, 50, 51,  0, 50, 50, 50,  0, 50,  0,
			50,  0,  0,  0, 50,  0,  0,  0, 50,  0, 52, 53, 50,  0,  0,  0, 50,
			50,  0, 50,  0,  0,  0,  0, 50, 56, 50,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50, 54, 55, 50, 50,  0,  0,  0, 50, 50, 54, 55, 50,  0, 50,
			50,  0, 50, 50, 50,  0,  0,  0, 50,  0,  0,  0, 50, 50, 50,  0, 50,
			50, 52, 53,  0,  0,  0,  0, 50, 50, 50,  0,  0,  0,  0, 52, 53, 50,
			 0, 50, 50, 50, 50, 50, 50, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0
		}
	},
	{
		{ // Test mode, floor 0
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 56,  0, 50,  0,  0, 52, 53,  0,  0,  0,  0, 52, 53,  0, 51, 50,
			50,  0,  0, 50,  0,  0,  0, 50,  0, 54, 55, 50,  0, 50, 50, 50, 50,
			50, 52, 53, 50, 50,  0,  0, 50, 50, 50, 50, 50,  0, 50,  0,  0, 50,
			50,  0,  0,  0,  0, 56,  0,  0,  0,  0,  0, 50, 49, 50,  0,  0, 50,
			50,  0, 54, 55,  0, 50, 50, 54, 55,  0, 50, 50, 50,  0,  0,  0, 50,
			50,  0,  0,  0,  0,  0,  0,  0,  0,  0, 52, 53,  0,  0, 54, 55, 50,
			50,  0, 50,  0, 50,  0,  0, 50,  0,  0,  0, 50,  0,  0,  0,  0, 50,
			50,  0, 50,  0, 50, 54, 55, 50,  0, 50, 50, 50,  0, 50,  0,  0, 50,
			50, 50, 50, 50, 50,  0,  0, 50,  0,  0,  0,  0,  0, 50, 54, 55, 50,
			50,  0,  0,  0,  0,  0,  0,  0, 50, 50, 50, 50, 50,  0,  0,  0, 50,
			50, 57,  0, 52, 53,  0,  0,  0,  0, 54, 55,  0,  0,  0,  0, 56, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		},
		{ // Test mode, floor 1
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50,
			50,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 54, 55,  0, 50,
			50,  0, 50, 52, 53,  0,  0, 50,  0,  0, 54, 55, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0,  0, 52, 53,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50, 50, 50, 50, 50, 49, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0,  0, 50, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50, 51,  0,  0, 52, 53, 50,  0, 50,  0, 50,
			50, 57, 50,  0, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0, 50,  0, 50,
			50, 50, 50,  0, 50, 56,  0,  0,  0, 54, 55,  0,  0,  0, 50,  0, 50,
			50, 56,  0,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0, 50,
			50, 50, 50, 50,  0,  0,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		},
		{ // Test mode, floor 2
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 57, 50, 54, 55,  0, 50, 54, 55,  0, 50,  0, 52, 53, 50, 51, 50,
			50,  0, 50,  0, 50,  0, 50,  0,  0,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50, 52, 53,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0,  0,  0, 50,  0, 50,  0, 50,  0,  0,  0, 50,  0, 50,  0, 50,
			50,  0,  0,  0, 50, 52, 53,  0, 50, 52, 53, 56, 50,  0, 54, 55, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		}
	}
};


Penetration::Position::Position(uint16 pX, uint16 pY) : x(pX), y(pY) {
}


Penetration::ManagedMouth::ManagedMouth(uint16 pX, uint16 pY, MouthType t) :
	Position(pX, pY), mouth(0), type(t) {
}

Penetration::ManagedMouth::~ManagedMouth() {
	delete mouth;
}


Penetration::ManagedSub::ManagedSub(uint16 pX, uint16 pY) : Position(pX, pY), sub(0) {
	mapX = x * kMapTileWidth;
	mapY = y * kMapTileHeight;
}

Penetration::ManagedSub::~ManagedSub() {
	delete sub;
}


Penetration::Penetration(GobEngine *vm) : _vm(vm), _background(0), _sprites(0), _objects(0), _sub(0),
	_shieldMeter(0), _healthMeter(0), _floor(0) {

	_background = new Surface(320, 200, 1);

	_shieldMeter = new Meter(11, 119, 92, 3, kColorShield, kColorBlack, 920, Meter::kFillToRight);
	_healthMeter = new Meter(11, 137, 92, 3, kColorHealth, kColorBlack, 920, Meter::kFillToRight);

	_map = new Surface(kMapWidth  * kMapTileWidth  + kPlayAreaWidth ,
	                   kMapHeight * kMapTileHeight + kPlayAreaHeight, 1);
}

Penetration::~Penetration() {
	deinit();

	delete _map;

	delete _shieldMeter;
	delete _healthMeter;

	delete _background;
}

bool Penetration::play(bool hasAccessPass, bool hasMaxEnergy, bool testMode) {
	_hasAccessPass = hasAccessPass;
	_hasMaxEnergy  = hasMaxEnergy;
	_testMode      = testMode;

	init();
	initScreen();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	while (!_vm->shouldQuit() && !isDead() && !hasWon()) {
		updateAnims();

		// Draw and wait for the end of the frame
		_vm->_draw->blitInvalidated();
		_vm->_util->waitEndFrame();

		// Handle input
		_vm->_util->processInput();

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		// Aborting the game
		if (key == kKeyEscape)
			break;

		// Handle the sub movement
		handleSub(key);
	}

	deinit();

	return hasWon();
}

void Penetration::init() {
	// Load sounds
	_vm->_sound->sampleLoad(&_soundShield, SOUND_SND, "boucl.snd");
	_vm->_sound->sampleLoad(&_soundBite  , SOUND_SND, "pervet.snd");
	_vm->_sound->sampleLoad(&_soundKiss  , SOUND_SND, "baise.snd");
	_vm->_sound->sampleLoad(&_soundShoot , SOUND_SND, "tirgim.snd");

	_background->clear();

	_vm->_video->drawPackedSprite("hyprmef2.cmp", *_background);

	_sprites = new CMPFile(_vm, "tcifplai.cmp", 320, 200);
	_objects = new ANIFile(_vm, "tcite.ani", 320);

	// The shield starts down
	_shieldMeter->setValue(0);

	// If we don't have the max energy tokens, the health starts at 1/3 strength
	if (_hasMaxEnergy)
		_healthMeter->setMaxValue();
	else
		_healthMeter->setValue(_healthMeter->getMaxValue() / 3);

	_floor = 0;

	createMap();

	for (Common::List<ManagedMouth>::iterator m = _mouths.begin(); m != _mouths.end(); m++)
		_mapAnims.push_back(m->mouth);

	_anims.push_back(_sub->sub);
}

void Penetration::deinit() {
	_soundShield.free();
	_soundBite.free();
	_soundKiss.free();
	_soundShoot.free();

	clearMap();

	delete _objects;
	delete _sprites;

	_objects = 0;
	_sprites = 0;
}

void Penetration::clearMap() {
	_mapAnims.clear();
	_anims.clear();

	_exits.clear();
	_shields.clear();
	_mouths.clear();

	delete _sub;

	_sub = 0;

	_map->fill(kColorBlack);
}

void Penetration::createMap() {
	if (_floor >= kFloorCount)
		error("Geisha: Invalid floor %d in minigame penetration", _floor);

	clearMap();

	const byte *mapTiles = kMaps[_testMode ? 1 : 0][_floor];

	bool exitWorks;

	// Draw the map tiles
	for (int y = 0; y < kMapHeight; y++) {
		for (int x = 0; x < kMapWidth; x++) {
			const byte mapTile = mapTiles[y * kMapWidth + x];

			bool *walkMap = _walkMap + (y * kMapWidth + x);

			const int posX = kPlayAreaBorderWidth  + x * kMapTileWidth;
			const int posY = kPlayAreaBorderHeight + y * kMapTileHeight;

			*walkMap = true;

			switch (mapTile) {
			case 0: // Floor
				_sprites->draw(*_map, kSpriteFloor, posX, posY);
				break;

			case 49: // Emergency exit (needs access pass)

				exitWorks = _hasAccessPass;
				if (exitWorks) {
					_exits.push_back(Position(x, y));
					_sprites->draw(*_map, kSpriteExit, posX, posY);
				} else {
					_sprites->draw(*_map, kSpriteWall, posX, posY);
					*walkMap = false;
				}

				break;

			case 50: // Wall
				_sprites->draw(*_map, kSpriteWall, posX, posY);
				*walkMap = false;
				break;

			case 51: // Regular exit

				// A regular exit works always in test mode.
				// But if we're in real mode, and on the last floor, it needs an access pass
				exitWorks = _testMode || (_floor < 2) || _hasAccessPass;

				if (exitWorks) {
					_exits.push_back(Position(x, y));
					_sprites->draw(*_map, kSpriteExit, posX, posY);
				} else {
					_sprites->draw(*_map, kSpriteWall, posX, posY);
					*walkMap = false;
				}

				break;

			case 52: // Left side of biting mouth
				_mouths.push_back(ManagedMouth(x, y, kMouthTypeBite));

				_mouths.back().mouth =
					new Mouth(*_objects, *_sprites, kAnimationMouthBite, kSpriteMouthBite, kSpriteFloor);

				_mouths.back().mouth->setPosition(posX, posY);
				break;

			case 53: // Right side of biting mouth
				break;

			case 54: // Left side of kissing mouth
				_mouths.push_back(ManagedMouth(x, y, kMouthTypeKiss));

				_mouths.back().mouth =
					new Mouth(*_objects, *_sprites, kAnimationMouthKiss, kSpriteMouthKiss, kSpriteFloor);

				_mouths.back().mouth->setPosition(posX, posY);
				break;

			case 55: // Right side of kissing mouth
				break;

			case 56: // Shield lying on the floor
				_sprites->draw(*_map, kSpriteFloor      , posX    , posY    ); // Floor
				_sprites->draw(*_map, kSpriteFloorShield, posX + 4, posY + 8); // Shield

				_map->fillRect(posX +  4, posY + 8, posX +  7, posY + 18, kColorFloor); // Area left to shield
				_map->fillRect(posX + 17, posY + 8, posX + 20, posY + 18, kColorFloor); // Area right to shield

				_shields.push_back(Position(x, y));
				break;

			case 57: // Start position
				_sprites->draw(*_map, kSpriteFloor, posX, posY);

				delete _sub;

				_sub = new ManagedSub(x, y);

				_sub->sub = new Submarine(*_objects);
				_sub->sub->setPosition(kPlayAreaX + kPlayAreaBorderWidth, kPlayAreaY + kPlayAreaBorderHeight);
				break;
			}
		}
	}

	if (!_sub)
		error("Geisha: No starting position in floor %d (testmode: %d", _floor, _testMode);
}

void Penetration::initScreen() {
	_vm->_util->setFrameRate(15);

	memcpy(_vm->_draw->_vgaPalette     , kPalette, 48);
	memcpy(_vm->_draw->_vgaSmallPalette, kPalette, 48);

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	// Draw the shield meter
	_sprites->draw(*_background,   0,   0,  95,   6, 9, 117, 0); // Meter frame
	_sprites->draw(*_background, 271, 176, 282, 183, 9, 108, 0); // Shield

	// Draw the health meter
	_sprites->draw(*_background,   0,   0,  95,   6, 9, 135, 0); // Meter frame
	_sprites->draw(*_background, 283, 176, 292, 184, 9, 126, 0); // Heart

	_vm->_draw->_backSurface->blit(*_background);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
}

int16 Penetration::checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons) {
	_vm->_util->getMouseState(&mouseX, &mouseY, &mouseButtons);

	return _vm->_util->checkKey();
}

bool Penetration::isWalkable(int16 x, int16 y) const {
	if ((x < 0) || (x >= kMapWidth) || (y < 0) || (y >= kMapHeight))
		return false;

	return _walkMap[y * kMapWidth + x];
}

void Penetration::handleSub(int16 key) {
	if      (key == kKeyLeft)
		subMove(-5,  0, Submarine::kDirectionW);
	else if (key == kKeyRight)
		subMove( 5,  0, Submarine::kDirectionE);
	else if (key == kKeyUp)
		subMove( 0, -5, Submarine::kDirectionN);
	else if (key == kKeyDown)
		subMove( 0,  5, Submarine::kDirectionS);
	else if (key == kKeySpace)
		subShoot();
}

void Penetration::subMove(int x, int y, Submarine::Direction direction) {
	if (!_sub->sub->canMove())
		return;

	// Limit the movement to walkable tiles

	int16 minX = 0;
	if (!isWalkable(_sub->x - 1, _sub->y))
		minX = _sub->x * kMapTileWidth;

	int16 maxX = kMapWidth * kMapTileWidth;
	if (!isWalkable(_sub->x + 1, _sub->y))
		maxX = _sub->x * kMapTileWidth;

	int16 minY = 0;
	if (!isWalkable(_sub->x, _sub->y - 1))
		minY = _sub->y * kMapTileHeight;

	int16 maxY = kMapHeight * kMapTileHeight;
	if (!isWalkable(_sub->x, _sub->y + 1))
		maxY = _sub->y * kMapTileHeight;

	_sub->mapX = CLIP<int16>(_sub->mapX + x, minX, maxX);
	_sub->mapY = CLIP<int16>(_sub->mapY + y, minY, maxY);

	// The tile the sub is on is where its mid-point is
	_sub->x = (_sub->mapX + (kMapTileWidth  / 2)) / kMapTileWidth;
	_sub->y = (_sub->mapY + (kMapTileHeight / 2)) / kMapTileHeight;

	_sub->sub->turn(direction);

	checkShields();
	checkMouths();
}

void Penetration::subShoot() {
	if (!_sub->sub->canMove())
		return;

	_sub->sub->shoot();

	_vm->_sound->blasterPlay(&_soundShoot, 1, 0);
}

void Penetration::checkShields() {
	for (Common::List<Position>::iterator pos = _shields.begin(); pos != _shields.end(); ++pos) {
		if ((pos->x == _sub->x) && (pos->y == _sub->y)) {
			// Charge shields
			_shieldMeter->setMaxValue();

			// Play the shield sound
			_vm->_sound->blasterPlay(&_soundShield, 1, 0);

			// Erase the shield from the map
			const int mapX = kPlayAreaBorderWidth  + pos->x * kMapTileWidth;
			const int mapY = kPlayAreaBorderHeight + pos->y * kMapTileHeight;
			_sprites->draw(*_map, 30, mapX, mapY);

			_shields.erase(pos);
			break;
		}
	}
}

void Penetration::checkMouths() {
	for (Common::List<ManagedMouth>::iterator m = _mouths.begin(); m != _mouths.end(); ++m) {
		if (!m->mouth->isDeactivated())
			continue;

		if ((( m->x      == _sub->x) && (m->y == _sub->y)) ||
		    (((m->x + 1) == _sub->x) && (m->y == _sub->y))) {

			m->mouth->activate();

			// Play the mouth sound and do health gain/loss
			if        (m->type == kMouthTypeBite) {
				_vm->_sound->blasterPlay(&_soundBite, 1, 0);
				healthLose(230);
			} else if (m->type == kMouthTypeKiss) {
				_vm->_sound->blasterPlay(&_soundKiss, 1, 0);
				healthGain(120);
			}
		}
	}
}

void Penetration::healthGain(int amount) {
	if (_shieldMeter->getValue() > 0)
		_healthMeter->increase(_shieldMeter->increase(amount));
	else
		_healthMeter->increase(amount);
}

void Penetration::healthLose(int amount) {
	_healthMeter->decrease(_shieldMeter->decrease(amount));

	if (_healthMeter->getValue() == 0)
		_sub->sub->die();
}

bool Penetration::isDead() const {
	return _sub && _sub->sub->isDead();
}

bool Penetration::hasWon() const {
	return _floor > kFloorCount;
}

void Penetration::updateAnims() {
	int16 left = 0, top = 0, right = 0, bottom = 0;

	// Clear the previous map animation frames
	for (Common::List<ANIObject *>::iterator a = _mapAnims.reverse_begin();
			 a != _mapAnims.end(); --a) {

		(*a)->clear(*_map, left, top, right, bottom);
	}

	// Draw the current map animation frames
	for (Common::List<ANIObject *>::iterator a = _mapAnims.begin();
			 a != _mapAnims.end(); ++a) {

		(*a)->draw(*_map, left, top, right, bottom);
		(*a)->advance();
	}

	// Clear the previous animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.reverse_begin();
			 a != _anims.end(); --a) {

		if ((*a)->clear(*_vm->_draw->_backSurface, left, top, right, bottom))
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	}

	if (_sub) {
		// Draw the map

		_vm->_draw->_backSurface->blit(*_map, _sub->mapX, _sub->mapY,
				_sub->mapX + kPlayAreaWidth - 1, _sub->mapY + kPlayAreaHeight - 1, kPlayAreaX, kPlayAreaY);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, kPlayAreaX, kPlayAreaY,
				kPlayAreaX + kPlayAreaWidth - 1, kPlayAreaY + kPlayAreaHeight - 1);
	}

	// Draw the current animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.begin();
			 a != _anims.end(); ++a) {

		if ((*a)->draw(*_vm->_draw->_backSurface, left, top, right, bottom))
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

		(*a)->advance();
	}

	// Draw the meters
	_shieldMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

	_healthMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
}

} // End of namespace Geisha

} // End of namespace Gob
