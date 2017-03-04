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


namespace TA3D
{

	void Battle::initRenderer()
	{
		cam_h = cam.rpos.y - map->get_unit_h(cam.rpos.x, cam.rpos.z);

		updateZFAR();

		gfx->SetDefState();
		updateFOG();

		render_time = ((float)units.current_tick) / TICKS_PER_SEC;

		// Copy unit data from simulation
		units.renderTick();

        gfx->glActiveTexture(GL_TEXTURE7_ARB);
        gfx->glDisable(GL_TEXTURE_2D);
        gfx->glBindTexture(GL_TEXTURE_2D, 0);
        gfx->glActiveTexture(GL_TEXTURE0_ARB);
		gfx->setShadowMapMode(false);
		if (g_useProgram)
            gfx->glUseProgram(0);
	}


	void Battle::renderReflection()
	{
		// Dessine les reflets sur l'eau / Render water reflection
		if (g_useProgram && g_useFBO && lp_CONFIG->water_quality >= 2 && map->water && !map->ota_data.lavaworld && !reflection_drawn_last_time)
		{
			reflection_drawn_last_time = true;

			gfx->clearAll();		// Clear screen

			glViewport(0, 0, 512, 512);

			gfx->ReInitAllTex();
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glDisable(GL_BLEND);

			double eqn[4]= { 0.0f, 1.0f, 0.0f, -map->sealvl };

			Camera refcam = cam;
			refcam.zfar *= 2.0f;
			refcam.mirror = true;
			refcam.mirrorPos = -2.0f * map->sealvl;

			refcam.setView();
			glClipPlane(GL_CLIP_PLANE1, eqn);
			glEnable(GL_CLIP_PLANE1);

			pSun.Set(refcam);
			pSun.Enable();

			refcam.zfar *= 100.0f;
			refcam.setView();
			glColor4ub(0xFF,0xFF,0xFF,0xFF);
			glEnable(GL_TEXTURE_2D);
			if (lp_CONFIG->render_sky)
			{
				glDisable(GL_FOG);
				glDisable(GL_LIGHTING);
				glDepthMask(GL_FALSE);

				glCullFace(GL_FRONT);
				glTranslatef(cam.rpos.x,-map->sealvl,cam.rpos.z);
				glRotatef( sky_angle, 0.0f, 1.0f, 0.0f);
				float scale_factor = 15.0f * ( cam.rpos.y + cam.shakeVector.y + sky.getW()) / sky.getW();
				glScalef( scale_factor, scale_factor, scale_factor);
				sky.draw();
			}
			refcam.zfar = (500.0f + (cam_h - 150.0f) * 2.0f) * 2.0f;
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
			glEnable(GL_FOG);
			glCullFace(GL_FRONT);
			refcam.setView();

			if (cam.rpos.y <= gfx->low_def_limit && lp_CONFIG->water_quality >= 4)
			{
				if (lp_CONFIG->wireframe)
					glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

				map->draw(&refcam, byte(1 << players.local_human_id),  false, 0.0f, t,
						  dt * units.apparent_timefactor,
						  false, false, false);

				if (lp_CONFIG->wireframe)
					glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

				// Dessine les éléments "2D" / "sprites"
				features.draw(render_time);
				refcam.setView();
				// Dessine les unités / draw units
				units.draw(false, true, false, lp_CONFIG->height_line);

				glDisable(GL_CULL_FACE);
				// Dessine les objets produits par les armes / draw weapons
                weapons.draw(map);
				// Dessine les particules
				refcam.setView(true);
				glClipPlane(GL_CLIP_PLANE1, eqn);

				particle_engine.draw(&refcam);

				refcam.setView();
				glClipPlane(GL_CLIP_PLANE1, eqn);
				// Effets spéciaux en surface / fx above water
				fx_manager.draw(refcam, map->sealvl);
			}

			glDisable(GL_CLIP_PLANE1);

			gfx->ReInitAllTex(true);

			glColor4ub(0xFF,0xFF,0xFF,0xFF);
			glDisable(GL_BLEND);

            reflectex->bind();							// Store what's on screen for reflection effect
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 512, 512, 0);

			glViewport(0, 0, SCREEN_W, SCREEN_H);

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

				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glDisable(GL_FOG);
				glShadeModel (GL_FLAT);

				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(3.0f, 1.0f);

				// Render all visible features from light's point of view
				for(std::vector<int>::const_iterator i = features.list.begin() ; i != features.list.end() ; ++i)
					features.feature[*i].draw = true;
				features.draw(render_time, true);

				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(3.0f, 1.0f);
				// Render all visible units from light's point of view
				units.draw(true, false, true, false);
				units.draw(false, false, true, false);

				// Render all visible weapons from light's point of view
				weapons.draw(true);
				weapons.draw(false);

				glDisable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(0.0f, 0.0f);

                gfx->renderToTextureDepth();
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

				glActiveTextureARB(GL_TEXTURE7_ARB);
				glEnable(GL_TEXTURE_2D);
                gfx->get_shadow_map()->bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
				glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);

