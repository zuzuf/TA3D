/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "fx.base.h"
#include "fx.manager.h"
#include <logs/logs.h>
#include <ingame/players.h>
#include <EngineClass.h>




namespace TA3D
{


    FX::FX()
        :time(0.0f), playing(false), Pos(), size(1.0f), anm(0)
    {}

    FX::~FX()
    {
        destroy();
    }

    void FX::init()
    {
        time = 0.0f;
        playing = false;
        Pos.x = Pos.y = Pos.z = 0.0f;
        size = 1.0f;
        anm = 0;
    }

    void FX::destroy()
    {
        init();
    }


	bool FX::move(const float dt, const std::vector<Gaf::Animation*> &anims)
    {
        if (!playing)
            return false;
        if (anm == -1) // Flash effect
        {
            if (time > 1.0f)
            {
                playing = false;
                return true;
            }
            time += dt;
            return false;
        }
        if (anm == -2 || anm == -3 || anm == -4 || anm == -5 ) // Wave effect on shores or ripple
        {
            if( time > 4.0f || (time > 2.0f && anm == -5))
            {
                playing = false;
                return true;
            }
            time += dt;
            return false;
        }
        if(anm < 0)
        {
            playing = false;
            return false;
        }
        time += dt;
        if(time * 15.0f >= anims[anm]->nb_bmp)
        {
            playing = false;
            return true;
        }
        return false;
    }


    void FX::load(const int anim, const Vector3D& p, const float s)
    {
        destroy();
        anm = anim;
        Pos = p;
		size = s * 0.25f;
        time = 0.0f;
		angle = float(TA3D::Math::RandomTable() % 3600) * 0.1f;
        playing = true;
    }

