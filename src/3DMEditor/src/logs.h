#ifndef LOGS_H
#define LOGS_H

#include "luaeditor.h"

#define LOG_ERROR(x) { (LuaEditor::instance()->getStream() << "[error] " << x << "\n"); LuaEditor::instance()->updateGUI(); }
#define LOG_DEBUG(x) { (LuaEditor::instance()->getStream() << "[debug] " << x << "\n"); LuaEditor::instance()->updateGUI(); }
#define LOG_INFO(x) { (LuaEditor::instance()->getStream() << "[info] " << x << "\n"); LuaEditor::instance()->updateGUI(); }
#define LOG_CRITICAL(x) { (LuaEditor::instance()->getStream() << "[critical] " << x << "\n"); LuaEditor::instance()->updateGUI(); }

#define LOG_PREFIX_SCRIPT "[script] "
#define LOG_PREFIX_LUA "[lua] "

#endif // LOGS_H
