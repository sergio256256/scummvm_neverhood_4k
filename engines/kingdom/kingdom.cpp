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

#include "common/scummsys.h"
#include "common/translation.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "common/system.h"
#include "image/iff.h"
#include "engines/util.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/events.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "kingdom/kingdom.h"

namespace Kingdom {

KingdomGame::KingdomGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_console = nullptr;
	_rnd = new Common::RandomSource("kingdom");

	DebugMan.addDebugChannel(kDebugGeneral, "general", "General debug level");

	_asPtr = nullptr;
	_quit = false;
	_kingartEntries = nullptr;

	_tickCount = 0;
	_oldTime = g_system->getMillis();

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "MAPS");
	SearchMan.addSubDirectoryMatching(gameDataDir, "PICS");
	SearchMan.addSubDirectoryMatching(gameDataDir, "SOUNDS");
	SearchMan.addSubDirectoryMatching(gameDataDir, "SOUNDM");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MOVIELF");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MOVIES");

	initVariables();
}

void KingdomGame::initVariables() {
	_tsIconOnly = false;
	_treeRightSta = 0;
	_treeRightPic = 0;
	_treeLeftSta = 0;
	_treeLeftPic = 0;
	_treeHGUPic = 0;
	_treeHGTimer = 0;
	_treeHGSta = 0;
	_treeHGPic = 0;
	_treeEyeTimer = 0;
	_treeEyeSta = 0;
	_treeEyePic = 0;
	_noIFScreen = false;
	_iconSel = 0;
	_iconSelect = 0;
	_iconsClosed = false;
	for (int i = 0; i < 7; i++)
		_iconPic[i] = 0;

	_iconRedraw = 0;
	_healthTmr = 0;
	_healthOld = 0;
	_health = 0;
	_palStepFlag = 0;
	_skylarTimerFlag = 0;
	_skylarTimer = 0;
	_cTimerFlag = 0;
	_cTimer = 0;
	_bTimerFlag = 0;
	_bTimer = 0;
	_aTimerFlag = 0;
	_aTimer = 0;
	_zoom = 0;
	_wizard = 0; 
	_userInput = 0;
	_tideCntl = 0;
//	_track = 0;
//	_statMap = 0;
	_statPlay = 0;
	_spell3 = 0;
	_spell2 = 0;
	_spell1 = 0;
	_soundNumber = 0;
	_robberyNode = 0;
	_rtnNode = 0;
	_replay = false;
	_resurrect = false;
//	_quitFlag = 0;
	_pouch = false;
	_pMovie = 0;
//	_options = 0;
	_oldPouch = false;
	_oldHelp = false;
	_oldEye = false;
	_oldTRS = 0;
	_oldTLS = 0;
	_oldStatPlay = 0;
	_oldNode = 0;
	_oldIconsClosed = false;
	_fstFwd = false;
	_noMusic = false;
	_nodeNum = 0;
	_nextNode = 0;
	_mouseValue = 0;
	_mapEx = 0;
	_mapStat = 0;
	_loopFlag = false;
	_lastSound = 0;
	_lastObs = 0;
	_lastObstacle = 0;
	_keyActive = false;
	_help = false;
	_itemInhibit = false;
//	_iconW = 0;
	_gameMode = 0;
	_fullScreen = false;
	_frameStop = 0;
	_eye = false;
	_daelonCntr = 0;
	_sound = 0;
	_asMode = false;
	for (int i = 0; i < 99; i++)
		_nodes[i] = 0;
	for (int i = 0; i < 510; i++) {
		_rezPointers[i] = nullptr;
		_rezSize[i] = 0;
	}
	_mouseDebound = false;
	_mouseButton = 0;
//	_cursorActive = false;
	_cursorDrawn = false;
	_oldCursorY = 0;
	_oldCursorX = 0;
//	_cursorShape = nullptr;
	_oldCursorDef = 0;
	_cursorDef = 0;
	_cursorY = 0;
	_cursorX = 0;
	_currMap = 0;
	_asMap = 0;
}

KingdomGame::~KingdomGame() {
	delete _console;
	delete _rnd;
}

bool KingdomGame::isDemo() const {
	return (bool)(_gameDescription->flags & ADGF_DEMO);
}

Common::Error KingdomGame::run() {
	initGraphics(320, 200, false);
	_console = new Console(this);

	setupPics();
	initTools();
	titlePage();

	initPlay();
	initHelp();

	while (!_quit) {
		_loopFlag = false;
		gameHelp();
		if (_gameMode == 0) {
			if (_statPlay < 250)
				GPLogic1();
			if (_statPlay > 249 && _statPlay < 500)
				GPLogic2();
			if (_statPlay > 499 && _statPlay < 900)
				GPLogic3();
			if (_statPlay > 899)
				GPLogic4();
		}

		if (!_loopFlag)
			getUserInput();

		refreshScreen();
	}

	fadeToBlack2();
	return Common::kNoError;
}

void KingdomGame::refreshScreen() {
	g_system->updateScreen();
	checkTimers();
}

void KingdomGame::checkTimers() {
	uint32 newTime = g_system->getMillis();
	int32 delay = 11 - (newTime - _oldTime);
	if (delay > 0)
		g_system->delayMillis(delay);
	_oldTime = newTime;
	_tickCount++;

	if (_tickCount == 5) {
		_tickCount = 0;
	} else
		return;

	if (_aTimer != 0) {
		_aTimer--;
		if (_aTimer == 0)
			_aTimerFlag = true;
	}

	if (_bTimer != 0) {
		_bTimer--;
		if (_bTimer == 0)
			_bTimerFlag = true;
	}

	if (_cTimer != 0) {
		_cTimer--;
		if (_cTimer == 0) {
			_cTimerFlag = true;
			_cTimer = 4;
		}
	} else
		_cTimer = 4;

	if (_skylarTimer != 0) {
		_skylarTimer--;
		if (_skylarTimer == 0)
			_skylarTimerFlag = true;
	}

	_palStepFlag = false;
}

void KingdomGame::drawScreen() {
	//TODO

	_console->onFrame();
}

void KingdomGame::setupPics() {
	loadKingArt();
}

void KingdomGame::initTools() {
	initMouse();
	//CHECKME: InitTimers?
	showPic(124);
	initCursor();
	setMouse();
	fadeToBlack2();
	initMPlayer();
}

