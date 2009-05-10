#include "shadereditor.h"
#include "gfx.h"
#include "mesh.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabWidget>
#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
    SyntaxHighlighter(QTextEdit *parent) : QSyntaxHighlighter(parent)   {}
protected:
    virtual void highlightBlock ( const QString & text );
    void colorize(const QString &text, QTextCharFormat &tokenFormat, QRegExp expression);
    void colorizeSingleLine(const QString &text, QTextCharFormat &tokenFormat, QRegExp startExpression, QRegExp endExpression);
    void colorizeMultiLine(const QString &text, QTextCharFormat &multiLineCommentFormat, QRegExp startExpression, QRegExp endExpression);
};

ShaderEditor *ShaderEditor::pInstance = NULL;

ShaderEditor *ShaderEditor::instance()
{
    if (!pInstance)
        pInstance = new ShaderEditor;
    return pInstance;
}

ShaderEditor::ShaderEditor()
{
    updating = true;
    fragEdit = new QTextEdit;
    vertEdit = new QTextEdit;
    output = new QTextEdit;
    output->setReadOnly(true);

    fragEdit->setWordWrapMode(QTextOption::NoWrap);
    vertEdit->setWordWrapMode(QTextOption::NoWrap);
    output->setWordWrapMode(QTextOption::NoWrap);
    new SyntaxHighlighter(fragEdit);
    new SyntaxHighlighter(vertEdit);

    setLayout( new QVBoxLayout );
    QTabWidget *tabs = new QTabWidget;
    layout()->addWidget(tabs);

    QWidget *vertexTab = new QWidget;
    QVBoxLayout *curLayout = new QVBoxLayout;
    vertexTab->setLayout( curLayout );
    curLayout->addWidget(vertEdit);
    QHBoxLayout *vertLayout = new QHBoxLayout;
    QPushButton *bSaveVert = new QPushButton(tr("&save vertex program"));
    QPushButton *bLoadVert = new QPushButton(tr("&load vertex program"));
    vertLayout->addWidget(bSaveVert);
    vertLayout->addWidget(bLoadVert);
    curLayout->addLayout(vertLayout);
    tabs->addTab( vertexTab, tr("&Vertex program"));

    QWidget *fragmentTab = new QWidget;
    curLayout = new QVBoxLayout;
    fragmentTab->setLayout( curLayout );
    curLayout->addWidget(fragEdit);
    QHBoxLayout *fragLayout = new QHBoxLayout;
    QPushButton *bSaveFrag = new QPushButton(tr("&save fragment program"));
    QPushButton *bLoadFrag = new QPushButton(tr("&load fragment program"));
    fragLayout->addWidget(bSaveFrag);
    fragLayout->addWidget(bLoadFrag);
    curLayout->addLayout(fragLayout);
    tabs->addTab( fragmentTab, tr("&Fragment program"));

    QWidget *outputTab = new QWidget;
    curLayout = new QVBoxLayout;
    outputTab->setLayout( curLayout );
    curLayout->addWidget(output);

    QPushButton *bBuild = new QPushButton(tr("&Build"));
    bBuild->setMaximumWidth(100);
    curLayout->addWidget(bBuild);
    curLayout->setAlignment(bBuild, Qt::AlignHCenter);
    tabs->addTab( outputTab, tr("&Output"));

    updateWindowTitle();
    resize(600, height());

    connect(vertEdit, SIGNAL(textChanged()), this, SLOT(readData()));
    connect(fragEdit, SIGNAL(textChanged()), this, SLOT(readData()));
    connect(bBuild, SIGNAL(clicked()), this, SLOT(compileShader()));

    connect(bSaveVert, SIGNAL(clicked()), this, SLOT(saveVertexProgram()));
    connect(bLoadVert, SIGNAL(clicked()), this, SLOT(loadVertexProgram()));
    connect(bSaveFrag, SIGNAL(clicked()), this, SLOT(saveFragmentProgram()));
    connect(bLoadFrag, SIGNAL(clicked()), this, SLOT(loadFragmentProgram()));

    updating = false;
}

void ShaderEditor::updateWindowTitle()
{
    int ID = Gfx::instance()->getSelectionID();
    if (ID >= 0)
        setWindowTitle(tr("Shader editor") + " - " + Mesh::instance()->getMesh(ID)->getName());
    else
        setWindowTitle(tr("Shader editor"));
}

void ShaderEditor::updateGUI()
{
    int ID = Gfx::instance()->getSelectionID();
    updating = true;
    if (ID >= 0)
    {
        Mesh *mesh = Mesh::instance()->getMesh(ID);
        fragEdit->setPlainText( mesh->fragmentProgram );
        vertEdit->setPlainText( mesh->vertexProgram );
    }
    else
    {
        fragEdit->clear();
        vertEdit->clear();
    }
    output->clear();
    updating = false;
    updateWindowTitle();
}

