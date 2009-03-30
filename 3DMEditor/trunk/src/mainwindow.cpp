#include "mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QLayout>
#include <QResizeEvent>
#include <QDesktopWidget>
#include "config.h"
#include "qpopup.h"
#include "gfx.h"
#include "aboutwindow.h"

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

    QMenu *mnuHelp = new QMenu( tr("&Help") );
    mnuHelp->addAction(tr("&About"), this, SLOT(about()));
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
    QAction *restart = new QAction(this);
    connect(restart, SIGNAL(triggered()), qApp, SLOT(quit()));
    QPopup::popup(tr("The program has to be restarted"),tr("Warning"), restart);
    setStatusBarMessage(tr("Language set to ") + tr("English"));
}

void MainWindow::setFrench()
{
    settings.setValue("language", "fr");
    QAction *restart = new QAction(this);
    connect(restart, SIGNAL(triggered()), qApp, SLOT(quit()));
    QPopup::popup(tr("The program has to be restarted"),tr("Warning"), restart);
    setStatusBarMessage(tr("Language set to ") + tr("French"));
}

void MainWindow::setStatusBarMessage(QString msg)
{
    statusBar->showMessage(msg);
}

void MainWindow::about()
{
    AboutWindow::popup();
}
