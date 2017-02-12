#ifndef __YUNI_AUDIO_SOUND_HXX__
# define __YUNI_AUDIO_SOUND_HXX__


namespace Yuni
{
namespace Audio
{


	inline Sound::Sound(Private::Audio::AudioStream* stream)
		:pStream(stream), pBufferCount(0)
	{
	}


	inline Sound::~Sound()
	{
	}


	inline unsigned int Sound::duration() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return Private::Audio::AV::GetStreamDuration(pStream);
	}



} // namespace Audio
} // namespace Yuni

#endif // __YUNI_AUDIO_SOUND_HXX__
