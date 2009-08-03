#include "mainwindow.h"
#include <QtGui>
#include "config.h"
#include "gfx.h"
#include "aboutwindow.h"
#include "geometrygraph.h"
#include "textureviewer.h"
#include "surfaceproperties.h"
#include "shadereditor.h"
#include "mesh.h"
#include "misc/camera.h"
#include "helpviewer.h"
#include "toolbox.h"
#include "animation.h"

MainWindow *MainWindow::pInstance = NULL;
MainWindow *MainWindow::instance()
{
    if (!pInstance)
        new MainWindow;
    return pInstance;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ctrlPressed(false)
{
    pInstance = this;

    // Menus
    QMenu *mnuFile = new QMenu( tr("&File"));
    mnuFile->addAction( QIcon("icons/new.png"), tr("&New"), this, SLOT(newMesh()));
    mnuFile->addAction( QIcon("icons/open.png"), tr("&Open"), this, SLOT(loadMesh()));
    mnuFile->addAction( QIcon("icons/save.png"), tr("&Save"), this, SLOT(saveMesh()));
    mnuFile->addAction( QIcon("icons/save-as.png"), tr("Save as"), this, SLOT(saveMeshAs()));
    mnuFile->addAction( QIcon("icons/exit.png"), tr("&Exit"), this, SLOT(close()));
    menuBar()->addMenu(mnuFile);

    QMenu *mnuEdit = new QMenu( tr("&Edit"));
    mnuEdit->addAction( QIcon("icons/edit.png"), tr("&Edit mode"), this, SLOT(setEditMode()));
    mnuEdit->addAction( QIcon("icons/view.png"), tr("&View mode"), this, SLOT(setViewMode()));
    mnuEdit->addAction( QIcon("icons/animation.png"), tr("&Animation mode"), this, SLOT(setAnimateMode()));
    menuBar()->addMenu(mnuEdit);

    QMenu *mnuView = new QMenu( tr("&View"));
    mnuView->addAction( tr("&Show selection"), Gfx::instance(), SLOT(showSelection()));
    mnuView->addAction( tr("&Hide selection"), Gfx::instance(), SLOT(hideSelection()));
    menuBar()->addMenu(mnuView);

    QMenu *mnuLanguage = new QMenu( tr("&Language"));
    mnuLanguage->addAction( tr("&English"), this, SLOT(setEnglish()));
    mnuLanguage->addAction( tr("&French"), this, SLOT(setFrench()));

    QMenu *mnuWindows = new QMenu( tr("&Windows"));
    mnuWindows->addAction( QIcon("icons/tree.png"), tr("&Geometry graph"), this, SLOT(showGeometryGraph()));
    mnuWindows->addAction( QIcon("icons/texture.png"), tr("&Texture viewer"), this, SLOT(showTextureViewer()));
    mnuWindows->addAction( QIcon("icons/configure.png"), tr("&Surface properties"), this, SLOT(showSurfaceProperties()));
    mnuWindows->addAction( QIcon("icons/toolbox.png"), tr("&Toolbox"), this, SLOT(showToolBox()));
    mnuWindows->addAction( QIcon("icons/animation.png"), tr("&Animation"), this, SLOT(showAnimation()));

    QMenu *mnuInterface = new QMenu( tr("&Interface") );
    mnuInterface->addMenu(mnuLanguage);
    mnuInterface->addMenu(mnuWindows);
    menuBar()->addMenu(mnuInterface);

    QMenu *mnuCreate = new QMenu( tr("&Create") );
    mnuCreate->addAction( QIcon("icons/cone.png"), tr("&Cone"), this, SLOT(createCone()));
    mnuCreate->addAction( QIcon("icons/cube.png"), tr("&Cube"), this, SLOT(createCube()));
    mnuCreate->addAction( QIcon("icons/cylinder.png"), tr("&Cylinder"), this, SLOT(createCylinder()));
    mnuCreate->addAction( QIcon("icons/sphere.png"), tr("&Sphere"), this, SLOT(createSphere()));
    mnuCreate->addAction( QIcon("icons/torus.png"), tr("&Torus"), this, SLOT(createTorus()));

    QMenu *mnuTransform = new QMenu( tr("&Transform") );
    mnuTransform->addAction( tr("&Mirror X"), this, SLOT(mirrorX()));
    mnuTransform->addAction( tr("&Mirror Y"), this, SLOT(mirrorY()));
    mnuTransform->addAction( tr("&Mirror Z"), this, SLOT(mirrorZ()));
    mnuTransform->addAction( tr("&Flip X<>Y"), this, SLOT(flipXY()));
    mnuTransform->addAction( tr("&Flip Y<>Z"), this, SLOT(flipYZ()));
    mnuTransform->addAction( tr("&Flip X<>Z"), this, SLOT(flipXZ()));
    mnuTransform->addAction( tr("&Scale"), this, SLOT(scale()));

    QMenu *mnuGeometry = new QMenu( tr("&Geometry") );
    mnuGeometry->addAction( QIcon("icons/split.png"), tr("&Split components"), this, SLOT(splitMesh()));
    mnuGeometry->addAction( QIcon("icons/merge.png"), tr("&Merge all"), this, SLOT(mergeAll()));

    QMenu *mnuModel = new QMenu( tr("&Model") );
    mnuCreate->setIcon(QIcon("icons/add.png"));
    mnuModel->addMenu(mnuCreate);
    mnuModel->addMenu(mnuTransform);
    mnuModel->addMenu(mnuGeometry);
    menuBar()->addMenu(mnuModel);

    QMenu *mnuHelp = new QMenu( tr("&Help") );
    mnuHelp->addAction(QIcon("icons/help.png"), tr("&Index"), this, SLOT(showHelpViewer()));
    mnuHelp->addSeparator();
    mnuHelp->addAction(QIcon("icons/about.png"), tr("&About"), this, SLOT(about()));
    mnuHelp->addAction(QIcon("icons/about.png"), tr("&About Qt"), this, SLOT(aboutQt()));
    menuBar()->addMenu(mnuHelp);

    // Global stuffs
    updateTitle();

    setCentralWidget(Gfx::instance());

    // Toolbars
#define TOOLBAR(mnu) \
    QToolBar *mnu##ToolBar = addToolBar(mnu->title());\
    mnu##ToolBar->addActions(mnu->actions());\
    mnu##ToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);\
    mnu##ToolBar->addSeparator();

    TOOLBAR(mnuFile);
    TOOLBAR(mnuCreate);
    TOOLBAR(mnuGeometry);

    QToolBar *mnuEditToolBar = addToolBar(mnuEdit->title());
    bEdit = new QToolButton;
    bEdit->setCheckable(true);
    bEdit->setIcon(QIcon("icons/edit.png"));
    bEdit->setText(tr("&Edit mode"));
    connect(bEdit, SIGNAL(clicked()), this, SLOT(setEditMode()));
    mnuEditToolBar->addWidget(bEdit);

    bView = new QToolButton;
    bView->setCheckable(true);
    bView->setIcon(QIcon("icons/view.png"));
    bView->setText(tr("&View mode"));
    connect(bView, SIGNAL(clicked()), this, SLOT(setViewMode()));
    mnuEditToolBar->addWidget(bView);

    bAnim = new QToolButton;
    bAnim->setCheckable(true);
    bAnim->setIcon(QIcon("icons/animation.png"));
    bAnim->setText(tr("&Animation mode"));
    connect(bAnim, SIGNAL(clicked()), this, SLOT(setAnimateMode()));
    mnuEditToolBar->addWidget(bAnim);

    mnuEditToolBar->addSeparator();

    // Status bar
    statusBar = new QStatusBar();
    setStatusBar(statusBar);

    setStatusBarMessage(tr("3DMEditor started"));

    QDesktopWidget desktop;

    setBaseSize(640, 480);
    resize( settings.value("mainwindow.size", QSize(640, 480)).toSize() );
    move( settings.value("mainwindow.pos", QPoint((desktop.width() - width()) / 2, (desktop.height() - height()) / 2) ).toPoint() );

    // Some mesh related stuffs
    connect(Mesh::instance(), SIGNAL(loaded()), GeometryGraph::instance(), SLOT(refreshTree()));
    connect(GeometryGraph::instance(), SIGNAL(objectSelected(int)), Gfx::instance(), SLOT(updateSelection(int)));
    connect(Gfx::instance(), SIGNAL(selectionChange(int)), TextureViewer::instance(), SLOT(updateSelection(int)));
    connect(Gfx::instance(), SIGNAL(selectionChange(int)), SurfaceProperties::instance(), SLOT(refreshGUI()));
    connect(SurfaceProperties::instance(), SIGNAL(surfaceChanged()), Gfx::instance(), SLOT(updateGL()));
    connect(Gfx::instance(), SIGNAL(selectionChange(int)), ShaderEditor::instance(), SLOT(updateGUI()));
    connect(Gfx::instance(), SIGNAL(selectionChange(int)), Animation::instance(), SLOT(updateGUI()));

    setViewMode();
}

MainWindow::~MainWindow()
{
    delete statusBar;
}

void MainWindow::setEditMode()
{
    bEdit->setChecked(true);
    bView->setChecked(false);
    bAnim->setChecked(false);
    Gfx::instance()->setEditMode();
}

void MainWindow::setViewMode()
{
    bEdit->setChecked(false);
    bView->setChecked(true);
    bAnim->setChecked(false);
    Gfx::instance()->setViewMode();
}

void MainWindow::setAnimateMode()
{
    bEdit->setChecked(false);
    bView->setChecked(false);
    bAnim->setChecked(true);
    Gfx::instance()->setAnimateMode();
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    settings.setValue("mainwindow.size", size());
}

void MainWindow::moveEvent(QMoveEvent *)
{
    settings.setValue("mainwindow.pos", pos());
}

void MainWindow::setEnglish()
{
    settings.setValue("language", "en");
    setStatusBarMessage(tr("Language set to ") + tr("English"));
    QMessageBox::information(this, tr("Warning"), tr("The program has to be restarted"));
    qApp->quit();
}

void MainWindow::setFrench()
{
    settings.setValue("language", "fr");
    setStatusBarMessage(tr("Language set to ") + tr("French"));
    QMessageBox::information(this, tr("Warning"), tr("The program has to be restarted"));
    qApp->quit();
}

void MainWindow::setStatusBarMessage(QString msg)
{
    statusBar->showMessage(msg);
}

void MainWindow::about()
{
    AboutWindow::popup();
}

void MainWindow::newMesh()
{
    filename.clear();
    updateTitle();
    Mesh::instance()->destroy();
    Gfx::instance()->updateSelection(-1);
    Gfx::instance()->updateGL();
}

void MainWindow::loadMesh()
{
    QString fileToLoad = QFileDialog::getOpenFileName(this, tr("open"), QString(), tr("3DMEditor meshs(*.3dm);;ASCII file(*.asc);;Spring Model(*.s3o);;TA 3D object(*.3do);;3D Studio model(*.3ds);;OBJ model(*.obj);;all files(*.*)"));
    if (fileToLoad.isEmpty())
        return;

    Mesh::instance()->load(fileToLoad);

    Gfx::instance()->updateGL();
    setStatusBarMessage(tr("mesh loaded"));

    filename = fileToLoad;
    updateTitle();
}

void MainWindow::saveMesh()
{
    if (filename.isEmpty() || !filename.endsWith(".3dm", Qt::CaseInsensitive))
        filename = QFileDialog::getSaveFileName(this, tr("save"), QString(), tr("3DMEditor meshs(*.3dm)"));
    if (filename.isEmpty())
        return;

    if (!filename.endsWith(".3dm", Qt::CaseInsensitive))
        filename += ".3dm";

    Mesh::instance()->save(filename);

    setStatusBarMessage(tr("mesh saved"));
    updateTitle();
}

void MainWindow::saveMeshAs()
{
    QString fileToSave = QFileDialog::getSaveFileName(this, tr("save as"), QString(), tr("3DMEditor meshs(*.3dm)"));
    if (fileToSave.isEmpty())
        return;

    if (!fileToSave.endsWith(".3dm", Qt::CaseInsensitive))
        fileToSave += ".3dm";

    Mesh::instance()->save(fileToSave);

    setStatusBarMessage(tr("mesh saved"));

    filename = fileToSave;
    updateTitle();
}

void MainWindow::showGeometryGraph()
{
    GeometryGraph::instance()->show();
}

void MainWindow::showAnimation()
{
    Animation::instance()->show();
}

void MainWindow::showSurfaceProperties()
{
    SurfaceProperties::instance()->show();
}

void MainWindow::showToolBox()
{
    ToolBox::instance()->show();
}

void MainWindow::endProgram()
{
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *)
{
    endProgram();
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::showTextureViewer()
{
    TextureViewer::instance()->show();
}

class MeshManip
{
public:
    static Mesh **getNewMeshPointer()
    {
        Mesh **mesh = NULL;
        if (Gfx::instance()->getSelectionID() == -1)
        {
            if (Mesh::instance()->isEmpty())
            {
                delete Mesh::pInstance;
                Mesh::pInstance = NULL;
                mesh = &Mesh::pInstance;
            }
            else
            {
                mesh = &(Mesh::instance()->next);
                while( *mesh )
                    mesh = &((*mesh)->next);
            }
        }
        else
        {
            mesh = &(Mesh::instance()->getMesh( Gfx::instance()->getSelectionID() )->child);
            while( *mesh )
                mesh = &((*mesh)->next);
        }
        return mesh;
    }
};

void MainWindow::createCube()
{
    float size = (float) QInputDialog::getDouble(this, tr("Cube size"), tr("Cube size:"), 1.0);
    Mesh **mesh = MeshManip::getNewMeshPointer();
    *mesh = Mesh::createCube(size);

    Mesh::instance()->computeInfo();
    GeometryGraph::instance()->refreshTree();
    Gfx::instance()->updateGL();
}

void MainWindow::createCone()
{
    float r = (float) QInputDialog::getDouble(this, tr("Cone radius"), tr("Cone radius:"), 1.0);
    float h = (float) QInputDialog::getDouble(this, tr("Cone height"), tr("Cone height:"), 1.0);
    int d = QInputDialog::getInt(this, tr("Cone details"), tr("Cone details:"), 5, 2);
    bool capped = QMessageBox::question(this, tr("Cone capped ?"), tr("Cone capped ?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
    Mesh **mesh = MeshManip::getNewMeshPointer();
    *mesh = Mesh::createCone(r, h, d, capped);

    Mesh::instance()->computeInfo();
    GeometryGraph::instance()->refreshTree();
    Gfx::instance()->updateGL();
}

void MainWindow::createCylinder()
{
    float r = (float) QInputDialog::getDouble(this, tr("Cylinder radius"), tr("Cylinder radius:"), 1.0);
    float h = (float) QInputDialog::getDouble(this, tr("Cylinder height"), tr("Cylinder height:"), 1.0);
    int d = QInputDialog::getInt(this, tr("Cylinder details"), tr("Cylinder details:"), 5, 2);
    bool capped = QMessageBox::question(this, tr("Cylinder capped ?"), tr("Cylinder capped ?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
    Mesh **mesh = MeshManip::getNewMeshPointer();
    *mesh = Mesh::createCylinder(r, h, d, capped);

    Mesh::instance()->computeInfo();
    GeometryGraph::instance()->refreshTree();
    Gfx::instance()->updateGL();
}

void MainWindow::createSphere()
{
    float r = (float) QInputDialog::getDouble(this, tr("Sphere radius"), tr("Sphere radius:"), 1.0);
    int dw = QInputDialog::getInt(this, tr("Sphere horizontal resolution"), tr("Sphere horizontal resolution:"), 10, 2);
    int dh = QInputDialog::getInt(this, tr("Sphere vertical resolution"), tr("Sphere vertical resolution:"), qMax(dw / 2, 2), 2);
    Mesh **mesh = MeshManip::getNewMeshPointer();
    *mesh = Mesh::createSphere(r, dw, dh);

    Mesh::instance()->computeInfo();
    GeometryGraph::instance()->refreshTree();
    Gfx::instance()->updateGL();
}

void MainWindow::createTorus()
{
    float R = (float) QInputDialog::getDouble(this, tr("Torus big radius"), tr("Torus big radius:"), 1.0);
    float r = (float) QInputDialog::getDouble(this, tr("Torus small radius"), tr("Torus small radius:"), R * 0.5);
    int D = QInputDialog::getInt(this, tr("Torus big resolution"), tr("Torus resolution of big circle:"), 10, 2);
    int d = QInputDialog::getInt(this, tr("Torus small resolution"), tr("Torus resolution of small circle:"), qMax(D / 2, 2), 2);
    Mesh **mesh = MeshManip::getNewMeshPointer();
    *mesh = Mesh::createTorus(R, r, D, d);

    Mesh::instance()->computeInfo();
    GeometryGraph::instance()->refreshTree();
    Gfx::instance()->updateGL();
}

void MainWindow::updateTitle()
{
    if (filename.isEmpty())
        setWindowTitle(tr("3DMEditor"));
    else
        setWindowTitle(tr("3DMEditor") + " - " + filename);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        event->ignore();
        return;
    }
    if (event->modifiers() == Qt::ControlModifier)
    {
        switch(event->key())
        {
        case Qt::Key_X:
            event->accept();
            Gfx::instance()->meshMatrix = Transpose( RotateY(-M_PI_2) );
            Camera::inGame->rpos = Vec(0,0,30);
            break;
        case Qt::Key_Y:
            event->accept();
            Gfx::instance()->meshMatrix = Transpose( RotateX(M_PI_2) );
            Camera::inGame->rpos = Vec(0,0,30);
            break;
        case Qt::Key_Z:
            event->accept();
            Gfx::instance()->meshMatrix = Transpose( Scale(1.0f) );
            Camera::inGame->rpos = Vec(0,0,30);
            break;
        case Qt::Key_Control:
            event->accept();
            if (!ctrlPressed)
            {
                if (Gfx::instance()->getEditMode() == Gfx::VIEW)
                {
                    ctrlPressed = true;
                    setEditMode();
                }
            }
            break;
        default:
            event->ignore();
        };
    }
    else
        event->ignore();
    Gfx::instance()->updateGL();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        event->ignore();
        return;
    }
    switch(event->key())
    {
    case Qt::Key_Control:
        event->accept();
        if (ctrlPressed)
        {
            ctrlPressed = false;
            if (Gfx::instance()->getEditMode() == Gfx::EDIT)
                setViewMode();
        }
        break;
    default:
        event->ignore();
    };
    Gfx::instance()->updateGL();
}

void MainWindow::showHelpViewer()
{
    HelpViewer::instance()->show();
}

void MainWindow::mirrorX()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        for(int i = 0 ; i < mesh->vertex.size() ; i++)
            mesh->vertex[i].x = -mesh->vertex[i].x;
        mesh->invertOrientation();
        mesh->computeNormals();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::mirrorY()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        for(int i = 0 ; i < mesh->vertex.size() ; i++)
            mesh->vertex[i].y = -mesh->vertex[i].y;
        mesh->invertOrientation();
        mesh->computeNormals();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::mirrorZ()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        for(int i = 0 ; i < mesh->vertex.size() ; i++)
            mesh->vertex[i].z = -mesh->vertex[i].z;
        mesh->invertOrientation();
        mesh->computeNormals();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::flipXY()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        for(int i = 0 ; i < mesh->vertex.size() ; i++)
        {
            Vec p = mesh->vertex[i];
            mesh->vertex[i] = Vec( p.y, p.x, p.z );
        }
        mesh->invertOrientation();
        mesh->computeNormals();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::flipXZ()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        for(int i = 0 ; i < mesh->vertex.size() ; i++)
        {
            Vec p = mesh->vertex[i];
            mesh->vertex[i] = Vec( p.z, p.y, p.x );
        }
        mesh->invertOrientation();
        mesh->computeNormals();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::flipYZ()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        for(int i = 0 ; i < mesh->vertex.size() ; i++)
        {
            Vec p = mesh->vertex[i];
            mesh->vertex[i] = Vec( p.x, p.z, p.y );
        }
        mesh->invertOrientation();
        mesh->computeNormals();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::scale()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        float s = (float)QInputDialog::getDouble(this, tr("Scale"), tr("Enter scale factor:"), 1.0);
        for(int i = 0 ; i < mesh->vertex.size() ; i++)
            mesh->vertex[i] = s * mesh->vertex[i];
        if (s < 0.0f)
            mesh->invertOrientation();
        mesh->computeNormals();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::splitMesh()
{
    Mesh *mesh = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (mesh)
    {
        mesh->splitGeometry();
        Mesh::instance()->computeInfo();
        GeometryGraph::instance()->refreshTree();
    }
    Gfx::instance()->updateGL();
}

void MainWindow::mergeAll()
{
    Mesh *mesh = Mesh::instance()->toSingleMesh();
    Mesh::instance()->destroy();
    Mesh::instance()->copy(mesh);
    Mesh::instance()->computeInfo();
    GeometryGraph::instance()->refreshTree();
    Gfx::instance()->updateGL();
    delete mesh;
}
