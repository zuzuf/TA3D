#include "qpopup.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

QPopup::QPopup(const QString &msg, const QString &title, const QAction *action)
{
    setWindowTitle(title);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *message = new QLabel();
    message->setText(msg);

    QPushButton *ok = new QPushButton(tr("ok"));
    ok->setMaximumWidth(80);
    connect(ok, SIGNAL(clicked()), this, SLOT(close()));
    if (action)
        connect(ok, SIGNAL(clicked()), action, SLOT(trigger()));

    layout->addWidget(message);
    QHBoxLayout *hlayout = new QHBoxLayout;
    layout->addLayout(hlayout);
    hlayout->addWidget(ok);
    hlayout->setAlignment(Qt::AlignHCenter);
}

void QPopup::popup(const QString &msg, const QString &title, const QAction *action)
{
    QPopup *pPopup = new QPopup(msg, title, action);
    pPopup->show();
}
