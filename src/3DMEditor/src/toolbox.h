#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QDockWidget>

class ToolBox : public QDockWidget
{
public:
    ToolBox();

private:
    static ToolBox *pInstance;
public:
    static ToolBox *instance();
};

#endif // TOOLBOX_H
