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

#include "battle.h"
#include "players.h"
#include <UnitEngine.h>
#include <gfx/fx.h>
#include <gfx/gfx.toolkit.h>
#include <input/keyboard.h>
#include <input/mouse.h>
#include <misc/timer.h>
#include <misc/paths.h>

#include <iostream>


namespace TA3D
{

	void Battle::initRenderer()
	{
		cam_h = cam.rpos.y - map->get_unit_h(cam.rpos.x, cam.rpos.z);

		updateZFAR();

		gfx->SetDefState();
		updateFOG();

		render_time = ((float)units->current_tick) / TICKS_PER_SEC;

		// Copy unit data from simulation
        units->renderTick();

        gfx->glActiveTexture(GL_TEXTURE7_ARB);
        CHECK_GL();
        gfx->glDisable(GL_TEXTURE_2D);
        CHECK_GL();
        gfx->glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL();
        gfx->glActiveTexture(GL_TEXTURE0_ARB);
        CHECK_GL();
        gfx->setShadowMapMode(false);
        CHECK_GL();
        gfx->glUseProgram(0);
        CHECK_GL();
    }


	void Battle::renderReflection()
	{
		// Dessine les reflets sur l'eau / Render water reflection
        if (lp_CONFIG->water_quality >= 2 && map->water && !map->ota_data.lavaworld && !reflection_drawn_last_time)
		{
			reflection_drawn_last_time = true;

            gfx->renderToTexture(reflectex, true);

			gfx->clearAll();		// Clear screen

            gfx->glViewport(0, 0, 512, 512);
            CHECK_GL();

			gfx->ReInitAllTex();
            glColor4f(1.0f,1.0f,1.0f,1.0f);
            CHECK_GL();
            gfx->glDisable(GL_BLEND);
            CHECK_GL();

			double eqn[4]= { 0.0f, 1.0f, 0.0f, -map->sealvl };

			Camera refcam = cam;
			refcam.zfar *= 2.0f;
			refcam.mirror = true;
			refcam.mirrorPos = -2.0f * map->sealvl;

			refcam.setView();
            glClipPlane(GL_CLIP_PLANE1, eqn);
            CHECK_GL();
            glEnable(GL_CLIP_PLANE1);
            CHECK_GL();

			pSun.Set(refcam);
			pSun.Enable();

			refcam.zfar *= 100.0f;
			refcam.setView();
			glColor4ub(0xFF,0xFF,0xFF,0xFF);
            CHECK_GL();
            gfx->glEnable(GL_TEXTURE_2D);
            CHECK_GL();
            if (lp_CONFIG->render_sky)
			{
                gfx->glDisable(GL_FOG);
                CHECK_GL();
                gfx->glDisable(GL_LIGHTING);
                CHECK_GL();
                gfx->glDepthMask(GL_FALSE);
                CHECK_GL();

                gfx->glCullFace(GL_FRONT);
                CHECK_GL();
                glTranslatef(cam.rpos.x,-map->sealvl,cam.rpos.z);
                CHECK_GL();
                glRotatef( sky_angle, 0.0f, 1.0f, 0.0f);
                CHECK_GL();
                float scale_factor = 15.0f * ( cam.rpos.y + cam.shakeVector.y + sky.getW()) / sky.getW();
				glScalef( scale_factor, scale_factor, scale_factor);
                CHECK_GL();
                sky.draw();
			}
			refcam.zfar = (500.0f + (cam_h - 150.0f) * 2.0f) * 2.0f;
            gfx->glDepthMask(GL_TRUE);
            CHECK_GL();
            gfx->glEnable(GL_CULL_FACE);
            CHECK_GL();
            gfx->glEnable(GL_LIGHTING);
            CHECK_GL();
            gfx->glEnable(GL_FOG);
            CHECK_GL();
            gfx->glCullFace(GL_FRONT);
            CHECK_GL();
            refcam.setView();

			if (cam.rpos.y <= gfx->low_def_limit && lp_CONFIG->water_quality >= 4)
			{
				map->draw(&refcam, byte(1 << players.local_human_id),  false, 0.0f, t,
						  dt * units->apparent_timefactor,
						  false, false, false);

				// Dessine les éléments "2D" / "sprites"
				features->draw(render_time);
				refcam.setView();
				// Dessine les unités / draw units
                units->draw(false, true, false, lp_CONFIG->height_line);

                gfx->glDisable(GL_CULL_FACE);
                CHECK_GL();
                // Dessine les objets produits par les armes / draw weapons
                weapons.draw(map);
				// Dessine les particules
				refcam.setView(true);
				glClipPlane(GL_CLIP_PLANE1, eqn);
                CHECK_GL();

				particle_engine.draw(&refcam);

				refcam.setView();
				glClipPlane(GL_CLIP_PLANE1, eqn);
                CHECK_GL();
                // Effets spéciaux en surface / fx above water
				fx_manager.draw(refcam, map->sealvl);
			}

            gfx->glDisable(GL_CLIP_PLANE1);
            CHECK_GL();

			gfx->ReInitAllTex(true);

			glColor4ub(0xFF,0xFF,0xFF,0xFF);
            CHECK_GL();
            gfx->glDisable(GL_BLEND);
            CHECK_GL();

            gfx->renderToTexture();
            gfx->glViewport(0, 0, SCREEN_W, SCREEN_H);
            CHECK_GL();

			gfx->SetDefState();
		}
		else
			reflection_drawn_last_time = false;
	}

