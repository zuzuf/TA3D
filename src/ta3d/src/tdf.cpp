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
#include "mesh/instancing.h"

namespace TA3D
{


    FeatureManager  feature_manager;
    Features::Ptr   features;

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
		not_loaded = true;
		need_convert = true;
		flamable = false;
		burnmin = 0;
		burnmax = 0;
		sparktime = 0;
		spreadchance = 0;

		geothermal = false;
		blocking = false;
		reclaimable = false;
		autoreclaimable = true;
		energy = 0;
		converted = false;
		m3d = false;
		model = NULL;
		vent = false;
		animating = false;
		footprintx = 0;
		footprintz = 0;
		height = 0;
		animtrans = false;
		shadtrans = false;
		hitdensity = 0;
		metal = 0;
		damage = 100;
		indestructible = false;

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
						// Try a GAF-like directory
            anim.loadGAFFromDirectory("anims/" + filename, seqname);
			if (anim.nb_bmp == 0)
			{
                const QString &tmp = "anims/" + filename + ".gaf";
                QIODevice* gaf = VFS::Instance()->readFile(tmp);
				if (gaf)
				{
					sint32 index = Gaf::RawDataGetEntryIndex(gaf, seqname);
					if (index >= 0)
						anim.loadGAFFromRawData(gaf, Gaf::RawDataGetEntryIndex(gaf, seqname), true, filename);
					else
						LOG_WARNING(LOG_PREFIX_TDF << "`" << name << "` has no picture to display (" << filename << ".gaf, " << seqname << ") !");
					delete gaf;
					need_convert = true;
				}
			}
			else
				need_convert = true;
		}
		if (need_convert)
		{
			need_convert = false;
			anim.convert(false,true);
			anim.clean();
		}
	}




	FeatureManager::FeatureManager()
	{
		init();
	}


	FeatureManager::~FeatureManager()
	{
		destroy();
		feature_hashtable.clear();
	}


	void FeatureManager::init()
	{
		nb_features = 0;
		feature.clear();
	}


	int FeatureManager::get_feature_index(const QString &name)
	{
        if (name.isEmpty())
			return -1;
        return feature_hashtable[name.toLower()] - 1;
	}


	int FeatureManager::add_feature(const QString& name)			// Ajoute un élément
	{
		MutexLocker mLock(mInternals);
		++nb_features;
		feature.push_back(new Feature);
		feature.back()->name = name;
        feature_hashtable[name.toLower()] = nb_features;
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

		feature_hashtable.clear();
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



    void FeatureManager::load_tdf(QIODevice *file)					// Charge un fichier tdf
	{
		TDFParser parser;
        const QByteArray &buffer = file->readAll();
        parser.loadFromMemory("TDF",buffer.data(),buffer.size(),false,true,true);
		file->close();

		std::vector<Feature*> vfeats;

        for (int g = 0 ; parser.exists(QString("gadget%1").arg(g)) ; ++g)
		{
            const QString &key = QString("gadget%1").arg(g) + '.';

			mInternals.lock();
            const int index = add_feature( parser.pullAsString(QString("gadget%1").arg(g)) );
			Feature *pFeature = feature[index];
			mInternals.unlock();

			vfeats.push_back(pFeature);
			pFeature->m3d = false;
			pFeature->world = parser.pullAsString( key + "world", pFeature->world );
			pFeature->description = parser.pullAsString( key + "description", pFeature->description );
			pFeature->category = parser.pullAsString( key + "category", pFeature->category );
			pFeature->filename = parser.pullAsString( key + "object" );
            pFeature->m3d = !pFeature->filename.isEmpty();
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
			pFeature->reclaimable = parser.pullAsBool( key + "reclaimable", pFeature->reclaimable );
			pFeature->autoreclaimable = parser.pullAsBool( key + "autoreclaimable", pFeature->autoreclaimable ) && pFeature->reclaimable;
			pFeature->blocking = parser.pullAsBool( key + "blocking", pFeature->blocking );
			pFeature->flamable = parser.pullAsBool( key + "flamable", pFeature->flamable );
			pFeature->geothermal = parser.pullAsBool( key + "geothermal", pFeature->geothermal );
			pFeature->feature_dead = parser.pullAsString( key + "featuredead" );
			pFeature->burnmin = short(parser.pullAsInt( key + "burnmin", pFeature->burnmin ));
			pFeature->burnmax = short(parser.pullAsInt( key + "burnmax", pFeature->burnmax ));
			pFeature->sparktime = short(parser.pullAsInt( key + "sparktime", pFeature->sparktime ));
			pFeature->spreadchance = byte(parser.pullAsInt( key + "spreadchance", pFeature->spreadchance ));
			pFeature->burnweapon = parser.pullAsString( key + "burnweapon" );
			pFeature->feature_burnt = parser.pullAsString( key + "featureburnt" );
			pFeature->feature_reclamate = parser.pullAsString( key + "featurereclamate" );

			// Build the repulsion grid
			pFeature->gRepulsion.resize(pFeature->footprintx * 5, pFeature->footprintz * 5);
			const float sigx = (float)pFeature->footprintx;
			const float sigz = (float)pFeature->footprintz;
			const float sigx2 = -0.5f / (sigx * sigx);
			const float sigz2 = -0.5f / (sigz * sigz);
			for(int z = 0 ; z < pFeature->gRepulsion.getHeight() ; ++z)
			{
				float dz = (float)z - (float)pFeature->gRepulsion.getHeight() * 0.5f;
				dz = (float)Math::Sgn(dz) * Math::Max(fabsf(dz) - (float)pFeature->footprintz * 0.5f, 0.0f);
				dz *= dz;
				for(int x = 0 ; x < pFeature->gRepulsion.getWidth() ; ++x)
				{
					float dx = (float)x - (float)pFeature->gRepulsion.getWidth() * 0.5f;
					dx = (float)Math::Sgn(dx) * Math::Max(fabsf(dx) - (float)pFeature->footprintx * 0.5f, 0.0f);
					dx *= dx;
					pFeature->gRepulsion(x,z) = 2550.0f * std::exp(sigx2 * dx + sigz2 * dz);
				}
			}
		}

		for (std::vector<Feature*>::iterator i = vfeats.begin() ; i != vfeats.end() ; ++i)// Charge les fichiers d'animation
		{
			Feature *pFeature = *i;
            if (!pFeature->category.isEmpty())
                pFeature->vent = pFeature->category.contains("vents");
            if (!pFeature->filename.isEmpty() && !pFeature->seqname.isEmpty() && !pFeature->m3d)
			{
                if (model_manager.get_model(pFeature->filename + '-' + pFeature->seqname) != NULL) // Check if there is a 3do version of it
				{
					pFeature->model = NULL;
					pFeature->m3d = true;
					pFeature->converted = false;
					pFeature->not_loaded = false;
				}
				else
				{
					pFeature->not_loaded = true;
					if (pFeature->height <= 10.0f && pFeature->height > 1.0f && pFeature->blocking
                        && pFeature->description.toLower() != "metal") // Tente une conversion en 3d
					{
                        const QString &tmp = "anims/" + pFeature->filename + ".gaf";
                        QIODevice* gaf = VFS::Instance()->readFile(tmp);
						if (gaf)
						{
							sint32 index = Gaf::RawDataGetEntryIndex(gaf, pFeature->seqname);
							if (index >= 0)
								pFeature->anim.loadGAFFromRawData(gaf, Gaf::RawDataGetEntryIndex(gaf, pFeature->seqname), true, pFeature->filename);
							else
								LOG_WARNING(LOG_PREFIX_TDF << "`" << pFeature->name << "` has no picture to display (" << pFeature->filename << ".gaf, " << pFeature->seqname << ") !");
							pFeature->not_loaded = false;
							delete gaf;
						}
					}
				}
			}
			else
			{
                if (!pFeature->filename.isEmpty() && pFeature->m3d)
					pFeature->model = NULL;
			}
		}
	}



	void load_features(ProgressNotifier *progress) // Charge tout les éléments
	{
		QStringList files;
        VFS::Instance()->getFilelist("features/*.tdf", files);
        std::atomic<int> n(0), m(0);

        QThread * const root_thread = QThread::currentThread();
        parallel_for<size_t>(0, files.size(), [&](const size_t i)
		{
            const QString &curFile = files[i];
            if (QThread::currentThread() == root_thread)
                if (progress != NULL && m >= 0xF)
                {
                    (*progress)((200.0f + float(n) * 50.0f / float(files.size() + 1)) / 7.0f, I18N::Translate("Loading graphical features"));
                    m -= 0xF;
                }
            ++m;
            ++n;

            QIODevice* file = VFS::Instance()->readFile(curFile);
            if (file)
            {
                LOG_DEBUG(LOG_PREFIX_TDF << "Loading feature: `" << curFile << "`...");
                feature_manager.load_tdf(file);
                delete file;
            }
            else
                LOG_WARNING(LOG_PREFIX_TDF << "Loading `" << curFile << "` failed");
        });

		// Foreach item...
        parallel_for<int>(0, feature_manager.getNbFeatures(), [&](const int i)
		{
			Feature *feature = feature_manager.getFeaturePointer(i);
			if (feature->m3d && feature->model == NULL
                && !feature->filename.isEmpty() && !feature->seqname.isEmpty())
			{
                const QString &tmp = feature->filename + "-" + feature->seqname;
				feature->model = model_manager.get_model(tmp);
				if (feature->model == NULL)
                    feature->model = model_manager.get_model("objects3d/" + tmp);
			}
			else
			{
                if (feature->m3d && feature->model == NULL && !feature->filename.isEmpty())
					feature->model = model_manager.get_model(feature->filename);
			}
        });
	}



	Features::Features()
		:nb_features(0), max_features(0), feature(NULL),
		burning_features(), sinking_features()
	{}


	Features::~Features()
	{
		destroy(false);
	}


	void Features::init()
	{
		p_wind_dir   = NULL;
		nb_features  = 0;
		max_features = 0;
		feature = NULL;
		list.clear();
        symbolic_features.clear();
        icons[0] = gfx->load_texture("gfx/tactical_icons/metal_deposit.tga");
        icons[1] = gfx->load_texture("gfx/tactical_icons/geothermal.tga");
	}


	void Features::destroy(bool bInit)
	{
		if (feature)
			DELETE_ARRAY(feature);
        icons[0] = nullptr;
        icons[1] = nullptr;
        list.clear();
		if (bInit)
			init();
        burning_features.clear();
        sinking_features.clear();
        symbolic_features.clear();
	}


    void Features::draw(float t, bool no_flat)
	{
		if(nb_features <= 0)
			return;
		gfx->ReInitAllTex(true);
		glAlphaFunc(GL_GREATER, 0.1f);
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
		bool texture_loaded = false;

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

		DrawingTable DrawingTable;
		QUAD_TABLE    quad_table;

        float ticks2sec = 1.0f / TICKS_PER_SEC;

		pMutex.lock();
		for (uint32 e = 0U ; e < list.size() ; ++e)
		{
			if (!(e & 15))
			{
				pMutex.unlock();
				pMutex.lock();
				if (e >= list.size())         // We need this because of the unlock/lock calls above
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
				if (pFeature->m3d)
					Pos.y += pFeature->model->size2;
				else
					Pos.y += float(pFeature->height) * 0.5f;

                const float a = Camera::inGame->rpos.y - units->map->sealvl;
                const float b = Pos.y - units->map->sealvl;
				const float c = a + b;
				if (Math::Zero(c))
					continue;
				Pos = (a / c) * Pos + (b / c) * Camera::inGame->rpos;
                Pos.y = units->map->get_unit_h( Pos.x, Pos.z );

                if (Pos.y > units->map->sealvl)	// If it's not visible don't draw it
					continue;
			}

			if (pFeature->not_loaded)
				pFeature->convert();		// Load data and convert texture

			if (!pFeature->m3d
				&& pFeature->anim.nb_bmp > 0)
			{
				pFeature->convert();		// Convert texture data if needed

                feature[i].frame = short(((units->current_tick + feature[i].timeRef) >> 1) % pFeature->anim.nb_bmp);

				if (!texture_loaded || old != pFeature->anim.glbmp[feature[i].frame])
				{
					old = pFeature->anim.glbmp[feature[i].frame];
					texture_loaded=true;
                    pFeature->anim.glbmp[feature[i].frame]->bind();
				}

				Vector3D Pos(feature[i].Pos);
				float h  = float(pFeature->height) * 0.5f;
				float dw = 0.5f * float(pFeature->anim.w[feature[i].frame]);

				if (pFeature->height > 5.0f)
				{
					dw *= h / float(pFeature->anim.h[feature[i].frame]);

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
					if (lp_CONFIG->shadow_quality >= 2)
						glFogi (GL_FOG_COORD_SRC, GL_FOG_COORD);
					glDrawRangeElements(GL_QUADS, 0, 17, 28,GL_UNSIGNED_BYTE,index);		// draw it
					glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
					glPopMatrix();
				}
				else
				{
						// no need to draw things we can't see
					if (!Camera::inGame->mirror && !no_flat && !gfx->getShadowMapMode())
					{
						dw *= 0.5f;
						h = 0.25f * float(pFeature->anim.h[feature[i].frame]);
						Pos.x += float(pFeature->anim.ofs_x[feature[i].frame]) * 0.5f - dw;
						Pos.z += float(pFeature->anim.ofs_y[feature[i].frame]) * 0.5f - h;

                        quad_table.queue_quad( pFeature->anim.glbmp[feature[i].frame], QUAD( Pos, dw, h, feature[i].grey ? 0xFF7F7F7F : 0xFFFFFFFF ) );
					}
				}
			}
			else
			{
				if (pFeature->m3d && pFeature->model != NULL)
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
                    if (lp_CONFIG->underwater_bright && the_map->water && feature[i].Pos.y < the_map->sealvl)
                    {
                        double eqn[4]= { 0.0f, -1.0f, 0.0f, the_map->sealvl - feature[i].Pos.y };
                        glClipPlane(GL_CLIP_PLANE2, eqn);
                    }
                    glRotatef( feature[i].angle, 0.0f, 1.0f, 0.0f );
                    glRotatef( feature[i].angle_x, 1.0f, 0.0f, 0.0f );
                    float lt = t + float(feature[i].timeRef) * ticks2sec;
                    pFeature->model->draw(lt, NULL, false, false, false, 0, NULL, NULL, NULL, 0.0f, NULL, false, 0, !feature[i].grey);

                    if (lp_CONFIG->underwater_bright && the_map->water && feature[i].Pos.y < the_map->sealvl)
                    {
                        glEnable(GL_CLIP_PLANE2);
                        glEnable( GL_BLEND );
                        glBlendFunc( GL_ONE, GL_ONE );
                        glDepthFunc( GL_EQUAL );
                        glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
                        pFeature->model->draw(lt, NULL, false, true, false, 0, NULL, NULL, NULL, 0.0f, NULL, false, 0, false);
                        glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
                        glDepthFunc( GL_LESS );
                        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        glDisable(GL_CLIP_PLANE2);
                    }


                    gfx->ReInitAllTex( true );

                    glPopMatrix();
                    glEnable(GL_BLEND);
                    if(!pFeature->converted)				// To fix opacity with converted models
                        glEnable(GL_ALPHA_TEST);
                    glDisable(GL_LIGHTING);
                    glDisable(GL_CULL_FACE);
                    glEnable(GL_TEXTURE_2D);
                    texture_loaded = false;
                    set = false;
				}
			}
		}
		pMutex.unlock();

		glColor4ub( 255, 255, 255, 255 );

		gfx->ReInitAllTex( true );
		gfx->enable_model_shading();

		if (!gfx->getShadowMapMode())
		{
			glDisableClientState(GL_NORMAL_ARRAY);
			if (HWLight::inGame)
				glNormal3fv( (GLfloat*)&(HWLight::inGame->Dir) );
			glPolygonOffset(-1.0f,-1.0f);
			glEnable(GL_POLYGON_OFFSET_FILL);
			quad_table.draw_all();
			glDisable(GL_POLYGON_OFFSET_FILL);
		}

		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);

		if (gfx->getShadowMapMode())
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(3.0f, 1.0f);
		}

		DrawingTable.draw_all();

		if (gfx->getShadowMapMode())
			glDisable(GL_POLYGON_OFFSET_FILL);

		gfx->disable_model_shading();

		glEnable(GL_CULL_FACE);
		glDisable(GL_ALPHA_TEST);
		glDepthFunc( GL_LESS );
		glEnable(GL_TEXTURE_2D);
	}

	void Features::move(const float dt, bool clean)
	{
		if (nb_features <= 0)
			return;

		pMutex.lock();

		for (std::vector<int>::const_iterator e = list.begin() ; e != list.end() ; ++e)
		{
			const int i = *e;
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
						Mesh::Ptr obj = pFeature->model->mesh;
						for (int base_n = Math::RandomTable(), n = 0 ; random_vector && n < (int)obj->nb_sub_obj ; ++n)
							random_vector = obj->random_pos(NULL, (base_n + n) % (int)obj->nb_sub_obj, &t_mod);
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
				feature[ idx ].time_to_burn = short((Math::RandomTable() % time_zone ) + pFeature->burnmin);		// How long it burns
                burning_features.push_back(idx);		// It's burning 8)

				// Start doing damages to things around
                if (!pFeature->burnweapon.isEmpty())
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
        for (uint32 i = 0U ; i < burning_features.size() ; )
		{
			++CS_count;
			if (!CS_count)
			{
				pMutex.unlock();
				pMutex.lock();
                if (i >= burning_features.size())
					break;
			}
			const uint32 e = burning_features[i];
			feature[e].burning_time += dt;
            Feature *pFeature = feature_manager.getFeaturePointer(features->feature[e].type);
			if (feature[e].burning_time >= feature[e].time_to_burn) // If we aren't burning anymore :(
			{
				if (network_manager.isServer())
					g_ta3d_network->sendFeatureDeathEvent(e);

				feature[e].burning = false;
				feature[e].hp = 0.0f;

				const int sx = feature[e].px; // Delete the feature
				const int sy = feature[e].py;
				const float angle = feature[e].angle;
				const float angle_x = feature[e].angle_x;
				// Remove it from map
				const Vector3D Pos = feature[e].Pos;
				delete_feature(e);

				// Replace the feature if needed (with the burnt feature)
                if (!pFeature->feature_burnt.isEmpty())
				{
					const int burnt_type = feature_manager.get_feature_index( pFeature->feature_burnt);
					if (burnt_type >= 0)
					{
                        const int nid = the_map->map_data(sx, sy).stuff = features->add_feature(Pos, burnt_type);
						if (nid >= 0)
						{
							// Preserve orientation
							feature[nid].angle = angle;
							feature[nid].angle_x = angle_x;
							drawFeatureOnMap(the_map->map_data(sx, sy).stuff);
							if (network_manager.isServer())
								g_ta3d_network->sendFeatureCreationEvent(the_map->map_data(sx, sy).stuff);
						}
					}
				}

                if (i + 1 != burning_features.size())
                    burning_features[i] = burning_features.back();
                burning_features.pop_back();
				erased = true;
			}
			else
			{
				erased = false;	// Still there

				if (feature[e].BW_idx >= 0 && !feature[e].weapon_counter) // Don't stop damaging things before the end!!
				{
					pMutex.unlock();
					const int w_idx = weapons.add_weapon( feature[ e ].BW_idx, -1);
					pMutex.lock();
					if (w_idx >= 0)
					{
						weapons.weapon[w_idx].just_explode = true;
						weapons.weapon[w_idx].Pos = feature[e].Pos;
						weapons.weapon[w_idx].owner = 0xFF;
						weapons.weapon[w_idx].local = true;
					}
				}

				feature[e].weapon_counter = byte(( feature[e].weapon_counter + TICKS_PER_SEC - 1 ) % TICKS_PER_SEC);

				if (!network_manager.isConnected() || network_manager.isServer())
				{
					feature[e].last_spread += dt;
					if (feature[e].burning_time >= pFeature->sparktime && feature[e].last_spread >= 0.1f) // Can spread
					{
						feature[e].last_spread = 0.0f;
						const int spread_score = Math::RandomTable() % 100;
						if (spread_score < pFeature->spreadchance)// It tries to spread :)
						{
							const int rnd_x = feature[e].px + (Math::RandomTable() % 12) - 6 + wind_x;	// Random pos in neighborhood, but affected by wind :)
							const int rnd_y = feature[e].py + (Math::RandomTable() % 12) - 6 + wind_z;

							if (rnd_x >= 0 && rnd_y >= 0 && rnd_x < the_map->bloc_w_db && rnd_y < the_map->bloc_h_db ) 	// Check coordinates are valid
							{
                                burn_feature(units->map->map_data(rnd_x, rnd_y).stuff); // Burn it if there is something to burn 8)
								if (network_manager.isServer())
                                    g_ta3d_network->sendFeatureFireEvent(units->map->map_data(rnd_x, rnd_y).stuff);
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
                if (pFeature == NULL)
                {
                    if (i + 1 != sinking_features.end())
					{
                        *i = sinking_features.back();
                        sinking_features.pop_back();
					}
					else
					{
                        sinking_features.pop_back();
						break;
					}
                    continue;
                }
				if (feature[*i].angle_x > -45.0f && !feature[*i].dive)
				{
					feature[*i].angle_x -= dt * 15.0f;
					feature[*i].dive_speed = 0.0f;
				}
				else
					feature[*i].dive = true;
				const float sea_ground = the_map->get_unit_h( feature[*i].Pos.x, feature[*i].Pos.z );
				if (sea_ground < feature[*i].Pos.y )
				{
					if (sinf(-feature[*i].angle_x * DEG2RAD) * float(pFeature->footprintx) * 8.0f > feature[*i].Pos.y - sea_ground)
					{
						feature[*i].angle_x = RAD2DEG * asinf( ( sea_ground - feature[*i].Pos.y ) / (float(pFeature->footprintx) * 8.0f) );
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
			{
                if (i + 1 != sinking_features.end())
				{
                    *i = sinking_features.back();
                    sinking_features.pop_back();
				}
				else
				{
                    sinking_features.pop_back();
					break;
				}
			}
		}
		pMutex.unlock();
	}


	void Features::display_info(const int idx) const
	{
		if (idx < 0 || idx >= max_features || feature[idx].type < 0)
			return; // Nothing to display

		const Feature *pFeature = feature_manager.getFeaturePointer(feature[idx].type);

        if (!pFeature->description.isEmpty())
		{
			const InterfaceData &side_data = ta3dSideData.side_int_data[ players.side_view ];
			if (pFeature->reclaimable)
				gfx->print(gfx->normal_font,
						   (float)side_data.Description.x1,
						   (float)side_data.Description.y1,
                           0.0f, 0xFFFFFFFF, I18N::Translate( pFeature->description ) + QString(" M:%1 E:%2").arg(pFeature->metal).arg(pFeature->energy) );
			else
				gfx->print(gfx->normal_font,
						   (float)side_data.Description.x1,
						   (float)side_data.Description.y1,
						   0.0f, 0xFFFFFFFF, I18N::Translate( pFeature->description ) );
		}
		glDisable(GL_BLEND);
	}

	void Features::delete_feature(const int index)
	{
		MutexLocker locker(pMutex);
		if (nb_features <= 0 || feature[index].type <= 0)
			return;

		removeFeatureFromMap(index);

		if (feature[index].burning)		// Remove it from the burning features list
		{
            std::vector<uint32>::iterator it = std::find(burning_features.begin(), burning_features.end(), index);
            if (it != burning_features.end())
			{
                if (it + 1 != burning_features.end())
                    *it = burning_features.back();
                burning_features.pop_back();
			}
		}

        symbolic_features.remove(index);

		--nb_features;
		feature[index].type = -1;		// On efface l'objet
	}



	void Features::resetListOfItemsToDisplay()
	{
		list.clear();
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
				n_feature[i].type = -1;
			DELETE_ARRAY(feature);
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
		const Feature* const pFeature = feature_manager.getFeaturePointer(type);
		feature[idx].Pos = Pos;
        feature[idx].timeRef = Math::RandomTable() % 100000;
		feature[idx].type = type;
		feature[idx].frame = 0;
		feature[idx].draw = false;
		feature[idx].hp = float(pFeature->damage);
		feature[idx].grey = false;
		feature[idx].dt = 0.0f;
		feature[idx].angle = 0.0f;
		feature[idx].burning = false;
		feature[idx].last_spread = 0.0f;
		feature[idx].drawnOnMap = false;

		feature[idx].sinking = false;
		feature[idx].dive = false;
		feature[idx].dive_speed = 0.0f;
		feature[idx].angle_x = 0.0f;
		compute_on_map_pos(idx);

		if (!pFeature->reclaimable && !pFeature->blocking && (pFeature->metal > 0 || pFeature->geothermal))
            symbolic_features.insert(idx);
		return idx;
	}


	void Features::drawFeatureOnMap(const int idx)
	{
		MutexLocker mLock(pMutex);
		if (idx < 0 || idx >= max_features || feature[idx].drawnOnMap)
			return;
		compute_on_map_pos(idx);
		const Feature* const pFeature = feature_manager.getFeaturePointer(feature[idx].type);
		if (pFeature && pFeature->blocking)        // Check if it is a blocking feature
		{
			const int X = pFeature->footprintx;
			const int Z = pFeature->footprintz;
			the_map->obstaclesRect( feature[idx].px - (X >> 1),
									feature[idx].py - (Z >> 1),
									X, Z, true);
			the_map->rect( feature[idx].px - (X >> 1),
						   feature[idx].py - (Z >> 1),
						   X, Z, -2 - idx);
			the_map->energy.add(pFeature->gRepulsion,
								feature[idx].px - (pFeature->gRepulsion.getWidth() >> 1),
								feature[idx].py - (pFeature->gRepulsion.getHeight() >> 1));
		}
		feature[idx].drawnOnMap = true;
	}


	void Features::removeFeatureFromMap(const int idx)
	{
		MutexLocker mLock(pMutex);
		if (idx < 0 || idx >= max_features || !feature[idx].drawnOnMap)
			return;
		const Feature* const pFeature = feature_manager.getFeaturePointer(feature[idx].type);
		if (pFeature && pFeature->blocking)        // Check if it is a blocking feature
		{
			const int X = pFeature->footprintx;
			const int Z = pFeature->footprintz;
			the_map->obstaclesRect(feature[idx].px - (X >> 1), feature[idx].py - (Z >> 1), X, Z, false);
			the_map->rect(feature[idx].px - (X >> 1), feature[idx].py - (Z >> 1), X, Z, -1);
			the_map->energy.sub(pFeature->gRepulsion,
								feature[idx].px - (pFeature->gRepulsion.getWidth() >> 1),
								feature[idx].py - (pFeature->gRepulsion.getHeight() >> 1));
		}
		the_map->map_data(feature[idx].px, feature[idx].py).stuff = -1;
		feature[idx].drawnOnMap = false;
	}


	void Features::draw_icons()
	{
		static std::vector<Vector3D> metal, geothermal;
		static std::vector<Vector2D> metalUV, geothermalUV;
		metal.clear();
		geothermal.clear();
		metalUV.clear();
		geothermalUV.clear();
		const Vector3D side = Camera::inGame->side;
		const Vector3D up = Camera::inGame->up;
        const float wmetal = (float)icons[0]->width() / 24.0f;
        const float hmetal = (float)icons[0]->height() / 24.0f;
        const float wgeothermal = (float)icons[1]->width() / 24.0f;
        const float hgeothermal = (float)icons[1]->height() / 24.0f;
		const Vector3D camdir = 12.0f / (float)gfx->height * Camera::inGame->dir;
		const float camzoom = Camera::inGame->zoomFactor * 9.0f;
		pMutex.lock();
		const uint32 player_mask = 1 << players.local_human_id;
        for(FeaturesSet::const_iterator it = symbolic_features.begin() ; it != symbolic_features.end() ; ++it)
		{
			const FeatureData* const pFeature = &(feature[*it]);
			const Feature* const pFeatureType = feature_manager.getFeaturePointer(pFeature->type);
			if (pFeatureType == NULL)
				continue;
			if (!(the_map->view_map(pFeature->px >> 1, pFeature->py >> 1) & player_mask))
				continue;
			const Vector3D D (pFeature->Pos - Camera::inGame->pos);
			const float size = lp_CONFIG->ortho_camera ? camzoom : (D % camdir);
			if (pFeatureType->geothermal)
			{
				const float sizew = size * wgeothermal;
				const float sizeh = size * hgeothermal;
				geothermal.push_back(-sizew * side + sizeh * up + pFeature->Pos);
				geothermal.push_back(sizew * side + sizeh * up + pFeature->Pos);
				geothermal.push_back(sizew * side - sizeh * up + pFeature->Pos);
				geothermal.push_back(-sizew * side - sizeh * up + pFeature->Pos);
				geothermalUV.push_back(Vector2D(0.0f,0.0f));
				geothermalUV.push_back(Vector2D(1.0f,0.0f));
				geothermalUV.push_back(Vector2D(1.0f,1.0f));
				geothermalUV.push_back(Vector2D(0.0f,1.0f));
			}
			else
			{
				const float sizew = size * wmetal;
				const float sizeh = size * hmetal;
				metal.push_back(-sizew * side + sizeh * up + pFeature->Pos);
				metal.push_back(sizew * side + sizeh * up + pFeature->Pos);
				metal.push_back(sizew * side - sizeh * up + pFeature->Pos);
				metal.push_back(-sizew * side - sizeh * up + pFeature->Pos);
				metalUV.push_back(Vector2D(0.0f,0.0f));
				metalUV.push_back(Vector2D(1.0f,0.0f));
				metalUV.push_back(Vector2D(1.0f,1.0f));
				metalUV.push_back(Vector2D(0.0f,1.0f));
			}
		}
		pMutex.unlock();

		gfx->ReInitTexSys(true);

		glDisable( GL_CULL_FACE );
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		if (!metal.empty())
		{
            icons[0]->bind();
			glVertexPointer(3, GL_FLOAT, 0, &(metal.front()));
			glTexCoordPointer(2, GL_FLOAT, 0, &(metalUV.front()));
			glDrawArrays(GL_QUADS, 0, (GLsizei)metal.size());
		}
		if (!geothermal.empty())
		{
            icons[1]->bind();
			glVertexPointer(3, GL_FLOAT, 0, &(geothermal.front()));
			glTexCoordPointer(2, GL_FLOAT, 0, &(geothermalUV.front()));
			glDrawArrays(GL_QUADS, 0, (GLsizei)geothermal.size());
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glDisable(GL_BLEND);
		glEnable( GL_CULL_FACE );
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}

} // namespace TA3D

