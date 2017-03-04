#ifndef __TA3D__GFX_Sky_H__
#define __TA3D__GFX_Sky_H__

#include "gfx.h"
#include <misc/vector.h>

namespace TA3D
{

	class Sky
	{
	private:
        class SkyData : public zuzuf::ref_count
		{
        public:
            typedef zuzuf::smartptr<SkyData>    Ptr;
		public:
			float			rotation_speed;
			float			rotation_offset;	// If you want the sun to match light dir ...
			QString			texture_name;		// Name of the texture used as sky
            QStringList     planet;				// Vector of planets that can use this sky
			float			FogColor[4];		// Color of the fog to use with this sky
            QStringList     MapName;			// Name of maps linked with this sky
			bool			full_sphere;		// The texture is for the whole sphere
			bool			def;

		public:
			SkyData();
			~SkyData();

            void load_tdf( const QString &filename );
		};

	private:
		int			s;
		float		w;
        GfxTexture::Ptr tex[6];			// Our cube textures
        SkyData::Ptr skyInfo;

	public:
		Sky();
		~Sky();

		void init();
		void destroy();

		float getW() const {	return w;	}
		const float *fogColor() const {	return skyInfo->FogColor;	}
		float rotationOffset() const	{	return skyInfo->rotation_offset;	}
		float rotationSpeed() const	{	return skyInfo->rotation_speed;	}

		void draw();
        GfxTexture::Ptr skyTex() const	{	return tex[5];	}

		void choose_a_sky( const QString &mapname, const QString &planet );

	protected:
		void build(int d, float size);
	};
}

#endif
