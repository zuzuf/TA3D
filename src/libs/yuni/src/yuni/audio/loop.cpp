#include "loop.h"
#include "queueservice.h"

namespace Yuni
{
namespace Audio
{

	Loop::Loop(QueueService* audioService)
		:pAudioService(audioService)
	{}


	bool Loop::onLoop()
	{
		if (!pAudioService)
			return false;
		pAudioService->updateDispatched();
		return true;
	}



} // namespace Audio
} // namespace Yuni
