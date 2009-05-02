#ifndef GEOMETRYGRAPH_H
#define GEOMETRYGRAPH_H

#include <QWidget>
#include <QTreeWidget>

class GeometryGraph : public QWidget
{
    Q_OBJECT;
public:
    GeometryGraph();

    QList<int> getSelection();

public slots:
    void refreshTree();
    void updateSelection(QTreeWidgetItem *item, int);
    void updateSelectionID(int ID);
    void deleteSelection();
    void renameSelection();
    void mergeSelection();

signals:
    void objectSelected(int ID);

private:
    QTreeWidget *tree;

private:
    static GeometryGraph *pInstance;
public:
    static GeometryGraph *instance();
};

#endif // GEOMETRYGRAPH_H
