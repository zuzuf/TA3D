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

/*-----------------------------------------------------------------\
  |                               tdf.cpp                            |
  |   contient toutes les fonctions et classes permettant la gestion |
  | des fichiers TDF du jeu Total Annihilation qui contienne divers  |
  | éléments graphiques.                                             |
  \-----------------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "tdf.h"
#include "EngineClass.h"
#include "UnitEngine.h"
#include "network/TA3D_Network.h"
#include "languages/i18n.h"
#include "logs/logs.h"
#include "misc/math.h"
#include "misc/material.light.h"
#include "ingame/players.h"



namespace TA3D
{


	FeatureManager		feature_manager;
	Features features;

	Feature::Feature()
	{
		init();
	}

	Feature::~Feature()
	{
		destroy();
	}


	void Feature::init()
	{
		not_loaded=true;
		need_convert=true;
		flamable=false;
		burnmin = 0;
		burnmax = 0;
		sparktime = 0;
		spreadchance = 0;

		geothermal=false;
		blocking=false;
		reclaimable=false;
		autoreclaimable=false;
		energy=0;
		converted=false;
		m3d=false;
		model=NULL;
		vent=false;
		animating=false;
		footprintx=0;
		footprintz=0;
		height=0;
		animtrans=false;
		shadtrans=false;
		hitdensity=0;
		metal=0;
		damage=100;
		indestructible=false;

		burnweapon.clear();
		feature_reclamate.clear();
		feature_burnt.clear();
		feature_dead.clear();
		name.clear();
		world.clear();
		description.clear();
		category.clear();
		filename.clear();
		seqname.clear();

		anim.init();
	}

	void Feature::destroy()
	{
		burnweapon.clear();
		feature_reclamate.clear();
		feature_burnt.clear();
		feature_dead.clear();
		name.clear();
		world.clear();
		description.clear();
		category.clear();
		filename.clear();
		seqname.clear();
		anim.destroy();
		init();
	}


	void Feature::convert()
	{
		if (not_loaded)
		{
			not_loaded = false;
			String tmp("anims\\");
			tmp << filename << ".gaf";
			byte* gaf = HPIManager->PullFromHPI(tmp);
			if (gaf)
			{
				sint32 index = Gaf::RawDataGetEntryIndex(gaf, seqname);
				if (index >= 0)
					anim.loadGAFFromRawData(gaf, Gaf::RawDataGetEntryIndex(gaf, seqname), true, filename);
				else
					LOG_WARNING(LOG_PREFIX_TDF << "`" << name << "` has no picture to display (" << filename << ".gaf, " << seqname << ") !");
				delete[] gaf;
				need_convert = true;
			}
		}
		if (need_convert)
		{
			need_convert = false;
			anim.convert(false,true);
			anim.clean();
		}
	}




	FeatureManager::FeatureManager()
		:feature_hashtable()
	{
		init();
	}


	FeatureManager::~FeatureManager()
	{
		destroy();
		feature_hashtable.emptyHashTable();
	}


	void FeatureManager::init()
	{
		nb_features = 0;
		feature.clear();
	}


	int FeatureManager::get_feature_index(const String &name)
	{
		if (name.empty())
			return -1;
		return feature_hashtable.find(String::ToLower(name)) - 1;
	}


	int FeatureManager::add_feature(const String& name)			// Ajoute un élément
	{
		++nb_features;
		feature.push_back(new Feature);
		feature.back()->name = name;
		feature_hashtable.insert(String::ToLower(name), nb_features);
		return nb_features - 1;
	}

	void FeatureManager::destroy()
	{
		if (nb_features > 0 && !feature.empty())			// Détruit les éléments
		{
			for (unsigned int i = 0; i < feature.size(); ++i)
				delete feature[i];
		}
		feature.clear();

		feature_hashtable.emptyHashTable();
		feature_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);
		init();
	}


	void FeatureManager::clean()
	{
		if (!feature.empty())
		{
			for (unsigned int i = 0; i < feature.size(); ++i)
			{
				if (!feature[i]->need_convert)
					feature[i]->anim.clean();
			}
		}
	}



	void FeatureManager::load_tdf(char *data,int size)					// Charge un fichier tdf
	{
		TDFParser parser;
		parser.loadFromMemory("TDF",data,size,false,true,true);
		int	first = nb_features;

		for (int g = 0 ; parser.exists(format("gadget%d",g)) ; g++)
		{
			const String& key = format("gadget%d.",g);

			int index = add_feature( parser.pullAsString(format("gadget%d", g)) );
			Feature *pFeature = feature[index];
			pFeature->m3d = false;
			pFeature->world = parser.pullAsString( key + "world", pFeature->world );
			pFeature->description = parser.pullAsString( key + "description", pFeature->description );
			pFeature->category = parser.pullAsString( key + "category", pFeature->category );
			pFeature->filename = parser.pullAsString( key + "object" );
			pFeature->m3d = !pFeature->filename.empty();
			if (!pFeature->m3d)
			{
				pFeature->filename = parser.pullAsString( key + "filename");
				pFeature->seqname = parser.pullAsString( key + "seqname");
			}
			pFeature->animating = parser.pullAsBool( key + "animating",pFeature->animating );
			pFeature->animating |= (pFeature->animtrans = parser.pullAsBool( key + "animtrans", pFeature->animtrans ));
			pFeature->shadtrans = parser.pullAsBool( key + "shadtrans", pFeature->shadtrans );
			pFeature->indestructible = parser.pullAsBool( key + "indestructible", pFeature->indestructible );
			pFeature->height = parser.pullAsInt( key + "height", pFeature->height );
			pFeature->hitdensity = parser.pullAsInt( key + "hitdensity", pFeature->hitdensity );
			pFeature->metal = parser.pullAsInt( key + "metal", pFeature->metal );
			pFeature->energy = parser.pullAsInt( key + "energy", pFeature->energy );
			pFeature->damage = parser.pullAsInt( key + "damage", pFeature->damage );
			pFeature->footprintx = parser.pullAsInt( key + "footprintx", pFeature->footprintx );
			pFeature->footprintz = parser.pullAsInt( key + "footprintz", pFeature->footprintz );
			pFeature->autoreclaimable = parser.pullAsBool( key + "autoreclaimable", pFeature->autoreclaimable );
			pFeature->reclaimable = parser.pullAsBool( key + "reclaimable", pFeature->reclaimable );
			pFeature->blocking = parser.pullAsBool( key + "blocking", pFeature->blocking );
			pFeature->flamable = parser.pullAsBool( key + "flamable", pFeature->flamable );
			pFeature->geothermal = parser.pullAsBool( key + "geothermal", pFeature->geothermal );
			pFeature->feature_dead = parser.pullAsString( key + "featuredead" );
			pFeature->burnmin = parser.pullAsInt( key + "burnmin", pFeature->burnmin );
			pFeature->burnmax = parser.pullAsInt( key + "burnmax", pFeature->burnmax );
			pFeature->sparktime = parser.pullAsInt( key + "sparktime", pFeature->sparktime );
			pFeature->spreadchance = parser.pullAsInt( key + "spreadchance", pFeature->spreadchance );
			pFeature->burnweapon = parser.pullAsString( key + "burnweapon" );
			pFeature->feature_burnt = parser.pullAsString( key + "featureburnt" );
			pFeature->feature_reclamate = parser.pullAsString( key + "featurereclamate" );
		}

		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGBA_ARB);
		else
			gfx->set_texture_format(GL_RGBA8);

		for (int i = first; i < nb_features; ++i)// Charge les fichiers d'animation
		{
			if (!feature[i]->category.empty())
				feature[i]->vent = (strstr(feature[i]->category.c_str(), "vents") != NULL);
			if (!feature[i]->filename.empty() && !feature[i]->seqname.empty() && !feature[i]->m3d)
			{
				if (model_manager.get_model(feature[i]->filename + "-" + feature[i]->seqname) != NULL) // Check if there is a 3do version of it
				{
					feature[i]->model=NULL;
					feature[i]->m3d=true;
					feature[i]->converted=false;
					feature[i]->not_loaded=false;
				}
				else
				{
					feature[i]->not_loaded=true;
					if (feature[i]->height<=10.0f && feature[i]->height>1.0f && feature[i]->blocking
						&& strcasecmp(feature[i]->description.c_str(),"Metal")!=0) // Tente une conversion en 3d
					{
						String tmp("anims\\");
						tmp << feature[i]->filename << ".gaf";
						byte* gaf = HPIManager->PullFromHPI(tmp);
						if (gaf)
						{
							sint32 index = Gaf::RawDataGetEntryIndex(gaf, feature[i]->seqname);
							if (index >= 0)
								feature[i]->anim.loadGAFFromRawData(gaf, Gaf::RawDataGetEntryIndex(gaf, feature[i]->seqname), true, feature[i]->filename);
							else
								LOG_WARNING(LOG_PREFIX_TDF << "`" << feature[i]->name << "` has no picture to display (" << feature[i]->filename << ".gaf, " << feature[i]->seqname << ") !");
							feature[i]->not_loaded = false;
							delete[] gaf;

							if (index>=0 && feature[i]->anim.nb_bmp>0
								&& feature[i]->anim.bmp[0]->w>=16 && feature[i]->anim.bmp[0]->h>=16) // Tente une conversion en 3d
							{
								String st(feature[i]->filename);
								st << "-" << feature[i]->seqname;
								model_manager.create_from_2d(feature[i]->anim.bmp[0],
															 feature[i]->footprintx * 8,
															 feature[i]->footprintz * 8,
															 feature[i]->height * H_DIV,
															 st);
								feature[i]->model = NULL;
								feature[i]->m3d = true;
								feature[i]->converted = true;
								feature[i]->anim.destroy();
								index = -1;
							}
							if (index < 0)
								feature[i]->need_convert = false;
						}
					}
				}
			}
			else
			{
				if (!feature[i]->filename.empty() && feature[i]->m3d)
					feature[i]->model = NULL;
			}
		}
	}



	void load_features(void (*progress)(float percent, const String& msg)) // Charge tout les éléments
	{
		String::List files;
		HPIManager->getFilelist("features\\*.tdf", files);
		int n = 0;

		for (String::List::const_iterator curFile = files.begin(); curFile != files.end(); ++curFile)
		{
			if (progress != NULL && !(n & 0xF))
			{
				progress((200.0f + n * 50.0f / (files.size() + 1)) / 7.0f,
						 I18N::Translate("Loading graphical features"));
			}

			++n;
			uint32 file_size(0);
			byte* data = HPIManager->PullFromHPI(curFile->c_str(), &file_size);
			if (data)
			{
				LOG_DEBUG(LOG_PREFIX_TDF << "Loading feature: `" << *curFile << "`...");
				feature_manager.load_tdf((char*)data, file_size);
				delete[] data;
			}
			else
				LOG_WARNING(LOG_PREFIX_TDF << "Loading `" << *curFile << "` failed");
		}

		// Temporary string - Avoid multiple and unnecessary malloc if outside of the loop
		String tmp;

		// Foreach item...
		for (int i = 0 ; i < feature_manager.getNbFeatures() ; ++i)
		{
			Feature *feature = feature_manager.getFeaturePointer(i);
			if (feature->m3d && feature->model == NULL
				&& !feature->filename.empty() && !feature->seqname.empty())
			{
				tmp = feature->filename;
				tmp << "-" << feature->seqname;
				feature->model = model_manager.get_model(tmp);
				if (feature->model == NULL)
					feature->model = model_manager.get_model(String("objects3d\\")+tmp);
			}
			else
			{
				if (feature->m3d && feature->model == NULL && !feature->filename.empty())
					feature->model = model_manager.get_model(feature->filename);
			}
		}
	}



	Features::Features()
		:nb_features(0), max_features(0), feature(NULL), min_idx(0), max_idx(0),
		burning_features(), sinking_features(),
		list(NULL), list_size(0)
	{}


	Features::~Features()
	{
		destroy();
	}


	void Features::init()
	{
		p_wind_dir   = NULL;
		nb_features  = 0;
		max_features = 0;
		feature = NULL;
		min_idx = 0;
		max_idx = 0;
		list = NULL;
		list_size = 0;
	}


	void Features::destroy()
	{
		if (feature)
		{
			for (int i = 0 ; i < max_features ; ++i)
			{
				if (feature[i].shadow_dlist)
				{
					glDeleteLists(feature[i].shadow_dlist, 1);
					feature[i].shadow_dlist = 0;
				}
			}
			delete[] feature;
		}
		if (list)
			delete[] list;
		init();
		burning_features.clear();
		sinking_features.clear();
	}


	void Features::draw(bool no_flat)
	{
		if(nb_features <= 0)
			return;
		gfx->ReInitAllTex(true);
		glAlphaFunc(GL_GREATER,0.1);
		glEnable(GL_ALPHA_TEST);

		glDepthFunc(GL_LEQUAL);

		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF);
		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		float sq2 = 1.0f / sqrtf(2.0f);
		GLuint old = 0;
		bool texture_loaded=false;

		static const GLubyte index[] =
		{
			0, 1, 2, 3,
			4, 1, 2, 5,
			6, 1, 2, 7,
			8, 9,10,11,
			1,12,13, 2,
			1,14,15, 2,
			1,16,17, 2
		};

		static const float texcoord[] =
		{
			0.0f,	0.0f,
			0.5f,	0.0f,
			0.5f,	1.0f,
			0.0f,	1.0f,
			0.0f,	0.0f,
			0.0f,	1.0f,
			0.0f,	0.0f,
			0.0f,	1.0f,
			0.0f,	0.0f,
			1.0f,	0.0f,
			1.0f,	1.0f,
			0.0f,	1.0f,
			1.0f,	0.0f,
			1.0f,	1.0f,
			1.0f,	0.0f,
			1.0f,	1.0f,
			1.0f,	0.0f,
			1.0f,	1.0f
		};
		const float points[] =
		{
			0.0f,		1.0f,		-1.0f,
			0.0f,		1.0f,		 0.0f,
			0.0f,		0.0f,		 0.0f,
			0.0f,		0.0f,		-1.0f,
			-sq2,		1.0f,		 -sq2,
			-sq2,		0.0f,		 -sq2,
			sq2,		1.0f,		 -sq2,
			sq2,		0.0f,		 -sq2,
			-1.0f,		1.0f,		 0.0f,
			1.0f,		1.0f,		 0.0f,
			1.0f,		0.0f,		 0.0f,
			-1.0f,		0.0f,		 0.0f,
			-sq2,		1.0f,		  sq2,
			-sq2,		0.0f,		  sq2,
			sq2,		1.0f,		  sq2,
			sq2,		0.0f,		  sq2,
			0.0f,		1.0f,		 1.0f,
			0.0f,		0.0f,		 1.0f
		};
		bool set = true;

		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoord);
		glVertexPointer( 3, GL_FLOAT, 0, points);

		float t = (float)units.current_tick / TICKS_PER_SEC;

		glPolygonOffset(-1.0f,-1.0f);

		DrawingTable DrawingTable;
		QUAD_TABLE    quad_table;

		pMutex.lock();
		for (int e = 0; e < list_size; ++e)
		{
			if (!(e & 15))
			{
				pMutex.unlock();
				pMutex.lock();
				if (e >= list_size)         // We need this because of the unlock/lock calls above
					break;
			}
			int i = list[e];
			if (feature[i].type < 0 || !feature[i].draw)
				continue;

			Feature *pFeature = feature_manager.getFeaturePointer(feature[i].type);
			if (Camera::inGame->mirror && ((pFeature->height > 5.0f && pFeature->m3d)			// Perform a small visibility check
				|| (pFeature->m3d && pFeature->model!=NULL)) )
			{
				Vector3D Pos(feature[i].Pos);
				if( pFeature->m3d )
					Pos.y += pFeature->model->size2;
				else
					Pos.y += pFeature->height * 0.5f;

				const float a = Camera::inGame->rpos.y - units.map->sealvl;
				const float b = Pos.y - units.map->sealvl;
				const float c = a + b;
				if (c == 0.0f)
					continue;
				Pos = (a / c) * Pos + (b / c) * Camera::inGame->rpos;
				Pos.y = units.map->get_unit_h( Pos.x, Pos.z );

				if (Pos.y > units.map->sealvl)	// If it's not visible don't draw it
					continue;
			}

			if (pFeature->not_loaded)
				pFeature->convert();		// Load data and convert texture

			if (!pFeature->m3d
				&& pFeature->anim.nb_bmp > 0)
			{
				pFeature->convert();		// Convert texture data if needed

				feature[i].frame = (units.current_tick >> 1) % pFeature->anim.nb_bmp;

				if (!texture_loaded || old != pFeature->anim.glbmp[feature[i].frame])
				{
					old = pFeature->anim.glbmp[feature[i].frame];
					texture_loaded=true;
					glBindTexture(GL_TEXTURE_2D, pFeature->anim.glbmp[feature[i].frame]);
				}

				Vector3D Pos(feature[i].Pos);
				float h  = pFeature->height * 0.5f;
				float dw = 0.5f * pFeature->anim.w[feature[i].frame];

				if (pFeature->height > 5.0f)
				{
					dw *= h / pFeature->anim.h[feature[i].frame];

					if (feature[i].grey)
						glColor4ub( 127, 127, 127, 255 );
					else
						glColor4ub( 255, 255, 255, 255 );

					if (!set)
					{
						set = true;
						glDisableClientState(GL_NORMAL_ARRAY);
						glDisableClientState(GL_COLOR_ARRAY);
						glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						glTexCoordPointer(2, GL_FLOAT, 0, texcoord);
						glVertexPointer( 3, GL_FLOAT, 0, points);
					}

					glPushMatrix();
					glTranslatef(feature[i].Pos.x,feature[i].Pos.y,feature[i].Pos.z);
					glScalef(dw,h,dw);
					glDrawElements(GL_QUADS, 28,GL_UNSIGNED_BYTE,index);		// dessine le tout
					glPopMatrix();
				}
				else
				{
					if (!Camera::inGame->mirror && !no_flat) 	// no need to draw things we can't see
					{
						dw *= 0.5f;
						h = 0.25f * pFeature->anim.h[feature[i].frame];

						quad_table.queue_quad( pFeature->anim.glbmp[feature[i].frame], QUAD( Pos, dw, h, feature[i].grey ? 0x7F7F7FFF : 0xFFFFFFFF ) );
					}
				}
			}
			else
			{
				if (pFeature->m3d && pFeature->model != NULL)
				{
					if (!pFeature->model->animated && !feature[i].sinking)
					{
						DrawingTable.queue_Instance( pFeature->model->id,
													 Instance(feature[i].Pos, feature[i].grey ? 0x7F7F7FFF : 0xFFFFFFFF,
															  feature[i].angle)  );
					}
					else
					{
						if(feature[i].grey)
							glColor4ub( 127, 127, 127, 255 );
						else
							glColor4ub( 255, 255, 255, 255 );
						glEnable(GL_LIGHTING);
						glDisable(GL_BLEND);
						if(!pFeature->converted)				// To fix opacity with converted models
							glDisable(GL_ALPHA_TEST);
						glPushMatrix();
						glTranslatef(feature[i].Pos.x,feature[i].Pos.y,feature[i].Pos.z);
						glRotatef( feature[i].angle, 0.0f, 1.0f, 0.0f );
						glRotatef( feature[i].angle_x, 1.0f, 0.0f, 0.0f );
						pFeature->model->draw(t,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,!feature[i].grey);

						if (lp_CONFIG->underwater_bright && the_map->water && feature[i].Pos.y < the_map->sealvl)
						{
							glEnable( GL_BLEND );
							glBlendFunc( GL_ONE, GL_ONE );
							glDepthFunc( GL_EQUAL );
							glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
							pFeature->model->draw(t,NULL,false,true,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
							glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
							glDepthFunc( GL_LESS );
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						}


						gfx->ReInitAllTex( true );

						glPopMatrix();
						glEnable(GL_BLEND);
						if(!pFeature->converted)				// To fix opacity with converted models
							glEnable(GL_ALPHA_TEST);
						glDisable(GL_LIGHTING);
						glDisable(GL_CULL_FACE);
						glEnable(GL_TEXTURE_2D);
						texture_loaded=false;
						set=false;
					}
				}
			}
		}
		pMutex.unlock();

		glColor4ub( 255, 255, 255, 255 );

		gfx->ReInitAllTex( true );
		gfx->enable_model_shading();
		if (HWLight::inGame)
			glNormal3fv( (GLfloat*)&(HWLight::inGame->Dir) );

		glEnable(GL_POLYGON_OFFSET_FILL);
		quad_table.draw_all();
		glDisable(GL_POLYGON_OFFSET_FILL);

		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);

		if (gfx->getShadowMapMode())
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(3.0f, 1.0f);
		}

		DrawingTable.draw_all();

		if (gfx->getShadowMapMode())
			glDisable(GL_POLYGON_OFFSET_FILL);

		gfx->disable_model_shading();

		glDisable(GL_ALPHA_TEST);
		glDepthFunc( GL_LESS );
		glEnable(GL_TEXTURE_2D);
	}



	void Features::draw_shadow(const Vector3D& Dir)
	{
		if (nb_features <= 0)
			return;
		float t = (float)units.current_tick / TICKS_PER_SEC;
		pMutex.lock();
		for (int e = 0; e < list_size; ++e)
		{
			pMutex.unlock();
			pMutex.lock();
			if (e >= list_size)         // We need this because of the unlock/lock calls above
				break;
			int i = list[e];
			if(feature[i].type < 0)
				continue;
			Feature *pFeature = feature_manager.getFeaturePointer(feature[i].type);

			if (!(!pFeature->m3d && pFeature->anim.nb_bmp > 0))
			{
				if (pFeature->m3d && pFeature->model != NULL)
				{
					if (!feature[i].draw || feature[i].grey || pFeature->converted)	// Quelques problèmes (graphiques et plantages) avec les modèles convertis
						continue;

					if (feature[i].delete_shadow_dlist && feature[i].shadow_dlist != 0 )
					{
						glDeleteLists( feature[i].shadow_dlist, 1);
						feature[i].shadow_dlist = 0;
						feature[i].delete_shadow_dlist = false;
					}

					if (pFeature->model->animated || feature[i].sinking || feature[i].shadow_dlist == 0)
					{
						bool create_display_list = false;
						if (!pFeature->model->animated && !feature[i].sinking && feature[i].shadow_dlist == 0)
						{
							feature[i].shadow_dlist = glGenLists (1);
							glNewList( feature[i].shadow_dlist, GL_COMPILE_AND_EXECUTE);
							create_display_list = true;
							feature[i].delete_shadow_dlist = false;
						}

						glPushMatrix();
						glTranslatef(feature[i].Pos.x,feature[i].Pos.y,feature[i].Pos.z);
						glRotatef( feature[i].angle, 0.0f, 1.0f, 0.0f );
						glRotatef( feature[i].angle_x, 1.0f, 0.0f, 0.0f );
						Vector3D R_Dir = (sqrtf(pFeature->model->size) * 2.0f + feature[i].Pos.y) * Dir * RotateY( -feature[i].angle * DEG2RAD ) * RotateX( -feature[i].angle_x * DEG2RAD );
						if(g_useStencilTwoSide)													// Si l'extension GL_EXT_stencil_two_side est disponible
							pFeature->model->draw_shadow( R_Dir,t,NULL);
						else
							pFeature->model->draw_shadow_basic( R_Dir,t,NULL);
						glPopMatrix();

						if (create_display_list)
							glEndList();
					}
					else
						glCallList(feature[i].shadow_dlist);
				}
			}
		}
		pMutex.unlock();
	}



	void Features::move(const float dt, MAP* map, bool clean)
	{
		if (nb_features <= 0)
			return;

		pMutex.lock();

		for (int e = 0; e < list_size; ++e)
		{
			int i = list[e];
			if (feature[i].type < 0)
				continue;
			if (feature[i].hp <= 0.0f && !feature[i].burning)
			{
				delete_feature(i);
				continue;
			}
			Feature *pFeature = feature_manager.getFeaturePointer(feature[i].type);
			if (!pFeature->vent && !feature[i].burning )
			{
				feature[i].draw = false;
				continue;
			}
			feature[i].dt += dt;
			if (feature[i].dt > 0.2f && feature[i].draw)
			{
				if (feature[i].burning && feature[i].dt > 0.3f)
				{
					Vector3D t_mod;
					bool random_vector = true;
					if (pFeature->m3d && pFeature->model != NULL)
					{
						OBJECT* obj = &(pFeature->model->obj);
						for (int base_n = Math::RandFromTable(), n = 0 ; random_vector && n < obj->nb_sub_obj ; ++n)
							random_vector = obj->random_pos(NULL, (base_n + n) % obj->nb_sub_obj, &t_mod);
					}
					else
						random_vector = false;

					if (random_vector)
						t_mod = feature[i].Pos + t_mod;
					else
						t_mod = feature[i].Pos;

					feature[i].dt = 0.0f;
					particle_engine.make_fire(t_mod, 1, 1, 30.0f);
				}
				else
				{
					if (!feature[i].burning)
					{
						feature[i].dt = 0.0f;
						particle_engine.make_smoke(feature[i].Pos, 0, 1, 5.0f, -1.0f, 0.0f, 0.25f);
					}
				}
			}
			if (clean)
				feature[i].draw = false;
		}
		pMutex.unlock();
	}


	void Features::compute_on_map_pos(const int idx)
	{
		feature[idx].px = ((int)(feature[idx].Pos.x) + the_map->map_w_d + 4) >> 3;
		feature[idx].py = ((int)(feature[idx].Pos.z) + the_map->map_h_d + 4) >> 3;
	}

	void Features::burn_feature(const int idx)
	{
		pMutex.lock();

		if (idx >= 0 && idx < max_features)
		{
			Feature *pFeature = feature_manager.getFeaturePointer(feature[idx].type);
			if (pFeature && pFeature->flamable && !feature[idx].burning )// We get something to burn !!
			{
				feature[idx].burning = true;
				feature[idx].burning_time = 0.0f;
				int time_zone = abs( pFeature->burnmax - pFeature->burnmin ) + 1;
				feature[ idx ].time_to_burn = (Math::RandFromTable() % time_zone ) + pFeature->burnmin;		// How long it burns
				burning_features.push_back(idx);		// It's burning 8)

				// Start doing damages to things around
				if (!pFeature->burnweapon.empty())
				{
					int w_idx = weapon_manager.get_weapon_index(pFeature->burnweapon);
					feature[ idx ].BW_idx = w_idx;
				}
				else
					feature[idx].BW_idx = -1;
				feature[idx].weapon_counter = 0;
			}
		}
		pMutex.unlock();
	}

	void Features::sink_feature(const int idx)
	{
		pMutex.lock();
		// We get something to sink
		if( idx >= 0 && idx < max_features && feature[idx].type >= 0 && !feature[idx].sinking)
		{
			feature[ idx ].sinking = true;
			sinking_features.push_back(idx);
		}
		pMutex.unlock();
	}

	void Features::move_forest(const float dt)			// Simulates forest fires & tree reproduction
	{
		pMutex.lock();

		const Vector3D wind = 0.1f * *p_wind_dir;

		int wind_x = (int)(2.0f * wind.x + 0.5f);
		int wind_z = (int)(2.0f * wind.z + 0.5f);

		byte CS_count = 0;
		bool erased = false;

		// Makes fire spread 8)
		for (FeaturesList::iterator i = burning_features.begin() ; i != burning_features.end() ; )
		{
			++CS_count;
			if (!CS_count)
			{
				pMutex.unlock();
				pMutex.lock();
			}
			feature[*i].burning_time += dt;
			Feature *pFeature = feature_manager.getFeaturePointer(features.feature[*i].type);
			if (feature[*i].burning_time >= feature[*i].time_to_burn) // If we aren't burning anymore :(
			{
				if (network_manager.isServer())
					g_ta3d_network->sendFeatureDeathEvent(*i);

				feature[*i].burning = false;
				feature[*i].hp = 0.0f;

				int sx = ((int)(feature[*i].Pos.x) + the_map->map_w_d - 4) >> 3; // Delete the feature
				int sy = ((int)(feature[*i].Pos.z) + the_map->map_h_d - 4) >> 3;
				// Remove it from map
				the_map->rect(sx - (pFeature->footprintx >> 1),
							  sy - (pFeature->footprintz >> 1),
							  pFeature->footprintx,
							  pFeature->footprintz, -1);

				// Replace the feature if needed (with the burnt feature)
				if (!pFeature->feature_burnt.empty())
				{
					int burnt_type = feature_manager.get_feature_index( pFeature->feature_burnt);
					if (burnt_type >= 0)
					{
						Feature *featBurn = feature_manager.getFeaturePointer(burnt_type);
						the_map->map_data[sy][sx].stuff = features.add_feature(feature[*i].Pos, burnt_type);
						if( featBurn && featBurn->blocking)
						{
							the_map->rect(sx - (featBurn->footprintx >> 1),
										  sy - (featBurn->footprintz >> 1),
										  featBurn->footprintx,
										  featBurn->footprintz,
										  -2 - the_map->map_data[sy][sx].stuff);
						}
						if (network_manager.isServer())
							g_ta3d_network->sendFeatureDeathEvent(the_map->map_data[sy][sx].stuff);
					}
				}

				delete_feature(*i);
				i = burning_features.erase(i);
				erased = true;
			}
			else
			{
				erased = false;	// Still there

				if (feature[*i].BW_idx >= 0 && !feature[*i].weapon_counter) // Don't stop damaging things before the end!!
				{
					pMutex.unlock();
					int w_idx = weapons.add_weapon( feature[ *i ].BW_idx, -1);
					pMutex.lock();
					if (w_idx >= 0)
					{
						weapons.weapon[w_idx].just_explode = true;
						weapons.weapon[w_idx].Pos = feature[*i].Pos;
						weapons.weapon[w_idx].owner = 0xFF;
						weapons.weapon[w_idx].local = true;
					}
				}

				feature[*i].weapon_counter = ( feature[*i].weapon_counter + TICKS_PER_SEC - 1 ) % TICKS_PER_SEC;

				if (!network_manager.isConnected() || network_manager.isServer())
				{
					feature[*i].last_spread += dt;
					if (feature[*i].burning_time >= pFeature->sparktime && feature[*i].last_spread >= 0.1f) // Can spread
					{
						feature[*i].last_spread = 0.0f;
						int spread_score = Math::RandFromTable() % 100;
						if (spread_score < pFeature->spreadchance)// It tries to spread :)
						{
							int rnd_x = feature[*i].px + (Math::RandFromTable() % 12) - 6 + wind_x;	// Random pos in neighborhood, but affected by wind :)
							int rnd_y = feature[*i].py + (Math::RandFromTable() % 12) - 6 + wind_z;

							if (rnd_x >= 0 && rnd_y >= 0 && rnd_x < the_map->bloc_w_db && rnd_y < the_map->bloc_h_db ) 	// Check coordinates are valid
							{
								burn_feature(units.map->map_data[rnd_y][rnd_x].stuff); // Burn it if there is something to burn 8)
								if (network_manager.isServer())
									g_ta3d_network->sendFeatureFireEvent(units.map->map_data[rnd_y][rnd_x].stuff);
							}
						}
					}
				}
			}

			if (!erased)// We don't want to skip an element :)
				++i;
		}

		for (FeaturesList::iterator i = sinking_features.begin() ; i != sinking_features.end() ; ) // A boat is sinking
		{
			if (feature[*i].sinking)
			{
				Feature *pFeature = feature_manager.getFeaturePointer(feature[*i].type);
				if (feature[*i].angle_x > -45.0f && !feature[*i].dive)
				{
					feature[*i].angle_x -= dt * 15.0f;
					feature[*i].dive_speed = 0.0f;
				}
				else
					feature[*i].dive = true;
				float sea_ground = the_map->get_unit_h( feature[*i].Pos.x, feature[*i].Pos.z );
				if (sea_ground < feature[*i].Pos.y )
				{
					if (sinf(-feature[*i].angle_x * DEG2RAD) * pFeature->footprintx * 8.0f > feature[*i].Pos.y - sea_ground)
					{
						feature[*i].angle_x = RAD2DEG * asinf( ( sea_ground - feature[*i].Pos.y ) / (pFeature->footprintx * 8.0f) );
						feature[*i].dive = true;
					}
					feature[*i].dive_speed = (feature[*i].dive_speed + 3.0f * dt) * expf(-dt);
					feature[*i].Pos.y -= feature[*i].dive_speed * dt;
				}
				else
				{
					feature[*i].sinking = false;
					feature[*i].dive_speed = 0.0f;
					feature[*i].angle_x = 0.0f;
				}
				++i;
			}
			else
				sinking_features.erase(i++);
		}
		pMutex.unlock();
	}


	void Features::display_info(const int idx) const
	{
		if (idx < 0 || idx >= max_features || feature[idx].type < 0)
			return; // Nothing to display

		Feature *pFeature = feature_manager.getFeaturePointer(feature[idx].type);

		if (!pFeature->description.empty())
		{
			if (pFeature->reclaimable)
				gfx->print(gfx->normal_font, ta3dSideData.side_int_data[ players.side_view ].Description.x1, ta3dSideData.side_int_data[ players.side_view ].Description.y1, 0.0f, 0xFFFFFFFF, format("%s M:%d E:%d",I18N::Translate( pFeature->description ).c_str(), pFeature->metal, pFeature->energy) );
			else
				gfx->print(gfx->normal_font, ta3dSideData.side_int_data[ players.side_view ].Description.x1, ta3dSideData.side_int_data[ players.side_view ].Description.y1, 0.0f, 0xFFFFFFFF, I18N::Translate( pFeature->description ) );
		}
		glDisable(GL_BLEND);
	}

	void Features::delete_feature(const int index)
	{
		MutexLocker locker(pMutex);
		if (nb_features <= 0 || feature[index].type <= 0)
			return;

		if (feature[index].shadow_dlist != 0)
			feature[index].delete_shadow_dlist = true;

		if (feature[index].burning)		// Remove it form the burning features list
			burning_features.remove(index);

		--nb_features;
		feature[index].type = -1;		// On efface l'objet
	}



	void Features::resetListOfItemsToDisplay()
	{
		if (list)
			delete[] list;
		list = new int[max_features];
		list_size = 0;
	}

	int Features::add_feature(const Vector3D& Pos, const int type)
	{
		if (type < 0 || type >= feature_manager.getNbFeatures())
			return -1;
		MutexLocker locker(pMutex);

		++nb_features;
		int idx = -1;
		if (nb_features > max_features) // Si besoin alloue plus de mémoire
		{
			if (max_features == 0)  max_features = 250;
			max_features *= 2;				// Double memory pool size
			FeatureData* n_feature = new FeatureData[max_features];
			if (feature && nb_features > 0)
			{
				for(int i = 0; i < nb_features - 1; ++i)
					n_feature[i] = feature[i];
			}
			for (int i = nb_features - 1; i < max_features; ++i)
			{
				n_feature[i].type = -1;
				n_feature[i].shadow_dlist = 0;
				n_feature[i].delete_shadow_dlist = false;
			}
			if (feature)
				delete[] feature;
			feature = n_feature;
			resetListOfItemsToDisplay();
			idx = nb_features - 1;
		}
		else
		{
			for (int i = 0; i < max_features; ++i)
			{
				if (feature[i].type < 0)
				{
					idx = i;
					break;
				}
			}
		}
		feature[idx].Pos = Pos;
		feature[idx].type = type;
		feature[idx].frame = 0;
		feature[idx].draw = false;
		feature[idx].hp = feature_manager.getFeaturePointer(type)->damage;
		feature[idx].grey = false;
		feature[idx].dt = 0.0f;
		feature[idx].angle = 0.0f;
		feature[idx].burning = false;
		feature[idx].last_spread = 0.0f;

		feature[idx].sinking = false;
		feature[idx].dive = false;
		feature[idx].dive_speed = 0.0f;
		feature[idx].angle_x = 0.0f;
		feature[idx].shadow_dlist = 0;
		compute_on_map_pos( idx );
		return idx;
	}


	void Features::drawFeatureOnMap(const int idx)
	{
		if (idx < 0 || idx >= max_features)    return;
		compute_on_map_pos(idx);
		Feature *pFeature = feature_manager.getFeaturePointer(feature[idx].type);
		if (pFeature && pFeature->blocking)        // Check if it is a blocking feature
		{
			int X = pFeature->footprintx;
			int Z = pFeature->footprintz;
			the_map->rect( feature[idx].px - (X>>1), feature[idx].py - (Z>>1), X, Z, -2 - idx);
		}
	}

	void Features::removeFeatureFromMap(const int idx)
	{
		if (idx < 0 || idx >= max_features)    return;
		Feature *pFeature = feature_manager.getFeaturePointer(feature[idx].type);
		if (pFeature && pFeature->blocking)        // Check if it is a blocking feature
		{
			int X = pFeature->footprintx;
			int Z = pFeature->footprintz;
			the_map->rect(feature[idx].px - (X >> 1), feature[idx].py - (Z >> 1), X, Z, -1);
			the_map->map_data[feature[idx].py] [feature[idx].px].stuff = -1;
		}
	}



} // namespace TA3D

