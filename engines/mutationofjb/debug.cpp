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

#include "mutationofjb/debug.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/script.h"
#include "mutationofjb/commands/command.h"
#include "mutationofjb/commands/seqcommand.h"
#include "mutationofjb/commands/conditionalcommand.h"
#include "mutationofjb/commands/callmacrocommand.h"
#include "common/debug-channels.h"
#include "common/translation.h"
#include "common/scummsys.h"

namespace MutationOfJB {

/* Converts CP895 string to 7bit ASCII, so we can show it in the console. */
static Common::String convertToASCII(const Common::String &str) {
	static const char conversionTable[] = {
		'C', 'u', 'e', 'd', 'a', 'D', 'T', 'c', 'e', 'E', 'L', 'I', 'l', 'l', 'A', 'A', /* 0x80-0x8F */
		'E', 'z', 'Z', 'o', 'o', 'O', 'u', 'U', 'y', 'O', 'U', 'S', 'L', 'Y', 'R', 't', /* 0x90-0x9F */
		'a', 'i', 'o', 'u', 'n', 'N', 'U', 'O', 's', 'r', 'r', 'R'						/* 0xA0-0xAB */
	};

	Common::String ret = str;
	for (Common::String::iterator it = ret.begin(); it != ret.end(); ++it) {
		const byte cp895Byte = reinterpret_cast<const byte &>(*it);
		if (cp895Byte >= 0x80 && cp895Byte <= 0xAB) {
			*it = conversionTable[cp895Byte - 0x80];
		} else if (cp895Byte == 0xE1) { // ß
			*it = 's';
		}
	}
	return ret;
}

Console::Console(MutationOfJBEngine *vm) : _vm(vm) {
	registerCmd("showallcommands", WRAP_METHOD(Console, cmd_showallcommands));
	registerCmd("listsections", WRAP_METHOD(Console, cmd_listsections));
	registerCmd("showsection", WRAP_METHOD(Console, cmd_showsection));
	registerCmd("listmacros", WRAP_METHOD(Console, cmd_listmacros));
	registerCmd("showmacro", WRAP_METHOD(Console, cmd_showmacro));
	registerCmd("liststartups", WRAP_METHOD(Console, cmd_liststartups));
	registerCmd("showstartup", WRAP_METHOD(Console, cmd_showstartup));
	registerCmd("changescene", WRAP_METHOD(Console, cmd_changescene));
	registerCmd("dumpsceneinfo", WRAP_METHOD(Console, cmd_dumpsceneinfo));
	registerCmd("dumpdoorinfo", WRAP_METHOD(Console, cmd_dumpdoorinfo));
	registerCmd("dumpobjectinfo", WRAP_METHOD(Console, cmd_dumpobjectinfo));
	registerCmd("dumpstaticinfo", WRAP_METHOD(Console, cmd_dumpstaticinfo));
}

bool Console::cmd_showallcommands(int argc, const char **argv) {
	if (argc == 2) {
		Script *const script = getScriptFromArg(argv[1]);
		if (script) {
			const Commands &commands = script->getAllCommands();

			for (Commands::const_iterator it = commands.begin(); it != commands.end(); ++it) {
				debugPrintf("%s\n", convertToASCII((*it)->debugString()).c_str());
			}
		}
	} else {
		debugPrintf(_("showallcommands <G|L>\n"));
	}

	return true;
}

bool Console::cmd_listsections(int argc, const char **argv) {
	if (argc == 3) {
		Script *const script = getScriptFromArg(argv[1]);
		if (script) {
			ActionInfo::Action action;
			const char *word = nullptr;
			if (strcmp(argv[2], "L") == 0) {
				action = ActionInfo::Look;
				word = _("Look");
			} else if (strcmp(argv[2], "W") == 0) {
				action = ActionInfo::Walk;
				word = _("Walk");
			} else if (strcmp(argv[2], "T") == 0) {
				action = ActionInfo::Talk;
				word = _("Talk");
			} else if (strcmp(argv[2], "U") == 0) {
				action = ActionInfo::Use;
				word = _("Use");
			} else if (strcmp(argv[2], "P") == 0) {
				action = ActionInfo::PickUp;
				word = _("Pick up");
			} else {
				debugPrintf(_("Choose 'L' (look), 'W' (walk), 'T' (talk), 'U' (use) or 'P' (pick up).\n"));
			}
			if (word) {
				const ActionInfos &actionInfos = script->getActionInfos(action);
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (action != ActionInfo::Use || actionInfo._entity2Name.empty()) {
						debugPrintf("%s %s\n", word, convertToASCII(actionInfo._entity1Name).c_str());
					} else {
						debugPrintf("%s %s %s\n", word, convertToASCII(actionInfo._entity1Name).c_str(), convertToASCII(actionInfo._entity2Name).c_str());
					}
				}
			}
		}
	} else {
		debugPrintf(_("listsections <G|L> <L|W|T|U|P>\n"));
	}
	return true;
}

