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
 * $URL$
 * $Id$
 *
 */

#include "sci/tools.h"
#include "sci/sfx/player/new_player.h"
#include "sci/sfx/sequencer.h"
#include "sci/sfx/iterator.h"
#include "sci/sfx/core.h"

#include "common/system.h"

#include "sci/sfx/softseq/pcjr.h"
#include "sci/sfx/softseq/adlib.h"

namespace Sci {

// TODO: Turn the following static vars into member vars
static MidiPlayer *mididrv;

static SongIterator *play_it = NULL;
static Audio::Timestamp wakeup_time;
static Audio::Timestamp current_time;
static uint32 play_pause_diff;

static int play_paused = 0;
static int play_it_done = 0;
static uint32 tempo;

static Common::Mutex *mutex;
static int volume = 15;

static void play_song(SongIterator *it) {
	while (play_it && wakeup_time.msecsDiff(current_time) <= 0) {
		int delay;
		byte buf[8];
		int result;

		switch ((delay = songit_next(&(play_it),
		                             buf, &result,
		                             IT_READER_MASK_ALL
		                             | IT_READER_MAY_FREE
		                             | IT_READER_MAY_CLEAN))) {

		case SI_FINISHED:
			delete play_it;
			play_it = NULL;
			play_it_done = 1;
			return;

		case SI_IGNORE:
		case SI_LOOP:
		case SI_RELATIVE_CUE:
		case SI_ABSOLUTE_CUE:
			break;

		case SI_PCM:
			sfx_play_iterator_pcm(play_it, 0);
			break;

		case 0:
			static_cast<MidiDriver *>(mididrv)->send(buf[0], buf[1], buf[2]);

			break;

		default:
			wakeup_time = wakeup_time.addFrames(delay);
		}
	}
}

void NewPlayer::tell_synth(int buf_nr, byte *buf) {
	byte op1 = (buf_nr < 2 ? 0 : buf[1]);
	byte op2 = (buf_nr < 3 ? 0 : buf[2]);

	static_cast<MidiDriver *>(mididrv)->send(buf[0], op1, op2);
}

static void player_timer_callback(void *refCon) {
	Common::StackLock lock(*mutex);

	if (play_it && !play_it_done && !play_paused) {
		play_song(play_it);
	}

	current_time = current_time.addFrames(1);
}

/* API implementation */

Common::Error NewPlayer::init(ResourceManager *resmgr, int expected_latency) {
	MidiDriverType musicDriver = MidiDriver::detectMusicDriver(MDT_PCSPK | MDT_ADLIB);

	switch(musicDriver) {
	case MD_ADLIB:
		mididrv = new MidiPlayer_Adlib();
		break;
	case MD_PCJR:
		mididrv = new MidiPlayer_PCJr();
		break;
	case MD_PCSPK:
		mididrv = new MidiPlayer_PCSpeaker();
		break;
	default:
		break;
	}

	assert(mididrv);

	this->polyphony = mididrv->getPolyphony();

	tempo = mididrv->getBaseTempo();
    uint32 time = g_system->getMillis();
	current_time = Audio::Timestamp(time, 1000000 / tempo);
	wakeup_time = Audio::Timestamp(time, SFX_TICKS_PER_SEC);

	mutex = new Common::Mutex();

	mididrv->setTimerCallback(NULL, player_timer_callback);
	mididrv->open(resmgr);
	mididrv->setVolume(volume);

	return Common::kNoError;
}

Common::Error NewPlayer::add_iterator(SongIterator *it, uint32 start_time) {
	mutex->lock();
	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(mididrv->getPlayMask()));
	SIMSG_SEND(it, SIMSG_SET_RHYTHM(mididrv->hasRhythmChannel()));

	if (play_it == NULL) {
		// Resync with clock
		current_time = Audio::Timestamp(g_system->getMillis(), 1000000 / tempo);
		wakeup_time = Audio::Timestamp(start_time, SFX_TICKS_PER_SEC);
	}

	play_it = sfx_iterator_combine(play_it, it);
	play_it_done = 0;
	mutex->unlock();

	return Common::kNoError;
}

Common::Error NewPlayer::stop(void) {
	debug(3, "Player: Stopping song iterator %p", (void *)play_it);
	mutex->lock();
	delete play_it;
	play_it = NULL;
	for (int i = 0; i < MIDI_CHANNELS; i++)
		static_cast<MidiDriver *>(mididrv)->send(0xb0 + i, SCI_MIDI_CHANNEL_NOTES_OFF, 0);
	mutex->unlock();

	return Common::kNoError;
}

Common::Error NewPlayer::iterator_message(const SongIterator::Message &msg) {
	Common::StackLock lock(*mutex);
	if (!play_it) {
		return Common::kUnknownError;
	}

	songit_handle_message(&play_it, msg);

	return Common::kNoError;
}

Common::Error NewPlayer::pause(void) {
	Common::StackLock lock(*mutex);

	play_paused = 1;
	play_pause_diff = wakeup_time.msecsDiff(current_time);

	mididrv->playSwitch(false);

	return Common::kNoError;
}

Common::Error NewPlayer::resume(void) {
	Common::StackLock lock(*mutex);

	wakeup_time = Audio::Timestamp(current_time.msecs() + play_pause_diff, SFX_TICKS_PER_SEC);
	mididrv->playSwitch(true);
	play_paused = 0;

	return Common::kNoError;
}

Common::Error NewPlayer::exit(void) {
	mididrv->close();
	delete mididrv;
	delete mutex;
	delete play_it;
	play_it = NULL;

	return Common::kNoError;
}

NewPlayer::NewPlayer() {
	name = "new";
	version = "0.1";
}

} // End of namespace Sci
