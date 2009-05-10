#include "helpviewer.h"
#include <QtWebKit>
#include <QVBoxLayout>

HelpViewer *HelpViewer::pInstance = NULL;

HelpViewer *HelpViewer::instance()
{
    if (!pInstance)
        pInstance = new HelpViewer;
    return pInstance;
}

HelpViewer::HelpViewer()
{
    setWindowTitle(tr("3DMEditor - Help"));

    QWebView *webView = new QWebView();
    QString helpPath = "";
    webView->load( QUrl(helpPath + tr("help/en/index.html")) );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(webView);
    layout->setSpacing(0);
    layout->setMargin(0);
}