void KingdomGame::titlePage() {
	// TODO: Check on QuitFlag == 2
	if (shouldQuit())
		return;

	_fstFwd = true;
	_noIFScreen = true;
	_sound = false;
	fadeToBlack2();
	playMovie(200);
	fadeToBlack2();
	playMovie(206);
	fadeToBlack2();
	playMovie(198);
	fadeToBlack2();
}

void KingdomGame::initPlay() {
	memset(_inventory, 0xFF, 19);

	for (int i = 0; i < 99; i++)
		_nodes[i] = 0;

	for (int i = 0; i < 7; i++)
		_iconPic[i] = 89 + i;

	_frameStop = 0;
	_gameMode = 0;
	_daelonCntr = 0;
	_statPlay = 10;
	_spell1 = false;
	_spell2 = false;
	_spell3 = false;
	_itemInhibit = false;
	_asMode = false;
	_aTimerFlag = false;
	_bTimerFlag = false;
	_cTimerFlag = false;
	_skylarTimerFlag = false;
	_aTimer = 0;
	_bTimer = 0;
	_cTimer = 0;
	_skylarTimer = 0;
	_tideCntl = false;
	_mapEx = false;
	_health = 12;
	_healthOld = 1;
	_healthTmr = 0;
	_treeEyeTimer = 0;
	_treeHGTimer = 0;
	_treeHGUPic = 147;
	_treeLeftPic = 0;
	_treeRightPic = 0;
	_treeRightSta = 1;
	_tsIconOnly = false;
	_lastObs = false;
	enAll();
	_pouch = true;
	_noIFScreen = true;
	_noMusic = false;
	_fstFwd = true;

	delete[] _asPtr;
	_asPtr = nullptr;
}

void KingdomGame::initHelp() {
	_gameMode = 0;
}

void KingdomGame::fadeToBlack1() {
	debug("STUB: FadeToBlack1");
}

void KingdomGame::fadeToBlack2() {
	debug("STUB: FadeToBlack2");
}

void KingdomGame::gameHelp_Sub43C() {
	fadeToBlack1();
	_currMap = _asMap;
	drawRect(4, 17, 228, 161, 0);
	restoreAS();
	_userInput = 0;
	_gameMode = 0;
	_iconsClosed = false;
	_treeLeftSta = _oldTLS;
	_eye = _oldEye;
	_help = _oldHelp;
	_pouch = _oldPouch;
}

void KingdomGame::gameHelp() {
	if (!_gameMode) {
		if (_userInput == 0x43C) {
			saveAS();
			_asMap = _currMap;
			_currMap = 0;
			fadeToBlack1();
			drawRect(4, 17, 228, 161, 0);
			drawHelpScreen();
			_gameMode = 1;
			_oldTLS = _treeLeftSta;
			_treeLeftSta = 0;
			_iconsClosed = true;
			_oldEye = _eye;
			_eye = false;
			_oldHelp = _help;
			_oldPouch = _pouch;
			_pouch = false;
			_userInput = 0;
		}
		if (_userInput == 0x44F) {
			saveAS();
			_asMap = _currMap;
			_gameMode = 2;
			_oldTLS = _treeLeftSta;
			_treeLeftSta = 0;
			_iconsClosed = true;
			_oldEye = _eye;
			_eye = false;
			_oldHelp = _help;
			_help = false;
			_oldPouch = _pouch;
			fadeToBlack1();
			drawRect(4, 17, 228, 161, 0);
			drawInventory();

			if (_nodes[68] == 1 || _nodes[29] == 1 || _itemInhibit)
				_currMap = 10;
			else
				_currMap = 11;

			_userInput = 0;
		}
	}
	if (_gameMode == 0)
		return;

	switch(_userInput) {
	case 0x240:
		fadeToBlack2();
		//TODO: Set _quitFlag to 1
		break;
	case 0x241:
		gameHelp_Sub43C();
		return;
		break;
	case 0x242:
		if (_noMusic) {
			_noMusic = false;
			playSound(1);
		} else {
			_noMusic = true;
			playSound(0);
		}
		drawHelpScreen();
		break;
	case 0x243: {
		fadeToBlack2();
		_keyActive = false;
		_noIFScreen = true;
		playSound(0);
		// The demo isn't saving pMovie.
		// It's obviously a bug and this behavior wasn't kept in ScummVM
		int oldPMovie = _pMovie;
		while(!_keyActive) {
			playMovie(54);
			fadeToBlack2();
		}
		_pMovie = oldPMovie;
		_noIFScreen = false;
		showPic(106);
		drawHelpScreen();
		_iconRedraw = true;
		playSound(1);
		}
		break;
	case 0x244:
		//TODO: Set _quitFlag to 2
		_quit = true;
		break;
	case 0x245: {
		fadeToBlack1();
		// The demo isn't saving pMovie.
		// It's obviously a bug and this behavior wasn't kept in ScummVM
		int oldPMovie = _pMovie;
		drawRect(4, 17, 228, 161, 0);
		playMovie(205);
		fadeToBlack1();
		drawRect(4, 17, 228, 161, 0);
		playMovie(199);
		fadeToBlack1();
		drawRect(4, 17, 228, 161, 0);
		drawHelpScreen();
		_pMovie = oldPMovie;
		}
		break;
	case 0x246:
		saveGame();
		break;
	case 0x43B:
	case 0x43C:
	case 0x44F:
		gameHelp_Sub43C();
		return;
	}

	if (_userInput > 0x427 && _userInput < 0x43A)
		fShowPic(130 + _userInput - 0x428);

	if (_userInput == 0x260) {
		drawInventory();
		if (_nodes[68] == 1 || _nodes[29] == 1)
			_currMap = 10;
		else
			_currMap = 11;

		_userInput = 0;
	}
}

void KingdomGame::loadKingArt() {
	loadAResource(0x97);
	Common::SeekableReadStream *kingartStream = _rezPointers[0x97];
	int val = kingartStream->readUint32LE();
	int size = val / 4;
	uint32 *kingartIdx = new uint32[size + 1];
	_kingartEntries = new KingArtEntry[size];
	kingartIdx[0] = val;
	for (int i = 1; i < size; i++)
		kingartIdx[i] = kingartStream->readUint32LE();
	kingartIdx[size] = kingartStream->size();

	for (int i = 0; i < size; i++) {
		int chunkSize = kingartIdx[i + 1] - kingartIdx[i];
		_kingartEntries[i]._width = kingartStream->readByte();
		_kingartEntries[i]._height = kingartStream->readByte();

		assert(_kingartEntries[i]._width * _kingartEntries[i]._height == chunkSize - 2);

		_kingartEntries[i]._data = new byte[chunkSize - 2];
		kingartStream->read(_kingartEntries[i]._data, chunkSize - 2);
	}

	delete[] kingartIdx;
}

