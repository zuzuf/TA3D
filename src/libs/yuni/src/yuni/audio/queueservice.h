#ifndef __YUNI_AUDIO_QUEUESERVICE_H__
# define __YUNI_AUDIO_QUEUESERVICE_H__

# include "../yuni.h"
# include <map>
# include "../core/atomic/int.h"
# include "../core/point3D.h"
# include "../core/vector3D.h"
# include "../core/string.h"
# include "../core/smartptr.h"
# include "../thread/policy.h"
# include "../thread/condition.h"
# include "emitter.h"
# include "loop.h"
# include "sound.h"


namespace Yuni
{
namespace Audio
{

	/*!
	** \brief The audio queue service is the service that manages everything sound-related
	**
	** It takes care of ffmpeg / openal inits.
	** It uses an event loop to solve MT problems.
	*/
	class QueueService: public Policy::ObjectLevelLockable<QueueService>
	{
	public:
		//! The threading policy
		typedef Policy::ObjectLevelLockable<QueueService>  ThreadingPolicy;

	public:

		//! Forward declaration
		class Bank;

		/*!
		** \brief This is the the access to all the emitters for this queue service
		**
		** All the emitters are managed here.
		** It can be accessed through: queueService.emitter
		*/
		class Emitters: public Policy::ObjectLevelLockable<Emitters>
		{
		public:
			typedef Policy::ObjectLevelLockable<Emitters>  ThreadingPolicy;

		private:
			Emitters()
			{}
			Emitters(const Emitters&);

			//! Map of currently registered emitters, with string tags as keys
			Emitter::Map pEmitters;

		public:
			//! Get an emitter
			template<typename StringT>
			Emitter::Ptr get(const StringT& name);

			//! Add an emitter
			template<typename StringT>
			bool add(const StringT& name);

			//! Attach an emitter to a buffer
			template<typename StringT, typename StringT2>
			bool attach(const StringT& name, const StringT2& attachedBuffer);
			//! Attach an emitter to a buffer
			template<typename StringT>
			bool attach(Emitter::Ptr name, const StringT& attachedBuffer);

			//! Modify an emitter's data
			template<typename StringT>
			bool modify(const StringT& name, bool loop);
			//! Modify an emitter's data
			bool modify(Emitter::Ptr name, bool loop);

			//! Move an emitter around
			template<typename StringT>
			bool move(const StringT& name, const Point3D<>& position);
			//! Move an emitter around
			bool move(Emitter::Ptr emitter, const Point3D<>& position);
			template<typename StringT>
			//! Move an emitter around
			bool move(const StringT& name, const Point3D<>& position,
				const Vector3D<>& velocity, const Vector3D<>& direction);
			//! Move an emitter around
			bool move(Emitter::Ptr emitter, const Point3D<>& position,
				const Vector3D<>& velocity, const Vector3D<>& direction);

			//! Get elapsed playback time on an emitter
			template<typename StringT>
			sint64 elapsedTime(const StringT& name);
			//! Get elapsed playback time on an emitter
			sint64 elapsedTime(Emitter::Ptr emitter);

			//! Start playback on an emitter
			template<typename StringT>
			bool play(const StringT& name);
			//! Start playback on an emitter
			bool play(Emitter::Ptr emitter);

			//! Stop playback on an emitter
			template<typename StringT>
			bool stop(const StringT& name);
			//! Stop playback on an emitter
			bool stop(const Emitter::Ptr& emitter);

			//! Remove an emitter
			template<typename StringT>
			bool remove(const StringT& name);
			//! Remove an emitter
			bool remove(Emitter::Ptr name);

		private:
			//! Friend declaration
			friend class QueueService;

			//! Associated queue service
			QueueService* pQueueService;
			//! Associated bank
			Bank* pBank;
		};




		/*!
		** \brief The bank contains the audio buffers currently loaded in the queue service
		**
		** It can be accessed through: queueService.bank
		*/
		class Bank: public Policy::ObjectLevelLockable<Bank>
		{
		public:
			typedef Policy::ObjectLevelLockable<Bank>  ThreadingPolicy;

		private:
			//! \name Constructors
			//@{
			//! Empty constructor
			Bank()
			{}

			//! Copy constructor
			Bank(const Bank&);
			//@}

			//! Map of currently loaded buffers, with string tags as keys
			Sound::Map pBuffers;

		public:
			//! Clear the bank, free the loaded buffers
			void clear();

			/*!
			** \brief Load sound file from given path
			**
			** \param name Path to file, used from now on as an identifier for the sound
			*/
			template<typename StringT>
			bool load(const StringT& name);

			//! Get the duration of a loaded sound
			template<typename StringT>
			unsigned int duration(const StringT& name);


		private:
			//! Get a sound from the bank
			template<typename StringT>
			Sound::Ptr get(const StringT& name);

		private:
			//! Friend declaration
			friend class QueueService;
			//! Friend declaration
			friend class QueueService::Emitters;

			//! Associated queue service
			QueueService* pQueueService;
		};




	public:
		//! \name Constructor and destructor
		//@{
		//! Constructor
		QueueService(): pReady(false), pAudioLoop(this)
		{
			bank.pQueueService = this;
			emitter.pQueueService = this;
			emitter.pBank = &bank;
		}
		//! Destructor
		~QueueService()
		{
			if (pReady)
				stop();
		}
		//@}

	public:
		/*!
		** \brief Start the audio service
		*/
		bool start();

		/*!
		** \brief Stop the audio service
		*/
		void stop();

	public:
		//! Control block for emitters
		Emitters emitter;
		//! Control block for audio buffers
		Bank bank;

	private:
		//! \name Private operations
		//@{
		/*!
		** \brief Copy constructor
		*/
		QueueService(const QueueService&);

		/*!
		** \brief Assignment
		*/
		QueueService& operator = (const QueueService&);
		//@}


	private:
		//! This is meant to aggregate a condition and a boolean for dispatching
		struct InitData
		{
			InitData(Thread::Condition& c, bool& r): condition(c), ready(r) {}

			Thread::Condition& condition;
			bool& ready;
		};

		//! This is meant to aggregate a condition with the emitter
		struct EmitterPositionData
		{
			EmitterPositionData(Thread::Condition& c, float& f): condition(c), data(f) {}

			Thread::Condition& condition;
			float& data;
		};


	private:
		/*!
		** \brief Audio initialization
		**
		** \note Dispatched in the audio loop
		*/
		bool initDispatched(const InitData& initData);

		/*!
		** \brief Sound loading
		**
		** \note Dispatched in the audio loop
		*/
		bool loadSoundDispatched(const String& filePath);

		/*!
		** \brief Buffer update
		**
		** \note Called in the Audio::Loop::onLoop()
		*/
		bool updateDispatched();

	private:
		//! Static to make sure only one manager is started
		static Atomic::Int<32> sHasRunningInstance;

		//! Has the manager been properly started ?
		bool pReady;
		//! Event loop for audio events
		Loop pAudioLoop;

	private:
		//! Friend declaration
		friend class Loop;
		//! Friend declaration
		friend class Emitters;
		//! Friend declaration
		friend class Bank;

	}; // class QueueService




} // namespace Audio
} // namespace Yuni

#include "queueservice.hxx"

#endif // __YUNI_AUDIO_QUEUESERVICE_H__
