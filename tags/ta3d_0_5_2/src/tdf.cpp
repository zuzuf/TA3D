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
#include "ingame/players.h"



namespace TA3D
{


    FEATURE_MANAGER		feature_manager;
    FEATURES features;

    FEATURE::FEATURE()
    {
        init();
    }

    FEATURE::~FEATURE()
    {
        destroy();
    }


    void FEATURE::init()
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

    void FEATURE::destroy()
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


    void FEATURE::convert()
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




    FEATURE_MANAGER::FEATURE_MANAGER()
        :feature_hashtable()
    {
        init();
    }


    FEATURE_MANAGER::~FEATURE_MANAGER()
    {
        destroy();
        feature_hashtable.emptyHashTable();
    }


    void FEATURE_MANAGER::init()
    {
        max_features = 0;
        nb_features = 0;
        feature = NULL;
    }


    int FEATURE_MANAGER::get_feature_index(const String &name)
    {
        if(name.empty() || nb_features <= 0)
            return -1;
        return feature_hashtable.find(String::ToLower(name)) - 1;
    }


    char* FEATURE_MANAGER::get_line(const char *data)
    {
        int pos = 0;
        while (data[pos] != 0 && data[pos] != 13 && data[pos] != 10)
            ++pos;
        char* d = new char[pos + 1];
        memcpy(d, data, pos);
        d[pos] = 0;
        return d;
    }

    int FEATURE_MANAGER::add_feature(const String& name)			// Ajoute un élément
    {
        ++nb_features;
        if (nb_features > max_features)
        {
            if (max_features == 0)  max_features = 10;
            max_features *= 2;
            FEATURE* n_feature = new FEATURE[max_features];
            if (feature && nb_features > 1)
            {
                for(int i = 0;i < nb_features-1; ++i)
                {
                    n_feature[i] = feature[i];
                    feature[i].init();
                }
            }
            if (feature)
                delete[] feature;
            feature = n_feature;
        }
        feature[nb_features-1].init();
        feature[nb_features-1].name = name;
        feature_hashtable.insert(String::ToLower(name), nb_features);
        return nb_features-1;
    }

