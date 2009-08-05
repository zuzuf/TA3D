#ifndef LOGS_H
#define LOGS_H

#include "luaeditor.h"

#define LOG_ERROR(x) { (LuaEditor::instance()->getStream() << "[error] " << x); LuaEditor::instance()->updateGUI(); }
#define LOG_DEBUG(x) { (LuaEditor::instance()->getStream() << "[debug] " << x); LuaEditor::instance()->updateGUI(); }
#define LOG_INFO(x) { (LuaEditor::instance()->getStream() << "[info] " << x); LuaEditor::instance()->updateGUI(); }
#define LOG_CRITICAL(x) { (LuaEditor::instance()->getStream() << "[critical] " << x); LuaEditor::instance()->updateGUI(); }

#define LOG_PREFIX_SCRIPT "[script] "
#define LOG_PREFIX_LUA "[lua] "

#endif // LOGS_H