void KingdomGame::loadAResource(int reznum) {
	Common::String path = Common::String(_rezNames[reznum]);
	path.toUppercase();

	debug("Loading resource: %i (%s)\n", reznum, path.c_str());

	if(!_rezSize[reznum]) {
		Common::File *file = new Common::File();
		if(!file->open(path))
			error("Failed to open %s", path.c_str());

		_rezSize[reznum] = file->size();
		file->seek(0, SEEK_SET);
		_rezPointers[reznum] = file->readStream(_rezSize[reznum]);
		file->close();
		delete file;
	}
}

void KingdomGame::releaseAResource(int reznum) {
	if (_rezSize[reznum]) {
		delete _rezPointers[reznum];
		_rezSize[reznum] = 0;
	}
}

void KingdomGame::showPic(int reznum) {
	eraseCursor();

	loadAResource(reznum);
	Image::IFFDecoder decoder;
	if (!decoder.loadStream(*_rezPointers[reznum]))
		return;

	const byte *palette = decoder.getPalette();
	int paletteColorCount = decoder.getPaletteColorCount();
	g_system->getPaletteManager()->setPalette(palette, 0, paletteColorCount);

	const Graphics::Surface *surface = decoder.getSurface();

	const byte *data = (const byte *)surface->getPixels();
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 320; curX++) {
		for (uint curY = 0; curY < 200; curY++) {
			const byte *src = data + (curY * 320) + curX;
			byte *dst = (byte *)screen->getBasePtr(curX, curY);
			if (*src != 0xFF)
				*dst = *src;
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();

	releaseAResource(reznum);
}

void KingdomGame::fShowPic(int reznum) {
	eraseCursor();
	fadeToBlack1();
	drawRect(4, 17, 228, 161, 0);
	showPic(reznum);
}

void KingdomGame::initCursor() {
	initMouse();
	setCursor(0x19C / 4);
	_cursorDrawn = false;
	drawCursor();
}

void KingdomGame::initMouse() {
	// No implementation required
}

void KingdomGame::setMouse() {
	g_system->warpMouse(272, 157);
	_cursorX = 272;
	_cursorY = 157;
}

void KingdomGame::initMPlayer() {
	debug("STUB: InitMPlayer");
}

void KingdomGame::playMovie(int movieNum) {
	debug("STUB: PlayMovie");

	if (movieNum == 1 || movieNum == 3 || movieNum == 54 || movieNum == 198 || movieNum == 200 || movieNum == 206)
		_fullScreen = true;
	else
		_fullScreen = false;

	_mixer->stopAll();

	_aTimer = 0;
	_asMode = false;

	eraseCursor();

	if (!_fullScreen) {
		_treeLeftSta = (_fstFwd == 0) ? 0 : 1;
		_treeRightSta = 0;
		_iconSel = _iconSelect;
		_iconsClosed = true;
		checkMainScreen();
		setMouse();
		_oldCursorX = _cursorX;
		_oldCursorY = _cursorY;
	}

	_pMovie = movieNum;
	readMouse();
	_mouseButton = 0;
	_keyActive = false;
	const char *path = _movieNames[movieNum];
	// Check if the file is available. If not the original does the following: _ATimer = 55, display of error with a check of timer, exit
	// That can be replaced by an error()

	/* Original behavior. To be uncommented when the codec is available
	_track = _sound;
	_vidState = 0;

	if (_fullScreen)
		MVE_RunMovie(fileHandle, 0, 0, _track);
	else
		MVE_RunMovie(fileHandle, 6, 17, _track);
	
	MVE_ReleaseMem();
	*/

	if (!_fullScreen) {
		_treeRightSta = 1;
		_iconsClosed = false;
		_iconSel = 9;
		_treeLeftSta = _replay ? 2 : 0;
		checkMainScreen();
		drawCursor();
		_fstFwd = true;
		_frameStop = 0;
		_lastSound = _sound;
		_sound = false;
		_userInput = 0;
	}
}

void KingdomGame::enAll() {
	_help = true;
	_eye = true;
	_replay = true;
	_pouch = true;
	_fstFwd = true;
}

void KingdomGame::dsAll() {
	_help = false;
	_eye = false;
	_replay = false;
	_pouch = false;
	_fstFwd = false;
}

void KingdomGame::saveAS() {
	byte palette[256 * 3];
	delete[] _asPtr;
	_asPtr = new byte[224 * 146 + 768];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);

	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 224; curX++) {
		for (uint curY = 0; curY < 146; curY++) {
			byte *ptr = (byte *)screen->getBasePtr(curX + 4, curY + 15);
			_asPtr[curY * 224 + curX] = *ptr;
		}
	}

	for (uint i = 0; i < 768; i++)
		_asPtr[224 * 146 + i] = palette[i];

	g_system->unlockScreen();
	g_system->updateScreen();
}

void KingdomGame::restoreAS() {
	byte palette[256 * 3];
	for (uint i = 0; i < 768; i++)
		palette[i] = _asPtr[224 * 146 + i];

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 224; curX++) {
		for (uint curY = 0; curY < 146; curY++) {
			byte *ptr = (byte *)screen->getBasePtr(curX + 4, curY + 15);
			*ptr = _asPtr[curY * 224 + curX];
		}
	}

	g_system->unlockScreen();
	g_system->updateScreen();
	delete[] _asPtr;
	_asPtr = nullptr;
}

void KingdomGame::switchAS() {
	_asMode = false;
	_currMap = _asMap;
	_treeLeftSta = _oldTLS;
	_treeRightSta = _oldTRS;
	_pouch = _oldPouch;
	_help = _oldHelp;
	_iconsClosed = _oldIconsClosed;
}

void KingdomGame::drawHelpScreen() {
	int picNum;

	switch(_health) {
	case 2:
		picNum = 166;
		break;
	case 4:
		picNum = 165;
		break;
	case 6:
		picNum = 164;
		break;
	case 8:
		picNum = 163;
		break;
	case 10:
		picNum = 162;
		break;
	case 12:
	default:
		picNum = 161;
		break;
	}

	if (_noMusic)
		picNum += 6;

	showPic(picNum);
}

