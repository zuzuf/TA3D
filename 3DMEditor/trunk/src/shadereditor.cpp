#include "shadereditor.h"
#include "gfx.h"
#include "mesh.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

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

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Vertex program:")));
    layout->addWidget(vertEdit);
    layout->addWidget(new QLabel(tr("Fragment program:")));
    layout->addWidget(fragEdit);
    layout->addWidget(new QLabel(tr("Output:")));
    layout->addWidget(output);

    QPushButton *bBuild = new QPushButton(tr("&Build"));
    bBuild->setMaximumWidth(100);
    layout->addWidget(bBuild);
    layout->setAlignment(bBuild, Qt::AlignHCenter);

    setLayout(layout);

    updateWindowTitle();
    resize(600, height());

    connect(vertEdit, SIGNAL(textChanged()), this, SLOT(readData()));
    connect(fragEdit, SIGNAL(textChanged()), this, SLOT(readData()));
    connect(bBuild, SIGNAL(clicked()), this, SLOT(compileShader()));

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
        fragEdit->setPlainText(mesh->fragmentProgram);
        vertEdit->setPlainText(mesh->vertexProgram);
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
