#ifndef IMAGELISTVIEW_H
#define IMAGELISTVIEW_H

#include <QScrollArea>
#include <QList>
#include <QImage>
#include <QFrame>
#include <QLabel>

class QImageItem : public QFrame
{
    Q_OBJECT;
public:
    QImageItem(int ID);

    void mousePressEvent(QMouseEvent *);
    void setImage(const QImage &image);
    bool isSelected();
    inline int ID() { return id;    }

public slots:
    void select();
    void unselect();

signals:
    void clicked(int ID);

private:
    QLabel  label;
    int     id;
    bool    selected;
};

class ImageListView : public QScrollArea
{
    Q_OBJECT;
public:
    ImageListView();
    ~ImageListView();
    const QList<QImage> &getImageList();

private:
    void buildLabelList();

public slots:
    void setImageList(const QList<QImage> &newImageList);

protected slots:
    void processClick(int ID);

signals:
    void clicked(int ID);

private:
    QWidget             *scrollWidget;
    QList<QImage>       imageList;
    QList<QImageItem*>  labelList;
};

#endif // IMAGELISTVIEW_H
