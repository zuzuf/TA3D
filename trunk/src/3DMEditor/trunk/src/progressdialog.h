#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QWidget>
#include <QProgressBar>

class ProgressDialog : public QWidget
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
