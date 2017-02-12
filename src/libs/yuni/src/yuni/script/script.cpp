
#include "script.h"


namespace Yuni
{
namespace Script
{


	AScript::~AScript()
	{
		clearWithoutUnbindWL();
	}


	bool AScript::loadFromFile(const String& file)
	{
		this->reset();
		return appendFromFile(file);
	}


	bool AScript::loadFromString(const String& script)
	{
		this->reset();
		return appendFromString(script);
	}


	bool AScript::loadFromBuffer(const char* scriptBuf, const unsigned int scriptSize)
	{
		this->reset();
		return appendFromBuffer(scriptBuf, scriptSize);
	}


	bool AScript::run()
	{
		return call(NULL, "main");
	}


	void AScript::clear()
	{
		ThreadingPolicy::MutexLocker locker(*this);

		if (!pBoundFunctions.empty())
		{
			const BoundFunctions::iterator end = pBoundFunctions.end();
			for (BoundFunctions::iterator i = pBoundFunctions.begin(); i != end; ++i)
			{
				internalUnbindWL(i->first.c_str());
				delete i->second;
			}
			pBoundFunctions.clear();
		}
	}


	void AScript::clearWithoutUnbindWL()
	{
		if (!pBoundFunctions.empty())
        {
            const BoundFunctions::iterator end = pBoundFunctions.end();
            for (BoundFunctions::iterator i = pBoundFunctions.begin(); i != end; ++i)
            {
                // We do not unbind. See the name of the function.
                delete i->second;
            }
			pBoundFunctions.clear();
        }
	}




} // namespace Script
} // namespace Yuni