void KingdomGame::drawRect(uint minX, uint minY, uint maxX, uint maxY, int color) {
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = minX; curX < maxX; curX++) {
		for (uint curY = minY; curY < maxY; curY++) {
			byte *dst = (byte *)screen->getBasePtr(curX, curY);
			*dst = color;
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

void KingdomGame::drawInventory() {
	fShowPic(108);
	if (_nodes[29] == 1 || _nodes[68] == 1 || _itemInhibit)
		return;

	if (_inventory[0] > 0)
		drawIcon(136, 102, 180);

	if (_inventory[1] > 0)
		drawIcon(73, 65, 175);

	if (_inventory[2] > 0)
		drawIcon(171, 96, 179);

	if (_inventory[3] > 0)
		drawIcon(120, 34, 174);

	if (_inventory[4] > 0)
		drawIcon(160, 41, 177);

	if (_inventory[5] > 0)
		drawIcon(21, 124, 184);

	if (_inventory[6] > 0)
		drawIcon(201, 42, 178);

	if (_inventory[7] > 0)
		drawIcon(76, 119, 186);

	if (_inventory[8] > 0)
		drawIcon(18, 31, 170);

	if (_inventory[9] > 0)
		drawIcon(57, 88, 185);

	if (_inventory[10] > 0)
		drawIcon(182, 124, 181);

	if (_inventory[11] > 0)
		drawIcon(176, 26, 183);

	if (_inventory[12] > 0)
		drawIcon(54, 23, 171);

	if (_inventory[13] > 0)
		drawIcon(120, 133, 182);

	if (_inventory[14] > 0)
		drawIcon(94, 92, 187);

	if (_inventory[15] > 0)
		drawIcon(135, 67, 176);

	if (_inventory[16] > 0)
		drawIcon(84, 30, 173);

	if (_inventory[17] > 0)
		drawIcon(20, 78, 172);

	if (_inventory[0] > 0)
		drawIcon(158, 117, 134 + _inventory[0]);

	if (_inventory[1] > 0)
		drawIcon(94, 67, 134 + _inventory[1]);

	if (_inventory[2] > 0)
		drawIcon(193, 105, 134 + _inventory[2]);

	if (_inventory[3] > 0)
		drawIcon(131, 39, 134 + _inventory[3]);
}

Common::String KingdomGame::getSavegameFilename(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

void KingdomGame::saveGame() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	int16 savegameId = dialog->runModalWithCurrentTarget();
	Common::String savegameDescription = dialog->getResultString();
	delete dialog;
	if (savegameId < 0)
		return; // dialog aborted
	saveGameState(savegameId, savegameDescription);
}

void KingdomGame::restoreGame() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	int16 savegameId = dialog->runModalWithCurrentTarget();
	delete dialog;

	if (savegameId < 0)
		return; // dialog aborted

	loadGameState(savegameId);
}

Common::Error KingdomGame::saveGameState(int slot, const Common::String &desc) {
	Common::String savegameFile = getSavegameFilename(slot);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *out = saveMan->openForSaving(savegameFile);

	if (!out)
		return Common::kCreatingFileFailed;

	KingdomSavegameHeader header;
	header._saveName = desc;
	writeSavegameHeader(out, header);

	Common::Serializer s(nullptr, out);
	synchronize(s);

	out->finalize();
	delete out;

	return Common::kNoError;
}

Common::Error KingdomGame::loadGameState(int slot) {
	Common::String savegameFile = getSavegameFilename(slot);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *inFile = saveMan->openForLoading(savegameFile);
	if (!inFile)
		return Common::kReadingFailed;

	Common::Serializer s(inFile, nullptr);

	KingdomSavegameHeader header;
	if (!readSavegameHeader(inFile, header))
		error("Invalid savegame");

	if (header._thumbnail) {
		header._thumbnail->free();
		delete header._thumbnail;
	}

	// Load most of the savegame data
	synchronize(s);
	delete inFile;

	delete[] _asPtr;
	_asPtr = nullptr;

	playSound(_soundNumber);
	for (int i = 0; i < 7; i++)
		_iconPic[i] = 89 + i;
	
	_frameStop = 0;
	_gameMode = 0;
	_asMode = false;
	_healthTmr = 0;
	_noIFScreen = false;
	_iconRedraw = true;
	_treeRightSta = 1;
	_aTimerFlag = false;
	_aTimer = 0;
	_bTimerFlag = false;
	_bTimer = 0;
	_treeEyeTimer = 0;
	_treeEyePic = 0;
	_treeHGUPic = 0;
	_cursorDrawn = false;
	showPic(106);
	_gameMode = 0;
	_iconsClosed = false;
	drawRect(4, 17, 228, 161, 0);
	_userInput = 0x43E;
	_loopFlag = true;

	return Common::kNoError;
}

void KingdomGame::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_statPlay);
	s.syncAsSint16LE(_asMap);
	s.syncAsSint16LE(_daelonCntr);
	s.syncAsSint16LE(_health);
	s.syncAsSint16LE(_healthOld);
	s.syncAsSint16LE(_lastObstacle);
	s.syncAsSint16LE(_nextNode);
	s.syncAsSint16LE(_nodeNum);
	s.syncAsSint16LE(_pMovie);
	s.syncAsSint16LE(_rtnNode);
	s.syncAsSint16LE(_robberyNode);
	s.syncAsSint16LE(_soundNumber);
	s.syncAsSint16LE(_treeEyePic);
	s.syncAsSint16LE(_treeEyeSta);
	s.syncAsSint16LE(_treeHGPic);
	s.syncAsSint16LE(_treeHGSta);
	s.syncAsSint16LE(_oldTLS);
	s.syncAsSint16LE(_cTimer);
	s.syncAsSint16LE(_skylarTimer);

	for (int i = 0; i < 18; i++)
		s.syncAsSint16LE(_inventory[i]);

	for (int i = 0; i < 99; i++)
		s.syncAsSint16LE(_nodes[i]);

	s.syncAsByte(_oldEye);
	s.syncAsByte(_fstFwd);
	s.syncAsByte(_help);
	s.syncAsByte(_itemInhibit);
	s.syncAsByte(_lastObs);
	s.syncAsByte(_lastSound);
	s.syncAsByte(_mapEx);
	s.syncAsByte(_noMusic);
	s.syncAsByte(_oldPouch);
	s.syncAsByte(_replay);
	s.syncAsByte(_spell1);
	s.syncAsByte(_spell2);
	s.syncAsByte(_spell3);
	s.syncAsByte(_tideCntl);
	s.syncAsByte(_wizard);
	s.syncAsByte(_tsIconOnly);
	s.syncAsByte(_cTimerFlag);
	s.syncAsByte(_skylarTimerFlag);

	// Present in the original. Looks unused.
	// s.syncAsSint16LE(_StatMap);
}