    void FEATURE_MANAGER::destroy()
    {
        if (nb_features > 0 && feature)			// Détruit les éléments
            for(int i = 0; i < nb_features; ++i)
                feature[i].destroy();
        if (feature)
            delete[] feature;

        feature_hashtable.emptyHashTable();
        feature_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);
        init();
    }


    void FEATURE_MANAGER::clean()
    {
        if(feature)
        {
            for(int i = 0; i < nb_features; ++i)
            {
                if (!feature[i].need_convert)
                    feature[i].anim.clean();
            }
        }
    }



    void FEATURE_MANAGER::load_tdf(char *data,int size)					// Charge un fichier tdf
    {
        char *pos=data;
        char *ligne=NULL;
        int nb=0;
        int index=0;
        int	first=nb_features;
        char *limit=data+size;
        do{
            do
            {
                ++nb;
                if (ligne)
                    delete[] ligne;
                ligne = get_line(pos);
                strlwr(ligne);
                while (pos[0] != 0 && pos[0] != 13 && pos[0] != 10)
                    ++pos;
                while (pos[0] == 13 || pos[0] == 10)
                    ++pos;

                if (strstr( ligne, " ") != NULL && strstr(ligne, "=") != NULL) // remove useless spaces
                {
                    int e = 0;
                    bool offset = true;
                    for(int i = 0 ; i <= (int)strlen(ligne) ; ++i)// <= because of NULL termination!!
                    {
                        if( ligne[ i ] == '=' )	offset = false;
                        if( offset && ligne[ i ] == ' ' ) e++;
                        if( i >= e )
                            ligne[ i - e ] = ligne[ i ];
                    }
                }

                if(ligne[0]=='[')
                {
                    if(strstr(ligne,"]"))
                        *(strstr(ligne,"]"))=0;
                    index=add_feature(ligne+1);
                    feature[index].m3d=false;
                }
                else if(strstr(ligne,"world=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].world = String(strstr(ligne,"world=")+6);
                }
                else if(strstr(ligne,"description=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].description = String(strstr(ligne,"description=")+12);
                }
                else if(strstr(ligne,"category=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].category = String(strstr(ligne,"category=")+9);
                }
                else if(strstr(ligne,"object=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].filename = String(strstr(ligne,"object=")+7);
                    feature[index].m3d=true;
                }
                else if(strstr(ligne,"filename=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].filename = String(strstr(ligne,"filename=")+9);
                    feature_hashtable.insert(String::ToLower(feature[index].filename), index + 1 );
                }
                else if(strstr(ligne,"seqname=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].seqname = String(strstr(ligne,"seqname=")+8);
                    feature_hashtable.insert(String::ToLower(feature[index].seqname ), index + 1 );
                }
                else if(strstr(ligne,"animating="))
                    feature[index].animating=(*(strstr(ligne,"animating=")+10)=='1');
                else if(strstr(ligne,"animtrans="))
                    feature[index].animtrans = feature[index].animating = (*(strstr(ligne,"animtrans=")+10)=='1');
                else if(strstr(ligne,"shadtrans="))
                    feature[index].shadtrans = (*(strstr(ligne,"shadtrans=")+10)=='1');
                else if(strstr(ligne,"indestructible="))
                    feature[index].indestructible = (*(strstr(ligne,"indestructible=")+15)=='1');
                else if(strstr(ligne,"height="))
                    feature[index].height = atoi(strstr(ligne,"height=")+7);
                else if(strstr(ligne,"hitdensity="))
                    feature[index].hitdensity = atoi(strstr(ligne,"hitdensity=")+11);
                else if(strstr(ligne,"metal="))
                    feature[index].metal = atoi(strstr(ligne,"metal=")+6);
                else if(strstr(ligne,"energy="))
                    feature[index].energy = atoi(strstr(ligne,"energy=")+7);
                else if(strstr(ligne,"damage="))
                    feature[index].damage = atoi(strstr(ligne,"damage=")+7);
                else if(strstr(ligne,"footprintx="))
                    feature[index].footprintx = atoi(strstr(ligne,"footprintx=")+11);
                else if(strstr(ligne,"footprintz="))
                    feature[index].footprintz = atoi(strstr(ligne,"footprintz=")+11);
                else if(strstr(ligne,"autoreclaimable="))
                    feature[index].autoreclaimable = (*(strstr(ligne,"autoreclaimable=")+16)=='1');
                else if(strstr(ligne,"reclaimable="))
                    feature[index].reclaimable = (*(strstr(ligne,"reclaimable=")+12)=='1');
                else if(strstr(ligne,"blocking="))
                    feature[index].blocking = (*(strstr(ligne,"blocking=")+9)=='1');
                else if(strstr(ligne,"flamable="))
                    feature[index].flamable = (*(strstr(ligne,"flamable=")+9)=='1');
                else if(strstr(ligne,"geothermal="))
                    feature[index].geothermal = (*(strstr(ligne,"geothermal=")+11)=='1');
                else if(strstr(ligne,"reproducearea="))	{}
                else if(strstr(ligne,"reproduce="))	{}
                else if(strstr(ligne,"featuredead=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].feature_dead = String(strstr(ligne,"featuredead=")+12);
                }
                else if(strstr(ligne,"seqnameshad=")) {}
                else if(strstr(ligne,"seqnamedie=")) {}
                else if(strstr(ligne,"seqnamereclamate=")) {}
                else if(strstr(ligne,"permanent=")) {}
                else if(strstr(ligne,"nodisplayinfo=")) {}
                else if(strstr(ligne,"burnmin="))
                    feature[index].burnmin = atoi(strstr(ligne,"burnmin=")+8);
                else if(strstr(ligne,"burnmax="))
                    feature[index].burnmax = atoi(strstr(ligne,"burnmax=")+8);
                else if(strstr(ligne,"sparktime="))
                    feature[index].sparktime = atoi(strstr(ligne,"sparktime=")+10);
                else if(strstr(ligne,"spreadchance="))
                    feature[index].spreadchance = atoi(strstr(ligne,"spreadchance=")+13);
                else if(strstr(ligne,"burnweapon=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].burnweapon = String(strstr(ligne,"burnweapon=")+11);
                }
                else if(strstr(ligne,"seqnameburn=")) {}
                else if(strstr(ligne,"seqnameburnshad=")) {}
                else if(strstr(ligne,"featureburnt=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].feature_burnt = String(strstr(ligne,"featureburnt=")+13);
                }
                else if(strstr(ligne,"featurereclamate=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].feature_reclamate = String(strstr(ligne,"featurereclamate=")+17);
                }
                else if(strstr(ligne,";"))
                    LOG_ERROR(LOG_PREFIX_TDF << "Unknown: `" << ligne << "`");

            } while(strstr(ligne,"}") == NULL && nb < 10000 && pos < limit);

            delete[] ligne;
            ligne=NULL;

        } while (pos[0]=='[' && nb<10000 && pos<limit);

        if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
            allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
        else
            allegro_gl_set_texture_format(GL_RGBA8);

        for (int i = first; i < nb_features; ++i)// Charge les fichiers d'animation
        {
            if (!feature[i].category.empty())
                feature[i].vent = (strstr(feature[i].category.c_str(), "vents") != NULL);
            if (!feature[i].filename.empty() && !feature[i].seqname.empty() && !feature[i].m3d)
            {
                if (model_manager.get_model(feature[i].filename + "-" + feature[i].seqname) != NULL) // Check if there is a 3do version of it
                {
                    feature[i].model=NULL;
                    feature[i].m3d=true;
                    feature[i].converted=false;
                    feature[i].not_loaded=false;
                }
                else
                {
                    feature[i].not_loaded=true;
                    if (feature[i].height<=10.0f && feature[i].height>1.0f && feature[i].blocking
                        && strcasecmp(feature[i].description.c_str(),"Metal")!=0) // Tente une conversion en 3d
                    {
                        String tmp("anims\\");
                        tmp << feature[i].filename << ".gaf";
                        byte* gaf = HPIManager->PullFromHPI(tmp);
                        if (gaf)
                        {
                            sint32 index = Gaf::RawDataGetEntryIndex(gaf, feature[i].seqname);
                            if (index >= 0)
                                feature[i].anim.loadGAFFromRawData(gaf, Gaf::RawDataGetEntryIndex(gaf, feature[i].seqname), true, feature[i].filename);
                            else
                                LOG_WARNING(LOG_PREFIX_TDF << "`" << feature[i].name << "` has no picture to display (" << feature[i].filename << ".gaf, " << feature[i].seqname << ") !");
                            feature[i].not_loaded = false;
                            delete[] gaf;

                            if (index>=0 && feature[i].anim.nb_bmp>0
                               && feature[i].anim.bmp[0]->w>=16 && feature[i].anim.bmp[0]->h>=16) // Tente une conversion en 3d
                            {
                                String st(feature[i].filename);
                                st << "-" << feature[i].seqname;
                                model_manager.create_from_2d(feature[i].anim.bmp[0],
                                                             feature[i].footprintx * 8,
                                                             feature[i].footprintz * 8,
                                                             feature[i].height * H_DIV,
                                                             st);
                                feature[i].model = NULL;
                                feature[i].m3d = true;
                                feature[i].converted = true;
                                feature[i].anim.destroy();
                                index = -1;
                            }
                            if (index < 0)
                                feature[i].need_convert = false;
                        }
                    }
                }
            }
            else
            {
                if (!feature[i].filename.empty() && feature[i].m3d)
                    feature[i].model = NULL;
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

        for (int i = 0; i < feature_manager.nb_features; ++i)
        {
            if (feature_manager.feature[i].m3d && feature_manager.feature[i].model == NULL
                && !feature_manager.feature[i].filename.empty() && !feature_manager.feature[i].seqname.empty())
            {
                String tmp(feature_manager.feature[i].filename);
                tmp += "-";
                tmp += feature_manager.feature[i].seqname;
                feature_manager.feature[i].model = model_manager.get_model(tmp);
                if (feature_manager.feature[i].model == NULL)
                    feature_manager.feature[i].model=model_manager.get_model(String("objects3d\\")+tmp);
            }
            else
            {
                if (feature_manager.feature[i].m3d && feature_manager.feature[i].model == NULL && !feature_manager.feature[i].filename.empty())
                    feature_manager.feature[i].model = model_manager.get_model(feature_manager.feature[i].filename);
            }
        }
    }



    FEATURES::FEATURES()
        :nb_features(0), max_features(0), feature(NULL), min_idx(0), max_idx(0),
        burning_features(), sinking_features(),
        list(NULL), list_size(0)
    {}


    FEATURES::~FEATURES()
    {
        destroy();
    }


    void FEATURES::init()
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


    void FEATURES::destroy()
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


    void FEATURES::draw(Camera& cam)
    {
        if(nb_features <= 0)
            return;
        cam.setView();			// Positionne la caméra
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
            }
            int i = list[e];
            if (feature[i].type < 0 || !feature[i].draw)
                continue;

            if (cam.mirror && ((feature_manager.feature[feature[i].type].height>5.0f && feature_manager.feature[feature[i].type].m3d)			// Perform a small visibility check
                                || (feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model!=NULL)) )
            {
                Vector3D Pos(feature[i].Pos);
                if( feature_manager.feature[feature[i].type].m3d )
                    Pos.y += feature_manager.feature[feature[i].type].model->size2;
                else
                    Pos.y += feature_manager.feature[feature[i].type].height*0.5f;

                float a = cam.rpos.y - units.map->sealvl;
                float b = Pos.y - units.map->sealvl;
                float c = a + b;
                if (c == 0.0f)
                    continue;
                Pos = (a / c) * Pos + (b / c) * cam.rpos;
                Pos.y = units.map->get_unit_h( Pos.x, Pos.z );

                if (Pos.y > units.map->sealvl)	// If it's not visible don't draw it
                    continue;
            }

            if (feature_manager.feature[feature[i].type].not_loaded)
                feature_manager.feature[feature[i].type].convert();		// Load data and convert texture

            if (!feature_manager.feature[feature[i].type].m3d
                && feature_manager.feature[feature[i].type].anim.nb_bmp > 0)
            {
                feature_manager.feature[feature[i].type].convert();		// Convert texture data if needed

                feature[i].frame = (units.current_tick >> 1) % feature_manager.feature[feature[i].type].anim.nb_bmp;

                if (!texture_loaded || old != feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame])
                {
                    old=feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame];
                    texture_loaded=true;
                    glBindTexture(GL_TEXTURE_2D,feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame]);
                }

                Vector3D Pos(feature[i].Pos);
                float h  = feature_manager.feature[feature[i].type].height * 0.5f;
                float dw = 0.5f * feature_manager.feature[feature[i].type].anim.w[feature[i].frame];

                if (feature_manager.feature[feature[i].type].height > 5.0f)
                {
                    dw *= h / feature_manager.feature[feature[i].type].anim.h[feature[i].frame];

                    if(feature[i].grey)
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
                    if (!cam.mirror) 	// no need to draw things we can't see
                    {
                        dw *= 0.5f;
                        h = 0.25f*feature_manager.feature[feature[i].type].anim.h[feature[i].frame];

                        quad_table.queue_quad( feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame], QUAD( Pos, dw, h, feature[i].grey ? 0x7F7F7FFF : 0xFFFFFFFF ) );
                    }
                }
            }
            else
            {
                if(feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model!=NULL)
                {
                    if (!feature_manager.feature[feature[i].type].model->animated && !feature[i].sinking)
                    {
                        DrawingTable.queue_Instance( feature_manager.feature[feature[i].type].model->id,
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
                        if(!feature_manager.feature[feature[i].type].converted)				// To fix opacity with converted models
                            glDisable(GL_ALPHA_TEST);
                        glPushMatrix();
                        glTranslatef(feature[i].Pos.x,feature[i].Pos.y,feature[i].Pos.z);
                        glRotatef( feature[i].angle, 0.0f, 1.0f, 0.0f );
                        glRotatef( feature[i].angle_x, 1.0f, 0.0f, 0.0f );
                        feature_manager.feature[feature[i].type].model->draw(t,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,!feature[i].grey);

                        if (lp_CONFIG->underwater_bright && the_map->water && feature[i].Pos.y < the_map->sealvl)
                        {
                            glEnable( GL_BLEND );
                            glBlendFunc( GL_ONE, GL_ONE );
                            glDepthFunc( GL_EQUAL );
                            glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
                            feature_manager.feature[feature[i].type].model->draw(t,NULL,false,true,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
                            glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
                            glDepthFunc( GL_LESS );
                            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        }


                        gfx->ReInitAllTex( true );

                        glPopMatrix();
                        glEnable(GL_BLEND);
                        if(!feature_manager.feature[feature[i].type].converted)				// To fix opacity with converted models
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

        glEnable(GL_POLYGON_OFFSET_FILL);
        quad_table.draw_all();
        glDisable(GL_POLYGON_OFFSET_FILL);

        glDisable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_CULL_FACE);

        DrawingTable.draw_all();

        glDisable(GL_ALPHA_TEST);
        glDepthFunc( GL_LESS );
        glEnable(GL_TEXTURE_2D);
    }



    void FEATURES::draw_shadow(Camera& cam, const Vector3D& Dir)
    {
        if (nb_features <= 0)
            return;
        cam.setView();
        float t = (float)units.current_tick / TICKS_PER_SEC;
        pMutex.lock();
        for (int e = 0; e < list_size; ++e)
        {
            pMutex.unlock();
            pMutex.lock();
            int i = list[e];
            if(feature[i].type<0)
                continue;

            if (!(!feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].anim.nb_bmp > 0))
            {
                if (feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model != NULL)
                {
                    if (!feature[i].draw || feature[i].grey || feature_manager.feature[feature[i].type].converted)	// Quelques problèmes (graphiques et plantages) avec les modèles convertis
                        continue;

                    if (feature[i].delete_shadow_dlist && feature[i].shadow_dlist != 0 )
                    {
                        glDeleteLists( feature[i].shadow_dlist, 1);
                        feature[i].shadow_dlist = 0;
                        feature[i].delete_shadow_dlist = false;
                    }

                    if (feature_manager.feature[feature[i].type].model->animated || feature[i].sinking || feature[i].shadow_dlist == 0)
                    {
                        bool create_display_list = false;
                        if (!feature_manager.feature[feature[i].type].model->animated && !feature[i].sinking && feature[i].shadow_dlist == 0)
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
                        Vector3D R_Dir = (sqrtf(feature_manager.feature[feature[i].type].model->size)*2.0f+feature[i].Pos.y) * Dir * RotateY( -feature[i].angle * DEG2RAD ) * RotateX( -feature[i].angle_x * DEG2RAD );
                        if(g_useStencilTwoSide)													// Si l'extension GL_EXT_stencil_two_side est disponible
                            feature_manager.feature[feature[i].type].model->draw_shadow( R_Dir,t,NULL);
                        else
                            feature_manager.feature[feature[i].type].model->draw_shadow_basic( R_Dir,t,NULL);
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



    void FEATURES::move(const float dt, MAP* map, bool clean)
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
            if (!feature_manager.feature[feature[i].type].vent && !feature[i].burning )
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
                    if (feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model != NULL)
                    {
                        OBJECT* obj = &(feature_manager.feature[feature[i].type].model->obj);
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


    void FEATURES::compute_on_map_pos(const int idx)
    {
        feature[idx].px = ((int)(feature[idx].Pos.x) + the_map->map_w_d + 4) >> 3;
        feature[idx].py = ((int)(feature[idx].Pos.z) + the_map->map_h_d + 4) >> 3;
    }

    void FEATURES::burn_feature(const int idx)
    {
        pMutex.lock();
        if (idx >= 0 && idx < max_features && feature[idx].type >= 0
            && feature_manager.feature[feature[idx].type].flamable && !feature[idx].burning )// We get something to burn !!
        {
            feature[idx].burning = true;
            feature[idx].burning_time = 0.0f;
            int time_zone = abs( feature_manager.feature[feature[idx].type].burnmax - feature_manager.feature[ feature[idx].type ].burnmin ) + 1;
            feature[ idx ].time_to_burn = (Math::RandFromTable() % time_zone ) + feature_manager.feature[ feature[idx].type ].burnmin;		// How long it burns
            burning_features.push_back(idx);		// It's burning 8)

            // Start doing damages to things around
            if (!feature_manager.feature[feature[idx].type].burnweapon.empty())
            {
                int w_idx = weapon_manager.get_weapon_index(feature_manager.feature[feature[idx].type].burnweapon);
                feature[ idx ].BW_idx = w_idx;
            }
            else
                feature[idx].BW_idx = -1;
            feature[idx].weapon_counter = 0;
        }
        pMutex.unlock();
    }

    void FEATURES::sink_feature(const int idx)
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

    void FEATURES::move_forest(const float dt)			// Simulates forest fires & tree reproduction
    {
        pMutex.lock();

        Vector3D wind = 0.1f * *p_wind_dir;

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
            if (feature[*i].burning_time >= feature[*i].time_to_burn) // If we aren't burning anymore :(
            {
                if (network_manager.isServer())
                    g_ta3d_network->sendFeatureDeathEvent(*i);

                feature[*i].burning = false;
                feature[*i].hp = 0.0f;

                int sx = ((int)(feature[*i].Pos.x) + the_map->map_w_d - 4) >> 3; // Delete the feature
                int sy = ((int)(feature[*i].Pos.z) + the_map->map_h_d - 4) >> 3;
                // Remove it from map
                the_map->rect(sx - (feature_manager.feature[features.feature[*i].type].footprintx >> 1),
                              sy - (feature_manager.feature[features.feature[*i].type].footprintz >> 1),
                              feature_manager.feature[features.feature[*i].type].footprintx,
                              feature_manager.feature[features.feature[*i].type].footprintz, -1);

                // Replace the feature if needed (with the burnt feature)
                if (!feature_manager.feature[feature[*i].type].feature_burnt.empty())
                {
                    int burnt_type = feature_manager.get_feature_index( feature_manager.feature[feature[*i].type].feature_burnt);
                    if (burnt_type >= 0)
                    {
                        the_map->map_data[sy][sx].stuff = features.add_feature(feature[*i].Pos, burnt_type);
                        if( burnt_type != -1 && feature_manager.feature[ burnt_type ].blocking)
                        {
                            the_map->rect(sx-(feature_manager.feature[ burnt_type ].footprintx >> 1),
                                          sy-(feature_manager.feature[ burnt_type ].footprintz >> 1),
                                          feature_manager.feature[burnt_type].footprintx,
                                          feature_manager.feature[burnt_type].footprintz,
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
                    if (feature[*i].burning_time >= feature_manager.feature[feature[*i].type].sparktime && feature[*i].last_spread >= 0.1f) // Can spread
                    {
                        feature[*i].last_spread = 0.0f;
                        int spread_score = Math::RandFromTable() % 100;
                        if (spread_score < feature_manager.feature[feature[*i].type].spreadchance)// It tries to spread :)
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
                    if (sinf(-feature[*i].angle_x * DEG2RAD) * feature_manager.feature[feature[*i].type].footprintx * 8.0f > feature[*i].Pos.y - sea_ground)
                    {
                        feature[*i].angle_x = RAD2DEG * asinf( ( sea_ground - feature[*i].Pos.y ) / ( feature_manager.feature[feature[*i].type].footprintx * 8.0f) );
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


    void FEATURES::display_info(const int idx) const
    {
        if (idx < 0 || idx >= max_features || feature[idx].type < 0)
            return; // Nothing to display

        if (!feature_manager.feature[feature[idx].type].description.empty())
        {
            if (feature_manager.feature[feature[idx].type].reclaimable)
                gfx->print(gfx->normal_font,ta3dSideData.side_int_data[ players.side_view ].Description.x1,ta3dSideData.side_int_data[ players.side_view ].Description.y1,0.0f,0xFFFFFFFF, format("%s M:%d E:%d",I18N::Translate( feature_manager.feature[ feature[ idx ].type ].description ).c_str(),feature_manager.feature[ feature[ idx ].type ].metal,feature_manager.feature[ feature[ idx ].type ].energy) );
            else
                gfx->print(gfx->normal_font,ta3dSideData.side_int_data[ players.side_view ].Description.x1,ta3dSideData.side_int_data[ players.side_view ].Description.y1,0.0f,0xFFFFFFFF, I18N::Translate( feature_manager.feature[ feature[ idx ].type ].description ) );
        }
        glDisable(GL_BLEND);
    }

    void FEATURES::delete_feature(const int index)
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



    void FEATURES::resetListOfItemsToDisplay()
    {
        if (list)
            delete[] list;
        list = new int[max_features];
        list_size = 0;
    }

    int FEATURES::add_feature(const Vector3D& Pos, const int type)
    {
        if (type < 0 || type >= feature_manager.nb_features)
            return -1;
        MutexLocker locker(pMutex);

        ++nb_features;
        int idx = -1;
        if (nb_features > max_features) // Si besoin alloue plus de mémoire
        {
            if (max_features == 0)  max_features = 250;
            max_features *= 2;				// Double memory pool size
            FEATURE_DATA* n_feature = new FEATURE_DATA[max_features];
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
        feature[idx].hp = feature_manager.feature[type].damage;
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


    void FEATURES::drawFeatureOnMap(const int idx)
    {
        if (idx < 0 || idx >= max_features)    return;
        compute_on_map_pos(idx);
        if (feature[idx].type != -1 && feature_manager.feature[feature[idx].type].blocking)        // Check if it is a blocking feature
        {
            int X = feature_manager.feature[ feature[idx].type ].footprintx;
            int Z = feature_manager.feature[ feature[idx].type ].footprintz;
            the_map->rect( feature[idx].px - (X>>1), feature[idx].py - (Z>>1), X, Z, -2 - idx);
        }
    }

    void FEATURES::removeFeatureFromMap(const int idx)
    {
        if (idx < 0 || idx >= max_features)    return;
        if (feature[idx].type != -1 && feature_manager.feature[feature[idx].type].blocking)        // Check if it is a blocking feature
        {
            int X = feature_manager.feature[feature[idx].type].footprintx;
            int Z = feature_manager.feature[feature[idx].type].footprintz;
            the_map->rect(feature[idx].px - (X >> 1), feature[idx].py - (Z >> 1), X, Z, -1);
            the_map->map_data[feature[idx].py] [feature[idx].px].stuff = -1;
        }
    }



} // namespace TA3D

