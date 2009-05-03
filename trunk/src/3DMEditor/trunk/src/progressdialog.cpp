#include <QVBoxLayout>
#include "progressdialog.h"

ProgressDialog *ProgressDialog::pInstance = NULL;

ProgressDialog *ProgressDialog::instance()
{
    if (!pInstance)
        pInstance = new ProgressDialog;
    return pInstance;
}

ProgressDialog::ProgressDialog()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    pbar = new QProgressBar;
    pbar->setMinimum(0);
    pbar->setMaximum(100);
    pbar->setValue(0);
    layout->addWidget(pbar);
    setWindowTitle(tr("Work in progress"));
}

void ProgressDialog::setProgress(int pcent)
{
    instance()->show();
    instance()->pbar->setValue(pcent);
    instance()->updateGeometry();
    instance()->update();
    if (pcent == 100)
        instance()->hide();
}