				glActiveTextureARB(GL_TEXTURE0_ARB);
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
			gfx->ReInitAllTex(true);

			if (!g_useProgram || !g_useFBO || lp_CONFIG->water_quality < 2)
			{
				gfx->set_alpha_blending();
				if (lp_CONFIG->water_quality == 1) // lp_CONFIG->water_quality=1
				{
					glColor4f(1.0f,1.0f,1.0f,0.5f);

					glActiveTextureARB(GL_TEXTURE0_ARB);
					glEnable(GL_TEXTURE_2D);
					glClientActiveTextureARB(GL_TEXTURE0_ARB);

					map->draw(&cam,1,true,map->sealvl,t,dt*lp_CONFIG->timefactor);
				}
				else 	// lp_CONFIG->water_quality=0
				{
					glColor4f(1.0f,1.0f,1.0f,0.5f);
					glDisable(GL_LIGHTING);

					glActiveTextureARB(GL_TEXTURE0_ARB);
					glEnable(GL_TEXTURE_2D);
                    map->low_tex->bind();

					cam.setView(true);
					glTranslatef(0.0f, map->sealvl, map->sea_dec);
					water_obj->draw(t,false);
					glColor4f(1.0f,1.0f,1.0f,0.75f);

					glEnable(GL_LIGHTING);
					glActiveTextureARB(GL_TEXTURE0_ARB);
					gfx->ReInitTexSys();
					glEnable(GL_TEXTURE_2D);
				}
				gfx->unset_alpha_blending();
			}
			else if (lp_CONFIG->water_quality <= 4)
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glDisable(GL_LIGHTING);

