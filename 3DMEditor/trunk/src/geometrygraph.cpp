#include "geometrygraph.h"
#include "mesh.h"
#include "gfx.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QQueue>
#include <QDropEvent>
#include <QVariant>
#include <QPair>

GeometryGraph *GeometryGraph::pInstance = NULL;

class TreeWidget : public QTreeWidget
{
protected:
    void dropEvent(QDropEvent *);
};

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

    tree = new TreeWidget;
    tree->setColumnCount(2);
    tree->setHeaderLabels(QStringList(tr("Parts")) << tr("ID"));
    connect(tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(updateSelection(QTreeWidgetItem*,int)));

    tree->setDragEnabled(true);
    tree->setDragDropMode(QTreeWidget::InternalMove);
    tree->setSelectionMode(QTreeView::ExtendedSelection);

    layout->addWidget(tree);

    QPushButton *bRename = new QPushButton(tr("&Rename"));
    QPushButton *bDelete = new QPushButton(tr("&Delete"));
    QPushButton *bMerge = new QPushButton(tr("&Merge"));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(bRename);
    buttonLayout->addWidget(bDelete);
    buttonLayout->addWidget(bMerge);
    layout->addLayout(buttonLayout);

    connect(bRename, SIGNAL(clicked()), this, SLOT(renameSelection()));
    connect(bDelete, SIGNAL(clicked()), this, SLOT(deleteSelection()));
    connect(bMerge, SIGNAL(clicked()), this, SLOT(mergeSelection()));
}

void GeometryGraph::refreshTree()
{
    tree->clear();

    QList<QTreeWidgetItem*> items;
    QQueue<Mesh*>               qMesh;
    QQueue<QTreeWidgetItem*>    qParent;
    qMesh.enqueue(Mesh::instance());    // Item
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
    if (tree->selectedItems().size() == 1)
        emit objectSelected(item->text(1).toInt());
}

void GeometryGraph::updateSelectionID(int ID)
{
    QList<QTreeWidgetItem*> items = tree->findItems(QString("%1").arg(ID), Qt::MatchExactly | Qt::MatchRecursive , 1);
    if (!items.isEmpty())
        tree->setCurrentItem(items.front());
}

void TreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidget::dropEvent(event);
    QVector<Mesh*> objects;
    QQueue<Mesh*> mQueue;
    objects.resize( Mesh::instance()->nbSubObjects() );

    mQueue.enqueue(Mesh::instance());       // Create an object table
    while(!mQueue.isEmpty())
    {
        Mesh *cur = mQueue.dequeue();
        if (cur->child)
        {
            mQueue.enqueue(cur->child);
            for( Mesh *m = cur->child ; m != NULL ; m = m->next)
                m->pos += cur->pos;
        }
        if (cur->next)
            mQueue.enqueue(cur->next);
        objects[cur->ID] = cur;
        cur->next = NULL;           // Clear obsolete information
        cur->child = NULL;
    }

    QQueue<QTreeWidgetItem*> iQueue;
    Mesh *last = NULL;
    for(int i = 0 ; i < topLevelItemCount() ; i++)
    {
        int curID = topLevelItem(i)->text(1).toInt();
        if (last == NULL)
        {
            last = objects[curID];
            Mesh::pInstance = last;
        }
        else
            last = (last->next = objects[curID]);
        iQueue.enqueue(topLevelItem(i));
    }

    while(!iQueue.isEmpty())
    {
        QTreeWidgetItem *item = iQueue.dequeue();
        int ID = item->text(1).toInt();
        last = NULL;
        for(int i = 0 ; i < item->childCount() ; i++)
        {
            int curID = item->child(i)->text(1).toInt();
            if (last == NULL)
                last = objects[ID]->child = objects[ curID ];
            else
                last = (last->next = objects[ curID ]);
            iQueue.enqueue(item->child(i));
        }
    }
    Mesh::instance()->computeID();
    GeometryGraph::instance()->refreshTree();
    GeometryGraph::instance()->updateSelectionID(Gfx::instance()->getSelectionID());

    QQueue< QPair<Mesh*, Vec> > vQueue;         // Compute new relative positions
    vQueue.enqueue( QPair<Mesh*, Vec>(Mesh::instance(), Vec()) );
    while(!vQueue.isEmpty())
    {
        QPair<Mesh*, Vec> cur = vQueue.dequeue();
        if (cur.first->child)
            vQueue.enqueue( QPair<Mesh*, Vec>(cur.first->child, cur.first->pos) );
        if (cur.first->next)
            vQueue.enqueue( QPair<Mesh*, Vec>(cur.first->next, cur.second) );
        cur.first->pos -= cur.second;
    }

    Gfx::instance()->updateGL();
}

void GeometryGraph::renameSelection()
{
    int ID = Gfx::instance()->getSelectionID();
    if (ID >= 0)
    {
        Mesh::instance()->getMesh(ID)->name = QInputDialog::getText(this, tr("Object name"), tr("enter new name"));
        refreshTree();
    }
}

void GeometryGraph::deleteSelection()
{
    int ID = Gfx::instance()->getSelectionID();
    if (ID >= 0)
    {
        Mesh::instance()->deleteMesh(ID);
        Mesh::instance()->computeInfo();
        Gfx::instance()->updateSelection(-1);
        refreshTree();
        Gfx::instance()->updateGL();
    }
}

QList<int> GeometryGraph::getSelection()
{
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    QList<int> itemIdx;

    foreach(QTreeWidgetItem* item, items)
        itemIdx.push_back( item->text(1).toInt() );

    return itemIdx;
}

void GeometryGraph::mergeSelection()
{
    QList<int> selection = getSelection();
    if (!selection.isEmpty())
    {
        int refParent = -2;
        QList<Mesh*> lMesh;
        foreach(int s, selection)
        {
            if (refParent == -2)
                refParent = Mesh::instance()->getParent(s);
            else if (refParent != Mesh::instance()->getParent(s))
            {
                QMessageBox::critical(this, tr("Merging error"), tr("Could not merge selected objects. You must select objects with the same parent"));
                return;
            }
            lMesh.push_back(Mesh::instance()->getMesh(s));
        }
        Mesh::merge(lMesh);
        Mesh::instance()->computeInfo();
        Gfx::instance()->updateSelection(-1);
        refreshTree();
        Gfx::instance()->updateGL();
    }
}
