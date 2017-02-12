#ifndef __YUNI_CORE_PROCESS_PROCESS_HXX__
# define __YUNI_CORE_PROCESS_PROCESS_HXX__


namespace Yuni
{

	inline bool Process::running() const
	{
		return !(!pRunning);
	}





} // namespace Yuni

#endif // __YUNI_CORE_PROCESS_PROCESS_HXX__
