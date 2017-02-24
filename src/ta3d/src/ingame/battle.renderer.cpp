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

		glActiveTextureARB(GL_TEXTURE7_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		gfx->setShadowMapMode(false);
		if (g_useProgram)
			glUseProgramObjectARB(0);
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
				weapons.draw(map.get());
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

			glBindTexture(GL_TEXTURE_2D,reflectex);								// Store what's on screen for reflection effect
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

				gfx->renderToTextureDepth(0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

				glActiveTextureARB(GL_TEXTURE7_ARB);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, gfx->get_shadow_map());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
				glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				gfx->setShadowMapMode(false);
				break;
			};
		}
	}

	void Battle::renderStencilShadow()
	{
		if (lp_CONFIG->shadow_quality > 0 && cam.rpos.y <= gfx->low_def_limit)
		{
			switch (lp_CONFIG->shadow_quality)
			{
					case 1:                     // Stencil Shadowing (shadow volumes)
						if (rotate_light)
						{
							pSun.Dir.x = -1.0f;
							pSun.Dir.y = 1.0f;
							pSun.Dir.z = 1.0f;
							pSun.Dir.unit();
							Vector3D Dir(-pSun.Dir);
							Dir.x = cosf(light_angle);
							Dir.z = sinf(light_angle);
							Dir.unit();
							pSun.Dir = -Dir;
							units.draw_shadow(render_time, Dir);
						}
						else
						{
							pSun.Dir.x = -1.0f;
							pSun.Dir.y = 1.0f;
							pSun.Dir.z = 1.0f;
							pSun.Dir.unit();
							units.draw_shadow(render_time, -pSun.Dir);
						}
						break;
					case 2:                     // Shadow mapping
					case 3:
						break;
					}
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
					glBindTexture(GL_TEXTURE_2D,map->low_tex);

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
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,water_FBO);
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,first_pass,0);

				//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
				glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

				glViewport(0,0,512,512);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,map->lava_map);
				glClientActiveTextureARB(GL_TEXTURE0_ARB);

				glActiveTextureARB(GL_TEXTURE1_ARB);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,water);
				glClientActiveTextureARB(GL_TEXTURE1_ARB);

				if (lp_CONFIG->water_quality == 2)
				{
					water_pass1_low.on();
					water_pass1_low.setvar1i("lava",0);
					water_pass1_low.setvar1i("map",1);
					water_pass1_low.setvar1f("t",t);
					water_pass1_low.setvar2f("factor", (float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
				}
				else
				{
					water_pass1.on();
					water_pass1.setvar1i("lava",0);
					water_pass1.setvar1i("map",1);
					water_pass1.setvar1f("t",t);
					water_pass1.setvar2f("factor", (float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
				}

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

				if (lp_CONFIG->water_quality == 2)
					water_pass1_low.off();
				else
					water_pass1.off();

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,second_pass,0);					// Second pass of water rendering, store viewing vector

				//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
				glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glDisable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);

				water_pass2.on();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

				water_pass2.off();

				if (lp_CONFIG->water_quality > 2)
				{
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,water_color,0);					// Third pass of water rendering, store water color

					glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

					glActiveTextureARB(GL_TEXTURE0_ARB);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D,map->low_tex);

					cam.setView();
					glTranslatef( 0.0f, map->sealvl, map->sea_dec);
					water_obj->draw(t, false);
				}

				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

				glViewport(0, 0, SCREEN_W, SCREEN_H);

				float logw = logf((float)SCREEN_W) / logf(2.0f);
				float logh = logf((float)SCREEN_H) / logf(2.0f);
				int wx = logw>(int)logw ? (int)logw+1 : (int)logw;
				int wy = logh>(int)logh ? (int)logh+1 : (int)logh;
				wx = 1 << wx;
				wy = 1 << wy;
				glBindTexture(GL_TEXTURE_2D,transtex);								// Store what's on screen for transparency effect
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, wx, wy, 0);

				glEnable(GL_STENCIL_TEST);											// Draw basic water in order to have correct texture mapping
				glClear(GL_STENCIL_BUFFER_BIT);
				glStencilFunc(GL_ALWAYS,128, 0xffffffff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glDisable(GL_TEXTURE_2D);
				glClientActiveTextureARB(GL_TEXTURE0_ARB);

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

				glDisable(GL_STENCIL_TEST);

				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);

				glEnable(GL_LIGHTING);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				if (map->ota_data.lavaworld)
					glBindTexture(GL_TEXTURE_2D, sky.skyTex());
				else
					glBindTexture(GL_TEXTURE_2D, reflectex);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE1_ARB);
				glBindTexture(GL_TEXTURE_2D,transtex);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE2_ARB);
				glBindTexture(GL_TEXTURE_2D,first_pass);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE3_ARB);
				glBindTexture(GL_TEXTURE_2D,second_pass);
				glEnable(GL_TEXTURE_2D);

				if (lp_CONFIG->water_quality == 2)
				{
					water_shader.on();
					water_shader.setvar1i("sky",0);
					water_shader.setvar1i("rtex",1);
					water_shader.setvar1i("bump",2);
					water_shader.setvar1i("view",3);
					water_shader.setvar2f("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
				}
				else
				{
					glActiveTextureARB(GL_TEXTURE4_ARB);
					glBindTexture(GL_TEXTURE_2D,water_color);
					glEnable(GL_TEXTURE_2D);

					water_shader_reflec.on();
					water_shader_reflec.setvar1i("sky",0);
					water_shader_reflec.setvar1i("rtex",1);
					water_shader_reflec.setvar1i("bump",2);
					water_shader_reflec.setvar1i("view",3);
					water_shader_reflec.setvar1i("water_color",4);
					water_shader_reflec.setvar2f("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
				}

				glColor4ub(0xFF,0xFF,0xFF,0xFF);
				glDisable(GL_DEPTH_TEST);

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				glEnable(GL_STENCIL_TEST);
				glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
				glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f);	glVertex3f(0,0,0);
				glTexCoord2f(1.0f,1.0f);	glVertex3f((float)SCREEN_W,0,0);
				glTexCoord2f(1.0f,0.0f);	glVertex3f((float)SCREEN_W,(float)SCREEN_H,0);
				glTexCoord2f(0.0f,0.0f);	glVertex3f(0,(float)SCREEN_H,0);
				glEnd();
				glDisable(GL_STENCIL_TEST);
				glEnable(GL_DEPTH_TEST);

				if (lp_CONFIG->water_quality == 2)
					water_shader.off();
				else
					water_shader_reflec.off();
			}
			else                            // New Ultimate quality water renderer
			{
				// Run water simulation entirely on the GPU
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,water_FBO);

				glViewport(0,0,256,256);

				glDisable(GL_DEPTH_TEST);
				glDisable(GL_LIGHTING);

				glMatrixMode (GL_PROJECTION);
				glLoadIdentity ();
				glMatrixMode (GL_MODELVIEW);
				glLoadIdentity();

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);

				const float time_step = 0.02f;
				const float time_to_simulate = Math::Min( dt * units.apparent_timefactor, time_step * 3.0f );

				// Simulate water
				for(float real_time = 0.0f ; real_time < time_to_simulate ; real_time += time_step)
				{
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,water_sim0,0);
					glBindTexture(GL_TEXTURE_2D,water_sim1);

					bool refresh = false;
					if (msec_timer - last_water_refresh >= 100000)
					{
						last_water_refresh = msec_timer;
						refresh = true;
					}
					float dt_step = Math::Min( time_to_simulate - real_time, time_step );
					water_simulator_shader.on();
					water_simulator_shader.setvar1i("sim",0);
					water_simulator_shader.setvar1f("fluid",50.0f * dt_step);
					water_simulator_shader.setvar1f("t", refresh ? 1.0f : 0.0f);

					glBegin( GL_QUADS );
					glTexCoord2i( 0, 0 ); glVertex2i( -1, -1 );
					glTexCoord2i( 1, 0 ); glVertex2i( 1, -1 );
					glTexCoord2i( 1, 1 ); glVertex2i( 1, 1 );
					glTexCoord2i( 0, 1 ); glVertex2i( -1, 1 );
					glEnd();

					water_simulator_shader.off();

					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,water_sim1,0);
					glBindTexture(GL_TEXTURE_2D,water_sim0);

					water_simulator_shader2.on();
					water_simulator_shader2.setvar1i("sim",0);
					water_simulator_shader2.setvar1f("dt", dt_step);

					glBegin( GL_QUADS );
					glTexCoord2i( 0, 0 ); glVertex2i( -1, -1 );
					glTexCoord2i( 1, 0 ); glVertex2i( 1, -1 );
					glTexCoord2i( 1, 1 ); glVertex2i( 1, 1 );
					glTexCoord2i( 0, 1 ); glVertex2i( -1, 1 );
					glEnd();

					water_simulator_shader2.off();
				}

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,water_sim2,0);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,water_sim1);

				water_simulator_shader3.on();
				water_simulator_shader3.setvar1i("sim",0);

				glBegin( GL_QUADS );
				glTexCoord2i( 0, 0 ); glVertex2i( -1, -1 );
				glTexCoord2i( 1, 0 ); glVertex2i( 1, -1 );
				glTexCoord2i( 1, 1 ); glVertex2i( 1, 1 );
				glTexCoord2i( 0, 1 ); glVertex2i( -1, 1 );
				glEnd();

				water_simulator_shader3.off();

				glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
				glDisable(GL_LIGHTING);

				glEnable(GL_DEPTH_TEST);

				int ln2w = Math::Log2(SCREEN_W);
				int ln2h = Math::Log2(SCREEN_H);
				if ((1 << ln2w) < SCREEN_W)
					++ln2w;
				if ((1 << ln2h) < SCREEN_H)
					++ln2h;
				const int workwidth = g_useNonPowerOfTwoTextures ? SCREEN_W : 1 << ln2w;
				const int workheight = g_useNonPowerOfTwoTextures ? SCREEN_H : 1 << ln2h;

				glViewport(0,0,workwidth,workheight);

				// Render water distortion effects (ripples, waves, ...)
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,water_FBO);
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,water_distortions,0);
				glClearColor(0,0,0,0);
				glClear(GL_COLOR_BUFFER_BIT);		// Efface la texture tampon

				cam.setView(true);
				water_distortions_shader.on();
				fx_manager.drawWaterDistortions();
				water_distortions_shader.off();

				glMatrixMode (GL_PROJECTION);
				glLoadIdentity ();
				glMatrixMode (GL_MODELVIEW);
				glLoadIdentity();

				// First pass of water rendering, store reflection vector
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,water_FBO);
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,first_pass,0);

				glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,map->lava_map);

				water_simulator_shader4.on();
				water_simulator_shader4.setvar1i("lava",0);
				water_simulator_shader4.setvar1f("t",t);
				water_simulator_shader4.setvar2f("factor",(float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

				water_simulator_shader4.off();

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,second_pass,0);					// Second pass of water rendering, store viewing vector

				glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glDisable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);

				water_pass2.on();

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

				water_pass2.off();

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,water_color,0);					// Third pass of water rendering, store water color

				glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

				glViewport(0,0,512,512);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,map->low_tex);

				cam.setView(true);
				glTranslatef( 0.0f, map->sealvl, map->sea_dec);
				water_obj->draw(t, false);

				gfx->renderToTexture( 0 );
				glViewport(0,0,workwidth,workheight);

				float logw = logf((float)SCREEN_W) / logf(2.0f);
				float logh = logf((float)SCREEN_H) / logf(2.0f);
				int wx = logw>(int)logw ? (int)logw+1 : (int)logw;
				int wy = logh>(int)logh ? (int)logh+1 : (int)logh;
				wx = 1 << wx;
				wy = 1 << wy;
				glBindTexture(GL_TEXTURE_2D,transtex);								// Store what's on screen for transparency effect
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, wx, wy, 0);

				glEnable(GL_STENCIL_TEST);											// Draw basic water in order to have correct texture mapping
				glClear(GL_STENCIL_BUFFER_BIT);
				glStencilFunc(GL_ALWAYS,128, 0xffffffff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glDisable(GL_TEXTURE_2D);

				cam.setView(true);
				glTranslatef(0.0f,map->sealvl,0.0f);
				water_obj->draw(t, true);

				glDisable(GL_STENCIL_TEST);

				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);

				glEnable(GL_LIGHTING);

				glActiveTextureARB(GL_TEXTURE0_ARB);
				if (map->ota_data.lavaworld)
					glBindTexture(GL_TEXTURE_2D, sky.skyTex());
				else
					glBindTexture(GL_TEXTURE_2D,reflectex);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE1_ARB);
				glBindTexture(GL_TEXTURE_2D,transtex);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE2_ARB);
				glBindTexture(GL_TEXTURE_2D,first_pass);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE3_ARB);
				glBindTexture(GL_TEXTURE_2D,second_pass);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE4_ARB);
				glBindTexture(GL_TEXTURE_2D,water_color);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE5_ARB);
				glBindTexture(GL_TEXTURE_2D,height_tex);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE6_ARB);
				glBindTexture(GL_TEXTURE_2D,water_sim2);
				glEnable(GL_TEXTURE_2D);

				glActiveTextureARB(GL_TEXTURE7_ARB);
				glBindTexture(GL_TEXTURE_2D,water_distortions);
				glEnable(GL_TEXTURE_2D);

				water_simulator_reflec.on();
				water_simulator_reflec.setvar1i("sky",0);
				water_simulator_reflec.setvar1i("rtex",1);
				water_simulator_reflec.setvar1i("bump",2);
				water_simulator_reflec.setvar1i("view",3);
				water_simulator_reflec.setvar1i("water_color",4);
				water_simulator_reflec.setvar1i("height_map",5);
				water_simulator_reflec.setvar1i("normal_map",6);
				water_simulator_reflec.setvar1i("distort_map",7);
				water_simulator_reflec.setvar2f("coef", (float)SCREEN_W / (float)wx, (float)SCREEN_H / (float)wy);
				water_simulator_reflec.setvar1f("cam_h_factor", 1.0f / cam.rpos.y);
				water_simulator_reflec.setvar2f("factor",(float)water_obj->w / (float)map->map_w, (float)water_obj->w / (float)map->map_h);
				water_simulator_reflec.setvar1f("t", t);

				glColor4ub(0xFF,0xFF,0xFF,0xFF);
				glDisable(GL_DEPTH_TEST);

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				glEnable(GL_STENCIL_TEST);
				glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
				glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f);	glVertex3f(0,0,0);
				glTexCoord2f(1.0f,1.0f);	glVertex3f((float)SCREEN_W,0,0);
				glTexCoord2f(1.0f,0.0f);	glVertex3f((float)SCREEN_W,(float)SCREEN_H,0);
				glTexCoord2f(0.0f,0.0f);	glVertex3f(0,(float)SCREEN_H,0);
				glEnd();
				glDisable(GL_STENCIL_TEST);
				glEnable(GL_DEPTH_TEST);

				glActiveTextureARB(GL_TEXTURE7_ARB);
				if (lp_CONFIG->shadow_quality >= 2 && cam.rpos.y <= gfx->low_def_limit)
					glBindTexture(GL_TEXTURE_2D, gfx->get_shadow_map());
				else
					glDisable(GL_TEXTURE_2D);

				water_simulator_reflec.off();
			}
			gfx->ReInitAllTex(true);
		}
		cam.setView();
	}

	void Battle::renderWorld()
	{
		gfx->SetDefState();
		glClearColor(FogColor[0],FogColor[1],FogColor[2],FogColor[3]);
		gfx->clearDepth();		// Clear screen

		cam.setView();

		pSun.Set(cam);
		pSun.Enable();

		cam.setView();

		cam.zfar *= 100.0f;
		cam.setView();
		glDisable(GL_FOG);
		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		if (lp_CONFIG->render_sky)
		{
			glDisable(GL_LIGHTING);
			glDepthMask(GL_FALSE);
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

		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_FOG);
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

		renderWater();

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
			glDisable(GL_FOG);
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
					glEnable(GL_CULL_FACE);
					gfx->ReInitAllTex( true);
					if (can_be_there)
						glColor4ub(0xFF,0xFF,0xFF,0xFF);
					else
						glColor4ub(0xFF,0,0,0xFF);
					glDepthFunc( GL_GREATER );
					unit_manager.unit_type[build]->model->draw(0.0f,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);
					glDepthFunc( GL_LESS );
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
			const float t = (float)msec_timer * 0.001f;
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
			const float t = (float)msec_timer * 0.001f;
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

		renderStencilShadow();

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
			rest( 100 );			// Wait for the engine to enter in pause mode so we can assemble several shots
									// of the same game tick

		Camera camBak = cam;

		cam.znear = -255.0f;
		SDL_Surface *poster = gfx->create_surface_ex(24,w,h);
		SDL_Surface *buf = gfx->create_surface_ex(24,SCREEN_W,SCREEN_H);

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
				glReadPixels(0, 0, SCREEN_W, SCREEN_H, GL_BGR, GL_UNSIGNED_BYTE, buf->pixels);

				// Fill current part of the poster
				blit(buf, poster, SCREEN_W / 4, SCREEN_H / 4, x, z, Math::Min(SCREEN_W / 2, poster->w - x), Math::Min(SCREEN_H / 2, poster->h - z));
			}
		}

		vflip_bitmap(poster);
        save_TGA(TA3D::Paths::Screenshots + "poster.tga", poster);

		SDL_FreeSurface(buf);
		SDL_FreeSurface(poster);

		cam = camBak;

		lp_CONFIG->pause = previous_pause_state;
		lp_CONFIG->ortho_camera = prevCameraType;
	}



} // namespace TA3D



