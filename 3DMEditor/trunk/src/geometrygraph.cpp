#include "geometrygraph.h"
#include "mesh.h"
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
    tree->setColumnCount(1);
    tree->setHeaderLabel(tr("Parts"));

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
        QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(cur->name));
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
