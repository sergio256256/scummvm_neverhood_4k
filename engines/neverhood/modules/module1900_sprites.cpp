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

#include "neverhood/modules/module1900.h"
#include "neverhood/modules/module1900_sprites.h"

namespace Neverhood {

static const NPoint kAsScene1907SymbolGroundPositions[] = {
	{UPSCALE(160, 310)}, {UPSCALE( 90, 340)}, {UPSCALE(210, 335)},
	{UPSCALE(210, 380)}, {UPSCALE(310, 340)}, {UPSCALE(290, 400)},
	{UPSCALE(400, 375)}, {UPSCALE(370, 435)}, {UPSCALE(475, 415)}
};

static const NPoint kAsScene1907SymbolPluggedInPositions[] = {
	{UPSCALE(275, 125)}, {UPSCALE(244, 125)}, {UPSCALE(238, 131)},
	{UPSCALE(221, 135)}, {UPSCALE(199, 136)}, {UPSCALE(168, 149)},
	{UPSCALE(145, 152)}, {UPSCALE(123, 154)}, {UPSCALE(103, 157)}
};

static const NPoint kAsScene1907SymbolGroundHitPositions[] = {
	{UPSCALE(275, 299)}, {UPSCALE(244, 299)}, {UPSCALE(238, 305)},
	{UPSCALE(221, 309)}, {UPSCALE(199, 310)}, {UPSCALE(168, 323)},
	{UPSCALE(145, 326)}, {UPSCALE(123, 328)}, {UPSCALE(103, 331)}
};

static const NPoint kAsScene1907SymbolPluggedInDownPositions[] = {
	{UPSCALE(275, 136)}, {UPSCALE(244, 156)}, {UPSCALE(238, 183)},
	{UPSCALE(221, 207)}, {UPSCALE(199, 228)}, {UPSCALE(168, 262)},
	{UPSCALE(145, 285)}, {UPSCALE(123, 307)}, {UPSCALE(103, 331)}
};

static const uint32 kAsScene1907SymbolFileHashes[] = {
	0x006A1034, 0x006A1010, 0x006A1814,
	0x006A1016, 0x006A0014, 0x002A1014,
	0x00EA1014, 0x206A1014, 0x046A1414
};

bool AsScene1907Symbol::_plugInFailed = false;
int AsScene1907Symbol::_plugInTryCount = 0;

AsScene1907Symbol::AsScene1907Symbol(NeverhoodEngine *vm, Scene1907 *parentScene, int elementIndex, int positionIndex)
	: AnimatedSprite(vm, 1000 - positionIndex), _parentScene(parentScene), _elementIndex(elementIndex), _isMoving(false) {

	_plugInFailed = false;
	_plugInTryCount = 0;

	if (getGlobalVar(V_STAIRS_PUZZLE_SOLVED)) {
		_isPluggedIn = true;
		_currPositionIndex = elementIndex;
		if (!getGlobalVar(V_STAIRS_DOWN)) {
			_x = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].x;
			_y = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].y;
		} else {
			_x = kAsScene1907SymbolPluggedInDownPositions[_currPositionIndex].x;
			_y = kAsScene1907SymbolPluggedInDownPositions[_currPositionIndex].y;
		}
		createSurface1(kAsScene1907SymbolFileHashes[_elementIndex], 1000 + _currPositionIndex);
		startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	} else {
		_isPluggedIn = false;
		_currPositionIndex = positionIndex;
		loadSound(0, 0x74231924);
		loadSound(1, 0x36691914);
		loadSound(2, 0x5421D806);
		_parentScene->setPositionFree(_currPositionIndex, false);
		_x = kAsScene1907SymbolGroundPositions[_currPositionIndex].x;
		_y = kAsScene1907SymbolGroundPositions[_currPositionIndex].y;
		createSurface1(kAsScene1907SymbolFileHashes[_elementIndex], 1000 + _currPositionIndex);
		startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
		_newStickFrameIndex = 0;
	}
	_collisionBoundsOffset.set(UPSCALE(0, 0), UPSCALE(80, 80));
	Sprite::updateBounds();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);

}

void AsScene1907Symbol::update() {
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
	if (_plugInFailed && _plugInTryCount == 0)
		_plugInFailed = false;
}