void Console::showIndent(int indentLevel) {
	for (int i = 0; i < indentLevel; ++i) {
		debugPrintf("  ");
	}
}

void Console::showCommands(Command *command, int indentLevel) {
	while (command) {
		showIndent(indentLevel);
		debugPrintf("%s\n", convertToASCII(command->debugString()).c_str());

		if (SeqCommand *const seqCmd = dynamic_cast<SeqCommand *>(command)) {
			command = seqCmd->next();
		} else if (ConditionalCommand *const condCmd = dynamic_cast<ConditionalCommand *>(command)) {
			showCommands(condCmd->getTrueCommand(), indentLevel + 1);
			showIndent(indentLevel);
			debugPrintf("ELSE\n");
			showCommands(condCmd->getFalseCommand(), indentLevel + 1);
			command = nullptr;
		} else if (CallMacroCommand* const callMacroCmd = dynamic_cast<CallMacroCommand *>(command)) {
			command = callMacroCmd->getReturnCommand();
		} else {
			command = nullptr;
		}
	}
}

bool Console::cmd_showsection(int argc, const char **argv) {
	if (argc >= 4) {
		Script *const script = getScriptFromArg(argv[1]);
		if (script) {
			Command *command = nullptr;
			ActionInfo::Action action;
			bool correctAction = true;
			bool found = false;

			if (strcmp(argv[2], "L") == 0) {
				action = ActionInfo::Look;
			} else if (strcmp(argv[2], "W") == 0) {
				action = ActionInfo::Walk;
			} else if (strcmp(argv[2], "T") == 0) {
				action = ActionInfo::Talk;
			} else if (strcmp(argv[2], "U") == 0) {
				action = ActionInfo::Use;
			} else if (strcmp(argv[2], "P") == 0) {
				action = ActionInfo::PickUp;
			} else {
				debugPrintf(_("Choose 'L' (look), 'W' (walk), 'T' (talk), 'U' (use) or 'P' (pick up).\n"));
				correctAction = false;
			}

			if (correctAction) {
				const ActionInfos &actionInfos = script->getActionInfos(action);
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (convertToASCII(actionInfo._entity1Name) == argv[3] && (action != ActionInfo::Use || ((argc == 4 && actionInfo._entity2Name.empty()) || (argc > 4 && convertToASCII(actionInfo._entity2Name) == argv[4])))) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}

				if (found) {
					if (command) {
						showCommands(command);
					}
				} else {
					debugPrintf("Section not found.\n");
				}
			}
		}
	} else {
		debugPrintf(_("showsection <G|L> <L|W|T|U|P> <sectionname>\n"));
	}

	return true;
}

bool Console::cmd_listmacros(int argc, const char **argv) {
	if (argc == 2) {
		Script *const script = getScriptFromArg(argv[1]);
		if (script) {
			const Macros &macros = script->getMacros();
			for (Macros::const_iterator it = macros.begin(); it != macros.end(); ++it) {
				debugPrintf("%s\n", it->_key.c_str());
			}
		}
	} else {
		debugPrintf(_("listmacros <G|L>\n"));
	}

	return true;
}

