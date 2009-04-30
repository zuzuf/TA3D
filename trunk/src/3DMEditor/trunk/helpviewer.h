#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QWidget>

class HelpViewer : public QWidget
{
public:
    HelpViewer();
public:
    static HelpViewer *instance();

private:
    static HelpViewer *pInstance;
};

#endif // HELPVIEWER_H
