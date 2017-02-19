#include "bugreportdialog.h"
#include "ui_bugreportdialog.h"

BugReportDialog::BugReportDialog(const QString &report) :
    QDialog(NULL),
    ui(new Ui::BugReportDialog)
{
    ui->setupUi(this);

    ui->pte_text->setPlainText(report);
    ui->lbl_size->setText(QString("(report size = %1 bytes)").arg(report.size()));
}

BugReportDialog::~BugReportDialog()
{
    delete ui;
}

void BugReportDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
