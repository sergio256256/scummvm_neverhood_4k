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

#include "twine/flamovies.h"
#include "common/file.h"
#include "common/system.h"
#include "image/pcx.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/input.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/grid.h"
#include "twine/twine.h"

namespace TwinE {

/** FLA Frame Opcode types */
enum FlaFrameOpcode {
	kLoadPalette = 0,
	kFade = 1,
	kPlaySample = 2,
	kStopSample = 4,
	kDeltaFrame = 5,
	kKeyFrame = 7
};

/** FLA movie sample structure */
struct FLASampleStruct {
	/** Number os samples */
	int16 sampleNum = 0;
	/** Sample frequency */
	int16 freq = 0;
	/** Numbers of time to repeat */
	int16 repeat = 0;
	/** Dummy variable */
	int8 dummy = 0;
	/** Unknown x */
	uint8 x = 0;
	/** Unknown y */
	uint8 y = 0;
};

/** FLA movie extension */
#define FLA_EXT ".fla"

void FlaMovies::drawKeyFrame(Common::MemoryReadStream &stream, int32 width, int32 height) {
	uint8 *destPtr = (uint8 *)flaBuffer;
	uint8 *startOfLine = destPtr;

	for (int32 y = 0; y < height; ++y) {
		const int8 lineEntryCount = stream.readByte();

		for (int8 a = 0; a < lineEntryCount; a++) {
			const int8 rleFlag = stream.readByte();

			if (rleFlag < 0) {
				const int8 rleCnt = ABS(rleFlag);
				for (int8 b = 0; b < rleCnt; ++b) {
					*destPtr++ = stream.readByte();
				}
			} else {
				const char colorFill = stream.readByte();
				Common::fill(&destPtr[0], &destPtr[rleFlag], colorFill);
				destPtr += rleFlag;
			}
		}

		startOfLine = destPtr = startOfLine + width;
	}
}

void FlaMovies::drawDeltaFrame(Common::MemoryReadStream &stream, int32 width) {
	const uint16 skip = stream.readUint16LE() * width;
	const int32 height = stream.readSint16LE();

	uint8 *destPtr = (uint8 *)flaBuffer + skip;
	uint8 *startOfLine = destPtr;
	for (int32 y = 0; y < height; ++y) {
		const int8 lineEntryCount = stream.readByte();

		for (int8 a = 0; a < lineEntryCount; ++a) {
			destPtr += stream.readByte();
			const int8 rleFlag = stream.readByte();

			if (rleFlag > 0) {
				for (int8 b = 0; b < rleFlag; ++b) {
					*destPtr++ = stream.readByte();
				}
			} else {
				const char colorFill = stream.readByte();
				const int8 rleCnt = ABS(rleFlag);
				Common::fill(&destPtr[0], &destPtr[rleCnt], colorFill);
				destPtr += rleCnt;
			}
		}

		startOfLine = destPtr = startOfLine + width;
	}
}

void FlaMovies::scaleFla2x() {
	uint8 *source = (uint8 *)flaBuffer;
	uint8 *dest = (uint8 *)_engine->imageBuffer.getPixels();

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		Common::fill(&dest[0], &dest[_engine->imageBuffer.w * 40], 0x00);
		dest += _engine->imageBuffer.w * 40;
	}

