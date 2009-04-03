#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    void closeEvent(QCloseEvent *);

public slots:
    void setEnglish();
    void setFrench();
    void setStatusBarMessage(const QString msg);
    void about();
    void aboutQt();
    void newMesh();
    void loadMesh();
    void saveMesh();
    void saveMeshAs();
    void showGeometryGraph();
    void endProgram();

private:
    QStatusBar  *statusBar;
    QString     filename;
};

#endif // MAINWINDOW_H
