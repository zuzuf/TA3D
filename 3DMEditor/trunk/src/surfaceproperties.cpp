#include "surfaceproperties.h"
#include "shadereditor.h"
#include "gfx.h"
#include "mesh.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

SurfaceProperties *SurfaceProperties::pInstance = NULL;

SurfaceProperties *SurfaceProperties::instance()
{
    if (!pInstance)
        pInstance = new SurfaceProperties;
    return pInstance;
}

SurfaceProperties::SurfaceProperties()
{
    updating = true;
    red = new QSpinBox;
    green = new QSpinBox;
    blue = new QSpinBox;
    alpha = new QSpinBox;
    redR = new QSpinBox;
    greenR = new QSpinBox;
    blueR = new QSpinBox;
    alphaR = new QSpinBox;

    red->setMinimum(0);
    red->setMaximum(255);
    green->setMinimum(0);
    green->setMaximum(255);
    blue->setMinimum(0);
    blue->setMaximum(255);
    alpha->setMinimum(0);
    alpha->setMaximum(255);
    redR->setMinimum(0);
    redR->setMaximum(255);
    greenR->setMinimum(0);
    greenR->setMaximum(255);
    blueR->setMinimum(0);
    blueR->setMaximum(255);
    alphaR->setMinimum(0);
    alphaR->setMaximum(255);

    flagBlended = new QCheckBox(tr("Blended"));
    flagGLSL = new QCheckBox(tr("GLSL"));
    flagGouraud = new QCheckBox(tr("Gouraud"));
    flagLighted = new QCheckBox(tr("Lighted"));
    flagPlayerColor = new QCheckBox(tr("Player Color"));
    flagReflec = new QCheckBox(tr("Reflective"));
    flagTextured = new QCheckBox(tr("Textured"));

    QFormLayout *colorLayout = new QFormLayout;
    colorLayout->addRow( new QLabel(tr("object color:")));
    colorLayout->addRow( new QLabel(tr("red")), red);
    colorLayout->addRow( new QLabel(tr("green")), green);
    colorLayout->addRow( new QLabel(tr("blue")), blue);
    colorLayout->addRow( new QLabel(tr("alpha")), alpha);

    QFormLayout *colorRLayout = new QFormLayout;
    colorRLayout->addRow( new QLabel(tr("reflection color:")));
    colorRLayout->addRow( new QLabel(tr("red")), redR);
    colorRLayout->addRow( new QLabel(tr("green")), greenR);
    colorRLayout->addRow( new QLabel(tr("blue")), blueR);
    colorRLayout->addRow( new QLabel(tr("alpha")), alphaR);

    QHBoxLayout *colorsLayout = new QHBoxLayout;
    colorsLayout->addLayout(colorLayout);
    colorsLayout->addSpacing(10);
    colorsLayout->addLayout(colorRLayout);

    QGridLayout *flagLayout = new QGridLayout;
    flagLayout->addWidget(flagBlended, 0, 0);
    flagLayout->addWidget(flagGLSL, 1, 0);
    flagLayout->addWidget(flagGouraud, 2, 0);
    flagLayout->addWidget(flagLighted, 3, 0);
    flagLayout->addWidget(flagPlayerColor, 0, 1);
    flagLayout->addWidget(flagReflec, 1, 1);
    flagLayout->addWidget(flagTextured, 2, 1);
    QPushButton *bShader = new QPushButton(tr("Shader"));
    flagLayout->addWidget(bShader, 3, 1);

    QVBoxLayout *finalLayout = new QVBoxLayout;
    finalLayout->addLayout(colorsLayout);
    finalLayout->addLayout(flagLayout);

    finalLayout->addWidget( new QLabel(tr("Textures:")) );

    imageListView = new ImageListView;
    finalLayout->addWidget( imageListView );

    setLayout(finalLayout);

    connect(red, SIGNAL(valueChanged(int)), this, SLOT(readData()));
    connect(green, SIGNAL(valueChanged(int)), this, SLOT(readData()));
    connect(blue, SIGNAL(valueChanged(int)), this, SLOT(readData()));
    connect(alpha, SIGNAL(valueChanged(int)), this, SLOT(readData()));
    connect(redR, SIGNAL(valueChanged(int)), this, SLOT(readData()));
    connect(greenR, SIGNAL(valueChanged(int)), this, SLOT(readData()));
    connect(blueR, SIGNAL(valueChanged(int)), this, SLOT(readData()));
    connect(alphaR, SIGNAL(valueChanged(int)), this, SLOT(readData()));

    connect(flagBlended, SIGNAL(stateChanged(int)), this, SLOT(readData()));
    connect(flagGLSL, SIGNAL(stateChanged(int)), this, SLOT(readData()));
    connect(flagGouraud, SIGNAL(stateChanged(int)), this, SLOT(readData()));
    connect(flagLighted, SIGNAL(stateChanged(int)), this, SLOT(readData()));
    connect(flagPlayerColor, SIGNAL(stateChanged(int)), this, SLOT(readData()));
    connect(flagReflec, SIGNAL(stateChanged(int)), this, SLOT(readData()));
    connect(flagTextured, SIGNAL(stateChanged(int)), this, SLOT(readData()));

    connect(bShader, SIGNAL(clicked()), ShaderEditor::instance(), SLOT(show()));

    updateWindowTitle();

    updating = false;
}

