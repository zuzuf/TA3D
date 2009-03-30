#include "aboutwindow.h"
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

AboutWindow *AboutWindow::aboutWindow = NULL;

AboutWindow::AboutWindow()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);
    setWindowTitle(tr("About 3DMEditor"));

    QTextEdit *info = new QTextEdit;
    info->setHtml(tr("ABOUT_TEXT", "text of about window"));
    info->setReadOnly(true);
    layout->addWidget(info);

    QPushButton *ok = new QPushButton(tr("ok"));
    connect(ok, SIGNAL(clicked()), this, SLOT(close()));
    ok->setMaximumWidth(80);
    QHBoxLayout *hlayout = new QHBoxLayout;
    layout->addLayout(hlayout);
    hlayout->addWidget(ok);
    hlayout->setAlignment(Qt::AlignHCenter);
}

void AboutWindow::popup()
{
    if (!aboutWindow)
        aboutWindow = new AboutWindow;
    aboutWindow->show();
}
