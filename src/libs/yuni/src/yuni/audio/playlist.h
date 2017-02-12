#ifndef __YUNI_AUDIO_PLAYLIST_H__
# define __YUNI_AUDIO_PLAYLIST_H__

# include <vector>
# include "sound.h"

namespace Yuni
{
namespace Audio
{



	/*!
	** \brief A play list contains an ordered list of sounds to be played
	*/
	class PlayList: public Policy::ObjectLevelLockable<PlayList>
	{
	public:
		//! \name Typedefs
		//@{

		//! The most suitable smart pointer for the class
		typedef SmartPtr<PlayList> Ptr;
		//! Size type
		typedef unsigned int Size;
		//! The Threading Policy
		typedef Policy::ObjectLevelLockable<ISound> ThreadingPolicy;

		//@}

	public:
		//! Empty constructor
		PlayList()
			: pCurrentIndex(0), pLoop(false)
		{}

	public:
		Size size() const { return pSoundList.size(); }

		/*!
		** \brief Add a sound at the end of the playlist
		*/
		void append(SmartPtr<ISound>& sound);

		template<typename StringT>
		void remove(const StringT& soundID);

		void play() const;
		void pause(bool paused);

		void previous();
		void next();
		void shuffle();

	private:
		//! All the sounds currently in the play list (ordered)
		ISound::Vector pSoundList;
		//! Index of the current song being played / next to play
		unsigned int pCurrentIndex;
		//! Should we loop around the selection?
		bool pLoop;

	}; // class PlayList



} // namespace Audio
} // namespace Yuni

#endif  // __YUNI_AUDIO_PLAYLIST_H__