void SurfaceProperties::updateWindowTitle()
{
    int ID = Gfx::instance()->getSelectionID();
    if (ID >= 0)
        setWindowTitle(tr("Surface properties") + " - " + Mesh::instance()->getMesh(ID)->getName());
    else
        setWindowTitle(tr("Surface properties"));
}

void SurfaceProperties::refreshGUI()
{
    updateWindowTitle();
    int ID = Gfx::instance()->getSelectionID();
    updating = true;
    if (ID >= 0)
    {
        Mesh *mesh = Mesh::instance()->getMesh(ID);
        red->setValue((mesh->color >> 24));
        green->setValue((mesh->color >> 16) & 0xFF);
        blue->setValue((mesh->color >> 8) & 0xFF);
        alpha->setValue(mesh->color & 0xFF);
        redR->setValue((mesh->rColor >> 24) & 0xFF);
        greenR->setValue((mesh->rColor >> 16) & 0xFF);
        blueR->setValue((mesh->rColor >> 8) & 0xFF);
        alphaR->setValue(mesh->rColor & 0xFF);

        flagBlended->setChecked(mesh->flag & SURFACE_BLENDED);
        flagGLSL->setChecked(mesh->flag & SURFACE_GLSL);
        flagGouraud->setChecked(mesh->flag & SURFACE_GOURAUD);
        flagLighted->setChecked(mesh->flag & SURFACE_LIGHTED);
        flagPlayerColor->setChecked(mesh->flag & SURFACE_PLAYER_COLOR);
        flagReflec->setChecked(mesh->flag & SURFACE_REFLEC);
        flagTextured->setChecked(mesh->flag & SURFACE_TEXTURED);

        if (!mesh->tex.isEmpty())
        {
            QList<QImage> imageList;
            for(int i = 0 ; i < mesh->tex.size() ; i++)
            {
                QImage image = Gfx::instance()->textureToImage( mesh->tex[i] );
                imageList.push_back( image.scaled(128, 128) );
            }
            imageListView->setImageList( imageList );
        }
    }
    else
    {
        red->setValue(0);
        green->setValue(0);
        blue->setValue(0);
        alpha->setValue(0);
        redR->setValue(0);
        greenR->setValue(0);
        blueR->setValue(0);
        alphaR->setValue(0);

        flagBlended->setChecked(false);
        flagGLSL->setChecked(false);
        flagGouraud->setChecked(false);
        flagLighted->setChecked(false);
        flagPlayerColor->setChecked(false);
        flagReflec->setChecked(false);
        flagTextured->setChecked(false);
    }
    updating = false;
}

void SurfaceProperties::readData()
{
    if (updating)
        return;
    int ID = Gfx::instance()->getSelectionID();
    if (ID >= 0)
    {
        Mesh *mesh = Mesh::instance()->getMesh(ID);
        mesh->color = 0;
        mesh->color |= red->value() << 24;
        mesh->color |= green->value() << 16;
        mesh->color |= blue->value() << 8;
        mesh->color |= alpha->value();
        mesh->rColor = 0;
        mesh->rColor |= redR->value() << 24;
        mesh->rColor |= greenR->value() << 16;
        mesh->rColor |= blueR->value() << 8;
        mesh->rColor |= alphaR->value();

        mesh->flag = 0;
        mesh->flag |= flagBlended->isChecked() ? SURFACE_BLENDED : 0;
        mesh->flag |= flagGLSL->isChecked() ? SURFACE_GLSL : 0;
        mesh->flag |= flagGouraud->isChecked() ? SURFACE_GOURAUD : 0;
        mesh->flag |= flagLighted->isChecked() ? SURFACE_LIGHTED : 0;
        mesh->flag |= flagPlayerColor->isChecked() ? SURFACE_PLAYER_COLOR : 0;
        mesh->flag |= flagReflec->isChecked() ? SURFACE_REFLEC : 0;
        mesh->flag |= flagTextured->isChecked() ? SURFACE_TEXTURED : 0;
        emit surfaceChanged();
    }
}