	void Battle::renderShadowMap()
	{
		if (lp_CONFIG->shadow_quality > 0 && cam.rpos.y <= gfx->low_def_limit)
		{
			switch (lp_CONFIG->shadow_quality)
			{
			case 3:
			case 2:                     // Render the shadow map
				gfx->setShadowMapMode(true);
				gfx->SetDefState();
				gfx->renderToTextureDepth( gfx->get_shadow_map() );
				gfx->clearDepth();
				pSun.SetView( map->get_visible_volume() );

				// We'll need this matrix later (when rendering with shadows)
				gfx->readShadowMapProjectionMatrix();

                gfx->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                CHECK_GL();
                gfx->glDisable(GL_FOG);
                CHECK_GL();
                glShadeModel (GL_FLAT);
                CHECK_GL();

                gfx->glEnable(GL_POLYGON_OFFSET_FILL);
                CHECK_GL();
                gfx->glPolygonOffset(3.0f, 1.0f);
                CHECK_GL();

				// Render all visible features from light's point of view
				for(std::vector<int>::const_iterator i = features->list.begin() ; i != features->list.end() ; ++i)
					features->feature[*i].draw = true;
				features->draw(render_time, true);

                gfx->glEnable(GL_POLYGON_OFFSET_FILL);
                CHECK_GL();
                gfx->glPolygonOffset(3.0f, 1.0f);
                CHECK_GL();
                // Render all visible units from light's point of view
                units->draw(true, false, true, false);
                units->draw(false, false, true, false);

				// Render all visible weapons from light's point of view
				weapons.draw(true);
				weapons.draw(false);

                gfx->glDisable(GL_POLYGON_OFFSET_FILL);
                CHECK_GL();
                gfx->glPolygonOffset(0.0f, 0.0f);
                CHECK_GL();

                gfx->renderToTextureDepth();
                gfx->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE7);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                gfx->get_shadow_map()->bind();
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->setShadowMapMode(false);
				break;
			};
		}
	}

	void Battle::renderWater()
	{
		if (map->water)
		{
			// Effets spéciaux sous-marins / Draw fx which are under water
			fx_manager.draw(cam, map->sealvl, true);

            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            CHECK_GL();
			gfx->ReInitAllTex(true);

            if (lp_CONFIG->water_quality < 2)
			{
				gfx->set_alpha_blending();
				if (lp_CONFIG->water_quality == 1) // lp_CONFIG->water_quality=1
				{
					glColor4f(1.0f,1.0f,1.0f,0.5f);
                    CHECK_GL();

                    gfx->glActiveTexture(GL_TEXTURE0);
                    CHECK_GL();
                    gfx->glEnable(GL_TEXTURE_2D);
                    CHECK_GL();
                    glClientActiveTextureARB(GL_TEXTURE0_ARB);
                    CHECK_GL();

					map->draw(&cam,1,true,map->sealvl,t,dt*lp_CONFIG->timefactor);
				}
				else 	// lp_CONFIG->water_quality=0
				{
					glColor4f(1.0f,1.0f,1.0f,0.5f);
                    CHECK_GL();
                    gfx->glDisable(GL_LIGHTING);
                    CHECK_GL();

                    gfx->glActiveTexture(GL_TEXTURE0);
                    CHECK_GL();
                    gfx->glEnable(GL_TEXTURE_2D);
                    CHECK_GL();
                    map->low_tex->bind();
                    CHECK_GL();

					cam.setView(true);
					glTranslatef(0.0f, map->sealvl, map->sea_dec);
                    CHECK_GL();
                    water_obj->draw(t,false);
					glColor4f(1.0f,1.0f,1.0f,0.75f);
                    CHECK_GL();

                    gfx->glEnable(GL_LIGHTING);
                    CHECK_GL();
                    gfx->glActiveTexture(GL_TEXTURE0);
                    CHECK_GL();
                    gfx->ReInitTexSys();
                    gfx->glEnable(GL_TEXTURE_2D);
                    CHECK_GL();
                }
				gfx->unset_alpha_blending();
			}
			else if (lp_CONFIG->water_quality <= 4)
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                CHECK_GL();
                gfx->glDisable(GL_LIGHTING);
                CHECK_GL();

				// First pass of water rendering, store reflection vector
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);
                CHECK_GL();
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, first_pass->textureId(), 0);
                CHECK_GL();
                gfx->glViewport(0,0,512,512);
                CHECK_GL();

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                map->lava_map->bind();
                CHECK_GL();
                glClientActiveTexture(GL_TEXTURE0);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE1);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                water->bind();
                CHECK_GL();
                glClientActiveTexture(GL_TEXTURE1);
                CHECK_GL();

				if (lp_CONFIG->water_quality == 2)
				{
                    water_pass1_low->bind();
                    CHECK_GL();
                    water_pass1_low->setUniformValue("lava",0);
                    CHECK_GL();
                    water_pass1_low->setUniformValue("map",1);
                    CHECK_GL();
                    water_pass1_low->setUniformValue("t",t);
                    CHECK_GL();
                    water_pass1_low->setUniformValue("factor", (float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
                    CHECK_GL();
                }
				else
				{
                    water_pass1->bind();
                    CHECK_GL();
                    water_pass1->setUniformValue("lava",0);
                    CHECK_GL();
                    water_pass1->setUniformValue("map",1);
                    CHECK_GL();
                    water_pass1->setUniformValue("t",t);
                    CHECK_GL();
                    water_pass1->setUniformValue("factor", (float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
                    CHECK_GL();
                }

				cam.setView(true);
                glTranslatef(0.0f,map->sealvl,0.0f);
                CHECK_GL();
                water_obj->draw(t, true);

				if (lp_CONFIG->water_quality == 2)
                    water_pass1_low->release();
				else
                    water_pass1->release();

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, second_pass->textureId(), 0);					// Second pass of water rendering, store viewing vector
                CHECK_GL();

				//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE1);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();

                water_pass2->bind();
                CHECK_GL();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
                CHECK_GL();
                water_obj->draw(t, true);

                water_pass2->release();

				if (lp_CONFIG->water_quality > 2)
				{
                    gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_color->textureId(), 0);					// Third pass of water rendering, store water color
                    CHECK_GL();

                    gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                    CHECK_GL();

                    gfx->glActiveTexture(GL_TEXTURE0);
                    CHECK_GL();
                    gfx->glEnable(GL_TEXTURE_2D);
                    CHECK_GL();
                    map->low_tex->bind();
                    CHECK_GL();

					cam.setView();
					glTranslatef( 0.0f, map->sealvl, map->sea_dec);
                    CHECK_GL();
                    water_obj->draw(t, false);
				}

                gfx->glBindFramebuffer(GL_FRAMEBUFFER, 0);
                CHECK_GL();

                gfx->glViewport(0, 0, SCREEN_W, SCREEN_H);
                CHECK_GL();

				float logw = logf((float)SCREEN_W) / logf(2.0f);
				float logh = logf((float)SCREEN_H) / logf(2.0f);
				int wx = logw>(int)logw ? (int)logw+1 : (int)logw;
				int wy = logh>(int)logh ? (int)logh+1 : (int)logh;
				wx = 1 << wx;
				wy = 1 << wy;
                transtex->bind();								// Store what's on screen for transparency effect
                CHECK_GL();
                gfx->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, wx, wy, 0);
                CHECK_GL();

                gfx->glEnable(GL_STENCIL_TEST);											// Draw basic water in order to have correct texture mapping
                CHECK_GL();
                gfx->glClear(GL_STENCIL_BUFFER_BIT);
                CHECK_GL();
                gfx->glStencilFunc(GL_ALWAYS,128, 0xffffffff);
                CHECK_GL();
                gfx->glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();
                glClientActiveTextureARB(GL_TEXTURE0);
                CHECK_GL();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
                CHECK_GL();
                water_obj->draw(t, true);

                gfx->glDisable(GL_STENCIL_TEST);
                CHECK_GL();

                glMatrixMode(GL_TEXTURE);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();
                glMatrixMode(GL_MODELVIEW);
                CHECK_GL();

                gfx->glEnable(GL_LIGHTING);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                if (map->ota_data.lavaworld)
                {
                    sky.skyTex()->bind();
                    CHECK_GL();
                }
                else
                {
                    reflectex->bind();
                    CHECK_GL();
                }
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE1);
                CHECK_GL();
                transtex->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE2);
                CHECK_GL();
                first_pass->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE3);
                CHECK_GL();
                second_pass->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

				if (lp_CONFIG->water_quality == 2)
				{
                    water_shader->bind();
                    CHECK_GL();
                    water_shader->setUniformValue("sky",0);
                    CHECK_GL();
                    water_shader->setUniformValue("rtex",1);
                    CHECK_GL();
                    water_shader->setUniformValue("bump",2);
                    CHECK_GL();
                    water_shader->setUniformValue("view",3);
                    CHECK_GL();
                    water_shader->setUniformValue("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
                    CHECK_GL();
                }
				else
				{
                    gfx->glActiveTexture(GL_TEXTURE4);
                    CHECK_GL();
                    water_color->bind();
                    CHECK_GL();
                    gfx->glEnable(GL_TEXTURE_2D);
                    CHECK_GL();

                    water_shader_reflec->bind();
                    CHECK_GL();
                    water_shader_reflec->setUniformValue("sky",0);
                    CHECK_GL();
                    water_shader_reflec->setUniformValue("rtex",1);
                    CHECK_GL();
                    water_shader_reflec->setUniformValue("bump",2);
                    CHECK_GL();
                    water_shader_reflec->setUniformValue("view",3);
                    CHECK_GL();
                    water_shader_reflec->setUniformValue("water_color",4);
                    CHECK_GL();
                    water_shader_reflec->setUniformValue("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
                    CHECK_GL();
                }

                glColor4ub(0xFF,0xFF,0xFF,0xFF);
                CHECK_GL();
                gfx->glDisable(GL_DEPTH_TEST);
                CHECK_GL();

				glMatrixMode(GL_PROJECTION);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();
                glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
                CHECK_GL();
                glMatrixMode(GL_MODELVIEW);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();

                gfx->glEnable(GL_STENCIL_TEST);
                CHECK_GL();
                gfx->glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
                CHECK_GL();
                gfx->glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
                CHECK_GL();
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f,1.0f);	glVertex3f(0,0,0);
                glTexCoord2f(1.0f,1.0f);	glVertex3f((float)SCREEN_W,0,0);
                glTexCoord2f(1.0f,0.0f);	glVertex3f((float)SCREEN_W,(float)SCREEN_H,0);
                glTexCoord2f(0.0f,0.0f);	glVertex3f(0,(float)SCREEN_H,0);
                glEnd();
                CHECK_GL();
                gfx->glDisable(GL_STENCIL_TEST);
                CHECK_GL();
                gfx->glEnable(GL_DEPTH_TEST);
                CHECK_GL();

				if (lp_CONFIG->water_quality == 2)
                {
                    water_shader->release();
                    CHECK_GL();
                }
                else
                {
                    water_shader_reflec->release();
                    CHECK_GL();
                }
            }
			else                            // New Ultimate quality water renderer
			{
				// Run water simulation entirely on the GPU
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);
                CHECK_GL();

                gfx->glViewport(0,0,256,256);
                CHECK_GL();

                gfx->glDisable(GL_DEPTH_TEST);
                CHECK_GL();
                gfx->glDisable(GL_LIGHTING);
                CHECK_GL();

				glMatrixMode (GL_PROJECTION);
                CHECK_GL();
                glLoadIdentity ();
                CHECK_GL();
                glMatrixMode (GL_MODELVIEW);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

				const float time_step = 0.02f;
				const float time_to_simulate = Math::Min( dt * units->apparent_timefactor, time_step * 3.0f );

				// Simulate water
				for(float real_time = 0.0f ; real_time < time_to_simulate ; real_time += time_step)
				{
                    gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_sim0->textureId(), 0);
                    CHECK_GL();
                    water_sim1->bind();
                    CHECK_GL();

					bool refresh = false;
					if (msectimer() - last_water_refresh >= 100000)
					{
						last_water_refresh = msectimer();
						refresh = true;
					}
					float dt_step = Math::Min( time_to_simulate - real_time, time_step );
                    water_simulator_shader->bind();
                    CHECK_GL();
                    water_simulator_shader->setUniformValue("sim",0);
                    CHECK_GL();
                    water_simulator_shader->setUniformValue("fluid",50.0f * dt_step);
                    CHECK_GL();
                    water_simulator_shader->setUniformValue("t", refresh ? 1.0f : 0.0f);
                    CHECK_GL();

					glBegin( GL_QUADS );
                    glTexCoord2f( 0, 0 ); glVertex2f( -1, -1 );
                    glTexCoord2f( 1, 0 ); glVertex2f( 1, -1 );
                    glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
                    glTexCoord2f( 0, 1 ); glVertex2f( -1, 1 );
                    glEnd();
                    CHECK_GL();

                    water_simulator_shader->release();
                    CHECK_GL();

                    gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_sim1->textureId(), 0);
                    CHECK_GL();
                    water_sim0->bind();
                    CHECK_GL();

                    water_simulator_shader2->bind();
                    CHECK_GL();
                    water_simulator_shader2->setUniformValue("sim",0);
                    CHECK_GL();
                    water_simulator_shader2->setUniformValue("dt", dt_step);
                    CHECK_GL();

					glBegin( GL_QUADS );
                    glTexCoord2f( 0, 0 ); glVertex2f( -1, -1 );
                    glTexCoord2f( 1, 0 ); glVertex2f( 1, -1 );
                    glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
                    glTexCoord2f( 0, 1 ); glVertex2f( -1, 1 );
                    glEnd();
                    CHECK_GL();

                    water_simulator_shader2->release();
                    CHECK_GL();
                }

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_sim2->textureId(), 0);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                water_sim1->bind();
                CHECK_GL();

                water_simulator_shader3->bind();
                CHECK_GL();
                water_simulator_shader3->setUniformValue("sim",0);
                CHECK_GL();

				glBegin( GL_QUADS );
                glTexCoord2f( 0, 0 ); glVertex2f( -1, -1 );
                glTexCoord2f( 1, 0 ); glVertex2f( 1, -1 );
                glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
                glTexCoord2f( 0, 1 ); glVertex2f( -1, 1 );
                glEnd();
                CHECK_GL();

                water_simulator_shader3->release();
                CHECK_GL();

                glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
                CHECK_GL();
                gfx->glDisable(GL_LIGHTING);
                CHECK_GL();

                gfx->glEnable(GL_DEPTH_TEST);
                CHECK_GL();

				int ln2w = Math::Log2(SCREEN_W);
				int ln2h = Math::Log2(SCREEN_H);
				if ((1 << ln2w) < SCREEN_W)
					++ln2w;
				if ((1 << ln2h) < SCREEN_H)
					++ln2h;
				const int workwidth = g_useNonPowerOfTwoTextures ? SCREEN_W : 1 << ln2w;
				const int workheight = g_useNonPowerOfTwoTextures ? SCREEN_H : 1 << ln2h;

                gfx->glViewport(0,0,workwidth,workheight);
                CHECK_GL();

				// Render water distortion effects (ripples, waves, ...)
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);
                CHECK_GL();
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_distortions->textureId(), 0);
                CHECK_GL();
                gfx->glClearColor(0,0,0,0);
                CHECK_GL();
                gfx->glClear(GL_COLOR_BUFFER_BIT);		// Efface la texture tampon
                CHECK_GL();

				cam.setView(true);
                water_distortions_shader->bind();
                CHECK_GL();
                fx_manager.drawWaterDistortions();
                water_distortions_shader->release();
                CHECK_GL();

				glMatrixMode (GL_PROJECTION);
                CHECK_GL();
                glLoadIdentity ();
                CHECK_GL();
                glMatrixMode (GL_MODELVIEW);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();

				// First pass of water rendering, store reflection vector
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);
                CHECK_GL();
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, first_pass->textureId(), 0);
                CHECK_GL();

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                map->lava_map->bind();
                CHECK_GL();

                water_simulator_shader4->bind();
                CHECK_GL();
                water_simulator_shader4->setUniformValue("lava",0);
                CHECK_GL();
                water_simulator_shader4->setUniformValue("t",t);
                CHECK_GL();
                water_simulator_shader4->setUniformValue("factor",(float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
                CHECK_GL();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
                CHECK_GL();
                water_obj->draw(t, true);

                water_simulator_shader4->release();
                CHECK_GL();

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, second_pass->textureId(), 0);					// Second pass of water rendering, store viewing vector
                CHECK_GL();

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE1);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();

                water_pass2->bind();
                CHECK_GL();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
                CHECK_GL();
                water_obj->draw(t, true);

                water_pass2->release();

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_color->textureId(), 0);					// Third pass of water rendering, store water color
                CHECK_GL();

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                CHECK_GL();

                gfx->glViewport(0,0,512,512);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                map->low_tex->bind();
                CHECK_GL();

				cam.setView(true);
				glTranslatef( 0.0f, map->sealvl, map->sea_dec);
                CHECK_GL();
                water_obj->draw(t, false);

                gfx->renderToTexture();
                gfx->glViewport(0, 0, workwidth, workheight);
                CHECK_GL();

				float logw = logf((float)SCREEN_W) / logf(2.0f);
				float logh = logf((float)SCREEN_H) / logf(2.0f);
				int wx = logw>(int)logw ? (int)logw+1 : (int)logw;
				int wy = logh>(int)logh ? (int)logh+1 : (int)logh;
				wx = 1 << wx;
				wy = 1 << wy;
                transtex->bind();								// Store what's on screen for transparency effect
                CHECK_GL();
                gfx->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, wx, wy, 0);
                CHECK_GL();

                gfx->glEnable(GL_STENCIL_TEST);											// Draw basic water in order to have correct texture mapping
                CHECK_GL();
                gfx->glClear(GL_STENCIL_BUFFER_BIT);
                CHECK_GL();
                gfx->glStencilFunc(GL_ALWAYS,128, 0xffffffff);
                CHECK_GL();
                gfx->glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
                CHECK_GL();
                water_obj->draw(t, true);

                gfx->glDisable(GL_STENCIL_TEST);
                CHECK_GL();

				glMatrixMode(GL_TEXTURE);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();
                glMatrixMode(GL_MODELVIEW);
                CHECK_GL();

                gfx->glEnable(GL_LIGHTING);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0);
                CHECK_GL();
                if (map->ota_data.lavaworld)
                {
                    sky.skyTex()->bind();
                    CHECK_GL();
                }
                else
                {
                    reflectex->bind();
                    CHECK_GL();
                }
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE1);
                CHECK_GL();
                transtex->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE2);
                CHECK_GL();
                first_pass->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE3);
                CHECK_GL();
                second_pass->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE4);
                CHECK_GL();
                water_color->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE5);
                CHECK_GL();
                height_tex->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE6);
                CHECK_GL();
                water_sim2->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE7);
                CHECK_GL();
                water_distortions->bind();
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();

                water_simulator_reflec->bind();
                CHECK_GL();
                water_simulator_reflec->setUniformValue("sky",0);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("rtex",1);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("bump",2);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("view",3);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("water_color",4);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("height_map",5);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("normal_map",6);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("distort_map",7);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("cam_h_factor", 1.0f / cam.rpos.y);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("factor",(float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
                CHECK_GL();
                water_simulator_reflec->setUniformValue("t", t);
                CHECK_GL();

				glColor4ub(0xFF,0xFF,0xFF,0xFF);
                CHECK_GL();
                gfx->glDisable(GL_DEPTH_TEST);
                CHECK_GL();

				glMatrixMode(GL_PROJECTION);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();
                glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
                CHECK_GL();
                glMatrixMode(GL_MODELVIEW);
                CHECK_GL();
                glLoadIdentity();
                CHECK_GL();

                gfx->glEnable(GL_STENCIL_TEST);
                CHECK_GL();
                gfx->glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
                CHECK_GL();
                gfx->glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
                CHECK_GL();
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f,1.0f);	glVertex3f(0,0,0);
                glTexCoord2f(1.0f,1.0f);	glVertex3f((float)SCREEN_W,0,0);
                glTexCoord2f(1.0f,0.0f);	glVertex3f((float)SCREEN_W,(float)SCREEN_H,0);
                glTexCoord2f(0.0f,0.0f);	glVertex3f(0,(float)SCREEN_H,0);
                glEnd();
                CHECK_GL();
                gfx->glDisable(GL_STENCIL_TEST);
                CHECK_GL();
                gfx->glEnable(GL_DEPTH_TEST);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE7);
                CHECK_GL();
                if (lp_CONFIG->shadow_quality >= 2 && cam.rpos.y <= gfx->low_def_limit)
                {
                    gfx->get_shadow_map()->bind();
                    CHECK_GL();
                }
                else
                {
                    gfx->glDisable(GL_TEXTURE_2D);
                    CHECK_GL();
                }

                water_simulator_reflec->release();
                CHECK_GL();
            }
			gfx->ReInitAllTex(true);
		}
		cam.setView();
	}

	void Battle::renderWorld()
	{
		gfx->SetDefState();
        gfx->glClearColor(FogColor[0],FogColor[1],FogColor[2],FogColor[3]);
        CHECK_GL();
        gfx->clearDepth();		// Clear screen

		cam.setView();

		pSun.Set(cam);
		pSun.Enable();

		cam.setView();

		cam.zfar *= 100.0f;
		cam.setView();
        gfx->glDisable(GL_FOG);
        CHECK_GL();
        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        CHECK_GL();
        gfx->glEnable(GL_TEXTURE_2D);
        CHECK_GL();
        gfx->glDisable(GL_BLEND);
        CHECK_GL();
        if (lp_CONFIG->render_sky)
		{
            gfx->glDisable(GL_LIGHTING);
            CHECK_GL();
            gfx->glDepthMask(GL_FALSE);
            CHECK_GL();
            glTranslatef(cam.rpos.x, cam.rpos.y + cam.shakeVector.y, cam.rpos.z);
            CHECK_GL();
            glRotatef(sky_angle, 0.0f, 1.0f, 0.0f);
            CHECK_GL();
            if (lp_CONFIG->ortho_camera)
			{
				const float scale = cam.zoomFactor / 800.0f * std::sqrt(float(SCREEN_H * SCREEN_H + SCREEN_W * SCREEN_W));
				glScalef( scale, scale, scale );
                CHECK_GL();
            }
			sky.draw();
		}
		else
			gfx->clearScreen();

        gfx->glDepthMask(GL_TRUE);
        CHECK_GL();
        gfx->glEnable(GL_CULL_FACE);
        CHECK_GL();
        gfx->glEnable(GL_LIGHTING);
        CHECK_GL();
        gfx->glEnable(GL_FOG);
        CHECK_GL();
        updateZFAR();

        map->draw(&cam, byte(1 << players.local_human_id), false, 0.0f, t, dt * units->apparent_timefactor);

		cam.setView(lp_CONFIG->shadow_quality < 2);

        features->draw(render_time);		// Dessine les éléments "2D"

		/*----------------------------------------------------------------------------------------------*/

        // Dessine les unités sous l'eau / Draw units which are under water
        cam.setView(lp_CONFIG->shadow_quality < 2);
        if (cam.rpos.y <= gfx->low_def_limit)
        {
            if (lp_CONFIG->shadow_quality >= 2)
            {
                glFogi (GL_FOG_COORD_SRC, GL_FOG_COORD);
                CHECK_GL();
            }
            units->draw(true, false, true, lp_CONFIG->height_line);
            glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
            CHECK_GL();
        }

        // Dessine les objets produits par les armes sous l'eau / Draw weapons which are under water
        weapons.draw(true);

        if (lp_CONFIG->particle)
            particle_engine.drawUW();

        renderWater();

        // Render map object icons (if in tactical mode)
        if (cam.rpos.y > gfx->low_def_limit)
        {
            cam.setView(true);
            features->draw_icons();
        }

        cam.setView(lp_CONFIG->shadow_quality < 2);
        if (lp_CONFIG->shadow_quality >= 2)
        {
            glFogi (GL_FOG_COORD_SRC, GL_FOG_COORD);
            CHECK_GL();
        }
        // Dessine les unités non encore dessinées / Draw units which have not been drawn
        units->draw(false, false, true, lp_CONFIG->height_line);
        glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
        CHECK_GL();

        // Dessine les objets produits par les armes n'ayant pas été dessinés / Draw weapons which have not been drawn
        weapons.draw(false);
	}

	void Battle::renderInfo()
	{
		if (build >= 0 && !IsOnGUI)	// Display the building we want to build (with nice selection quads)
		{
            gfx->glDisable(GL_FOG);
            CHECK_GL();
            Vector3D target(cursorOnMap(cam, *map));
			pMouseRectSelection.x2 = ((int)(target.x) + map->map_w_d) >> 3;
			pMouseRectSelection.y2 = ((int)(target.z) + map->map_h_d) >> 3;

			if (mouse_b != 1 && omb3 != 1)
			{
				pMouseRectSelection.x1 = pMouseRectSelection.x2;
				pMouseRectSelection.y1 = pMouseRectSelection.y2;
			}

			int d = Math::Max(abs(pMouseRectSelection.x2 - pMouseRectSelection.x1), abs( pMouseRectSelection.y2 - pMouseRectSelection.y1));

			int ox = pMouseRectSelection.x1 + 0xFFFF;
			int oy = pMouseRectSelection.y1 + 0xFFFF;

			for (int c = 0; c <= d; ++c)
			{
				target.x = float(pMouseRectSelection.x1 + (pMouseRectSelection.x2 - pMouseRectSelection.x1) * c / Math::Max(d, 1));
				target.z = float(pMouseRectSelection.y1 + (pMouseRectSelection.y2 - pMouseRectSelection.y1) * c / Math::Max(d, 1));

				if (abs( ox - (int)target.x) < unit_manager.unit_type[build]->FootprintX
					&& abs( oy - (int)target.z) < unit_manager.unit_type[build]->FootprintZ)
					continue;
				ox = (int)target.x;
				oy = (int)target.z;

				target.y = map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[build]->FootprintX, unit_manager.unit_type[build]->FootprintZ);
				if (unit_manager.unit_type[build]->floatting())
					target.y = Math::Max(target.y, map->sealvl + ((float)unit_manager.unit_type[build]->AltFromSeaLevel - (float)unit_manager.unit_type[build]->WaterLine) * H_DIV);
				target.x = target.x * 8.0f - (float)map->map_w_d;
				target.z = target.z * 8.0f - (float)map->map_h_d;

				can_be_there = can_be_built(target, build, players.local_human_id);

				cam.setView();

				glTranslatef(target.x,target.y,target.z);
                CHECK_GL();
                glScalef(unit_manager.unit_type[build]->Scale,unit_manager.unit_type[build]->Scale,unit_manager.unit_type[build]->Scale);
                CHECK_GL();
                const float DX = float(unit_manager.unit_type[build]->FootprintX << 2);
				const float DZ = float(unit_manager.unit_type[build]->FootprintZ << 2);
				if (unit_manager.unit_type[build]->model)
				{
                    gfx->glEnable(GL_CULL_FACE);
                    CHECK_GL();
                    gfx->ReInitAllTex( true);
                    CHECK_GL();
                    if (can_be_there)
                    {
						glColor4ub(0xFF,0xFF,0xFF,0xFF);
                        CHECK_GL();
                    }
                    else
                    {
						glColor4ub(0xFF,0,0,0xFF);
                        CHECK_GL();
                    }
                    gfx->glDepthFunc( GL_GREATER );
                    CHECK_GL();
                    unit_manager.unit_type[build]->model->draw(0.0f,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);
                    gfx->glDepthFunc( GL_LESS );
                    CHECK_GL();
                    unit_manager.unit_type[build]->model->draw(0.0f,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);

					const bool old_mode = gfx->getShadowMapMode();
					gfx->setShadowMapMode(true);
					double eqn[4]= { 0.0f, -1.0f, 0.0f, map->sealvl - target.y };
					glClipPlane(GL_CLIP_PLANE2, eqn);
                    CHECK_GL();

                    gfx->glEnable(GL_CLIP_PLANE2);
                    CHECK_GL();

                    gfx->glEnable( GL_BLEND );
                    CHECK_GL();
                    gfx->glBlendFunc( GL_ONE, GL_ONE );
                    CHECK_GL();
                    gfx->glDepthFunc( GL_EQUAL );
                    CHECK_GL();
                    glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
                    CHECK_GL();
                    unit_manager.unit_type[build]->model->draw(0.0f,NULL,false,true,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);
					glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
                    CHECK_GL();
                    gfx->glDepthFunc( GL_LESS );
                    CHECK_GL();
                    gfx->glDisable( GL_BLEND );
                    CHECK_GL();

                    gfx->glDisable(GL_CLIP_PLANE2);
                    CHECK_GL();
                    gfx->setShadowMapMode(old_mode);
				}
				cam.setView();
				glTranslatef(target.x,Math::Max( target.y, map->sealvl ),target.z);
                CHECK_GL();
                byte red = 0xFF, green = 0x00;
				if (can_be_there)
				{
					green = 0xFF;
					red   = 0x00;
				}
                gfx->glDisable(GL_CULL_FACE);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();
                gfx->glDisable(GL_LIGHTING);
                CHECK_GL();
                gfx->glEnable(GL_BLEND);
                CHECK_GL();
                gfx->glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                CHECK_GL();
                glBegin(GL_QUADS);
                CHECK_GL();
                glColor4ub(red,green,0x00,0xFF);
                CHECK_GL();
                glVertex3f(-DX,0.0f,-DZ);			// First quad
                CHECK_GL();
                glVertex3f(DX,0.0f,-DZ);
                CHECK_GL();
                glColor4ub(red,green,0x00,0x00);
                CHECK_GL();
                glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
                CHECK_GL();
                glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);
                CHECK_GL();

				glColor4ub(red,green,0x00,0xFF);
                CHECK_GL();
                glVertex3f(-DX,0.0f,-DZ);			// Second quad
                CHECK_GL();
                glVertex3f(-DX,0.0f,DZ);
                CHECK_GL();
                glColor4ub(red,green,0x00,0x00);
                CHECK_GL();
                glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
                CHECK_GL();
                glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);
                CHECK_GL();

				glColor4ub(red,green,0x00,0xFF);
                CHECK_GL();
                glVertex3f(DX,0.0f,-DZ);			// Third quad
                CHECK_GL();
                glVertex3f(DX,0.0f,DZ);
                CHECK_GL();
                glColor4ub(red,green,0x00,0x00);
                CHECK_GL();
                glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
                CHECK_GL();
                glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
                CHECK_GL();

				glColor4ub(red,green,0x00,0xFF);
                CHECK_GL();
                glVertex3f(-DX,0.0f,DZ);			// Fourth quad
                CHECK_GL();
                glVertex3f(DX,0.0f,DZ);
                CHECK_GL();
                glColor4ub(red,green,0x00,0x00);
                CHECK_GL();
                glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
                CHECK_GL();
                glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
                CHECK_GL();
                glEnd();
                CHECK_GL();
                gfx->glDisable(GL_BLEND);
                CHECK_GL();
                gfx->glEnable(GL_LIGHTING);
                CHECK_GL();
                gfx->glEnable(GL_CULL_FACE);
                CHECK_GL();
            }
            gfx->glEnable(GL_FOG);
            CHECK_GL();
        }


		if ((selected || units->last_on >= 0) && TA3D_SHIFT_PRESSED)
		{
            gfx->glDisable(GL_FOG);
            CHECK_GL();
            cam.setView();
			bool builders = false;
			const float t = (float)msectimer() * 0.001f;
			const float mt = std::fmod(0.5f * t, 1.0f);
			for (unsigned int e = 0; e < units->index_list_size ; ++e)
			{
				const int i = units->idx_list[e];
				if ((units->unit[i].flags & 1)
					&& units->unit[i].owner_id == players.local_human_id
					&& (units->unit[i].sel || i == units->last_on))
				{
					const int type_id = units->unit[i].type_id;
					if (type_id >= 0)
					{
						const UnitType* const pType = unit_manager.unit_type[type_id];
						builders |= pType->Builder;

						const float x = units->unit[i].render.Pos.x;
						const float z = units->unit[i].render.Pos.z;
						if (pType->kamikaze)
						{
							the_map->drawCircleOnMap(x, z, pType->kamikazedistance, makeacol(0xFF,0x0,0x0,0xFF), 1.0f);
							const int idx = weapon_manager.get_weapon_index(pType->SelfDestructAs);
							const WeaponDef* const pWeapon = idx >= 0 && idx < weapon_manager.nb_weapons ? &(weapon_manager.weapon[idx]) : NULL;
							if (pWeapon)
								the_map->drawCircleOnMap(x, z, (float)pWeapon->areaofeffect * 0.25f * mt, makeacol(0xFF,0x0,0x0,0xFF), 1.0f);
						}
						if (pType->mincloakdistance && units->unit[i].cloaked)
							the_map->drawCircleOnMap(x, z, (float)pType->mincloakdistance, makeacol(0xFF,0xFF,0xFF,0xFF), 1.0f);
					}
					if (units->unit[i].sel)
						units->unit[i].show_orders();					// Dessine les ordres reçus par l'unité / Draw given orders
				}
			}

			if (builders)
			{
				for (unsigned int e = 0; e < units->index_list_size; ++e)
				{
					const int i = units->idx_list[e];
					const int type_id = units->unit[i].type_id;
					if (type_id < 0)
						continue;
					if ((units->unit[i].flags & 1) && units->unit[i].owner_id == players.local_human_id && !units->unit[i].sel
						&& unit_manager.unit_type[type_id]->Builder && unit_manager.unit_type[type_id]->BMcode)
					{
						units->unit[i].show_orders(true);					// Dessine les ordres reçus par l'unité / Draw given orders
					}
				}
			}
            gfx->glEnable(GL_FOG);
            CHECK_GL();
        }
		if ((selected || units->last_on >= 0) && TA3D_CTRL_PRESSED)
		{
            gfx->glDisable(GL_FOG);
            CHECK_GL();
            cam.setView();
			const float t = (float)msectimer() * 0.001f;
			const float mt = std::fmod(0.5f * t, 1.0f);
			const float mt2 = std::fmod(0.5f * t + 0.5f, 1.0f);
			for (unsigned int e = 0; e < units->index_list_size ; ++e)
			{
				const int i = units->idx_list[e];
				if ((units->unit[i].flags & 1)
					&& units->unit[i].owner_id == players.local_human_id
					&& (units->unit[i].sel || i == units->last_on))
				{
					const int type_id = units->unit[i].type_id;
					if (type_id >= 0)
					{
						const UnitType* const pType = unit_manager.unit_type[type_id];

						const float x = units->unit[i].render.Pos.x;
						const float z = units->unit[i].render.Pos.z;
						if (!TA3D_SHIFT_PRESSED)
						{
							if (pType->kamikaze)
							{
								the_map->drawCircleOnMap(x, z, pType->kamikazedistance, makeacol(0xFF,0x0,0x0,0xFF), 1.0f);
								const int idx = weapon_manager.get_weapon_index(pType->SelfDestructAs);
								const WeaponDef* const pWeapon = idx >= 0 && idx < weapon_manager.nb_weapons ? &(weapon_manager.weapon[idx]) : NULL;
								if (pWeapon)
									the_map->drawCircleOnMap(x, z, (float)pWeapon->areaofeffect * 0.25f * mt, makeacol(0xFF,0x0,0x0,0xFF), 1.0f);
							}
							if (pType->mincloakdistance && units->unit[i].cloaked)
								the_map->drawCircleOnMap(x, z, (float)pType->mincloakdistance, makeacol(0xFF,0xFF,0xFF,0xFF), 1.0f);
						}
						if (!pType->onoffable || units->unit[i].port[ACTIVATION])
						{
							if (pType->RadarDistance)
								the_map->drawCircleOnMap(x, z, (float)pType->RadarDistance * mt, makeacol(0x00,0x00,0xFF,0xFF), 1.0f);
							if (pType->RadarDistanceJam)
								the_map->drawCircleOnMap(x, z, (float)pType->RadarDistanceJam * mt, makeacol(0x00,0x00,0x00,0xFF), 1.0f);
							if (pType->SonarDistance)
								the_map->drawCircleOnMap(x, z, (float)pType->SonarDistance * mt2, makeacol(0xFF,0xFF,0xFF,0xFF), 1.0f);
							if (pType->SonarDistanceJam)
								the_map->drawCircleOnMap(x, z, (float)pType->SonarDistanceJam * mt2, makeacol(0x7F,0x7F,0x7F,0xFF), 1.0f);
						}
					}
				}
			}
            gfx->glEnable(GL_FOG);
            CHECK_GL();
        }
		if (showHealthBars)
		{
			cam.setView();
            units->drawHealthBars();
		}
	}

	void Battle::renderPostEffects()
	{
		particle_engine.draw(&cam);	// Dessine les particules

		if (!map->water)
			fx_manager.draw(cam, map->sealvl, true);		// Effets spéciaux en surface
		fx_manager.draw(cam, map->sealvl);		// Effets spéciaux en surface
	}

	void Battle::renderScene()
	{
		if (lp_CONFIG->ortho_camera)
			cam.znear = -512.0f;
		else
			cam.znear = 1.0f;
        renderReflection();

        renderShadowMap();

        renderWorld();

        renderInfo();

		renderPostEffects();
	}

	void Battle::makePoster(int w, int h)
	{
		bool previous_pause_state = lp_CONFIG->pause;
		bool prevCameraType = lp_CONFIG->ortho_camera;
		lp_CONFIG->pause = true;
		lp_CONFIG->ortho_camera = true;

		while (!lp_CONFIG->paused)
			QThread::msleep( 100 );			// Wait for the engine to enter in pause mode so we can assemble several shots
									// of the same game tick

		Camera camBak = cam;

		cam.znear = -255.0f;
		QImage poster = gfx->create_surface_ex(24,w,h);
		QImage buf = gfx->create_surface_ex(24,SCREEN_W,SCREEN_H);

		for (int z = 0; z < h; z += SCREEN_H / 2)
		{
			for (int x = 0; x < w; x += SCREEN_W / 2)
			{
				reflection_drawn_last_time = false;		// We need to refresh everything

				// Set camera to current part of the scene
				cam.rpos = camBak.rpos
						   + camBak.zoomFactor
							* (float(x - w / 2 - SCREEN_W / 4) * camBak.side
							   + float(z - h / 2 - SCREEN_H / 4) * camBak.up);
				if (!Math::Zero(camBak.dir.y))
					cam.rpos = cam.rpos + ((camBak.rpos - cam.rpos).y / camBak.dir.y) * camBak.dir;

				// Render this part of the scene
				gfx->clearAll();
				initRenderer();
				renderScene();

				// Read the pixels
                glReadPixels(0, 0, SCREEN_W, SCREEN_H, GL_BGR, GL_UNSIGNED_BYTE, buf.bits());

				// Fill current part of the poster
                blit(buf, poster, SCREEN_W / 4, SCREEN_H / 4, x, z, Math::Min(SCREEN_W / 2, poster.width() - x), Math::Min(SCREEN_H / 2, poster.height() - z));
			}
		}

		vflip_bitmap(poster);
        save_bitmap(TA3D::Paths::Screenshots + "poster.png", poster);

		cam = camBak;

		lp_CONFIG->pause = previous_pause_state;
		lp_CONFIG->ortho_camera = prevCameraType;
	}



} // namespace TA3D