				// First pass of water rendering, store reflection vector
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, first_pass, 0);

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                gfx->glViewport(0,0,512,512);

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glEnable(GL_TEXTURE_2D);
                map->lava_map->bind();
                glClientActiveTexture(GL_TEXTURE0);

                gfx->glActiveTexture(GL_TEXTURE1);
                gfx->glEnable(GL_TEXTURE_2D);
                water->bind();
                glClientActiveTexture(GL_TEXTURE1);

				if (lp_CONFIG->water_quality == 2)
				{
                    water_pass1_low->bind();
                    water_pass1_low->setUniformValue("lava",0);
                    water_pass1_low->setUniformValue("map",1);
                    water_pass1_low->setUniformValue("t",t);
                    water_pass1_low->setUniformValue("factor", (float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
				}
				else
				{
                    water_pass1->bind();
                    water_pass1->setUniformValue("lava",0);
                    water_pass1->setUniformValue("map",1);
                    water_pass1->setUniformValue("t",t);
                    water_pass1->setUniformValue("factor", (float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
				}

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

				if (lp_CONFIG->water_quality == 2)
                    water_pass1_low->release();
				else
                    water_pass1->release();

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, second_pass->textureId(), 0);					// Second pass of water rendering, store viewing vector

				//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glDisable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE1);
                gfx->glDisable(GL_TEXTURE_2D);

                water_pass2->bind();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

                water_pass2->release();

				if (lp_CONFIG->water_quality > 2)
				{
                    gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_color->textureId(), 0);					// Third pass of water rendering, store water color

                    gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                    gfx->glActiveTexture(GL_TEXTURE0);
                    gfx->glEnable(GL_TEXTURE_2D);
                    map->low_tex->bind();

					cam.setView();
					glTranslatef( 0.0f, map->sealvl, map->sea_dec);
					water_obj->draw(t, false);
				}

                gfx->glBindFramebuffer(GL_FRAMEBUFFER, 0);

                gfx->glViewport(0, 0, SCREEN_W, SCREEN_H);

				float logw = logf((float)SCREEN_W) / logf(2.0f);
				float logh = logf((float)SCREEN_H) / logf(2.0f);
				int wx = logw>(int)logw ? (int)logw+1 : (int)logw;
				int wy = logh>(int)logh ? (int)logh+1 : (int)logh;
				wx = 1 << wx;
				wy = 1 << wy;
                transtex->bind();								// Store what's on screen for transparency effect
                gfx->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, wx, wy, 0);

                gfx->glEnable(GL_STENCIL_TEST);											// Draw basic water in order to have correct texture mapping
                gfx->glClear(GL_STENCIL_BUFFER_BIT);
                gfx->glStencilFunc(GL_ALWAYS,128, 0xffffffff);
                gfx->glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glDisable(GL_TEXTURE_2D);
                glClientActiveTextureARB(GL_TEXTURE0);

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

                gfx->glDisable(GL_STENCIL_TEST);

                glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);

                gfx->glEnable(GL_LIGHTING);

                gfx->glActiveTexture(GL_TEXTURE0);
				if (map->ota_data.lavaworld)
                    sky.skyTex()->bind();
				else
                    reflectex->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE1);
                transtex->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE2);
                first_pass->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE3);
                second_pass->bind();
                gfx->glEnable(GL_TEXTURE_2D);

				if (lp_CONFIG->water_quality == 2)
				{
                    water_shader->bind();
                    water_shader->setUniformValue("sky",0);
                    water_shader->setUniformValue("rtex",1);
                    water_shader->setUniformValue("bump",2);
                    water_shader->setUniformValue("view",3);
                    water_shader->setUniformValue("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
				}
				else
				{
                    gfx->glActiveTexture(GL_TEXTURE4);
                    water_color->bind();
                    gfx->glEnable(GL_TEXTURE_2D);

                    water_shader_reflec->bind();
                    water_shader_reflec->setUniformValue("sky",0);
                    water_shader_reflec->setUniformValue("rtex",1);
                    water_shader_reflec->setUniformValue("bump",2);
                    water_shader_reflec->setUniformValue("view",3);
                    water_shader_reflec->setUniformValue("water_color",4);
                    water_shader_reflec->setUniformValue("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
				}

                glColor4ub(0xFF,0xFF,0xFF,0xFF);
                gfx->glDisable(GL_DEPTH_TEST);

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

                gfx->glEnable(GL_STENCIL_TEST);
                gfx->glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
                gfx->glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
                glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f);	glVertex3f(0,0,0);
				glTexCoord2f(1.0f,1.0f);	glVertex3f((float)SCREEN_W,0,0);
				glTexCoord2f(1.0f,0.0f);	glVertex3f((float)SCREEN_W,(float)SCREEN_H,0);
				glTexCoord2f(0.0f,0.0f);	glVertex3f(0,(float)SCREEN_H,0);
				glEnd();
                gfx->glDisable(GL_STENCIL_TEST);
                gfx->glEnable(GL_DEPTH_TEST);

				if (lp_CONFIG->water_quality == 2)
                    water_shader->release();
				else
                    water_shader_reflec->release();
			}
			else                            // New Ultimate quality water renderer
			{
				// Run water simulation entirely on the GPU
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);

                gfx->glViewport(0,0,256,256);

                gfx->glDisable(GL_DEPTH_TEST);
                gfx->glDisable(GL_LIGHTING);

				glMatrixMode (GL_PROJECTION);
				glLoadIdentity ();
				glMatrixMode (GL_MODELVIEW);
				glLoadIdentity();

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glEnable(GL_TEXTURE_2D);

				const float time_step = 0.02f;
				const float time_to_simulate = Math::Min( dt * units.apparent_timefactor, time_step * 3.0f );

				// Simulate water
				for(float real_time = 0.0f ; real_time < time_to_simulate ; real_time += time_step)
				{
                    gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_sim0->textureId(), 0);
                    water_sim1->bind();

					bool refresh = false;
					if (msectimer() - last_water_refresh >= 100000)
					{
						last_water_refresh = msectimer();
						refresh = true;
					}
					float dt_step = Math::Min( time_to_simulate - real_time, time_step );
                    water_simulator_shader->bind();
                    water_simulator_shader->setUniformValue("sim",0);
                    water_simulator_shader->setUniformValue("fluid",50.0f * dt_step);
                    water_simulator_shader->setUniformValue("t", refresh ? 1.0f : 0.0f);

					glBegin( GL_QUADS );
					glTexCoord2i( 0, 0 ); glVertex2i( -1, -1 );
					glTexCoord2i( 1, 0 ); glVertex2i( 1, -1 );
					glTexCoord2i( 1, 1 ); glVertex2i( 1, 1 );
					glTexCoord2i( 0, 1 ); glVertex2i( -1, 1 );
					glEnd();

                    water_simulator_shader->release();

                    gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_sim1->textureId(), 0);
                    water_sim0->bind();

                    water_simulator_shader2->bind();
                    water_simulator_shader2->setUniformValue("sim",0);
                    water_simulator_shader2->setUniformValue("dt", dt_step);

					glBegin( GL_QUADS );
					glTexCoord2i( 0, 0 ); glVertex2i( -1, -1 );
					glTexCoord2i( 1, 0 ); glVertex2i( 1, -1 );
					glTexCoord2i( 1, 1 ); glVertex2i( 1, 1 );
					glTexCoord2i( 0, 1 ); glVertex2i( -1, 1 );
					glEnd();

                    water_simulator_shader2->release();
				}

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_sim2->textureId(), 0);

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glEnable(GL_TEXTURE_2D);
                water_sim1->bind();

                water_simulator_shader3->bind();
                water_simulator_shader3->setUniformValue("sim",0);

				glBegin( GL_QUADS );
				glTexCoord2i( 0, 0 ); glVertex2i( -1, -1 );
				glTexCoord2i( 1, 0 ); glVertex2i( 1, -1 );
				glTexCoord2i( 1, 1 ); glVertex2i( 1, 1 );
				glTexCoord2i( 0, 1 ); glVertex2i( -1, 1 );
				glEnd();

                water_simulator_shader3->release();

                glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
                gfx->glDisable(GL_LIGHTING);

                gfx->glEnable(GL_DEPTH_TEST);

				int ln2w = Math::Log2(SCREEN_W);
				int ln2h = Math::Log2(SCREEN_H);
				if ((1 << ln2w) < SCREEN_W)
					++ln2w;
				if ((1 << ln2h) < SCREEN_H)
					++ln2h;
				const int workwidth = g_useNonPowerOfTwoTextures ? SCREEN_W : 1 << ln2w;
				const int workheight = g_useNonPowerOfTwoTextures ? SCREEN_H : 1 << ln2h;

                gfx->glViewport(0,0,workwidth,workheight);

				// Render water distortion effects (ripples, waves, ...)
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_distortions, 0);
                gfx->glClearColor(0,0,0,0);
                gfx->glClear(GL_COLOR_BUFFER_BIT);		// Efface la texture tampon

				cam.setView(true);
                water_distortions_shader->bind();
				fx_manager.drawWaterDistortions();
                water_distortions_shader->release();

				glMatrixMode (GL_PROJECTION);
				glLoadIdentity ();
				glMatrixMode (GL_MODELVIEW);
				glLoadIdentity();

				// First pass of water rendering, store reflection vector
                gfx->glBindFramebuffer(GL_FRAMEBUFFER, water_FBO);
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, first_pass, 0);

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glEnable(GL_TEXTURE_2D);
                map->lava_map->bind();

                water_simulator_shader4->bind();
                water_simulator_shader4->setUniformValue("lava",0);
                water_simulator_shader4->setUniformValue("t",t);
                water_simulator_shader4->setUniformValue("factor",(float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

                water_simulator_shader4->release();

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, second_pass, 0);					// Second pass of water rendering, store viewing vector

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glDisable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE1);
                gfx->glDisable(GL_TEXTURE_2D);

                water_pass2->bind();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

                water_pass2->release();

                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_color, 0);					// Third pass of water rendering, store water color

                gfx->glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                gfx->glViewport(0,0,512,512);

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glEnable(GL_TEXTURE_2D);
                map->low_tex->bind();

				cam.setView(true);
				glTranslatef( 0.0f, map->sealvl, map->sea_dec);
				water_obj->draw(t, false);

                gfx->renderToTexture();
                gfx->glViewport(0, 0, workwidth, workheight);

				float logw = logf((float)SCREEN_W) / logf(2.0f);
				float logh = logf((float)SCREEN_H) / logf(2.0f);
				int wx = logw>(int)logw ? (int)logw+1 : (int)logw;
				int wy = logh>(int)logh ? (int)logh+1 : (int)logh;
				wx = 1 << wx;
				wy = 1 << wy;
                transtex->bind();								// Store what's on screen for transparency effect
                gfx->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, wx, wy, 0);

                gfx->glEnable(GL_STENCIL_TEST);											// Draw basic water in order to have correct texture mapping
                gfx->glClear(GL_STENCIL_BUFFER_BIT);
                gfx->glStencilFunc(GL_ALWAYS,128, 0xffffffff);
                gfx->glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

                gfx->glActiveTexture(GL_TEXTURE0);
                gfx->glDisable(GL_TEXTURE_2D);

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

                gfx->glDisable(GL_STENCIL_TEST);

				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);

                gfx->glEnable(GL_LIGHTING);

                gfx->glActiveTexture(GL_TEXTURE0);
				if (map->ota_data.lavaworld)
                    sky.skyTex()->bind();
				else
                    reflectex->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE1);
                transtex->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE2);
                first_pass->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE3);
                second_pass->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE4);
                water_color->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE5);
                height_tex->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE6);
                water_sim2->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                gfx->glActiveTexture(GL_TEXTURE7);
                water_distortions->bind();
                gfx->glEnable(GL_TEXTURE_2D);

                water_simulator_reflec->bind();
                water_simulator_reflec->setUniformValue("sky",0);
                water_simulator_reflec->setUniformValue("rtex",1);
                water_simulator_reflec->setUniformValue("bump",2);
                water_simulator_reflec->setUniformValue("view",3);
                water_simulator_reflec->setUniformValue("water_color",4);
                water_simulator_reflec->setUniformValue("height_map",5);
                water_simulator_reflec->setUniformValue("normal_map",6);
                water_simulator_reflec->setUniformValue("distort_map",7);
                water_simulator_reflec->setUniformValue("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
                water_simulator_reflec->setUniformValue("cam_h_factor", 1.0f / cam.rpos.y);
                water_simulator_reflec->setUniformValue("factor",(float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
                water_simulator_reflec->setUniformValue("t", t);

				glColor4ub(0xFF,0xFF,0xFF,0xFF);
                gfx->glDisable(GL_DEPTH_TEST);

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

                gfx->glEnable(GL_STENCIL_TEST);
                gfx->glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
                gfx->glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f);	glVertex3f(0,0,0);
				glTexCoord2f(1.0f,1.0f);	glVertex3f((float)SCREEN_W,0,0);
				glTexCoord2f(1.0f,0.0f);	glVertex3f((float)SCREEN_W,(float)SCREEN_H,0);
				glTexCoord2f(0.0f,0.0f);	glVertex3f(0,(float)SCREEN_H,0);
				glEnd();
                gfx->glDisable(GL_STENCIL_TEST);
                gfx->glEnable(GL_DEPTH_TEST);

                gfx->glActiveTexture(GL_TEXTURE7);
				if (lp_CONFIG->shadow_quality >= 2 && cam.rpos.y <= gfx->low_def_limit)
                    gfx->get_shadow_map()->bind();
				else
                    gfx->glDisable(GL_TEXTURE_2D);

                water_simulator_reflec->release();
			}
			gfx->ReInitAllTex(true);
		}
		cam.setView();
	}

	void Battle::renderWorld()
	{
		gfx->SetDefState();
        gfx->glClearColor(FogColor[0],FogColor[1],FogColor[2],FogColor[3]);
		gfx->clearDepth();		// Clear screen

		cam.setView();

		pSun.Set(cam);
		pSun.Enable();

		cam.setView();

		cam.zfar *= 100.0f;
		cam.setView();
        gfx->glDisable(GL_FOG);
		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        gfx->glEnable(GL_TEXTURE_2D);
        gfx->glDisable(GL_BLEND);
		if (lp_CONFIG->render_sky)
		{
            gfx->glDisable(GL_LIGHTING);
            gfx->glDepthMask(GL_FALSE);
			glTranslatef(cam.rpos.x, cam.rpos.y + cam.shakeVector.y, cam.rpos.z);
			glRotatef(sky_angle, 0.0f, 1.0f, 0.0f);
			if (lp_CONFIG->ortho_camera)
			{
				const float scale = cam.zoomFactor / 800.0f * std::sqrt(float(SCREEN_H * SCREEN_H + SCREEN_W * SCREEN_W));
				glScalef( scale, scale, scale );
			}
			sky.draw();
		}
		else
			gfx->clearScreen();

        gfx->glDepthMask(GL_TRUE);
        gfx->glEnable(GL_CULL_FACE);
        gfx->glEnable(GL_LIGHTING);
        gfx->glEnable(GL_FOG);
		updateZFAR();

		if (lp_CONFIG->wireframe)
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

        map->draw(&cam, byte(1 << players.local_human_id), false, 0.0f, t, dt * units.apparent_timefactor);

		if (lp_CONFIG->wireframe)
			glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

		cam.setView(lp_CONFIG->shadow_quality < 2);

        features.draw(render_time);		// Dessine les éléments "2D"

		/*----------------------------------------------------------------------------------------------*/

        // Dessine les unités sous l'eau / Draw units which are under water
        cam.setView(lp_CONFIG->shadow_quality < 2);
        if (cam.rpos.y <= gfx->low_def_limit)
        {
            if (lp_CONFIG->shadow_quality >= 2)
                glFogi (GL_FOG_COORD_SRC, GL_FOG_COORD);
            units.draw(true, false, true, lp_CONFIG->height_line);
            glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
        }

        // Dessine les objets produits par les armes sous l'eau / Draw weapons which are under water
        weapons.draw(true);

        if (lp_CONFIG->particle)
            particle_engine.drawUW();

//        renderWater();

        // Render map object icons (if in tactical mode)
        if (cam.rpos.y > gfx->low_def_limit)
        {
            cam.setView(true);
            features.draw_icons();
        }

        cam.setView(lp_CONFIG->shadow_quality < 2);
        if (lp_CONFIG->shadow_quality >= 2)
            glFogi (GL_FOG_COORD_SRC, GL_FOG_COORD);
        // Dessine les unités non encore dessinées / Draw units which have not been drawn
        units.draw(false, false, true, lp_CONFIG->height_line);
        glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);

        // Dessine les objets produits par les armes n'ayant pas été dessinés / Draw weapons which have not been drawn
        weapons.draw(false);
	}

	void Battle::renderInfo()
	{
		if (build >= 0 && !IsOnGUI)	// Display the building we want to build (with nice selection quads)
		{
            gfx->glDisable(GL_FOG);
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
				glScalef(unit_manager.unit_type[build]->Scale,unit_manager.unit_type[build]->Scale,unit_manager.unit_type[build]->Scale);
				const float DX = float(unit_manager.unit_type[build]->FootprintX << 2);
				const float DZ = float(unit_manager.unit_type[build]->FootprintZ << 2);
				if (unit_manager.unit_type[build]->model)
				{
                    gfx->glEnable(GL_CULL_FACE);
					gfx->ReInitAllTex( true);
					if (can_be_there)
						glColor4ub(0xFF,0xFF,0xFF,0xFF);
					else
						glColor4ub(0xFF,0,0,0xFF);
                    gfx->glDepthFunc( GL_GREATER );
					unit_manager.unit_type[build]->model->draw(0.0f,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);
                    gfx->glDepthFunc( GL_LESS );
					unit_manager.unit_type[build]->model->draw(0.0f,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);

					const bool old_mode = gfx->getShadowMapMode();
					gfx->setShadowMapMode(true);
					double eqn[4]= { 0.0f, -1.0f, 0.0f, map->sealvl - target.y };
					glClipPlane(GL_CLIP_PLANE2, eqn);

					glEnable(GL_CLIP_PLANE2);

					glEnable( GL_BLEND );
					glBlendFunc( GL_ONE, GL_ONE );
					glDepthFunc( GL_EQUAL );
					glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
					unit_manager.unit_type[build]->model->draw(0.0f,NULL,false,true,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);
					glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
					glDepthFunc( GL_LESS );
					glDisable( GL_BLEND );

					glDisable(GL_CLIP_PLANE2);
					gfx->setShadowMapMode(old_mode);
				}
				cam.setView();
				glTranslatef(target.x,Math::Max( target.y, map->sealvl ),target.z);
				byte red = 0xFF, green = 0x00;
				if (can_be_there)
				{
					green = 0xFF;
					red   = 0x00;
				}
				glDisable(GL_CULL_FACE);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LIGHTING);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_QUADS);
				glColor4ub(red,green,0x00,0xFF);
				glVertex3f(-DX,0.0f,-DZ);			// First quad
				glVertex3f(DX,0.0f,-DZ);
				glColor4ub(red,green,0x00,0x00);
				glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
				glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

				glColor4ub(red,green,0x00,0xFF);
				glVertex3f(-DX,0.0f,-DZ);			// Second quad
				glVertex3f(-DX,0.0f,DZ);
				glColor4ub(red,green,0x00,0x00);
				glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
				glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

				glColor4ub(red,green,0x00,0xFF);
				glVertex3f(DX,0.0f,-DZ);			// Third quad
				glVertex3f(DX,0.0f,DZ);
				glColor4ub(red,green,0x00,0x00);
				glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
				glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);

				glColor4ub(red,green,0x00,0xFF);
				glVertex3f(-DX,0.0f,DZ);			// Fourth quad
				glVertex3f(DX,0.0f,DZ);
				glColor4ub(red,green,0x00,0x00);
				glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
				glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
				glEnd();
				glDisable(GL_BLEND);
				glEnable(GL_LIGHTING);
				glEnable(GL_CULL_FACE);
			}
			glEnable(GL_FOG);
		}


		if ((selected || units.last_on >= 0) && TA3D_SHIFT_PRESSED)
		{
			glDisable(GL_FOG);
			cam.setView();
			bool builders = false;
			const float t = (float)msectimer() * 0.001f;
			const float mt = std::fmod(0.5f * t, 1.0f);
			for (unsigned int e = 0; e < units.index_list_size ; ++e)
			{
				const int i = units.idx_list[e];
				if ((units.unit[i].flags & 1)
					&& units.unit[i].owner_id == players.local_human_id
					&& (units.unit[i].sel || i == units.last_on))
				{
					const int type_id = units.unit[i].type_id;
					if (type_id >= 0)
					{
						const UnitType* const pType = unit_manager.unit_type[type_id];
						builders |= pType->Builder;

						const float x = units.unit[i].render.Pos.x;
						const float z = units.unit[i].render.Pos.z;
						if (pType->kamikaze)
						{
							the_map->drawCircleOnMap(x, z, pType->kamikazedistance, makeacol(0xFF,0x0,0x0,0xFF), 1.0f);
							const int idx = weapon_manager.get_weapon_index(pType->SelfDestructAs);
							const WeaponDef* const pWeapon = idx >= 0 && idx < weapon_manager.nb_weapons ? &(weapon_manager.weapon[idx]) : NULL;
							if (pWeapon)
								the_map->drawCircleOnMap(x, z, (float)pWeapon->areaofeffect * 0.25f * mt, makeacol(0xFF,0x0,0x0,0xFF), 1.0f);
						}
						if (pType->mincloakdistance && units.unit[i].cloaked)
							the_map->drawCircleOnMap(x, z, (float)pType->mincloakdistance, makeacol(0xFF,0xFF,0xFF,0xFF), 1.0f);
					}
					if (units.unit[i].sel)
						units.unit[i].show_orders();					// Dessine les ordres reçus par l'unité / Draw given orders
				}
			}

			if (builders)
			{
				for (unsigned int e = 0; e < units.index_list_size; ++e)
				{
					const int i = units.idx_list[e];
					const int type_id = units.unit[i].type_id;
					if (type_id < 0)
						continue;
					if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && !units.unit[i].sel
						&& unit_manager.unit_type[type_id]->Builder && unit_manager.unit_type[type_id]->BMcode)
					{
						units.unit[i].show_orders(true);					// Dessine les ordres reçus par l'unité / Draw given orders
					}
				}
			}
			glEnable(GL_FOG);
		}
		if ((selected || units.last_on >= 0) && TA3D_CTRL_PRESSED)
		{
			glDisable(GL_FOG);
			cam.setView();
			const float t = (float)msectimer() * 0.001f;
			const float mt = std::fmod(0.5f * t, 1.0f);
			const float mt2 = std::fmod(0.5f * t + 0.5f, 1.0f);
			for (unsigned int e = 0; e < units.index_list_size ; ++e)
			{
				const int i = units.idx_list[e];
				if ((units.unit[i].flags & 1)
					&& units.unit[i].owner_id == players.local_human_id
					&& (units.unit[i].sel || i == units.last_on))
				{
					const int type_id = units.unit[i].type_id;
					if (type_id >= 0)
					{
						const UnitType* const pType = unit_manager.unit_type[type_id];

						const float x = units.unit[i].render.Pos.x;
						const float z = units.unit[i].render.Pos.z;
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
							if (pType->mincloakdistance && units.unit[i].cloaked)
								the_map->drawCircleOnMap(x, z, (float)pType->mincloakdistance, makeacol(0xFF,0xFF,0xFF,0xFF), 1.0f);
						}
						if (!pType->onoffable || units.unit[i].port[ACTIVATION])
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
			glEnable(GL_FOG);
		}
		if (showHealthBars)
		{
			cam.setView();
			units.drawHealthBars();
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



