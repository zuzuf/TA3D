#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QToolButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

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
    void showTextureViewer();
    void showSurfaceProperties();
    void showHelpViewer();
    void endProgram();
    void createCube();
    void createSphere();
    void createCylinder();
    void createCone();
    void createTorus();
    void updateTitle();
    void mirrorX();
    void mirrorY();
    void mirrorZ();
    void flipXY();
    void flipYZ();
    void flipXZ();
    void scale();
    void splitMesh();
    void mergeAll();
    void setEditMode();
    void setViewMode();
    void setAnimateMode();

private:
    QStatusBar  *statusBar;
    QString     filename;
    bool        ctrlPressed;
    QToolButton *bEdit;
    QToolButton *bView;
    QToolButton *bAnim;

private:
    static MainWindow   *pInstance;
public:
    static MainWindow   *instance();
};

#endif // MAINWINDOW_H
