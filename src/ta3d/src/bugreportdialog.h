#ifndef BUGREPORTDIALOG_H
#define BUGREPORTDIALOG_H

#include <QDialog>

namespace Ui {
class BugReportDialog;
}

class BugReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BugReportDialog(const QString &report);
    ~BugReportDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::BugReportDialog *ui;
};

#endif // BUGREPORTDIALOG_H
