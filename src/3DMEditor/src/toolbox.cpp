#include "toolbox.h"
#include "mainwindow.h"
#include <QToolBox>
#include <QToolButton>
#include <QButtonGroup>
#include "flowlayout.h"

ToolBox *ToolBox::pInstance = NULL;
ToolBox *ToolBox::instance()
{
    if (!pInstance)
        pInstance = new ToolBox;
    return pInstance;
}

ToolBox::ToolBox() : QDockWidget(MainWindow::instance())
{
    // Docking options
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    MainWindow::instance()->addDockWidget(Qt::LeftDockWidgetArea, this);
    QToolBox *wnd = new QToolBox;
    setWidget(wnd);

    // Normal initialization
    setWindowTitle(tr("toolbox"));
    QWidget *tShape = new QWidget;
    QButtonGroup *bgShape = new QButtonGroup;
    wnd->addItem(tShape, QIcon(), tr("Shape"));
    FlowLayout *lShape = new FlowLayout(tShape);

#define ADD_BUTTON(name, title, icon) \
    QToolButton *b##name = new QToolButton();\
    b##name->setIcon(QIcon("icons/" icon));\
    b##name->setToolTip(tr(title));\
    b##name->setCheckable(true);\
    bgShape->addButton(b##name);\
    lShape->addWidget(b##name);

    ADD_BUTTON(ScaleX, "Scale X", "open.png");
    ADD_BUTTON(ScaleY, "Scale Y", "open.png");
    ADD_BUTTON(ScaleZ, "Scale Z", "open.png");
    ADD_BUTTON(ScaleRX, "Scale radial X", "open.png");
    ADD_BUTTON(ScaleRY, "Scale radial Y", "open.png");
    ADD_BUTTON(ScaleRZ, "Scale radial Z", "open.png");
    ADD_BUTTON(Scale, "Scale", "open.png");
}
