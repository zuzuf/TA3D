/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

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

#ifndef __TA3D_I_INTERFACES_H__
# define __TA3D_I_INTERFACES_H__

# include <threads/thread.h>
# include <stdafx.h>


# define I_Msg( x, xx ) InterfaceManager->DispatchMsg( x, xx )


namespace TA3D
{

	class IInterfaceManager;
	class IInterface;



	enum TA3D_INTERFACE_MESSAGES
	{
		TA3D_IM_DEBUG_MSG	= 0x01,
		TA3D_IM_GFX_MSG		= 0x02,
		TA3D_IM_GUI_MSG		= 0x04,
		TA3D_IM_ANY_MSG		= 0xFFFFFFFF
	};

	enum INTERFACE_RESULT
	{
		INTERFACE_RESULT_HANDLED  = 0,
		INTERFACE_RESULT_CONTINUE = 1
	//	INTERFACE_RESULT_IGNORE_THIS_MSG =2
	};


    class IInterface : public virtual zuzuf::ref_count
	{
		friend class IInterfaceManager;

	public:
		virtual ~IInterface() {}

	protected:
		void InitInterface();
		void DeleteInterface();

	private:
		uint32 m_InterfaceID;

        /*!
        ** \brief Callback to manage  broadcasted messages
        ** \param msg The received message
        ** \return The return status
        */
		virtual uint32 InterfaceMsg(const uint32 MsgID, const String &msg) = 0;

	}; // class IInterface





	class IInterfaceManager : public ObjectSync
	{
	public:
        typedef zuzuf::smartptr<IInterfaceManager>	Ptr;
	public:
        //! \name Constructor & destructor
        //@{
        //! Default constructor
        IInterfaceManager();
        //! Destructor
		~IInterfaceManager();
        //@}

        /*!
        ** \brief Dispatch a message (from its ID) to all registered interfaces
        **
		** The broadcast of the message ends when an interface returns
        ** `INTERFACE_RESULT_HANDLED`.
        **
        ** \param mID ID of the message
		** \param msg Parameter
        **
        ** \see IInterface::InterfaceMsg()
        */
		void DispatchMsg(const uint32 mID, const String &msg);

	private:
        /*!
        ** \brief Add an interface in the list
        ** \param i Interface to add
        ** \warning The parameter `i` must not be null
        */
		void AddInterface(IInterface* i);

        /*!
        ** \brief Remove an interface from the list
        ** \param i Interface to remove
        ** \warning The parameter `i` must not be null
        */
		void RemoveInterface(IInterface* i);

    private:
		friend class IInterface;
        typedef std::vector<IInterface*> InterfacesList;

	private:
		InterfacesList pInterfaces;
		uint32 pNextInterfaceID;

	}; // class IInterfaceManager
}


#endif // __TA3D_I_INTERFACES_H__
