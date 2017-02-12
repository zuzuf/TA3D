#include <set>
#include "id.h"
#include "../thread/mutex.h"

namespace Yuni
{
namespace UI
{

	namespace // Anonymous
	{

		ID::Type NextID = ID::MinID;

		std::set<ID::Type> UsedIDs;

		Yuni::Mutex IDMutex;

	} // namespace Anonymous




	ID::Type ID::New()
	{
		unsigned int count = 0;
		IDMutex.lock();
		do
		{
			++NextID;
			if (InvalidID == NextID)
			{
				if (2 == ++count)
					throw "All UI IDs are taken !";
				continue;
			}
			if (UsedIDs.end() == UsedIDs.find(NextID))
				break;
		}
		while (true);
		UsedIDs.insert(NextID);
		IDMutex.unlock();
		return NextID;
	}



} // namespace UI
} // namespace Yuni
