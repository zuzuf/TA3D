
#include "socketlist.h"


namespace TA3D
{


SockList::SockList()
{
    maxid = 0;
    list = NULL;
}

SockList::~SockList()
{
    Shutdown();
}

void
SockList::Shutdown()
{
    while(list)
    {
        Remove(list->id);
    }
}

int
SockList::Add(TA3DSock* sock)
{
    if (maxid > 10000) //arbitrary limit
        return -1;
    SocketListNode *node,*ptr;

    node = new SocketListNode;
    maxid++;
    node->id = maxid;
    node->sock = sock;

    if(!list)
        list = node;
    else{
        ptr=list;
        while(ptr->next)
            ptr=ptr->next;
        ptr->next = node;
    }

    return node->id;
}

int
SockList::Remove(const int id)
{
    SocketListNode *node,*prev;

    if(list==NULL)
        return -1;
    if(list->id == id)
    {
        node = list->next;
        delete list;
        list = node;
        return 0;
    }

    node=list->next;
    prev=list;
    while(node)
    {
        if(node->id == id)
        {
            prev->next = node->next;
            delete node;
            return 0;
        }
        node=node->next;
        prev=prev->next;
    }

    return -1;
}

SocketThread*
SockList::getThread(const int id) const
{
    for(SocketListNode* node = list; node; node = node->next)
    {
        if (id == node->id)
            return &(node->thread);
    }
    return NULL;
}

TA3DSock*
SockList::getSock(const int id) const
{
    for(SocketListNode* node = list; node; node = node->next)
    {
        if (node->id == id)
            return node->sock;
    }
    return NULL;
}


} // namespace TA3D
