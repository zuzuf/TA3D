/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "superqueue.h"
#include <stdlib.h>
#include <string.h>

// TODO : the entire data structure needs to be
// rewritten to more gracefully accomodate
// queue overflow



SuperQueue::SuperQueue()
    :qsize(32), size(4), extra(NULL), ep(NULL), 
    qfront(0), qback(0), full(0), empty(1),
    freespace(32/*=qsize*/)
{
	queue = (char*)malloc(sizeof(char) * qsize);
}

SuperQueue::SuperQueue(int number,int itemsize)
    :qsize(number*itemsize), size(itemsize), extra(NULL), ep(NULL), 
    qfront(0), qback(0), full(0), empty(1)
{
	freespace = qsize;
	queue = (char*)malloc(sizeof(char)*qsize);
}

SuperQueue::~SuperQueue()
{
	struct SQNode* node1;
    struct SQNode* node2;
	node1 = extra;
	while (node1)
    {
		node2 = node1->next;
		free(node1->item);
		free(node1);
		node1 = node2;
	}
	free(queue);
}


int SuperQueue::enqueue(void* item)
{
    if (!full)
    {
        if ( freespace == 0 || freespace-size < 0 )
        {
            //expand
            struct SQNode* node;
            node = (struct SQNode*)malloc(sizeof(struct SQNode));
            node->next = NULL;
            node->item = (void*)malloc(size);
            memcpy(node->item,item,size);
            extra = node;
            ep = node;
            full = 1;
        }
        else
        {
            if (qback + size > qsize)
            {
                //wrap
                memcpy(queue+qback,item,qsize-qback);
                memcpy(queue,(char*)item+qsize-qback,size-(qsize-qback));
                qback=(qback+size) % qsize;
                freespace-=size;
            }
            else
            {
                //normal
                memcpy(queue+qback,item,size);
                qback=(qback+size) % qsize;
                freespace-=size;
            }
        }
        empty = 0;
    }
    else
    {
        //full, use linked list
        struct SQNode* node;
        node = (struct SQNode*)malloc(sizeof(struct SQNode));
        node->next = NULL;
        node->item = (void*)malloc(size);
        memcpy(node->item,item,size);
        if(ep)
            ep->next = node;
        else
            extra = node;
        ep = node;
    }
    return 0;
}





int SuperQueue::dequeue(void* item)
{
    //the front of the queue is always in the array

    if(empty)
        return -1;

    if(qfront + size > qsize)
    {
        //unwrap
        memcpy(item,queue+qfront,qsize-qfront);
        memcpy((char*)item+(qsize-qfront),queue,size-(qsize-qfront));
        qfront=(qfront+size) % qsize;
    }
    else
    {
        //normal
        memcpy(item,queue+qfront,size);
        qfront=(qfront+size) % qsize;
    }

    if (!full)
        freespace+=size;
    if (freespace == qsize)
        empty = 1;

    //but the back of the queue may be out in the list
    if (full)
    {
        //shrink

        if(qback + size > qsize)
        {
            //wrap
            memcpy(queue+qback,extra->item,qsize-qback);
            memcpy(queue,(char*)(extra->item)+qsize-qback,size-(qsize-qback));
        }
        else
        {
            //normal
            memcpy(queue+qback,extra->item,size);
        }
        qback=(qback+size) % qsize;
        struct SQNode* node;
        free(extra->item);
        node = extra->next;
        free(extra);
        extra = node;
        if (extra == NULL)
            full = 0;
    }
    return 0;
}

