#ifndef __TA3D_CACHE_H__
# define __TA3D_CACHE_H__


namespace TA3D
{
namespace Cache
{


	/*!
	** \brief Clear the cache if needed (useful when mod has changed)
	**
	** \param force True to force the cleaning
	*/
	void Clear(const bool force = false);


} // namespace Cache
} // namespace TA3D

#endif // __TA3D_CACHE_H__
