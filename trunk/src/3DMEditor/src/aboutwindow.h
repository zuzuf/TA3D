#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>

class AboutWindow : public QDialog
{
    Q_OBJECT;
public:
    AboutWindow();

public slots:

public:
    static void popup();
private:
    static AboutWindow *aboutWindow;
};

#endif // ABOUTWINDOW_H