    void FX::doDrawAnimFlash()
    {
        glDisable(GL_DEPTH_TEST);
		fx_manager.flash_tex.bind();
        glBlendFunc(GL_ONE,GL_ONE);

		const float t = std::min(time, 1.0f);
		const float rsize = -4.0f * t * ( t - 1.0f ) * size;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);	glVertex3f(Pos.x - rsize,Pos.y,Pos.z - rsize);
        glTexCoord2f(1.0f,0.0f);	glVertex3f(Pos.x + rsize,Pos.y,Pos.z - rsize);
        glTexCoord2f(1.0f,1.0f);	glVertex3f(Pos.x + rsize,Pos.y,Pos.z + rsize);
        glTexCoord2f(0.0f,1.0f);	glVertex3f(Pos.x - rsize,Pos.y,Pos.z + rsize);
        glEnd();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable( GL_DEPTH_TEST );
    }


    void FX::doDrawAnimWave(const int animIndx)
    {
		fx_manager.wave_tex[animIndx + 4].bind();

        glPushMatrix();

        glTranslatef(Pos.x, Pos.y, Pos.z);
        glRotatef(size, 0.0f, 1.0f, 0.0f);

		const float wsize = 0.25f * float(fx_manager.wave_tex[animIndx + 4].getWidth());
		const float hsize = 0.25f * float(fx_manager.wave_tex[animIndx + 4].getHeight());
		const float dec = time * 0.125f;

        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f - 0.5f * fabsf( 2.0f - time ) );

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,dec );       glVertex3f(-wsize,4.0f,-hsize);
        glTexCoord2f(1.0f,dec );       glVertex3f(wsize,4.0f,-hsize);
        glTexCoord2f(1.0f,dec+1.0f );  glVertex3f(wsize,0.0f,hsize);
        glTexCoord2f(0.0f,dec+1.0f );  glVertex3f(-wsize,0.0f,hsize);
        glEnd();

		glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
        glPopMatrix();
    }

    void FX::doDrawAnimRipple()
    {
		fx_manager.ripple_tex.bind();

        glPushMatrix();

        glTranslatef(Pos.x, Pos.y, Pos.z);
		glRotatef(size * time + angle, 0.0f, 1.0f, 0.0f);

        float rsize = 16.0f * time;

        glColor4f(1.0f, 1.0f, 1.0f, 0.5f - 0.25f * time);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f ); glVertex3f( -rsize, 0.0f, -rsize);
        glTexCoord2f(1.0f,0.0f ); glVertex3f(  rsize, 0.0f, -rsize);
        glTexCoord2f(1.0f,1.0f ); glVertex3f(  rsize, 0.0f,  rsize);
        glTexCoord2f(0.0f,1.0f ); glVertex3f( -rsize, 0.0f,  rsize);
        glEnd();

		glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
		glPopMatrix();
    }

	void FX::doDrawAnimDefault(Camera& cam, const std::vector<Gaf::Animation*> &anims)
    {
		if (anims.empty())
        {
            playing = false;
            return;
        }
		const int img = (int)round((time * 15.0f));
        if (img >= anims[anm]->nb_bmp )
        {
            playing = false;
            return;
        }
		const float wsize = size * float(anims[anm]->w[img]);
		const float hsize = size * float(anims[anm]->h[img]);
        glBindTexture(GL_TEXTURE_2D,anims[anm]->glbmp[img]);

		const float hux = hsize * cam.up.x;
		const float wsx = wsize * cam.side.x;
		const float huy = hsize * cam.up.y;
		const float wsy = wsize * cam.side.y;
		const float huz = hsize * cam.up.z;
		const float wsz = wsize * cam.side.z;

        glPushMatrix();
        glTranslatef(Pos.x, Pos.y, Pos.z);

        if (cam.mirror)
        {
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f); glVertex3f(  hux-wsx, -huy+wsy,  huz-wsz);
            glTexCoord2f(1.0f,0.0f); glVertex3f(  hux+wsx, -huy-wsy,  huz+wsz);
            glTexCoord2f(1.0f,1.0f); glVertex3f( -hux+wsx,  huy-wsy, -huz+wsz);
            glTexCoord2f(0.0f,1.0f); glVertex3f( -hux-wsx,  huy+wsy, -huz-wsz);
            glEnd();
        }
        else
        {
			const Vector3D vDir(-0.5f * (hsize + wsize) * cam.dir);
			glTranslatef(vDir.x, vDir.y, vDir.z);
			const float s = 1.0f - 0.5f * (hsize + wsize) / (cam.pos - Pos).norm();
			glScalef(s, s, s);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f); glVertex3f(  hux-wsx,  huy-wsy,  huz-wsz);
            glTexCoord2f(1.0f,0.0f); glVertex3f(  hux+wsx,  huy+wsy,  huz+wsz);
            glTexCoord2f(1.0f,1.0f); glVertex3f( -hux+wsx, -huy+wsy, -huz+wsz);
            glTexCoord2f(0.0f,1.0f); glVertex3f( -hux-wsx, -huy-wsy, -huz-wsz);
            glEnd();
        }
        glPopMatrix();
    }


	bool FX::doCanDrawAnim() const
    {
		const int px = (int)(Pos.x + float(the_map->map_w_d)) >> 4;
		const int py = (int)(Pos.z + float(the_map->map_h_d)) >> 4;
		if (px < 0 || py < 0 || px >= the_map->bloc_w || py >= the_map->bloc_h)
            return false;
		const byte player_mask = byte(1 << players.local_human_id);
		return ! (((the_map->view(px, py) != 1 || !(the_map->sight_map(px, py) & player_mask))
           && (anm > -2 || anm < -4)));
    }


	void FX::draw(Camera& cam, const std::vector<Gaf::Animation*> &anims)
    {
		if(!playing || !doCanDrawAnim())
            return;
        switch (anm)
        {
            // Flash
            case -1 : { doDrawAnimFlash(); break; }
                      // Waves
            case -2 :
            case -3 :
			case -4 :
				if (lp_CONFIG->water_quality < 5)
					doDrawAnimWave(anm);
				break;
			case -5 :
				if (lp_CONFIG->water_quality < 5)
					doDrawAnimRipple();
				break;
            default : doDrawAnimDefault(cam, anims);
        }
    }

	void FX::drawWaterDistortions()
	{
		switch (anm)
		{
					  // Waves
			case -2 :
			case -3 :
			case -4 : { doDrawAnimWave(anm); break; }
			case -5 : { doDrawAnimRipple(); break; }
		}
	}

} // namespace TA3D
