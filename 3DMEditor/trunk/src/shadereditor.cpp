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
        fragEdit->setHtml( colorize( mesh->fragmentProgram ) );
        vertEdit->setHtml( colorize( mesh->vertexProgram ) );
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
        QTextCursor vertCursor( vertEdit->textCursor() );
        QTextCursor fragCursor( fragEdit->textCursor() );
        int vertPos = vertCursor.position();
        int fragPos = fragCursor.position();

        Mesh *mesh = Mesh::instance()->getMesh(ID);
        mesh->fragmentProgram = fragEdit->toPlainText();
        mesh->vertexProgram = vertEdit->toPlainText();

        updating = true;
        fragEdit->setHtml( colorize( mesh->fragmentProgram ) );
        vertEdit->setHtml( colorize( mesh->vertexProgram ) );
        vertCursor = vertEdit->textCursor();
        fragCursor = fragEdit->textCursor();
        vertCursor.setPosition(vertPos);
        fragCursor.setPosition(fragPos);
        fragEdit->setTextCursor(fragCursor);
        vertEdit->setTextCursor(vertCursor);
        updating = false;
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

QString ShaderEditor::colorize(const QString &code)
{
    if (code.isEmpty())
        return QString();

    QMap< QString, QString > colorMap;
    // comments
    colorMap.insert("//","<font color=\"#007f00\">");
    // basic types
    colorMap.insert("void","<font color=\"#7f007f\">");
    colorMap.insert("bool","<font color=\"#7f007f\">");
    colorMap.insert("int","<font color=\"#7f007f\">");
    colorMap.insert("float","<font color=\"#7f007f\">");
    colorMap.insert("vec2","<font color=\"#7f007f\">");
    colorMap.insert("vec3","<font color=\"#7f007f\">");
    colorMap.insert("vec4","<font color=\"#7f007f\">");
    colorMap.insert("bvec2","<font color=\"#7f007f\">");
    colorMap.insert("bvec3","<font color=\"#7f007f\">");
    colorMap.insert("bvec4","<font color=\"#7f007f\">");
    colorMap.insert("ivec2","<font color=\"#7f007f\">");
    colorMap.insert("ivec3","<font color=\"#7f007f\">");
    colorMap.insert("ivec4","<font color=\"#7f007f\">");
    colorMap.insert("mat2","<font color=\"#7f007f\">");
    colorMap.insert("mat3","<font color=\"#7f007f\">");
    colorMap.insert("mat4","<font color=\"#7f007f\">");
    colorMap.insert("sampler1D","<font color=\"#7f007f\">");
    colorMap.insert("sampler2D","<font color=\"#7f007f\">");
    colorMap.insert("sampler3D","<font color=\"#7f007f\">");
    colorMap.insert("samplerCube","<font color=\"#7f007f\">");
    colorMap.insert("sampler1DShadow","<font color=\"#7f007f\">");
    colorMap.insert("sampler2DShadow","<font color=\"#7f007f\">");
    // type qualifiers
    colorMap.insert("const","<font color=\"#7f7f7f\">");
    colorMap.insert("uniform","<font color=\"#7f7f7f\">");
    colorMap.insert("varying","<font color=\"#7f7f7f\">");
    colorMap.insert("attribute","<font color=\"#7f7f7f\">");
    colorMap.insert("in","<font color=\"#7f7f7f\">");
    colorMap.insert("out","<font color=\"#7f7f7f\">");
    colorMap.insert("inout","<font color=\"#7f7f7f\">");
    // other keywords
    colorMap.insert("struct","<font color=\"#7f7f00\">");
    colorMap.insert("break","<font color=\"#7f7f00\">");
    colorMap.insert("continue","<font color=\"#7f7f00\">");
    colorMap.insert("do","<font color=\"#7f7f00\">");
    colorMap.insert("for","<font color=\"#7f7f00\">");
    colorMap.insert("while","<font color=\"#7f7f00\">");
    colorMap.insert("if","<font color=\"#7f7f00\">");
    colorMap.insert("else","<font color=\"#7f7f00\">");
    colorMap.insert("discard","<font color=\"#7f7f00\">");
    colorMap.insert("return","<font color=\"#7f7f00\">");
    // reserved
    colorMap.insert("asm","<font color=\"#ff0000\">");
    colorMap.insert("class","<font color=\"#ff0000\">");
    colorMap.insert("union","<font color=\"#ff0000\">");
    colorMap.insert("enum","<font color=\"#ff0000\">");
    colorMap.insert("typedef","<font color=\"#ff0000\">");
    colorMap.insert("template","<font color=\"#ff0000\">");
    colorMap.insert("this","<font color=\"#ff0000\">");
    colorMap.insert("packed","<font color=\"#ff0000\">");
    colorMap.insert("goto","<font color=\"#ff0000\">");
    colorMap.insert("switch","<font color=\"#ff0000\">");
    colorMap.insert("default","<font color=\"#ff0000\">");
    colorMap.insert("inline","<font color=\"#ff0000\">");
    colorMap.insert("noinline","<font color=\"#ff0000\">");
    colorMap.insert("volatile","<font color=\"#ff0000\">");
    colorMap.insert("public","<font color=\"#ff0000\">");
    colorMap.insert("static","<font color=\"#ff0000\">");
    colorMap.insert("extern","<font color=\"#ff0000\">");
    colorMap.insert("external","<font color=\"#ff0000\">");
    colorMap.insert("interface","<font color=\"#ff0000\">");
    colorMap.insert("long","<font color=\"#ff0000\">");
    colorMap.insert("short","<font color=\"#ff0000\">");
    colorMap.insert("double","<font color=\"#ff0000\">");
    colorMap.insert("half","<font color=\"#ff0000\">");
    colorMap.insert("fixed","<font color=\"#ff0000\">");
    colorMap.insert("unsigned","<font color=\"#ff0000\">");
    colorMap.insert("input","<font color=\"#ff0000\">");
    colorMap.insert("output","<font color=\"#ff0000\">");
    colorMap.insert("hvec2","<font color=\"#ff0000\">");
    colorMap.insert("hvec3","<font color=\"#ff0000\">");
    colorMap.insert("hvec4","<font color=\"#ff0000\">");
    colorMap.insert("dvec2","<font color=\"#ff0000\">");
    colorMap.insert("dvec3","<font color=\"#ff0000\">");
    colorMap.insert("dvec4","<font color=\"#ff0000\">");
    colorMap.insert("fvec2","<font color=\"#ff0000\">");
    colorMap.insert("fvec3","<font color=\"#ff0000\">");
    colorMap.insert("fvec4","<font color=\"#ff0000\">");
    colorMap.insert("sizeof","<font color=\"#ff0000\">");
    colorMap.insert("cast","<font color=\"#ff0000\">");
    colorMap.insert("using","<font color=\"#ff0000\">");
    colorMap.insert("namespace","<font color=\"#ff0000\">");
    colorMap.insert("sampler2DRect","<font color=\"#ff0000\">");
    colorMap.insert("sampler3DRect","<font color=\"#ff0000\">");
    colorMap.insert("sampler2DRectShadow","<font color=\"#ff0000\">");
    // preprocessor
    colorMap.insert("#","<font color=\"#00007f\">");
    colorMap.insert("#define","<font color=\"#00007f\">");
    colorMap.insert("#undef","<font color=\"#00007f\">");
    colorMap.insert("#if","<font color=\"#00007f\">");
    colorMap.insert("#ifdef","<font color=\"#00007f\">");
    colorMap.insert("#ifndef","<font color=\"#00007f\">");
    colorMap.insert("#else","<font color=\"#00007f\">");
    colorMap.insert("#elif","<font color=\"#00007f\">");
    colorMap.insert("#endif","<font color=\"#00007f\">");
    colorMap.insert("#error","<font color=\"#00007f\">");
    colorMap.insert("#pragma","<font color=\"#00007f\">");
    colorMap.insert("#extension","<font color=\"#00007f\">");
    colorMap.insert("#version","<font color=\"#00007f\">");
    colorMap.insert("#line","<font color=\"#00007f\">");
    // true/false
    colorMap.insert("true","<font color=\"#00007f\">");
    colorMap.insert("false","<font color=\"#00007f\">");

    QString buf("<html>\n<head></head>\n<body>\n");
    QString currentWord;
    int commentMode = 0;
    buf.reserve(code.size());
    for(int i = 0 ; i < code.size() ; i++)
    {
        QChar c = code[i];
        if ((commentMode == 1 && c != '\n') || commentMode == 2)
        {
            if (c == '\n')
                currentWord += "<br>\n";
            else if (c.isSpace())
                currentWord += "&nbsp;";
            else
                currentWord += c;
            if (commentMode == 2 && currentWord.endsWith("*/"))
            {
                buf += colorMap.find("//").value();
                buf += currentWord;
                buf += "</font>";
                commentMode = 0;
                currentWord.clear();
            }
        }
        else if (c.isSpace() || c == '\n')
        {
            QString prefix;
            QString postfix;
            if (commentMode)
                prefix = colorMap.find("//").value();
            else if (colorMap.find(currentWord) != colorMap.end())
                prefix = colorMap.find(currentWord).value();
            if (!prefix.isEmpty())
                postfix = "</font>";
            buf += prefix;
            buf += currentWord;
            buf += postfix;
            if (c == '\n')
                buf += "<br>\n";
            else if (c.isSpace())
                buf += "&nbsp;";
            else
                buf += c;
            currentWord.clear();
            commentMode = 0;
        }
        else
        {
            currentWord += c;
            if (currentWord.startsWith("//"))
                commentMode = 1;
            else if (currentWord.startsWith("/*"))
                commentMode = 2;
        }
    }
    if (!currentWord.isEmpty())
    {
        QString prefix;
        QString postfix;
        if (commentMode)
            prefix = colorMap.find("//").value();
        else if (colorMap.find(currentWord) != colorMap.end())
            prefix = colorMap.find(currentWord).value();
        if (!prefix.isEmpty())
            postfix = "</font>";
        buf += prefix;
        buf += currentWord;
        buf += postfix;
    }
    buf += "</body>\n</html>";
    return buf;
}
