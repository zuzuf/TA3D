#ifndef __LIBYUNI_CONFIG_PROGRAM_HXX__
# define __LIBYUNI_CONFIG_PROGRAM_HXX__




namespace Yuni
{


	inline bool LibConfigProgram::isCoreModule(const String& name) const
	{
		return (name == "core" || name == "gfx");
	}



} // namespace Yuni

#endif // __LIBYUNI_CONFIG_PROGRAM_HXX__
