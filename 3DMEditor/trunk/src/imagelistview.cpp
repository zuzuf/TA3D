#include "imagelistview.h"
#include <QHBoxLayout>
#include <QMouseEvent>

QImageItem::QImageItem(int ID) : id(ID)
{
    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLayout(new QHBoxLayout);
    layout()->addWidget(&label);
    layout()->setAlignment(Qt::AlignCenter);
}

bool QImageItem::isSelected()
{
    return selected;
}

void QImageItem::select()
{
    selected = true;
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
}

void QImageItem::unselect()
{
    selected = false;
    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken);
}

void QImageItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked(id);
        select();
    }
}

void QImageItem::setImage(const QImage &image)
{
    label.setPixmap( QPixmap::fromImage(image) );
    label.setMaximumSize(image.size());
    label.setMinimumSize(image.size());
    label.resize(image.size());
    resize( layout()->sizeHint() );
}

ImageListView::ImageListView()
{
    scrollWidget = new QWidget;
    scrollWidget->setLayout( new QHBoxLayout );
    setWidget(scrollWidget);
    imageList.clear();
    buildLabelList();
}

ImageListView::~ImageListView()
{
    imageList.clear();
    buildLabelList();
}

const QList<QImage> &ImageListView::getImageList()
{
    return imageList;
}

void ImageListView::setImageList(const QList<QImage> &newImageList)
{
    imageList = newImageList;
    buildLabelList();
}

void ImageListView::buildLabelList()
{
    for(int i = 0 ; i < labelList.size() ; i++)
        delete labelList[i];
    labelList.clear();

    for(int i = 0 ; i < imageList.size() ; i++)
    {
        labelList.push_back( new QImageItem(i) );
        labelList.back()->setImage( imageList[i] );
        connect(labelList.back(), SIGNAL(clicked(int)), this, SLOT(processClick(int)));
    }

    int w = 0, h = 0;
    for(int i = 0 ; i < labelList.size() ; i++)
    {
        scrollWidget->layout()->addWidget(labelList[i]);
        h = qMax( h, labelList[i]->height() );
        w += labelList[i]->width();
    }
    if (!labelList.isEmpty())
        w += (labelList.size() - 1) * 10;
    scrollWidget->resize( w, h );

    update();
}

void ImageListView::processClick(int ID)
{
    for(int i = 0 ; i < labelList.size() ; i++)
        if (labelList[i]->ID() != ID)
            labelList[i]->unselect();
    emit clicked(ID);
}
