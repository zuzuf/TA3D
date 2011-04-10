#ifndef __TA3D_LOGS_LOGS_HXX__
# define __TA3D_LOGS_LOGS_HXX__


namespace TA3D
{


	template<class U> inline void ResetTheLoggingMecanism(const U& logfile)
	{
		logs.applicationName("ta3d");
		logs.logfile(logfile);
	}


} // namespace TA3D

#endif // __TA3D_LOGS_LOGS_HXX__
