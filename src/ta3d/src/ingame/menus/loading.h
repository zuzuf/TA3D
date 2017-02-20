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
#ifndef __TA3D_INGAME_MENUS_XX_LOADING_H__
# define __TA3D_INGAME_MENUS_XX_LOADING_H__

# include <stdafx.h>
# include <threads/mutex.h>
# include <misc/string.h>
# include <sdl.h>
# include <misc/progressnotifier.h>


namespace TA3D
{
namespace Menus
{


	/*! \class Loading
	**
	** \brief The window loading for TA3D
	**
	** This class is thread-safe
	*/
	class Loading : public ProgressNotifier
	{
	public:
		//! \name Constructors & Destructor
		//@{
		//! Default constructor
		Loading();
		//! Destructor
		virtual ~Loading();
		//@}

		//! \name SDL interaction
		//@{
		/*!
		** \brief Update percent and caption, redraw the entire screen if required
		**
		** Nothing will be done if there was no changes since the last call
		** to this method.
		**
		** This method must be called from the main thread.
		*/
		virtual void operator()(const float percent, const QString &message);
		//@}

	private:
		/*!
		** \brief Notice other connected players about the progress
		** \param percent the progress value to broadcast
		**
		** This method must be called from the main thread and is not thread-safe
		*/
		void doNoticeOtherPlayers(const float percent);

		/*!
		** \brief Load the background texture
		*/
		void loadTheBackgroundTexture();

		void initializeDrawing();

		void finalizeDrawing();

	private:
		//! The last percent value
		float pLastPercent;
		//! The last caption
		QString pLastCaption;
		//! All messages
		QStringList pMessages;

		//! The background texture
		GLuint pBackgroundTexture;
		//! The height of the font
		float pCurrentFontHeight;

		//! Start time
		int pStartTime;

		float pCacheScreenRatioWidth;
		float pCacheScreenRatioHeight;

	}; // class Loading



} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_XX_LOADING_H__
