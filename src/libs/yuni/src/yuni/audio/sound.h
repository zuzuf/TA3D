#ifndef __YUNI_AUDIO_SOUND_H__
# define __YUNI_AUDIO_SOUND_H__

# include <map>
# include "../yuni.h"
# include "../core/string.h"
# include "../core/smartptr.h"
# include "../core/customstring.h"
# include "../private/audio/av.h"

namespace Yuni
{
namespace Audio
{


	/*!
	** \brief A sound or music loaded from a file or stream
	*/
	class Sound: public Policy::ObjectLevelLockable<Sound>
	{
	public:
		//! Pointer type
		typedef SmartPtr<Sound> Ptr;
		//! Map from a string to a Sound::Ptr
		typedef std::map<String, Ptr> Map;
		//! Threading Policy
		typedef Policy::ObjectLevelLockable<Sound>  ThreadingPolicy;

	public:
		enum // anonymous
		{
			maxBufferCount = 4,
			bufferSize = 65536
		}; // enum anonymous

	public:
		//! Constructor
		Sound(Private::Audio::AudioStream* stream);
		~Sound();

		bool prepareDispatched(unsigned int source);

		bool destroyDispatched();

		bool updateDispatched(unsigned int source);

		void stream(Private::Audio::AudioStream* stream) { pStream = stream; }

		bool valid() { return NULL != pStream; }

		//! Get the duration of the stream, 0 if not set
		unsigned int duration() const;

	private:
		//! The audio stream contains the link to the file and the data
		Private::Audio::AudioStream* pStream;

		//! Actual number of buffers
		unsigned int pBufferCount;

		//! Identifiers of the OpenAL buffers used
		unsigned int pIDs[maxBufferCount];

		//! Buffer for storing raw data
		CustomString<bufferSize, false, false> pData;

	}; // class Sound



} // namespace Audio
} // namespace Yuni


# include "sound.hxx"


#endif // __YUNI_AUDIO_SOUND_H__
