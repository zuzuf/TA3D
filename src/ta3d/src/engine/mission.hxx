#ifndef __TA3D_ENGINE_MISSION_HXX__
# define __TA3D_ENGINE_MISSION_HXX__


namespace TA3D
{


	inline Mission::Mission()
		: qStep(), time(0.), last_d(0.), move_data(0), path(),
		node(0)
	{}

	inline Mission::Mission(const Mission& rhs)
		:qStep(rhs.qStep), time(rhs.time), last_d(rhs.last_d), move_data(rhs.move_data),
		path(rhs.path), node(rhs.node)
	{}


	inline Mission::~Mission()
	{}


	inline Mission& Mission::operator = (const Mission& rhs)
	{
		qStep = rhs.qStep;
		time = rhs.time;
		last_d = rhs.last_d;
		move_data = rhs.move_data;
		path = rhs.path;
		node = rhs.node;
		return *this;
	}




} // namespace TA3D


#endif // __TA3D_ENGINE_MISSION_HXX__
