#ifndef QPOPUP_H
#define QPOPUP_H

#include <QDialog>
#include <QString>
#include <QAction>

class QPopup : public QDialog
{
    Q_OBJECT;
public:
    QPopup(const QString &msg, const QString &title, const QAction *action = NULL);

public slots:

public:
    static void popup(const QString &msg, const QString &title, const QAction *action = NULL);
};

#endif // QPOPUP_H