	for (int32 i = 0; i < FLASCREEN_HEIGHT; i++) {
		for (int32 j = 0; j < FLASCREEN_WIDTH; j++) {
			*dest++ = *source;
			*dest++ = *source++;
		}
		if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) { // include wide bars
			memcpy(dest, dest - _engine->imageBuffer.w, FLASCREEN_WIDTH * 2);
			dest += FLASCREEN_WIDTH * 2;
		} else { // stretch the movie like original game.
			if (i % 2) {
				memcpy(dest, dest - _engine->imageBuffer.w, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
			if (i % 10) {
				memcpy(dest, dest - _engine->imageBuffer.w, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
		}
	}

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		Common::fill(&dest[0], &dest[_engine->imageBuffer.w * 40], 0x00);
	}
}

void FlaMovies::processFrame() {
	FLASampleStruct sample;

	file.read(&frameData.videoSize, 1);
	file.read(&frameData.dummy, 1);
	file.read(&frameData.frameVar0, 4);
	if (frameData.frameVar0 > _engine->imageBuffer.w * _engine->imageBuffer.h) {
		warning("Skipping video frame - it would exceed the screen buffer: %i", frameData.frameVar0);
		return;
	}

	uint8 *outBuf = (uint8 *)_engine->imageBuffer.getPixels();
	file.read(outBuf, frameData.frameVar0);

	if ((int32)frameData.videoSize <= 0) {
		return;
	}

	Common::MemoryReadStream stream(outBuf, frameData.frameVar0);
	for (int32 frame = 0; frame < frameData.videoSize; ++frame) {
		const uint8 opcode = stream.readByte();
		stream.skip(1);
		const uint32 opcodeBlockSize = stream.readUint16LE();
		const int32 pos = stream.pos();

		switch (opcode - 1) {
		case kLoadPalette: {
			int16 numOfColor = stream.readSint16LE();
			int16 startColor = stream.readSint16LE();
			uint8 *dest = _engine->_screens->palette + (startColor * 3);
			stream.read(dest, numOfColor * 3);
			break;
		}
		case kFade: {
			// FLA movies don't use cross fade
			// fade out tricky
			if (_fadeOut != 1) {
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
				_fadeOut = 1;
			}
			break;
		}
		case kPlaySample: {
			sample.sampleNum = stream.readSint16LE();
			sample.freq = stream.readSint16LE();
			sample.repeat = stream.readSint16LE();
			sample.dummy = stream.readSByte();
			sample.x = stream.readByte();
			sample.y = stream.readByte();
			_engine->_sound->playFlaSample(sample.sampleNum, sample.repeat, sample.x, sample.y);
			break;
		}
		case kStopSample: {
			_engine->_sound->stopSample(sample.sampleNum);
			break;
		}
		case kDeltaFrame: {
			drawDeltaFrame(stream, FLASCREEN_WIDTH);
			if (_fadeOut == 1) {
				++fadeOutFrames;
			}
			break;
		}
		case kKeyFrame: {
			drawKeyFrame(stream, FLASCREEN_WIDTH, FLASCREEN_HEIGHT);
			break;
		}
		default: {
			break;
		}
		}

		stream.seek(pos + opcodeBlockSize);
	}
}

FlaMovies::FlaMovies(TwinEEngine *engine) : _engine(engine) {}

void FlaMovies::preparePCX(int index) {
	Image::PCXDecoder pcxDecoder;
	Common::SeekableReadStream *stream = HQR::makeReadStream("FLA_PCX.HQR", index);
	if (stream != nullptr) {
		if (!pcxDecoder.loadStream(*stream)) {
			delete stream;
			return;
		}
	}
	const Graphics::Surface *surface = pcxDecoder.getSurface();
	if (surface != nullptr) {
		const Common::Rect srect(0, 0, surface->w, surface->h);
		_engine->frontVideoBuffer.transBlitFrom(*surface, srect, _engine->frontVideoBuffer.getBounds());
	}
	delete stream;
	// TODO FLA_GIF.HQR
}

void FlaMovies::playPCXMovie(const char *flaName) {
	if (!Common::File::exists("FLA_PCX.HQR") || !Common::File::exists("FLA_GIF.HQR")) {
		warning("FLA_PCX file doesn't exist!");
		return;
	}

	// TODO: use the HQR 23th entry (movies informations)
	if (!strcmp(flaName, FLA_INTROD)) {
		preparePCX(1);
		g_system->delayMillis(5000);
		preparePCX(2);
		g_system->delayMillis(5000);
		preparePCX(3);
		g_system->delayMillis(5000);
		preparePCX(4);
		g_system->delayMillis(5000);
		preparePCX(5);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "BAFFE") || !strcmp(flaName, "BAFFE2") || !strcmp(flaName, "BAFFE3") || !strcmp(flaName, "BAFFE4")) {
		preparePCX(6);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "bateau") || !strcmp(flaName, "bateau2")) {
		preparePCX(7);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "flute2")) {
		preparePCX(8);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "navette")) {
		preparePCX(15);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "templebu")) {
		preparePCX(12);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "glass2")) {
		preparePCX(8);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "surf")) {
		preparePCX(9);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "verser") || !strcmp(flaName, "verser2")) {
		preparePCX(10);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "capture")) {
		preparePCX(14); // TODO: same as sendel?
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "neige2")) {
		preparePCX(11);
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "sendel")) {
		preparePCX(14); // TODO: same as capture?
		g_system->delayMillis(5000);
	} else if (!strcmp(flaName, "sendel2")) {
		preparePCX(17);
		g_system->delayMillis(5000);
	}
}

