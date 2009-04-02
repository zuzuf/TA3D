#ifndef GEOMETRYGRAPH_H
#define GEOMETRYGRAPH_H

#include <QWidget>
#include <QTreeWidget>

class GeometryGraph : public QWidget
{
    Q_OBJECT;
public:
    GeometryGraph();

public slots:
    void refreshTree();

private:
    QTreeWidget *tree;

private:
    static GeometryGraph *pInstance;
public:
    static GeometryGraph *instance();
};

#endif // GEOMETRYGRAPH_H
