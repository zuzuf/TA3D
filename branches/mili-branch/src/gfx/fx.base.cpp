
#include "fx.base.h"
#include "fx.manager.h"


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


    bool FX::move(float dt,ANIM **anims)
    {
        if(!playing)	return false;
        if(anm == -1) // Flash effect
        {
            if( time > 1.0f )
            {
                playing=false;
                return true;
            }
            time+=dt;
            return false;
        }
        if (anm == -2 || anm == -3 || anm == -4 || anm == -5 ) // Wave effect on shores or ripple
        {
            if( time > 4.0f || ( time > 2.0f && anm == -5 ) )
            {
                playing = false;
                return true;
            }
            time += dt;
            return false;
        }
        if(anm < 0)
        {
            playing=false;
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

    void FX::load(int anim,VECTOR P,float s)
    {
        destroy();

        anm = anim;
        Pos = P;
        size = s*0.25f;
        time = 0.0f;
        playing = true;
    }


    void FX::draw(CAMERA *cam, MAP *map, ANIM **anims)
    {
        if(!playing)
            return;
        if(map!=NULL)
        {
            int px=(int)(Pos.x+map->map_w*0.5f)>>4;
            int py=(int)(Pos.z+map->map_h*0.5f)>>4;
            if (px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)
                return;
            byte player_mask = 1 << players.local_human_id;
            if((map->view[py][px]!=1
                || !(map->sight_map->line[py][px]&player_mask))
               && ( anm > -2 || anm < -4 ))
                return;
        }

        if( anm == -1 ) // It's a flash
        {
            glDisable( GL_DEPTH_TEST );
            glBindTexture(GL_TEXTURE_2D, fx_manager.flash_tex);
            glBlendFunc(GL_ONE,GL_ONE);

            float rsize = -4.0f * time * ( time - 1.0f ) * size;

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);	glVertex3f(Pos.x - rsize,Pos.y,Pos.z - rsize);
            glTexCoord2f(1.0f,0.0f);	glVertex3f(Pos.x + rsize,Pos.y,Pos.z - rsize);
            glTexCoord2f(1.0f,1.0f);	glVertex3f(Pos.x + rsize,Pos.y,Pos.z + rsize);
            glTexCoord2f(0.0f,1.0f);	glVertex3f(Pos.x - rsize,Pos.y,Pos.z + rsize);
            glEnd();
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glEnable( GL_DEPTH_TEST );

            return;
        }
        if( anm == -2 || anm == -3 || anm == -4 ) // It's a wave
        {
            glPolygonOffset(0.0f, 0.0f);
            glBindTexture(GL_TEXTURE_2D, fx_manager.wave_tex[ anm + 4 ]);
            gfx->set_alpha_blending();

            glPushMatrix();

            glTranslatef( Pos.x, Pos.y, Pos.z );
            glRotatef( size, 0.0f, 1.0f, 0.0f );

            float wsize = 24.0f;
            float hsize = 8.0f;
            float dec = time * 0.125f;

            glColor4f( 1.0f, 1.0f, 1.0f, 1.0f - 0.5f * fabs( 2.0f - time ) );

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,dec );		glVertex3f(-wsize,4.0f,-hsize);
            glTexCoord2f(1.0f,dec );		glVertex3f(wsize,4.0f,-hsize);
            glTexCoord2f(1.0f,dec+1.0f );	glVertex3f(wsize,0.0f,hsize);
            glTexCoord2f(0.0f,dec+1.0f );	glVertex3f(-wsize,0.0f,hsize);
            glEnd();

            glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

            glPopMatrix();

            gfx->unset_alpha_blending();

            glPolygonOffset(0.0f,-1600.0f);
            return;
        }
        if( anm == -5 ) // It's a ripple
        {
            glPolygonOffset(0.0f,0.0f);
            glBindTexture(GL_TEXTURE_2D, fx_manager.ripple_tex);
            gfx->set_alpha_blending();

            glPushMatrix();

            glTranslatef( Pos.x, Pos.y, Pos.z );
            glRotatef( size*time, 0.0f, 1.0f, 0.0f );

            float rsize = 16.0f * time;

            glColor4f( 1.0f, 1.0f, 1.0f, 0.5f - 0.25f * time );

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f );		glVertex3f(-rsize,0.0f,-rsize);
            glTexCoord2f(1.0f,0.0f );		glVertex3f(rsize,0.0f,-rsize);
            glTexCoord2f(1.0f,1.0f );	glVertex3f(rsize,0.0f,rsize);
            glTexCoord2f(0.0f,1.0f );	glVertex3f(-rsize,0.0f,rsize);
            glEnd();

            glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

            glPopMatrix();

            gfx->unset_alpha_blending();

            glPolygonOffset(0.0f,-1600.0f);
            return;
        }

        if (NULL == anims[anm])
        {
            playing=false;
            return;
        }
        int img=(int)(time*15.0f);
        float wsize=size*anims[anm]->w[img];
        float hsize=size*anims[anm]->h[img];
        glBindTexture(GL_TEXTURE_2D,anims[anm]->glbmp[img]);

        float hux=hsize*cam->Up.x;
        float wsx=wsize*cam->Side.x;
        float huy=hsize*cam->Up.y;
        float wsy=wsize*cam->Side.y;
        float huz=hsize*cam->Up.z;
        float wsz=wsize*cam->Side.z;

        glPushMatrix();
        glTranslatef( Pos.x, Pos.y, Pos.z );

        if(cam->mirror)
        {
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);	glVertex3f(  hux-wsx, -huy+wsy,  huz-wsz);
            glTexCoord2f(1.0f,0.0f);	glVertex3f(  hux+wsx, -huy-wsy,  huz+wsz);
            glTexCoord2f(1.0f,1.0f);	glVertex3f( -hux+wsx,  huy-wsy, -huz+wsz);
            glTexCoord2f(0.0f,1.0f);	glVertex3f( -hux-wsx,  huy+wsy, -huz-wsz);
            glEnd();
        }
        else
        {
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);	glVertex3f(  hux-wsx,  huy-wsy,  huz-wsz);
            glTexCoord2f(1.0f,0.0f);	glVertex3f(  hux+wsx,  huy+wsy,  huz+wsz);
            glTexCoord2f(1.0f,1.0f);	glVertex3f( -hux+wsx, -huy+wsy, -huz+wsz);
            glTexCoord2f(0.0f,1.0f);	glVertex3f( -hux-wsx, -huy-wsy, -huz-wsz);
            glEnd();
        }
        glPopMatrix();
    }


} // namespace TA3D
