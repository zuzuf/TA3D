#include "qpopup.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

QPopup::QPopup(const QString &msg, const QString &title, const QAction *action)
{
    setWindowTitle(title);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *message = new QLabel();
    message->setText(msg);

    QPushButton *ok = new QPushButton(tr("ok"));
    connect(ok, SIGNAL(clicked()), this, SLOT(close()));
    if (action)
        connect(ok, SIGNAL(clicked()), action, SLOT(trigger()));

    layout->addWidget(message);
    layout->addWidget(ok);
}

void QPopup::popup(const QString &msg, const QString &title, const QAction *action)
{
    QPopup *pPopup = new QPopup(msg, title, action);
    pPopup->show();
}
