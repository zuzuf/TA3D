#include "geometrygraph.h"
#include "mesh.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QQueue>

GeometryGraph *GeometryGraph::pInstance = NULL;

GeometryGraph *GeometryGraph::instance()
{
    if (!pInstance)
        pInstance = new GeometryGraph;

    return pInstance;
}

GeometryGraph::GeometryGraph()
{
    setWindowTitle(tr("Geometry Graph"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    tree = new QTreeWidget;
    tree->setColumnCount(2);
    tree->setHeaderLabels(QStringList(tr("Parts")) << tr("ID"));
    connect(tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(updateSelection(QTreeWidgetItem*,int)));

    layout->addWidget(tree);
}

void GeometryGraph::refreshTree()
{
    tree->clear();

    QList<QTreeWidgetItem*> items;
    QQueue<Mesh*>               qMesh;
    QQueue<QTreeWidgetItem*>    qParent;
    qMesh.enqueue(&Mesh::instance);     // Item
    qParent.enqueue(NULL);              // Parent

    while (!qMesh.isEmpty())
    {
        QTreeWidgetItem *parent = qParent.dequeue();
        Mesh *cur = qMesh.dequeue();
        QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(cur->getName()) << QString("%1").arg(cur->getID()));
        if (parent == NULL)
            items.append(item);
        if (cur->child)
        {
            qMesh.enqueue(cur->child);
            qParent.enqueue(item);
        }
        if (cur->next)
        {
            qMesh.enqueue(cur->next);
            qParent.enqueue(parent);
        }
    }

    tree->insertTopLevelItems(0, items);

    update();
}

void GeometryGraph::updateSelection(QTreeWidgetItem *item, int column)
{
    emit objectSelected(item->text(1).toInt());
}

void GeometryGraph::updateSelectionID(int ID)
{
    QList<QTreeWidgetItem*> items = tree->findItems(QString("%1").arg(ID), Qt::MatchExactly , 1);
    if (!items.isEmpty())
        tree->setCurrentItem(items.front());
}
