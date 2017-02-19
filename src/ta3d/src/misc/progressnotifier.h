#ifndef PROGRESSNOTIFIER_H
#define PROGRESSNOTIFIER_H

#include "string.h"
#include <zuzuf/smartptr.h>

namespace TA3D
{

    class ProgressNotifier : public zuzuf::ref_count
	{
	public:
		virtual ~ProgressNotifier()	{}

		virtual void operator()(const float percent, const String &message) = 0;
	};

}

#endif // PROGRESSNOTIFIER_H