const char *const SAVEGAME_STR = "KTFR";
#define SAVEGAME_STR_SIZE 4
#define KTFR_SAVEGAME_VERSION 1

void KingdomGame::writeSavegameHeader(Common::OutSaveFile *out, KingdomSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(KTFR_SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(header._saveName);
	out->writeByte('\0');

	Common::MemoryWriteStreamDynamic *tempThumbnail = new Common::MemoryWriteStreamDynamic;
	Graphics::saveThumbnail(*tempThumbnail);
	out->write(tempThumbnail->getData(), tempThumbnail->size());
	delete tempThumbnail;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
}

bool KingdomGame::readSavegameHeader(Common::InSaveFile *in, KingdomSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header._thumbnail = nullptr;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > KTFR_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header._saveName += ch;

	// Get the thumbnail
	header._thumbnail = Graphics::loadThumbnail(*in);
	if (!header._thumbnail)
		return false;

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();

	return true;
}

void KingdomGame::playSound(int idx) {
	if (idx > 43 || _soundNumber == idx)
		return;

	// Stop Sound
	if (_mixer->isSoundHandleActive(_soundHandle)) {
		_mixer->stopHandle(_soundHandle);
		releaseAResource(idx);
	}

	_soundNumber = idx;
	if (_soundNumber == 0 || _noMusic)
		return;

	int realIdx = _soundNumber + 200; // Or +250, depending in the original on the sound card
	debug("PlaySound %d : %s", idx, _rezNames[realIdx]);
	loadAResource(realIdx);

	Common::SeekableReadStream *soundStream = _rezPointers[realIdx];
	Audio::RewindableAudioStream *rewindableStream = Audio::makeRawStream(soundStream, 22050, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, rewindableStream);
//  In the original, there's an array describing whether a sound should loop or not.
//  The array is full of 'false'. If a variant uses looping sound/music, the following code
//	and the loop array should be added.
//	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(rewindableStream, _loopArray[idx]);
//	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, stream);
}

void KingdomGame::eraseCursor() {
	CursorMan.showMouse(false);
}

void KingdomGame::readMouse() {
	_mouseButton = 0;

	Common::Event event;
	g_system->getEventManager()->pollEvent(event);
	if (event.type == Common::EVENT_MOUSEMOVE) {
	_cursorX = event.mouse.x;
	_cursorY = event.mouse.y;
	}
	if (event.type == Common::EVENT_LBUTTONUP)
		_mouseButton |= 1;
	if (event.type == Common::EVENT_RBUTTONUP)
		_mouseButton |= 2;

	g_system->getEventManager()->pushEvent(event);
}

void KingdomGame::getUserInput() {
	// CHECKME: _QuitFlag != 0
	if (_quit)
		return;

	_userInput = waitKey();

	if (_quit)
		return;

	if (_userInput == 2 && _eye)
		_userInput = _asMode ? 0x43B : 0x43A;

	if (_userInput == 1)
		_userInput = _mouseValue;

	if (_userInput == 0x2F5) {
		_statPlay = 600;
		_loopFlag = true;
	}

	if (_userInput == 0x42B && _statPlay == 53 && _gameMode == 0) {
		_oldStatPlay = _statPlay;
		_statPlay = 900;
		_loopFlag = true;
	}

	if (_userInput == 0x12D && _currMap == 1)
		// CHECKME: _quitFlag = 2;
		_quit = true;
}

void KingdomGame::eraseCursorAsm() {
	CursorMan.showMouse(false);
}

void KingdomGame::drawLocation() {
	if (_daelonCntr > 0)
		_daelonCntr--;

	playSound(0);
	_iconsClosed = true;
	_tsIconOnly = false;
	_aTimer = 0;
	_aTimerFlag = false;

	int emlValue = _emlTable[_nodeNum];
	if (emlValue > 0)
		enAll();

	if (!_mapEx || !emlValue || _resurrect) {
		if (_statPlay != 50)
			_resurrect = false;
		_iconsClosed = false;
	} else {
		_mapEx = false;
		saveAS();
		fShowPic(emlValue);
		_bTimer = 16;
		while(_bTimer) {
			checkTimers();
			refreshSound();
			checkMainScreen();
		}
		fadeToBlack1();
		drawRect(4, 17, 228, 161, 0);
		_iconsClosed = false;
		_tsIconOnly = false;
	}
}

void KingdomGame::processMap(int mapNum, int zoom) {
	int var6 = _zoomTable[mapNum][zoom][0];
	if (!_asMode)
		switchAtoM();
	fShowPic(var6);
	_currMap = _zoomTable[mapNum][zoom][1];

	if (zoom > 0)
		_treeLeftSta = _zoomTable[mapNum][zoom - 1][0] == 0 ? 0 : 3;
	else
		_treeLeftSta = 0;

	if (zoom < 8)
		_treeRightSta = _zoomTable[mapNum][zoom + 1][0] == 0 ? 0 : 2;
	else
		_treeRightSta = 0;
}

void KingdomGame::processMapInput(int mapNum) {
	switch(_userInput) {
	case 0x43B:
	case 0x443:
		switchMtoA();
		_mapStat = 0;
		_statPlay--;
		break;
	case 0x43F:
		if (_treeLeftSta == 3) {
			_zoom--;
			processMap(mapNum, _zoom);
		} else
			_userInput = 0;
		break;
	case 0x440:
		if (_treeRightSta == 2) {
			_zoom++;
			processMap(mapNum, _zoom);
		} else
			_userInput = 0;
		break;
	default:
		if (_userInput > 0x3FF && _userInput < 0x428) {
			_statPlay = _mapExit[_userInput - 0x400];
			_mapEx = true;
			_loopFlag = true;
			switchAS();
		}

		if (_userInput > 0x440) {
			switchMtoA();
			_mapStat = false;
			_statPlay--;
			_loopFlag = true;
		}
		break;
	}
}

void KingdomGame::inventoryDel(int item) {
	if (_inventory[item] > 0)
		_inventory[item]--;
}

void KingdomGame::inventoryAdd(int item) {
	if (item >= 4)
		_inventory[item] = 1;
	else
		_inventory[item] = 3;
}

void KingdomGame::drawPic(int reznum) {
	eraseCursor();
	loadAResource(reznum);

	Image::IFFDecoder decoder;
	if (!decoder.loadStream(*_rezPointers[reznum]))
		return;

	const Graphics::Surface *surface = decoder.getSurface();

	const byte *data = (const byte *)surface->getPixels();
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 320; curX++) {
		for (uint curY = 0; curY < 200; curY++) {
			const byte *src = data + (curY * 320) + curX;
			byte *dst = (byte *)screen->getBasePtr(curX, curY);
			if (*src != 0xFF)
				*dst = *src;
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();

	releaseAResource(reznum);
}

void KingdomGame::displayIcon(int reznum) {
	// The demo isn't calling playsound(0).
	// We keep the call because it stops properly the previous sound, if any.
	playSound(0);
	playSound(30);
	saveAS();
	fShowPic(reznum);
	_bTimer = 76;
	readMouse();
	
	while(_bTimer != 0 && _mouseButton == 0) {
		checkTimers();
		refreshSound();
		readMouse();
	}

	fadeToBlack1();
	drawRect(4, 17, 228, 161, 0);
	restoreAS();
}

void KingdomGame::setATimer() {
	_aTimerFlag = true;
	_aTimer = 0;
	int wrkNodeNum = _nodeNum;
	if (_nodes[29] == 1 || _nodes[68] == 1)
		return;

	if (_tsIconOnly)
		wrkNodeNum = 79;

	if (_nodeNum == 56 && _inventory[8] < 1 && _wizard)
		wrkNodeNum = 80;
	
	for (int i = 0; i < 7; i++) {
		int idx = _iconActTable[wrkNodeNum][i];
		if (_inventory[idx] > 0) {
			_aTimerFlag = false;
			_aTimer = _wizard ? 114 : 133;
			playSound(0);
			if (!isDemo())
				playSound(34);
			break;
		}
	}
}

bool KingdomGame::wound() {
	bool retval = false;
	if (_health == 12 || _health == 8 || _health == 4) {
		_health -= 2;
		retval = true;
	}
	return retval;
}

void KingdomGame::refreshSound() {
// No implementation needed in ScummVM
// The only useful check is against _sndReplayTbl, which determines
// which sound should be replayed. But this array contains only false
}

void KingdomGame::increaseHealth() {
	if (_health <= 3)
		_health = 4;
	else if (_health <= 7)
		_health = 8;
	else
		_health = 12;
}

void KingdomGame::checkMainScreen() {
	if (_cTimerFlag || _statPlay == 900 || _statPlay == 901)
		return;

	_cTimerFlag = false;
	if (_noIFScreen)
		return;

	if (_healthOld != _health) {
		if (_healthTmr > 0)
			_healthTmr--;
		else {
			if (_health <= _healthOld)
				_healthOld--;
			else
				_healthOld++;

			int iconIndex;
			if (_healthOld == 0)
				iconIndex = 12 - 1;
			else
				iconIndex = 12 - _healthOld;

			drawIcon(4, 0, iconIndex);
			_healthTmr = 1;
		}
	}

	if (_iconRedraw) {
		_iconRedraw = false;
		drawIcon(4, 0, 12 - _healthOld);
		drawIcon(11, 178, _iconPic[0]);
		drawIcon(38, 178, _iconPic[1]);
		drawIcon(65, 178, _iconPic[2]);
		drawIcon(92, 178, _iconPic[3]);
		drawIcon(119, 178, _iconPic[4]);
		drawIcon(146, 178, _iconPic[5]);
		drawIcon(173, 178, _iconPic[6]);
		_treeLeftPic = 0;
		_treeRightPic = 0;
		_treeEyeTimer = 0;
		if (_skylarTimer != 0 || _aTimer != 0) {
			_treeHGTimer = 0;
			_treeHGUPic = 0;
		}
		if (_tideCntl)
			drawPic(178);
	}

	for (int i = 0; i < 7; i++) {
		int wrkNodeNum = _nodeNum;
		if (_tsIconOnly)
			wrkNodeNum = 79;
		if (_nodeNum == 56 && _inventory[16] < 1 && _wizard)
			wrkNodeNum = 80;
		if (_nodeNum == 21 && _nodes[21] == 9 && !isDemo())
			wrkNodeNum = 81;
		int idx = _iconActTable[wrkNodeNum][i];

		if (_inventory[idx] >= 1 && _nodes[29] != 1 && _nodes[68] != 1 && !_itemInhibit && !_iconsClosed) {
			if (_iconPic[i] != 12 + idx) {
				if (_iconPic[i] == 89 + i)
					_iconPic[i] = 96 + i;
				else if (_iconPic[i] == 96 + i)
					_iconPic[i] = 31;
				else if (_iconPic[i] == 31)
					_iconPic[i] = 32;
				else if (_iconPic[i] == 32)
					_iconPic[i] = 12 + idx;
				else
					_iconPic[i] = 89 + i;
			} 
		} else if (_iconSel != i && _iconPic[i] != 89 + i) {
			if (_iconPic[i] != 12 + idx)
				_iconPic[i] = 32;
			else if (_iconPic[i] == 32)
				_iconPic[i] = 31;
			else if (_iconPic[i] == 31)
				_iconPic[i] = 96 + i;
			else if (_iconPic[i] == 96 + i)
				_iconPic[i] = 32;
			else
				_iconPic[i] = 89 + i;
		} else
			continue;

		int posX = (27 * i) + 11;
		drawIcon(posX, 178, _iconPic[i]);
	}

	switch (_treeLeftSta) {
	case 0:
		if (_treeLeftPic != 33) {
			drawIcon(243, 141, 33);
			_treeLeftPic = 33;
		}
		break;
	case 1:
		if (_treeLeftPic != 34) {
			drawIcon(243, 141, 34);
			_treeLeftPic = 34;
		}
		break;
	case 2:
		if (!_replay) {
			if (_treeLeftPic != 33) {
				drawIcon(243, 141, 33);
				_treeLeftPic = 33;
			}
		} else if (_treeLeftPic != 35) {
			drawIcon(243, 141, 35);
			_treeLeftPic = 35;
		}
		break;
	case 3:
		if (_treeLeftPic != 36) {
			drawIcon(243, 141, 36);
			_treeLeftPic = 36;
		}
		break;
	default:
		_treeLeftPic = 33;
		_treeLeftSta = 0;
		drawIcon(243, 141, 33);
		break;
	}

	switch (_treeRightSta) {
	case 0:
		if (_treeRightPic == 37) {
			drawIcon(290, 143, 37);
			_treeRightPic = 37;
		}
		break;
	case 1:
		if (_help) {
			if (_treeRightPic != 38) {
				drawIcon(290, 143, 38);
				_treeRightPic = 38;
			}
		} else if (_treeRightPic != 37) {
			drawIcon(290, 143, 37);
			_treeRightPic = 37;
		}
		break;
	case 2:
		if (_treeRightPic != 39) {
			drawIcon(290, 143, 39);
			_treeRightPic = 39;
		}
		break;
	default:
		_treeRightPic = 37;
		_treeRightSta = 0;
		drawIcon(290, 143, 37);
		break;
	}

	if (_eye) {
		if (_treeEyeTimer == 0) {
			_treeEyePic = _teaSeq[_treeEyeSta][0];
			drawIcon(261, 51, _treeEyePic);
			_treeEyeTimer = _teaSeq[_treeEyeSta][1];
			_treeEyeSta++;
			if (_treeEyeSta == 5)
				_treeEyeSta = 0;
		} else
			_treeEyeTimer--;
	} else if (_treeEyePic != 37) {
		drawIcon(261, 51, 146);
		_treeEyePic = 37;
		_treeEyeSta = 0;
		_treeEyeTimer = 0;
	}

	int timer = 0;
	int delta = 7; // CHECKME: the variable is the same than the one used for the first for(), and the value should therefore be 7  
	if (_skylarTimer != 0) {
		delta = 772;
		timer = _skylarTimer;
	}
	if (_aTimer != 0) {
		delta = 19;
		timer = _aTimer;
	}

	if (timer == 0) {
		if (_treeHGUPic != 147) {
			eraseCursor();
			drawIcon(249, 171, 147);
			_treeHGUPic = 147;
		}
	} else if (_treeHGTimer == 0) {
		_treeHGPic = _hgaSeq[_treeHGSta][0];
		drawIcon(249, 185, _treeHGPic);
		_treeHGTimer = _hgaSeq[_treeHGSta][1];
		_treeHGSta++;
		if (_treeHGSta > 3)
			_treeHGSta = 0;

		int var2 = 6;
		while (true) {
			if (timer <= 1)
				break;

			timer -= delta;
			if (timer > 1)
				var2--;
			else {
				drawIcon(249, 171, 40 + var2);
				_treeHGUPic = 40 + var2;
			}
		}
	} else
		_treeHGTimer--;
}

bool KingdomGame::chkDesertObstacles() {
	if (!_wizard)
		return false;

	_nextNode = _nodeNum;
	if (_lastObs) {
		_lastObs = false;
		return false;
	}

	if (_nodes[29] || _rnd->getRandomNumber(6) == 0) {
		if (!_nodes[49] || _robberyNode != _nodeNum) {
			if (_lastObstacle != _nodeNum) {
				if (_rnd->getRandomNumber(5) == 0) {
					_statPlay = 250;
					_lastObstacle = _nodeNum;
					_lastObs = true;
					_loopFlag = true;
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			_statPlay = 490;
			_loopFlag = true;
			return true;
		}
	} else {
		_statPlay = 280;
		_robberyNode = _nodeNum;
		_lastObstacle = _nodeNum;
		_lastObs = true;
		_loopFlag = true;
		return true;
	}
}

void KingdomGame::switchAtoM() {
	_asMode = true;
	_asMap = _currMap;
	saveAS();
	_iconSel = 9;
	_oldTLS = _treeLeftSta;
	_oldTRS = _treeRightSta;
	_oldPouch = _pouch;
	_oldHelp = _help;
	_oldIconsClosed = _iconsClosed;
	_treeLeftSta = 0;
	_treeRightSta = 0;
	_pouch = false;
	_help = false;
	_iconsClosed = true;
}

void KingdomGame::switchMtoA() {
	switchAS();
	fadeToBlack1();
	drawRect(4, 17, 228, 161, 0);
	restoreAS();
}

void KingdomGame::drawIcon(int x, int y, int index) {
	const byte *data = _kingartEntries[index]._data;
	int width = _kingartEntries[index]._width;
	int height = _kingartEntries[index]._height;

	::Graphics::Surface *screen = g_system->lockScreen();
	for (int curX = 0; curX < width; curX++) {
		for (int curY = 0; curY < height; curY++) {
			const byte *src = data + (curY * width) + curX;
			byte *dst = (byte *)screen->getBasePtr(curX + x, curY + y);
			if (*src != 0xFF)
				*dst = *src;
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();

}

int KingdomGame::getAKey() {
	drawCursor();
	if (_mouseButton != 0 && _mouseDebound == false) {
		_mouseDebound = true;
		return (_mouseButton & 2) ? 2 : 1;
	}

	int retval = 0;
	_mouseDebound = false;
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			_quit = true;
			break;

		case Common::EVENT_LBUTTONDOWN:
			break;
		case Common::EVENT_KEYDOWN:
			// if keyboard used, retVal = getch() + 0x100
			if (!event.kbd.hasFlags(Common::KBD_CTRL) && !event.kbd.hasFlags(Common::KBD_ALT))
				retval = 0x100 + event.kbd.keycode;
			else if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL))
				_console->attach();
			else if (event.kbd.keycode == Common::KEYCODE_c && event.kbd.hasFlags(Common::KBD_CTRL)) {
				_userInput = 0x12D;
				// _QuitFlag = 2;
				_quit = true; 
			}
			break;
		case Common::EVENT_LBUTTONUP: // retval == 2?
			if (_eye)
				retval = !_asMode ? 0x43A : 0x43B;
			break;
		case Common::EVENT_RBUTTONUP: // retval == 1?
			retval = _mouseValue;
			break;
		case Common::EVENT_MOUSEMOVE:
			_cursorX = event.mouse.x;
			_cursorY = event.mouse.y;

		default:
			refreshSound();
			checkMainScreen();
			if (_aTimerFlag) {
				_aTimerFlag = false;
				retval = 0x2F1;
			} else if (_bTimerFlag) {
				_bTimerFlag = false;
				retval = 0x2F2;
			} else if (_skylarTimerFlag) {
				_skylarTimerFlag = false;
				retval = 0x2F5;
			} else
				retval= 0;
			break;
		}
	}
	return retval;
}

int KingdomGame::waitKey() {
	return getAKey();
}

void KingdomGame::drawCursor() {
	readMouse();

	cursorType();
	setCursor(_cursorDef);
	_oldCursorX = _cursorX;
	_oldCursorY = _cursorY;
	_oldCursorDef = _cursorDef;

	CursorMan.showMouse(true);
	_cursorDrawn = true;
}

void KingdomGame::cursorType() {
	_mouseValue = 0;
	if (_currMap != 1 && _statPlay >= 30) {
		int var2 = _statPlay == 901 ? 16 : 0;
		int var6 = _statPlay == 901 ? 35 : 16;
		for (int i = 0; i < var6 + 1; i++) {
			if (i == var6) {
				int tmpVal = checkMouseMapAS();
				if (tmpVal == -1) {
					cursorTypeExit();
					return;
				} else
					_mouseValue = tmpVal;
			} else if (_cursorX >= _mouseMapMS[var2 + i]._minX && _cursorX < _mouseMapMS[var2 + i]._maxX && _cursorY >= _mouseMapMS[var2 + i]._minY && _cursorY < _mouseMapMS[var2 + i]._maxY) {
				_mouseValue = _mouseMapMS[var2 + i]._mouseValue;
				break;
			}
		}
	} else {
		int tmpVal = checkMouseMapAS();
		if (tmpVal == -1) {
			cursorTypeExit();
			return;
		} else {
			_mouseValue = tmpVal;
		}
	}

	switch(_mouseValue) {
	case 0x18A:
		if (_eye)
			_mouseValue = !_asMode ? 0x43A : 0x43B;
		else
			_mouseValue = 0;
		break;
	case 0x18C:
		if (_treeLeftSta == 1)
			_mouseValue = 0x43D;
		else if (_treeLeftSta == 3)
			_mouseValue = 0x43F;
		else if (_treeLeftSta == 0)
			_mouseValue = 0;
		else if (_treeLeftSta == 2 && _replay)
			_mouseValue = 0x43E;
		else
			_mouseValue = 0;
		break;
	case 0x18D:
		if (_treeRightSta == 1)
			_mouseValue = _help ? 0x43C : 0;
		if (_treeRightSta == 2)
			_mouseValue = 0x440;
		break;
	case 0x24A:
		// Restore game.
		// No more check in ScummVM, we display the load screen
		break;
	case 0x407:
		if (_statPlay == 182 && _nodes[18] < 9)
			_mouseValue = 0;
		break;
	case 0x40D:
		if (_nodes[29] == 1)
			_mouseValue = 0;
		break;
	case 0x41F:
		if (_nodes[32] == 0)
			_mouseValue = 0;
		break;
	case 0x422:
	case 0x425:
		if (!_wizard)
			_mouseValue = 0;
		break;
	case 0x428:
		if (_nodeNum == 5 && _gameMode != 2 && _spell1)
			_mouseValue = 0;
		break;
	case 0x42A:
		if (_nodeNum == 5 && _gameMode != 2 && _spell2)
			_mouseValue = 0;
		break;
	case 0x42B:
		if (_nodeNum == 5 && _gameMode != 2 && _spell3)
			_mouseValue = 0;
		break;
	case 0x445:
		if (_statPlay == 161 && _nodes[16] == 0 && _wizard)
			_mouseValue = 0x450;
		break;
	case 0x44F:
		if (!_pouch)
			_mouseValue = 0;
		break;
	case 0x457:
		if (!_tideCntl)
			_mouseValue = 0;
		break;
	}

	_iconSelect = 9;
	for (int var6 = 0; var6 < 8; var6++) {
		if (_mouseValue == 181 + var6) {
			int var2 = _nodeNum;
			if (_tsIconOnly)
				var2 = 79;
			if (_nodeNum == 56 && _inventory[8] < 1 && _wizard)
				var2 = 80;
			int indx = _iconActTable[var2][var6];
			if (_inventory[indx] != 0 && _nodes[29] != 1 && _nodes[68] != 1 && !_iconsClosed && !_itemInhibit) {
				_mouseValue = indx + 0x428;
				_iconSelect = var6;
				break;
			}
			_mouseValue = 0;
		}
	}

	if (_currMap == 11) {
		if (_mouseValue > 0x427 && _mouseValue < 0x43A) {
			if (_inventory[_mouseValue - 0x428] < 1)
				_mouseValue = 0x241;
		}
	}
	cursorTypeExit();
}

void KingdomGame::cursorTypeExit() {
	if (_mouseValue >= 0x400)
		_cursorDef = _cursorTable[_mouseValue - 0x400];
	else 
		_cursorDef = (_mouseValue != 0) ? 0x68 : 0x67;
}

int KingdomGame::checkMouseMapAS() {
	for (int i = 0; i < 16; i++) {
		if (_cursorX >= _mouseMapAS[_currMap][i]._minX && _cursorX < _mouseMapAS[_currMap][i]._maxX
			&& _cursorY >= _mouseMapAS[_currMap][i]._minY && _cursorY < _mouseMapAS[_currMap][i]._maxY)
			return _mouseMapAS[_currMap][i]._mouseValue;
	}
	if (_currMap == 11) {
		for (int i = 0; i < 16; i++) {
			if (_cursorX >= _mouseMapAS[12][i]._minX && _cursorX < _mouseMapAS[12][i]._maxX
				&& _cursorY >= _mouseMapAS[12][i]._minY && _cursorY < _mouseMapAS[12][i]._maxY)
				return _mouseMapAS[12][i]._mouseValue;
		}
	}
	return -1;
}
void KingdomGame::setCursor(int cursor) {
	KingArtEntry Cursor = _kingartEntries[cursor];
	CursorMan.replaceCursor(Cursor._data, Cursor._width, Cursor._height, 0, 0, 255);
}

} // End of namespace Kingdom
