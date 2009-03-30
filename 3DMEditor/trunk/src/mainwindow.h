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

public slots:
    void setEnglish();
    void setFrench();
    void setStatusBarMessage(const QString msg);
    void about();

private:
    QStatusBar *statusBar;
};

#endif // MAINWINDOW_H
