#ifndef LUAEDITOR_H
#define LUAEDITOR_H

#include <QMainWindow>
#include <QTextStream>
#include <QTimer>

class QPushButton;
class QTextEdit;
class QLineEdit;

class LuaEditor : public QMainWindow
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
    QTimer    luaTimer;
    QPushButton *bRun;
    QLineEdit *commandInput;
	QString filename;

public slots:
    void updateWindowTitle();
    void updateGUI();
    void compileCode();
    void saveProgram();
	void quickSave();
    void loadProgram();
    void runLuaCode();
    void runLuaCommand();
    void toggleTimer(bool);
    void loadTemplate();

public:
    static LuaEditor *instance();

private:
    static LuaEditor *pInstance;
};

#endif // LUAEDITOR_H
