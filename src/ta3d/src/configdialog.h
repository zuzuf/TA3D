#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

    void do_config();
protected:
    void changeEvent(QEvent *e);

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
