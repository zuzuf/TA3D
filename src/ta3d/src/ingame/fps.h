#ifndef __TA3D_INGAME_FPS_INFOS_H__
# define __TA3D_INGAME_FPS_INFOS_H__

# include <misc/string.h>



namespace TA3D
{


	class FPSInfos
	{
	public:
		//! \name Constructor
		//@{
		FPSInfos();
		//@}

		/*!
		** \brief Take into account that a new frame has been rendered
		**
		** This method must be execute each frame
		*/
		void statisticsAddFrame();

		/*!
		** \brief Draw the FPS count (OpenGL)
		*/
		void draw() const;

	public:
		//!
		int countSinceLastTime;
		//!
		int average;
		//!
		int lastTime;
		//!
		QString toStr;
	};


} // namespace TA3D

#endif // __TA3D_INGAME_FPS_INFOS_H__
