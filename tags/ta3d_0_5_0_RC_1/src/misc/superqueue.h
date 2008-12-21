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

#ifndef __TA3D_MISC_SUPERQUEUE_H__
# define __TA3D_MISC_SUPERQUEUE_H__



/*! \class SQNode
** \brief
*/
struct SQNode
{
    //! 
	char* item;
    //!
	int size;
    //!
	struct SQNode* next;
};



/*!
** \brief A queue for events that come across the network.
**
** It is implemented as an array but will expand into a linked
** list when full (and then shrink back to normal). The array
** should be big enough to hold it all, but in case it doesn't
** the expanding behavior keeps the game from crashing
**
** TODO, the linked list really sucks for high performance
** need to do a more efficient bulk memory allocation
*/
class SuperQueue
{
public:
    //! \name Constructors & Destructors
    //@{
    //! Default constructor
    SuperQueue();
    /*!
    ** \brief Constructor
    ** \param number
    ** \param itemsize
    */
    SuperQueue(int number,int itemsize);
    //! Destructor
    ~SuperQueue();
    //@}

    /*!
    ** \brief Enqueue
    ** \param item Item to enqueue
    */
    int enqueue(void* item);

    /*!
    ** \brief Dequeue
    ** \param item Item to dequeue
    */
    int dequeue(void* item);

    /*!
    ** \brief
    */
    int isFull();
    /*!
    ** \brief
    */
    int isEmpty();

private:
    //!
    char* queue;
    //!
    int qsize;
    //!
    int size;
    //! Extra list
    struct SQNode* extra;//extra list
    //! points to back off list
    struct SQNode* ep;
    //! items enter queue in back
    int qfront;
    //! items enter queue in back
    int qback;
    //! 
    int full;
    //! 
    int empty;
    //! 
    int freespace;

}; // class SuperQueue



#endif // __TA3D_MISC_SUPERQUEUE_H__
