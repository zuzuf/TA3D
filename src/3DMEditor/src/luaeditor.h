#ifndef LUAEDITOR_H
#define LUAEDITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QTextStream>

class LuaEditor : public QWidget
{
    Q_OBJECT;
public:
    LuaEditor();
    inline QTextStream &getStream() {   return stream;  }

private:
    QTextEdit *code;
    QTextEdit *output;
    bool      updating;
    QString   logs;
    QTextStream stream;

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