uint32 AsScene1907Symbol::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isPluggedIn && !_plugInFailed) {
			tryToPlugIn();
			messageResult = 1;
		} else
			messageResult = 0;
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene1907Symbol::hmTryToPlugIn(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1907Symbol::suTryToPlugIn() {
	_currStep++;
	_x -= _deltaX;
	_y -= _deltaY;
	if (_currStep == 16) {
		_x -= _smallDeltaX;
		_y -= _smallDeltaY;
		SetSpriteUpdate(nullptr);
	}
}

void AsScene1907Symbol::suFallOff() {
	if (_fallOffDelay != 0) {
		_fallOffDelay--;
	} else {
		_y += _yAccel;
		_yAccel += UPSCALE_Y(8);
		if (_y >= kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y) {
			_y = kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y;
			stFallOffHitGround();
		}
	}
}

void AsScene1907Symbol::suFallOffHitGround() {

	if (_x == _someX - _xBreak)
		_x -= _smallDeltaX;
	else
		_x -= _deltaX;

	if (_y == kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y) {
		_y -= _someY;
	}

	if (_currStep < 8) {
		_y -= _yAccel;
		_yAccel -= UPSCALE_Y(4);
		if (_yAccel < UPSCALE_Y(0))
			_yAccel = UPSCALE_Y(0);
	} else if (_currStep < 15) {
		_y += _yAccel;
		_yAccel += UPSCALE_Y(4);
	} else {
		_y = kAsScene1907SymbolGroundPositions[_newPositionIndex].y;
		cbFallOffHitGroundEvent();
	}

	_currStep++;
}

void AsScene1907Symbol::suMoveDown() {
	_y += _yIncr;
	if (_yIncr < UPSCALE_Y(11))
		_yIncr += UPSCALE_Y(1);
	if (_y >= kAsScene1907SymbolPluggedInDownPositions[_elementIndex].y) {
		_y = kAsScene1907SymbolPluggedInDownPositions[_elementIndex].y;
		_isMoving = false;
		SetSpriteUpdate(nullptr);
	}
}

void AsScene1907Symbol::suMoveUp() {
	_y -= _yIncr;
	if (getGlobalVar(V_WALL_BROKEN)) {
		if (_y - (UPSCALE_Y(9) + (_elementIndex > 5 ? UPSCALE_Y(31) : 0)) < kAsScene1907SymbolPluggedInPositions[_elementIndex].y)
			_yIncr -= UPSCALE_Y(1);
		else
			_yIncr += UPSCALE_Y(1);
	} else
		_yIncr = UPSCALE_Y(2);
	if (_yIncr > UPSCALE_Y(9))
		_yIncr = UPSCALE_Y(9);
	else if (_yIncr < UPSCALE_Y(1))
		_yIncr = UPSCALE_Y(1);
	if (_y < kAsScene1907SymbolPluggedInPositions[_elementIndex].y) {
		_y = kAsScene1907SymbolPluggedInPositions[_elementIndex].y;
		_isMoving = false;
		SetSpriteUpdate(nullptr);
	}
}

void AsScene1907Symbol::tryToPlugIn() {
	_isPluggedIn = true;
	_plugInTryCount++;
	_newPositionIndex = _parentScene->getNextPosition();
	_parentScene->setPositionFree(_currPositionIndex, true);
	sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1100 + _newPositionIndex);
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
	SetUpdateHandler(&AsScene1907Symbol::update);
	SetMessageHandler(&AsScene1907Symbol::hmTryToPlugIn);
	SetSpriteUpdate(&AsScene1907Symbol::suTryToPlugIn);
	_currStep = 0;
	_deltaX = (_x - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].x) / 16;
	_smallDeltaX = _x - _deltaX * 16 - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].x;
	_deltaY = (_y - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].y) / 16;
	_smallDeltaY = _y - _deltaY * 16 - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].y;
	if (_elementIndex == _newPositionIndex) {
		NextState(&AsScene1907Symbol::stPlugIn);
	} else {
		_plugInFailed = true;
		NextState(&AsScene1907Symbol::stPlugInFail);
	}
}

void AsScene1907Symbol::fallOff(int newPositionIndex, int fallOffDelay) {
	_isPluggedIn = false;
	_newPositionIndex = newPositionIndex;
	_fallOffDelay = fallOffDelay;
	_parentScene->setPositionFree(_newPositionIndex, false);
	_x = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].x;
	_y = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].y;
	_someX = _x;
	_someY = _y;
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, 0);
	_playBackwards = true;
	_newStickFrameIndex = STICK_LAST_FRAME;
	_currStep = 0;
	_yAccel = UPSCALE_Y(1);
	SetUpdateHandler(&AsScene1907Symbol::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(&AsScene1907Symbol::suFallOff);
}