void ShaderEditor::readData()
{
    if (updating)
        return;
    int ID = Gfx::instance()->getSelectionID();
    if (ID >= 0)
    {
        Mesh *mesh = Mesh::instance()->getMesh(ID);
        mesh->fragmentProgram = fragEdit->toPlainText();
        mesh->vertexProgram = vertEdit->toPlainText();
    }
}

void ShaderEditor::compileShader()
{
    int ID = Gfx::instance()->getSelectionID();
    if (ID >= 0)
    {
        readData();
        Mesh *mesh = Mesh::instance()->getMesh(ID);
        output->setPlainText( mesh->shader.load_memory( mesh->vertexProgram, mesh->fragmentProgram ) );
        Gfx::instance()->updateGL();
    }
}

void ShaderEditor::saveVertexProgram()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("save vertex progam as"), QString(), tr("all files(*.*);;vertex programs(*.vert)"));
    if (filename.isEmpty())
        return;
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    if (!file.exists() || !file.isOpen())
    {
        QMessageBox::critical(this, tr("Error opening file"), tr("Error: could not open file %1 for writing").arg(filename));
        return;
    }
    file.write(vertEdit->toPlainText().toAscii());
    file.close();
}

void ShaderEditor::loadVertexProgram()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("load vertex progam"), QString(), tr("all files(*.*);;vertex programs(*.vert)"));
    if (filename.isEmpty())
        return;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.exists() || !file.isOpen())
    {
        QMessageBox::critical(this, tr("Error opening file"), tr("Error: could not read file %1").arg(filename));
        return;
    }
    vertEdit->setPlainText(QString(file.readAll()));
    file.close();
    readData();
}

void ShaderEditor::saveFragmentProgram()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("save fragment progam as"), QString(), tr("all files(*.*);;fragment programs(*.frag)"));
    if (filename.isEmpty())
        return;
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    if (!file.exists() || !file.isOpen())
    {
        QMessageBox::critical(this, tr("Error opening file"), tr("Error: could not open file %1 for writing").arg(filename));
        return;
    }
    file.write(fragEdit->toPlainText().toAscii());
    file.close();
}

