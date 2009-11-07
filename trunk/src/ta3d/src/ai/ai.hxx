#ifndef __TA3D_AI_AI_HXX__
# define __TA3D_AI_AI_HXX__


namespace TA3D
{


	inline int AI_PLAYER::getType() const
	{
		return type;
	}


	inline AiScript::Ptr AI_PLAYER::getAiScript() const
	{
		return ai_script;
	}



} // namespace TA3D

#endif // __TA3D_AI_AI_HXX__