bool Console::cmd_showmacro(int argc, const char **argv) {
	if (argc == 3) {
		Script *script = nullptr;
		if (strcmp(argv[1], "G") == 0) {
			script = _vm->getGame().getGlobalScript();
		} else if (strcmp(argv[1], "L") == 0) {
			script = _vm->getGame().getLocalScript();
		}
		if (!script) {
			debugPrintf(_("Choose 'G' (global) or 'L' (local) script.\n"));
		} else {
			const Macros &macros = script->getMacros();
			Macros::const_iterator itMacro = macros.find(argv[2]);
			if (itMacro != macros.end()) {
				if (itMacro->_value) {
					showCommands(itMacro->_value);
				}
			} else {
				debugPrintf("Macro not found.\n");
			}
		}
	} else {
		debugPrintf(_("showmacro <G|L> <macroname>\n"));
	}

	return true;
}

bool Console::cmd_liststartups(int argc, const char **argv) {
	if (argc == 2) {
		Script *const script = getScriptFromArg(argv[1]);
		if (script) {
			const Startups &startups = script->getStartups();
			for (Startups::const_iterator it = startups.begin(); it != startups.end(); ++it) {
				debugPrintf("%u\n", (unsigned int) it->_key);
			}
		}
	} else {
		debugPrintf(_("liststartups <G|L>\n"));
	}

	return true;
}

bool Console::cmd_showstartup(int argc, const char **argv) {
	if (argc == 3) {
		Script *const script = getScriptFromArg(argv[1]);
		if (script) {
			const Startups &startups = script->getStartups();
			Startups::const_iterator itMacro = startups.find((uint8) atoi(argv[2]));
			if (itMacro != startups.end()) {
				if (itMacro->_value) {
					showCommands(itMacro->_value);
				}
			} else {
				debugPrintf("Startup not found.\n");
			}
		}
	} else {
		debugPrintf(_("showstartup <G|L> <startupid>\n"));
	}

	return true;
}

bool Console::cmd_changescene(int argc, const char **argv) {
	if (argc == 2) {
		const uint8 sceneId = atoi(argv[1]);
		const bool partB = argv[1][strlen(argv[1]) - 1] == 'B';

		_vm->getGame().changeScene(sceneId, partB);
	} else {
		debugPrintf(_("changescene <scenename>\n"));
	}

	return true;
}

bool Console::cmd_dumpsceneinfo(int argc, const char **argv) {
	if (argc == 2) {
		const uint8 sceneId = atoi(argv[1]);
		Scene *scene = _vm->getGame().getGameData().getScene(sceneId);
		if (scene) {
			debugPrintf("Startup: %u\n", (unsigned int) scene->_startup);
			debugPrintf("Delay: %u\n", (unsigned int) scene->_DL);
			debugPrintf("Doors: %u\n", (unsigned int) scene->_noDoors);
			debugPrintf("Objects: %u\n", (unsigned int) scene->_noObjects);
			debugPrintf("Statics: %u\n", (unsigned int) scene->_noStatics);
			debugPrintf("ObstacleY1: %u\n", (unsigned int) scene->_obstacleY1);
			debugPrintf("PalRotStart: %u\n", (unsigned int) scene->_palRotStart);
			debugPrintf("PalRotEnd: %u\n", (unsigned int) scene->_palRotEnd);
			debugPrintf("PalRotPeriod: %u\n", (unsigned int) scene->_palRotPeriod);
		} else {
			debugPrintf(_("Scene %u not found.\n"), (unsigned int) sceneId);
		}
	} else {
		debugPrintf(_("dumpsceneinfo <sceneid>\n"));
	}

	return true;
}

