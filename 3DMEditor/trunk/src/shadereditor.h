#ifndef SHADEREDITOR_H
#define SHADEREDITOR_H

#include <QWidget>
#include <QTextEdit>

class ShaderEditor : public QWidget
{
    Q_OBJECT;
public:
    ShaderEditor();

private:
    QTextEdit *fragEdit;
    QTextEdit *vertEdit;
    QTextEdit *output;
    bool      updating;

public slots:
    void updateWindowTitle();
    void updateGUI();
    void readData();
    void compileShader();
    void saveFragmentProgram();
    void saveVertexProgram();
    void loadFragmentProgram();
    void loadVertexProgram();

public:
    static ShaderEditor *instance();

private:
    static ShaderEditor *pInstance;
};

#endif // SHADEREDITOR_H
