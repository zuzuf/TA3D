#ifndef __YUNI_AUDIO_EMITTER_H__
# define __YUNI_AUDIO_EMITTER_H__

# include "../yuni.h"
# include <map>
# include "../core/string.h"
# include "../core/point3D.h"
# include "../core/vector3D.h"
# include "sound.h"


namespace Yuni
{
namespace Audio
{

	/*!
	** \brief An audio emitter is an object from which the sound is played
	**
	** Audio emitters can be placed in space, and be moved around.
	*/
	class Emitter: public Policy::ObjectLevelLockable<Emitter>
	{
	public:
		//! The most suitable smart pointer for the class
		typedef SmartPtr<Emitter> Ptr;
		//! Threading Policy
		typedef Policy::ObjectLevelLockable<Emitter> ThreadingPolicy;
		//! Map
		typedef std::map<String, Ptr> Map;

	public:
		//! Default value for pitch (1.0)
		static const float DefaultPitch;
		//! Default value for gain (1.0)
		static const float DefaultGain;
		//! Default value for attenuation (enabled)
		static const bool DefaultAttenuation;
		//! Default value for looping (false)
		static const bool DefaultLooping;

	public:
		//! \name Constructors & Destructor
		//@{
		/*!
		** \brief Shortest constructor
		**
		** Position, speed and direction default to (0,0,0)
		*/
		Emitter(bool loop = DefaultLooping);

		/*!
		** \brief Constructor with 3D position
		**
		** Speed and velocity default to (0,0,0)
		*/
		Emitter(const Point3D<>& position, bool loop);

		/*!
		** \brief Constructor with position, velocity and direction
		*/
		Emitter(const Point3D<>& position, const Vector3D<>& velocity,
			const Vector3D<>& direction, bool loop);

		/*!
		** \brief Destructor
		*/
		~Emitter() {}
		//@}


		//! \name Methods
		//@{
		//! Attach a buffer to the emitter
		bool attachBufferDispatched(Sound::Ptr& buffer);

		//! Prepare the emitter for playing
		bool prepareDispatched();

		//! Play the sound
		bool playSoundDispatched();
		//! Play the sound
		bool playSoundDispatched(Sound::Ptr& buffer);

		//! Update buffers if necessary
		bool updateDispatched();
		//@}


		//! \name Accessors
		//@{
		//! Set the 3D position of the emitter
		void position(const Point3D<>& position);
		//! Get the 3D position of the emitter
		Point3D<> position() const;

		//! Set the velocity of the emitter
		void velocity(const Vector3D<>& position);
		//! Get the velocity of the emitter
		Vector3D<> velocity() const;

		//! Set the direction of the emitter
		void direction(const Vector3D<>& position);
		//! Get the direction of the emitter
		Vector3D<> direction() const;

		/*!
		** \brief Set the volume modifier on the emitter
		** \param newGain 0.0f for no sound, 1.0f to keep sound as is, > 1.0f to amplify sound
		*/
		//! Set the volume modifier
		void gain(float newGain);
		//! Get the current volume modifier
		float gain() const;

		/*!
		** \brief Get the elapsed playback time (in seconds)
		** \returns Time elapsed since the emitter started playback. 0 if not playing.
		*/
		sint64 elapsedTime() const;

		//! Get the identifier for the emitter
		unsigned int id() const;
		//@}


	private:
		Emitter(const Emitter&);
		Emitter& operator= (const Emitter&);

	private:
		//! String identifier for the emitter
		String pName;
		//! OpenAL identifier for the emitter
		unsigned int pID;
		//! Position of the emitter in space
		Point3D<> pPosition;
		//! Speed of the emitter
		Vector3D<> pVelocity;
		//! Direction of the movement of the emitter
		Vector3D<> pDirection;
		//! Should the emitter loop on itself when finished playing?
		bool pLoop;
		//! Volume modifier, 1.0 means no modification
		float pGain;
		//! Current playback position
		sint64 pStartTime;
		//! Sound to play. NULL if none
		Sound::Ptr  pBuffer;

		//! Is the emitter ready for use?
		bool pReady;
		//! Is the emitter currently playing?
		bool pPlaying;
		//! Has the emitter's values been modified ?
		bool pModified;

	}; // class Emitter




} // namespace Audio
} // namespace Yuni

# include "emitter.hxx"

#endif // __YUNI_AUDIO_EMITTER_H__