void ShaderEditor::loadFragmentProgram()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("load fragment progam"), QString(), tr("all files(*.*);;fragment programs(*.frag)"));
    if (filename.isEmpty())
        return;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.exists() || !file.isOpen())
    {
        QMessageBox::critical(this, tr("Error opening file"), tr("Error: could not read file %1").arg(filename));
        return;
    }
    fragEdit->setPlainText(QString(file.readAll()));
    file.close();
    readData();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    if (text.isEmpty())
        return;
    // Comments
    {
        QTextCharFormat tokenFormat;
        tokenFormat.setForeground(QColor::fromRgb(0,0x7F,0));

        colorizeMultiLine(text, tokenFormat, QRegExp("/\\*"), QRegExp("\\*/"));
        colorizeSingleLine(text, tokenFormat, QRegExp("//"), QRegExp("\\n"));
    }
    // basic types
    {
        QTextCharFormat tokenFormat;
        tokenFormat.setForeground(QColor::fromRgb(0x7F,0,0x7F));

        QRegExp token("\\bvoid\\b"
                      "|\\bbool\\b"
                      "|\\bint\\b"
                      "|\\bfloat\\b"
                      "|\\bvec2\\b"
                      "|\\bvec3\\b"
                      "|\\bvec4\\b"
                      "|\\bbvec2\\b"
                      "|\\bbvec3\\b"
                      "|\\bbvec4\\b"
                      "|\\bivec2\\b"
                      "|\\bivec3\\b"
                      "|\\bivec4\\b"
                      "|\\bmat2\\b"
                      "|\\bmat3\\b"
                      "|\\bmat4\\b"
                      "|\\bsampler1D\\b"
                      "|\\bsampler2D\\b"
                      "|\\bsampler3D\\b"
                      "|\\bsamplerCube\\b"
                      "|\\bsampler1DShadow\\b"
                      "|\\bsampler2DShadow\\b");
        colorize(text, tokenFormat, token);
    }
    // type qualifiers
    {
        QTextCharFormat tokenFormat;
        tokenFormat.setForeground(QColor::fromRgb(0x7F,0x7F,0x7F));

        QRegExp token("\\bconst\\b"
                      "|\\buniform\\b"
                      "|\\bvarying\\b"
                      "|\\battribute\\b"
                      "|\\bin\\b"
                      "|\\bout\\b"
                      "|\\binout\\b");
        colorize(text, tokenFormat, token);
    }
    // other keywords
    {
        QTextCharFormat tokenFormat;
        tokenFormat.setForeground(QColor::fromRgb(0x7F,0x7F,0));

        QRegExp token("\\bstruct\\b"
                      "|\\bbreak\\b"
                      "|\\bcontinue\\b"
                      "|\\bdo\\b"
                      "|\\bfor\\b"
                      "|\\bwhile\\b"
                      "|\\bif\\b"
                      "|\\belse\\b"
                      "|\\bdiscard\\b"
                      "|\\breturn\\b");
        colorize(text, tokenFormat, token);
    }
    // reserved
    {
        QTextCharFormat tokenFormat;
        tokenFormat.setForeground(QColor::fromRgb(0xFF,0,0));

        QRegExp token("\\basm\\b"
                      "|\\bclass\\b"
                      "|\\bunion\\b"
                      "|\\benum\\b"
                      "|\\btypedef\\b"
                      "|\\btemplate\\b"
                      "|\\bthis\\b"
                      "|\\bpacked\\b"
                      "|\\bgoto\\b"
                      "|\\bswitch\\b"
                      "|\\bdefault\\b"
                      "|\\binline\\b"
                      "|\\bnoinline\\b"
                      "|\\bvolatile\\b"
                      "|\\bpublic\\b"
                      "|\\bstatic\\b"
                      "|\\bextern\\b"
                      "|\\bexternal\\b"
                      "|\\binterface\\b"
                      "|\\blong\\b"
                      "|\\bshort\\b"
                      "|\\bdouble\\b"
                      "|\\bhalf\\b"
                      "|\\bfixed\\b"
                      "|\\bunsigned\\b"
                      "|\\binput\\b"
                      "|\\boutput\\b"
                      "|\\bhvec2\\b"
                      "|\\bhvec3\\b"
                      "|\\bhvec4\\b"
                      "|\\bdvec2\\b"
                      "|\\bdvec3\\b"
                      "|\\bdvec4\\b"
                      "|\\bfvec2\\b"
                      "|\\bfvec3\\b"
                      "|\\bfvec4\\b"
                      "|\\bsizeof\\b"
                      "|\\bcast\\b"
                      "|\\busing\\b"
                      "|\\bnamespace\\b"
                      "|\\bsampler2DRect\\b"
                      "|\\bsampler3DRect\\b"
                      "|\\bsampler2DRectShadow\\b");
        colorize(text, tokenFormat, token);
    }
    // preprocessor
    {
        QTextCharFormat tokenFormat;
        tokenFormat.setForeground(QColor::fromRgb(0,0,0x7F));

        QRegExp token("\\b#\\b"
                      "|\\b#define\\b"
                      "|\\b#undef\\b"
                      "|\\b#if\\b"
                      "|\\b#ifdef\\b"
                      "|\\b#ifndef\\b"
                      "|\\b#else\\b"
                      "|\\b#elif\\b"
                      "|\\b#endif\\b"
                      "|\\b#error\\b"
                      "|\\b#pragma\\b"
                      "|\\b#extension\\b"
                      "|\\b#version\\b"
                      "|\\b#line\\b");
        colorize(text, tokenFormat, token);
    }
    // Values
    {
        QTextCharFormat tokenFormat;
        tokenFormat.setForeground(QColor::fromRgb(0,0,0x7F));

        QRegExp token("\\btrue\\b"
                      "|\\bfalse\\b"
                      "|\\b\\d+\\b"
                      "|\\b0x\\d+\\b"
                      "|\\b0X\\d+\\b");
        colorize(text, tokenFormat, token);
    }
}

void SyntaxHighlighter::colorize(const QString &text, QTextCharFormat &tokenFormat, QRegExp expression)
{
    int startIndex = 0;
    startIndex = text.indexOf(expression);

    while (startIndex >= 0)
    {
        int tokenLength = expression.matchedLength();
        setFormat(startIndex, tokenLength, tokenFormat);
        startIndex = text.indexOf(expression,
                                  startIndex + tokenLength);
    }
}

void SyntaxHighlighter::colorizeSingleLine(const QString &text, QTextCharFormat &tokenFormat, QRegExp startExpression, QRegExp endExpression)
{
    int startIndex = 0;
    startIndex = text.indexOf(startExpression);

    while (startIndex >= 0)
    {
        int endIndex = text.indexOf(endExpression, startIndex);
        int commentLength;
        if (endIndex == -1)
        {
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex
                            + endExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, tokenFormat);
        startIndex = text.indexOf(startExpression,
                                  startIndex + commentLength);
    }
}

void SyntaxHighlighter::colorizeMultiLine(const QString &text, QTextCharFormat &multiLineCommentFormat, QRegExp startExpression, QRegExp endExpression)
{
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(startExpression);

    while (startIndex >= 0)
    {
        int endIndex = text.indexOf(endExpression, startIndex);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex
                            + endExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(startExpression,
                                  startIndex + commentLength);
    }
}
