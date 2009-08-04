#ifndef LUAEDITOR_H
#define LUAEDITOR_H

#include <QWidget>
#include <QTextEdit>

class LuaEditor : public QWidget
{
    Q_OBJECT;
public:
    LuaEditor();

private:
    QTextEdit *code;
    QTextEdit *output;
    bool      updating;

public slots:
    void updateWindowTitle();
    void updateGUI();
    void compileCode();
    void saveProgram();
    void loadProgram();

public:
    static LuaEditor *instance();

private:
    static LuaEditor *pInstance;
};

#endif // LUAEDITOR_H
