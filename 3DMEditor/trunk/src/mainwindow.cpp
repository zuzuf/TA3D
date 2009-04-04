#include "mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QLayout>
#include <QResizeEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include "config.h"
#include "gfx.h"
#include "aboutwindow.h"
#include "geometrygraph.h"
#include "mesh.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QMenu *mnuFile = new QMenu( tr("&File"));
    mnuFile->addAction( tr("&New"), this, SLOT(newMesh()));
    mnuFile->addAction( tr("&Open"), this, SLOT(loadMesh()));
    mnuFile->addAction( tr("&Save"), this, SLOT(saveMesh()));
    mnuFile->addAction( tr("Save as"), this, SLOT(saveMeshAs()));
    mnuFile->addAction( tr("&Exit"), this, SLOT(close()));
    menuBar()->addMenu(mnuFile);

    QMenu *mnuLanguage = new QMenu( tr("&Language"));
    mnuLanguage->addAction( tr("&English"), this, SLOT(setEnglish()));
    mnuLanguage->addAction( tr("&French"), this, SLOT(setFrench()));

    QMenu *mnuWindows = new QMenu( tr("&Windows"));
    mnuWindows->addAction( tr("&Geometry graph"), this, SLOT(showGeometryGraph()));

    QMenu *mnuInterface = new QMenu( tr("&Interface") );
    mnuInterface->addMenu(mnuLanguage);
    mnuInterface->addMenu(mnuWindows);
    menuBar()->addMenu(mnuInterface);

    QMenu *mnuHelp = new QMenu( tr("&Help") );
    mnuHelp->addAction(tr("&About"), this, SLOT(about()));
    mnuHelp->addAction(tr("&About Qt"), this, SLOT(aboutQt()));
    menuBar()->addMenu(mnuHelp);

    setWindowTitle(tr("3DMEditor"));

    setCentralWidget(Gfx::instance());

    statusBar = new QStatusBar();
    setStatusBar(statusBar);

    setStatusBarMessage(tr("3DMEditor started"));

    QDesktopWidget desktop;

    setBaseSize(640, 480);
    resize( settings.value("mainwindow.size", QSize(640, 480)).toSize() );
    move( settings.value("mainwindow.pos", QPoint((desktop.width() - width()) / 2, (desktop.height() - height()) / 2) ).toPoint() );

    // Some mesh related stuffs
    connect(&Mesh::instance, SIGNAL(loaded()), GeometryGraph::instance(), SLOT(refreshTree()));
    connect(GeometryGraph::instance(), SIGNAL(objectSelected(int)), Gfx::instance(), SLOT(updateSelection(int)));
}

MainWindow::~MainWindow()
{
    delete statusBar;
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
    Mesh::instance.destroy();
    Gfx::instance()->updateGL();
}

void MainWindow::loadMesh()
{
    QString fileToLoad = QFileDialog::getOpenFileName(this, tr("open"), QString(), tr("3DMEditor meshs(*.3dm);;ASCII file(*.asc);;TA 3D object(*.3do);;3D Studio model(*.3ds);;OBJ model(*.obj);;all files(*.*)"));
    if (fileToLoad.isEmpty())
        return;

    Mesh::instance.load(fileToLoad);

    Gfx::instance()->updateGL();
    setStatusBarMessage(tr("mesh loaded"));

    filename = fileToLoad;
}

void MainWindow::saveMesh()
{
    if (filename.isEmpty() || !filename.endsWith(".3dm", Qt::CaseInsensitive))
        filename = QFileDialog::getSaveFileName(this, tr("save"), QString(), tr("3DMEditor meshs(*.3dm)"));
    if (filename.isEmpty())
        return;

    if (!filename.endsWith(".3dm", Qt::CaseInsensitive))
        filename += ".3dm";

    Mesh::instance.save(filename);

    setStatusBarMessage(tr("mesh saved"));
}

void MainWindow::saveMeshAs()
{
    QString fileToSave = QFileDialog::getSaveFileName(this, tr("save as"), QString(), tr("3DMEditor meshs(*.3dm)"));
    if (fileToSave.isEmpty())
        return;

    if (!fileToSave.endsWith(".3dm", Qt::CaseInsensitive))
        fileToSave += ".3dm";

    Mesh::instance.save(fileToSave);

    setStatusBarMessage(tr("mesh saved"));

    filename = fileToSave;
}

void MainWindow::showGeometryGraph()
{
    GeometryGraph::instance()->show();
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
