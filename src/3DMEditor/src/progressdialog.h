#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>

class ProgressDialog : public QDialog
{
    Q_OBJECT;
public:
    ProgressDialog();

private:
    QProgressBar *pbar;

public:
    static ProgressDialog *instance();
    static void setProgress(int pcent);

private:
    static ProgressDialog *pInstance;
};

#endif // PROGRESSDIALOG_H
