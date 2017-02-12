#ifndef __YUNI_UI_ID_H__
# define __YUNI_UI_ID_H__

# include "../yuni.h"


namespace Yuni
{
namespace UI
{

	/*!
	** \brief Component identifier management
	*/
	class ID
	{
	public:
		enum IDValues
		{
			InvalidID = -1,
			MinID = 0
		};

	public:
		typedef sint64  Type;
		static const Type MaxID = sizeof (Type) - 1;

	public:
		//! Get a new free identifier
		static Type New();

	private:
		//! \name Constructors
		//@{
		/*!
		** \brief Default constructor
		*/
		ID();
		/*!
		** \brief Copy constructor
		*/
		ID(const ID&);
		//@}

	}; // class ID





} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_ID_H__
