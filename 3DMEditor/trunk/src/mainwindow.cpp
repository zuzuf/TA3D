#include "mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include "config.h"
#include "qpopup.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QMenu *mnuFile = new QMenu( tr("&File"));
    mnuFile->addAction( tr("&Exit"), this, SLOT(close()));
    menuBar()->addMenu(mnuFile);

    QMenu *mnuLanguage = new QMenu( tr("&Language"));
    mnuLanguage->addAction( tr("&English"), this, SLOT(setEnglish()));
    mnuLanguage->addAction( tr("&French"), this, SLOT(setFrench()));

    QMenu *mnuInterface = new QMenu( tr("&Interface") );
    mnuInterface->addMenu(mnuLanguage);

    menuBar()->addMenu(mnuInterface);

    setWindowTitle(tr("3DMEditor"));
}

MainWindow::~MainWindow()
{

}

void MainWindow::setEnglish()
{
    settings.setValue("language", "en");
    QAction *restart = new QAction(this);
    connect(restart, SIGNAL(triggered()), qApp, SLOT(quit()));
    QPopup::popup(tr("The program has to be restarted"),tr("Warning"), restart);
}

void MainWindow::setFrench()
{
    settings.setValue("language", "fr");
    QAction *restart = new QAction(this);
    connect(restart, SIGNAL(triggered()), qApp, SLOT(quit()));
    QPopup::popup(tr("The program has to be restarted"),tr("Warning"), restart);
}
