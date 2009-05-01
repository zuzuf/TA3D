#ifndef __TA3D_ENGINE_MISSION_HXX__
# define __TA3D_ENGINE_MISSION_HXX__


namespace TA3D
{


	inline Mission::Mission()
		:time(0.), last_d(0.), data(0), move_data(0), next(NULL), mission(0), path(NULL),
		target(), step(false), flags(0), p(NULL), target_ID(0), node(0)
	{}

	inline Mission::Mission(const Mission& rhs)
		:time(rhs.time), last_d(rhs.last_d), data(rhs.data), move_data(rhs.move_data),
		next(rhs.next), mission(rhs.mission), path(rhs.path),
		target(rhs.target), step(rhs.step), flags(rhs.flags), p(rhs.p),
		target_ID(rhs.target_ID), node(rhs.node)
	{}


	inline Mission::~Mission()
	{}


	inline Mission& Mission::operator = (const Mission& rhs)
	{
		time = rhs.time;
		last_d = rhs.last_d;
		data = rhs.data;
		move_data = rhs.move_data;
		next = rhs.next;
		mission = rhs.mission;
		path = rhs.path;
		target = rhs.target;
		step = rhs.step;
		flags = rhs.flags;
		p = rhs.p;
		target_ID = rhs.target_ID;
		node = rhs.node;
		return *this;
	}




} // namespace TA3D


#endif // __TA3D_ENGINE_MISSION_HXX__