void AsScene1907Symbol::stFallOffHitGround() {
	playSound(1);
	sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1000 + _newPositionIndex);
	Entity::_priority = 1000 - _newPositionIndex;
	_parentScene->removeCollisionSprite(this);
	_parentScene->addCollisionSprite(this);
	SetSpriteUpdate(&AsScene1907Symbol::suFallOffHitGround);
	NextState(&AsScene1907Symbol::cbFallOffHitGroundEvent);
	_newStickFrameIndex = 0;
	_currStep = 0;
	_yAccel = UPSCALE_Y(30);
	_deltaX = (_x - kAsScene1907SymbolGroundPositions[_newPositionIndex].x) / 15;
	_xBreak = _deltaX * 15;
	_smallDeltaX = _x - kAsScene1907SymbolGroundPositions[_newPositionIndex].x - _xBreak;
	_someY = 0;
	if (kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y > kAsScene1907SymbolGroundPositions[_newPositionIndex].y)
		_someY = kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y - kAsScene1907SymbolGroundPositions[_newPositionIndex].y;
}

void AsScene1907Symbol::cbFallOffHitGroundEvent() {
	_currPositionIndex = _newPositionIndex;
	if (_plugInTryCount > 0)
		_plugInTryCount--;
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(nullptr);
	updateBounds();
	playSound(2);
}

void AsScene1907Symbol::stPlugIn() {
	playSound(0);
	_currPositionIndex = _newPositionIndex;
	stopAnimation();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(nullptr);
	if (_elementIndex == 8)
		sendMessage(_parentScene, 0x2001, 0);
}

void AsScene1907Symbol::stPlugInFail() {
	_currPositionIndex = _newPositionIndex;
	stopAnimation();
	_parentScene->plugInFailed();
}

void AsScene1907Symbol::moveUp() {
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);
	stopAnimation();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(&AsScene1907Symbol::suMoveUp);
	_yIncr = UPSCALE_Y(1);
	_isMoving = true;
}

void AsScene1907Symbol::moveDown() {
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);
	stopAnimation();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(&AsScene1907Symbol::suMoveDown);
	_yIncr = UPSCALE_Y(4);
	_isMoving = true;
}

SsScene1907UpDownButton::SsScene1907UpDownButton(NeverhoodEngine *vm, Scene1907 *parentScene, AsScene1907Symbol *asScene1907Symbol)
	: StaticSprite(vm, 1400), _parentScene(parentScene), _asScene1907Symbol(asScene1907Symbol),
	_countdown1(0) {

	loadSprite(0x64516424, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 1400);
	setVisible(false);
	loadSound(0, 0x44061000);
	SetUpdateHandler(&SsScene1907UpDownButton::update);
	SetMessageHandler(&SsScene1907UpDownButton::handleMessage);
	if (getGlobalVar(V_STAIRS_PUZZLE_SOLVED)) {
		if (getGlobalVar(V_STAIRS_DOWN))
			setToDownPosition();
		else
			setToUpPosition();
	}
}

void SsScene1907UpDownButton::update() {
	updatePosition();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		setVisible(false);
		sendMessage(_parentScene, 0x2000, 0);
	}
}

uint32 SsScene1907UpDownButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown1 == 0 && !_asScene1907Symbol->isMoving() && getGlobalVar(V_STAIRS_PUZZLE_SOLVED)) {
			setVisible(true);
			_countdown1 = 4;
			updatePosition();
			playSound(0);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

void SsScene1907UpDownButton::setToUpPosition() {
	_y = _spriteResource.getPosition().y;
	updateBounds();
	updatePosition();
}

void SsScene1907UpDownButton::setToDownPosition() {
	_y = _spriteResource.getPosition().y + 174;
	updateBounds();
	updatePosition();
}

AsScene1907WaterHint::AsScene1907WaterHint(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1400) {

	createSurface1(0x110A1061, 1500);
	_x = UPSCALE_X(320);
	_y = UPSCALE_Y(240);
	startAnimation(0x110A1061, 0, -1);
	_newStickFrameIndex = 0;
	setVisible(false);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
	SetUpdateHandler(&AsScene1907WaterHint::update);
	SetMessageHandler(&Sprite::handleMessage);
}

void AsScene1907WaterHint::update() {
	updateAnim();
	updatePosition();
}

uint32 AsScene1907WaterHint::hmShowing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1907WaterHint::show() {
	setVisible(true);
	startAnimation(0x110A1061, 0, -1);
	SetMessageHandler(&AsScene1907WaterHint::hmShowing);
	NextState(&AsScene1907WaterHint::hide);
}

void AsScene1907WaterHint::hide() {
	stopAnimation();
	setVisible(false);
	SetMessageHandler(&Sprite::handleMessage);
}

KmScene1901::KmScene1901(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1901::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x482D:
		setDoDeltaX(DOWNSCALE_X(_x) > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x483F:
		startSpecialWalkRight(UPSCALE_X(param.asInteger()));
		break;
	case 0x4840:
		startSpecialWalkLeft(UPSCALE_X(param.asInteger()));
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