void FlaMovies::playFlaMovie(const char *flaName) {
	_engine->_sound->stopSamples();

	// Play FLA PCX instead of movies
	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAPCX) {
		playPCXMovie(flaName);
		return;
	}

	_engine->_music->stopMusic();

	Common::String fileNamePath = Common::String::format("%s", flaName);
	const size_t n = fileNamePath.findLastOf(".");
	if (n != Common::String::npos) {
		fileNamePath.erase(n);
	}
	fileNamePath += FLA_EXT;

	_fadeOut = -1;
	fadeOutFrames = 0;

	file.close();
	if (!file.open(fileNamePath)) {
		warning("Failed to open fla movie '%s'", fileNamePath.c_str());
		return;
	}

	file.read(&flaHeaderData.version, 6);
	flaHeaderData.numOfFrames = file.readUint32LE();
	flaHeaderData.speed = file.readByte();
	flaHeaderData.var1 = file.readByte();
	flaHeaderData.xsize = file.readUint16LE();
	flaHeaderData.ysize = file.readUint16LE();

	samplesInFla = file.readUint16LE();
	file.skip(2);

	file.skip(4 * samplesInFla);

	if (!strcmp((const char *)flaHeaderData.version, "V1.3")) {
		int32 currentFrame = 0;

		debug("Play fla: %s", flaName);

		ScopedKeyMap scopedKeyMap(_engine, cutsceneKeyMapId);

		do {
			ScopedFPS scopedFps(flaHeaderData.speed);
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}
			if (currentFrame == flaHeaderData.numOfFrames) {
				break;
			}
			processFrame();
			scaleFla2x();
			_engine->frontVideoBuffer.transBlitFrom(_engine->imageBuffer, _engine->imageBuffer.getBounds(), _engine->frontVideoBuffer.getBounds());

			// Only blit to screen if isn't a fade
			if (_fadeOut == -1) {
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				if (currentFrame == 0) {
					// fade in the first frame
					_engine->_screens->fadeIn(_engine->_screens->paletteRGBACustom);
				} else {
					_engine->setPalette(_engine->_screens->paletteRGBACustom);
				}
			}

			// TRICKY: fade in tricky
			if (fadeOutFrames >= 2) {
				_engine->flip();
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				_engine->_screens->fadeToPal(_engine->_screens->paletteRGBACustom);
				_fadeOut = -1;
				fadeOutFrames = 0;
			}

			currentFrame++;
		} while (!_engine->_input->toggleAbortAction());
	}

	if (_engine->cfgfile.CrossFade) {
		_engine->crossFade(_engine->frontVideoBuffer, _engine->_screens->paletteRGBACustom);
	} else {
		_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
	}

	_engine->_sound->stopSamples();
}

} // namespace TwinE