bool Console::cmd_dumpdoorinfo(int argc, const char **argv) {
	if (argc == 3) {
		const uint8 sceneId = atoi(argv[1]);
		const uint8 doorId = atoi(argv[2]);

		Scene *const scene = _vm->getGame().getGameData().getScene(sceneId);
		if (scene) {
			Door *const door = scene->getDoor(doorId);
			if (door) {
				debugPrintf("Name: '%s'\n", convertToASCII(door->_name).c_str());
				debugPrintf("DestSceneId: %u\n", (unsigned int) door->_destSceneId);
				debugPrintf("DestX: %u\n", (unsigned int) door->_destX);
				debugPrintf("DestY: %u\n", (unsigned int) door->_destY);
				debugPrintf("X: %u\n", (unsigned int) door->_x);
				debugPrintf("Y: %u\n", (unsigned int) door->_y);
				debugPrintf("Width: %u\n", (unsigned int) door->_width);
				debugPrintf("Height: %u\n", (unsigned int) door->_height);
				debugPrintf("WalkToX: %u\n", (unsigned int) door->_walkToX);
				debugPrintf("WalkToY: %u\n", (unsigned int) door->_walkToY);
				debugPrintf("SP: %u\n", (unsigned int) door->_SP);
			} else {
				debugPrintf(_("Door %u not found.\n"), (unsigned int) doorId);
			}
		} else {
			debugPrintf(_("Scene %u not found.\n"), (unsigned int) sceneId);
		}
	} else {
		debugPrintf(_("dumpdoorinfo <sceneid> <doorid>\n"));
	}

	return true;
}
bool Console::cmd_dumpobjectinfo(int argc, const char **argv) {
	if (argc == 3) {
		const uint8 sceneId = atoi(argv[1]);
		const uint8 objectId = atoi(argv[2]);

		Scene *const scene = _vm->getGame().getGameData().getScene(sceneId);
		if (scene) {
			Object *const object = scene->getObject(objectId);
			if (object) {
				debugPrintf("AC: %u\n", (unsigned int) object->_AC);
				debugPrintf("FA: %u\n", (unsigned int) object->_FA);
				debugPrintf("FR: %u\n", (unsigned int) object->_FR);
				debugPrintf("NA: %u\n", (unsigned int) object->_NA);
				debugPrintf("FS: %u\n", (unsigned int) object->_FS);
				debugPrintf("Unknown: %u\n", (unsigned int) object->_unknown);
				debugPrintf("CA: %u\n", (unsigned int) object->_CA);
				debugPrintf("X: %u\n", (unsigned int) object->_x);
				debugPrintf("Y: %u\n", (unsigned int) object->_y);
				debugPrintf("XL: %u\n", (unsigned int) object->_XL);
				debugPrintf("YL: %u\n", (unsigned int) object->_YL);
				debugPrintf("WX: %u\n", (unsigned int) object->_WX);
				debugPrintf("WY: %u\n", (unsigned int) object->_WY);
				debugPrintf("SP: %u\n", (unsigned int) object->_SP);
			} else {
				debugPrintf(_("Object %u not found.\n"), (unsigned int) objectId);
			}
		} else {
			debugPrintf(_("Scene %u not found.\n"), (unsigned int) sceneId);
		}
	} else {
		debugPrintf(_("dumpobjectinfo <sceneid> <objectid>\n"));
	}

	return true;
}

bool Console::cmd_dumpstaticinfo(int argc, const char **argv) {
	if (argc == 3) {
		const uint8 sceneId = atoi(argv[1]);
		const uint8 staticId = atoi(argv[2]);

		Scene *const scene = _vm->getGame().getGameData().getScene(sceneId);
		if (scene) {
			Static *const stat = scene->getStatic(staticId, true);
			if (stat) {
				debugPrintf("Active: %u\n", (unsigned int) stat->_active);
				debugPrintf("Name: '%s'\n", convertToASCII(stat->_name).c_str());
				debugPrintf("X: %u\n", (unsigned int) stat->_x);
				debugPrintf("Y: %u\n", (unsigned int) stat->_y);
				debugPrintf("Width: %u\n", (unsigned int) stat->_width);
				debugPrintf("Height: %u\n", (unsigned int) stat->_height);
				debugPrintf("WalkToX: %u\n", (unsigned int) stat->_walkToY);
				debugPrintf("WalkToY: %u\n", (unsigned int) stat->_walkToX);
				debugPrintf("WalkToFrame: %u\n", (unsigned int) stat->_SP);
			} else {
				debugPrintf(_("Static %u not found.\n"), (unsigned int) staticId);
			}
		} else {
			debugPrintf(_("Scene %u not found.\n"), (unsigned int) sceneId);
		}
	} else {
		debugPrintf(_("dumpstaticinfo <sceneid> <staticid>\n"));
	}

	return true;
}

Script *Console::getScriptFromArg(const char *arg) {
	Script *script = nullptr;
	if (strcmp(arg, "G") == 0) {
		script = _vm->getGame().getGlobalScript();
	} else if (strcmp(arg, "L") == 0) {
		script = _vm->getGame().getLocalScript();
	}
	if (!script) {
		debugPrintf(_("Choose 'G' (global) or 'L' (local) script.\n"));
	}

	return script;
}

}
