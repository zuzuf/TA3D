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

/*-----------------------------------------------------------------------------------\
  |                                         fbi.cpp                                    |
  |  Ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers fbi du jeu totalannihilation qui sont les fichiers de données sur les |
  | unités du jeu. Cela inclus les classes pour gérer les différents types d'unités et |
  | le système de gestion de liens entre unités.                                       |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include <vector>
#include <algorithm>
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "mesh/mesh.h"
#include "EngineClass.h"
#include "UnitEngine.h"
#include "languages/i18n.h"
#include "misc/math.h"
#include "logs/logs.h"
#include "ingame/players.h"
#include "misc/tdf.h"
#include "misc/paths.h"
#include "engine/mission.h"
#include "input/mouse.h"
#include "gfx/gui/area.h"

namespace TA3D
{



	UnitManager unit_manager;

	inline bool overlaps( int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2 )
	{
		const int w = w1 + w2;
		const int h = h1 + h2;
		const int X1 = Math::Min(x1, x2);
		const int Y1 = Math::Min(y1, y2);
		const int X2 = Math::Max(x1 + w1, x2 + w2);
		const int Y2 = Math::Max(y1 + h1, y2 + h2);
		return X2 - X1 < w && Y2 - Y1 < h;
	}

	void UnitType::AddUnitBuild(int index, int px, int py, int pw, int ph, int p, GLuint Pic)
	{
		if (index < -1)
			return;

		if (!BuildList.empty() && nb_unit > 0)		// Vérifie si l'unité n'est pas déjà répertoriée / check if not already there
		{
			for (int i = 0; i < nb_unit; ++i)
			{
				if (BuildList[i] == index) // We already have it so leave now
				{
					if (Pic)
						gfx->destroy_texture(Pic);
					return;
				}
			}
		}

		if (p == -1)
		{
			if (dl_data == NULL)		// We can't add a menu entry if we don't know where to add it
			{
				LOG_ERROR(LOG_PREFIX_RESOURCES << "I can't add this menu entry without a list of available menu buttons");
				if (Pic)
					gfx->destroy_texture(Pic);
				return;
			}
			for(int i = 0 ; i <= nb_pages && p == -1 ; ++i)
			{
				for(uint32 k = 0 ; k < dl_data->size() && p == -1 ; ++k)
				{
					bool found = true;
					for(int j = 0 ; j < nb_unit && found ; ++j)
					{
						if (Pic_p[j] == i
							&& overlaps(Pic_x[j], Pic_y[j], Pic_w[j], Pic_h[j],
										(*dl_data)[k].x, (*dl_data)[k].y, (*dl_data)[k].w, (*dl_data)[k].h))
							found = false;
					}
					if (found)
					{
						p = i;
						px = (*dl_data)[k].x;
						py = (*dl_data)[k].y;
						pw = (*dl_data)[k].w;
						ph = (*dl_data)[k].h;
					}
				}
			}
		}
		nb_pages = Math::Max(nb_pages, short(p + 1));
		++nb_unit;
		if (BuildList.empty())
			nb_unit = 1;
		BuildList.push_back(short(index));
		PicList.push_back(Pic);
		Pic_x.push_back(short(px));
		Pic_y.push_back(short(py));
		Pic_w.push_back(short(pw));
		Pic_h.push_back(short(ph));
		Pic_p.push_back(short(p));
	}

    GLuint loadBuildPic(const QString &gafFileName, const QString &name, int *w = NULL, int *h = NULL)
	{
        if (name.isEmpty())
			return 0;
		
        if (unit_manager.name2gaf.empty())
		{
            const QStringList &animsList = unit_manager.animsList;

            for(QStringList::const_iterator it = animsList.begin() ; it != animsList.end() ; ++it)
			{
                const QString &gafName = *it;
                QStringList entries;
                if (VFS::Instance()->getDirlist(gafName + "/*", entries))				// GAF-like directory
				{
					for(uint32 i = 0 ; i < entries.size() ; ++i)
					{
                        const QString &key = Paths::ExtractFileName(entries[i]).toUpper();
                        if (key.isEmpty())
							continue;
						if (unit_manager.name2gaf.find(key) == unit_manager.name2gaf.end())
							unit_manager.name2gaf[key] = gafName;
					}
				}
				else																	// normal GAF
				{
                    QIODevice* gaf_file = VFS::Instance()->readFile( gafName );
					if (gaf_file)
					{
						const int nbEntries = Gaf::RawDataEntriesCount(gaf_file);
						for(int i = 0 ; i < nbEntries ; ++i)
						{
                            const QString &key = Gaf::RawDataGetEntryName(gaf_file, i).toUpper();
                            if (key.isEmpty())
								continue;
							if (unit_manager.name2gaf.find(key) == unit_manager.name2gaf.end())
								unit_manager.name2gaf[key] = gafName;
						}
						delete gaf_file;
					}
				}
			}
		}

		GLuint tex = 0;
		gfx->set_texture_format(gfx->defaultTextureFormat_RGB());

        QString key = ToUpper(name);
        HashMap< QString >::Dense::iterator item = unit_manager.name2gaf.find(key);
		if (item != unit_manager.name2gaf.end())
		{
            QStringList test;
			if (ToUpper(*item) != ToUpper(gafFileName))
				test.push_back(gafFileName);
			test.push_back(*item);
            for(QStringList::iterator it = test.begin() ; it != test.end() && tex == 0 ; ++it)
			{
                if (it->contains(".gaf"))						// Is it a normal GAF ?
				{
                    QIODevice* gaf_file = VFS::Instance()->readFile( *it );
					if (gaf_file)
					{
						SDL_Surface *img = Gaf::RawDataToBitmap(gaf_file, Gaf::RawDataGetEntryIndex(gaf_file, name), 0);
						if (img)
						{
							if (w)
								*w = img->w;
							if (h)
								*h = img->h;
							tex = gfx->make_texture(img, FILTER_LINEAR);
							SDL_FreeSurface(img);
						}

						delete gaf_file;
					}
				}
				else								// GAF-like directory
				{
					tex = Gaf::ToTexture(*it, name, w, h, true, FILTER_LINEAR);
				}
			}
		}
		return tex;
	}

    void UnitManager::analyse(const QString &filename,int unit_index)
	{
		TDFParser gui_parser(filename, false, false, true);

        QString number = Paths::ExtractFileNameWithoutExtension(filename);
		int first = int(number.size() - 1);
		while (first >= 0 && number[first] >= '0' && number[first] <= '9')
			--first;
		++first;
		number = Substr(number,first, number.size() - first);

        const int page = number.toInt(nullptr, 0) - 1;		// Extract the page number

		const int NbObj = gui_parser.pullAsInt("gadget0.totalgadgets");

		const int x_offset = gui_parser.pullAsInt("gadget0.common.xpos");
		const int y_offset = gui_parser.pullAsInt("gadget0.common.ypos");

		gfx->set_texture_format(GL_RGB5);
        QString name;
		for (int i = 1; i <= NbObj; ++i)
		{
            const int attribs = gui_parser.pullAsInt( QString("gadget%1.common.commonattribs").arg(i) );
			if (!(attribs & 4) && !(attribs & 8))	// Neither a unit nor a weapon
				continue;
            name = gui_parser.pullAsString( QString("gadget%1.common.name").arg(i) );
			const int idx = (attribs & 4) ? get_unit_index(name) : -1;		// attribs & 4 ==> unit, attribs & 8 ==> weapon
			if ((attribs & 4) && idx == -1)
			{
				if (name != "IGPATCH")
					LOG_ERROR(LOG_PREFIX_RESOURCES << "Can't add unit to build menu : unit not found : '" << name << "'");
				continue;
			}

			if (unit_type[unit_index]->canBuild(idx))
				continue;
            int w = gui_parser.pullAsInt( QString("gadget%1.common.width").arg(i)  );
            int h = gui_parser.pullAsInt( QString("gadget%1.common.height").arg(i) );
            const GLuint tex = loadBuildPic( "anims/" + unit_type[unit_index]->Unitname + QString::number(page + 1) + ".gaf", name, &w, &h);
            const int x = gui_parser.pullAsInt( QString("gadget%1.common.xpos").arg(i) ) + x_offset;
            const int y = gui_parser.pullAsInt( QString("gadget%1.common.ypos").arg(i) ) + y_offset;
			unit_type[unit_index]->AddUnitBuild(idx, x, y, w, h, page, tex);
		}
	}



    void UnitManager::analyse2(QIODevice *file)
	{
		TDFParser parser;
        const QByteArray &buffer = file->readAll();
        parser.loadFromMemory("analyse2", buffer.data(), buffer.size(), false, false, true);
		file->close();

        for(int g = 0 ; parser.exists(QString("gadget%1").arg(g)) ; ++g)
		{
            const QString &unitmenu = parser.pullAsString(QString("gadget%1.unitmenu").arg(g));
            const QString &unitname = parser.pullAsString(QString("gadget%1.unitname").arg(g));

            if (unitmenu.isEmpty() || unitname.isEmpty()) continue;

			int unit_index = get_unit_index(unitmenu);
			if (unit_index == -1)
			{
				LOG_DEBUG("unit '" << unitmenu << "' not found");
				continue;		// Au cas où l'unité n'existerait pas
			}
			int idx = get_unit_index(unitname);
			if (idx >= 0 && idx < nb_unit)
			{
				if (!unit_type[unit_index]->canBuild(idx))
				{
                    GLuint tex = loadBuildPic( "anims/" + unitname + "_gadget.gaf", unitname);
					if (!tex && !unit_type[idx]->glpic && unit_type[idx]->unitpic)
					{
						gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
						unit_type[idx]->glpic = gfx->make_texture(unit_type[idx]->unitpic, FILTER_LINEAR, true);
						SDL_FreeSurface(unit_type[idx]->unitpic);
						unit_type[idx]->unitpic = NULL;
					}
					if (tex || unit_type[idx]->glpic)
						unit_type[unit_index]->AddUnitBuild(idx, -1, -1, 64, 64, -1, tex);
					else
					{	LOG_DEBUG("no build picture found for unit '" << unitname << "', cannot add it to " << unitmenu << " build menu");	}
				}
			}
			else
			{	LOG_DEBUG("unit '" << unitname << "' not found, cannot add it to " << unitmenu << " build menu");	}
		}
	}



    UnitType *UnitManager::load_unit(const QString &filename)			// Ajoute une nouvelle unité
	{
		UnitType *pUnitType = new UnitType();
		pUnitType->load(filename);
		mInternals.lock();
		unit_type.push_back(pUnitType);
        if (!pUnitType->Unitname.isEmpty())
            unit_hashtable[pUnitType->Unitname.toLower()] = nb_unit;
        if (!pUnitType->name.isEmpty())
            unit_hashtable[pUnitType->name.toLower()] = nb_unit;
        if (!pUnitType->ObjectName.isEmpty())
            unit_hashtable[pUnitType->ObjectName.toLower()] = nb_unit;
        if (!pUnitType->Description.isEmpty())
            unit_hashtable[pUnitType->Description.toLower()] = nb_unit;
        if (!pUnitType->Designation_Name.isEmpty())
            unit_hashtable[pUnitType->Designation_Name.toLower()] = nb_unit;
		nb_unit++;
		mInternals.unlock();
		return pUnitType;
	}



	void UnitManager::gather_build_data()
	{
        QStringList file_list;
        VFS::Instance()->getFilelist( ta3dSideData.download_dir + "*.tdf", file_list);

        for (const QString &f : file_list) // Cherche un fichier pouvant contenir des informations sur l'unité unit_name
		{
            QIODevice* file = VFS::Instance()->readFile(f);		// Lit le fichier
			if (file)
			{
				analyse2(file);
				delete file;
			}
		}
	}

	void UnitManager::start_threaded_stuffs()
	{
		ready = false;
		new UnitDataLoader;
	}


	void UnitManager::gather_all_build_data()
	{
		animsList.clear();
        VFS::Instance()->getDirlist("anims/*", animsList);				// GAF-like directories
        VFS::Instance()->getFilelist("anims/*.gaf", animsList);		// normal GAF files
		name2gaf.clear();

		// Cherche un fichier pouvant contenir des informations sur l'unité unit_name
		for (int i = 0; i < nb_unit; ++i)
		{
            QString name;
            for(int n = 1 ; VFS::Instance()->fileExists(name = ta3dSideData.guis_dir + unit_type[i]->Unitname + QString::number(n) + ".gui") ; ++n)
				analyse(name, i);
		}

		// Fill build menus with information parsed from the sidedata.tdf file
        TDFParser sidedata_parser(ta3dSideData.gamedata_dir + "sidedata.tdf", false, true);
		for (int i = 0 ; i < nb_unit; ++i)
		{
			int n = 1;
            while(!sidedata_parser.pullAsString(ToLower("canbuild." + unit_type[i]->Unitname + ".canbuild" + QString::number(n) ) ).isEmpty())  n++;

			n--;
            QString canbuild = sidedata_parser.pullAsString(ToLower("canbuild." + unit_type[i]->Unitname + ".canbuild" + QString::number(n) ) );
			while (n > 0)
			{
				int idx = get_unit_index( canbuild );
				if (idx >= 0 && idx < nb_unit)
				{
					if (!unit_type[i]->canBuild(idx))		// Check if it's already in the list
					{
                        GLuint tex = loadBuildPic( "anims/" + canbuild + "_gadget", canbuild);
						if (!tex && !unit_type[idx]->glpic && unit_type[idx]->unitpic)
						{
							unit_type[idx]->glpic = gfx->make_texture(unit_type[idx]->unitpic, FILTER_LINEAR, true);
							SDL_FreeSurface(unit_type[idx]->unitpic);
							unit_type[idx]->unitpic = NULL;
							gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
						}
						if (unit_type[idx]->glpic || tex)
						{
							const int px = ((n - 1) & 1) * 64;
							const int py = 155 + (((n - 1) >> 1) % 3) * 64;
							const int p = (n - 1)  / 6;
							unit_type[i]->AddUnitBuild(idx, px, py, 64, 64, p, tex);
						}
						else
						{	LOG_DEBUG("unit '" << canbuild << "' picture not found in build menu for unit '" << unit_type[i]->Unitname << "'");	}
					}
				}
				else
				{	LOG_DEBUG("unit '" << canbuild << "' not found (" << __FILE__ << " l." << __LINE__ << ')');	}
				--n;
                canbuild = sidedata_parser.pullAsString( "canbuild." + unit_type[i]->Unitname + ".canbuild" + QString::number(n) );
			}
		}

		gather_build_data();			// Read additionnal build data
		animsList.clear();
		name2gaf.clear();
	}


    void UnitManager::load_script_file(const QString &unit_name)
	{
        const QString &uprname = QString(unit_name).toUpper();
		const int unit_index = get_unit_index(uprname);
		if (unit_index == -1)
			return;

		// Everything is done in the SCRIPT_DATA interface, it tries script types in priority order
        unit_type[unit_index]->script = ScriptData::loadScriptFile("scripts/" + uprname);
	}


	void UnitManager::Identify()			// Identifie les pièces aux quelles les scripts font référence
	{
		for (int i = 0; i < nb_unit; ++i)
		{
			if (unit_type[i]->script && unit_type[i]->model)
                unit_type[i]->model->Identify(unit_type[i]->script);
		}
	}

	bool UnitType::floatting() const
	{
		return Floater || canhover || !Math::Zero(WaterLine);
	}

	void UnitType::destroy()
	{
		MovementClass.clear();
		soundcategory.clear();
		ExplodeAs.clear();
		SelfDestructAs.clear();
		script = NULL;

		w_badTargetCategory.clear();
		BadTargetCategory.clear();
		NoChaseCategory.clear();
		aim_data.clear();

		BuildList.clear();
		Pic_x.clear();
		Pic_y.clear();
		Pic_w.clear();
		Pic_h.clear();
		Pic_p.clear();

		if (!PicList.empty())
		{
			for (unsigned int i = 0; i < PicList.size(); ++i)
				gfx->destroy_texture(PicList[i]);
			PicList.clear();
		}

		yardmap.clear();
		model = NULL;
		if (unitpic)
			SDL_FreeSurface(unitpic);
		gfx->destroy_texture(glpic);
		Corpse.clear();
		Unitname.clear();
		name.clear();
		side.clear();
		ObjectName.clear();
		Designation_Name.clear();
		Description.clear();
		Category.clear();
		categories.clear();

		init();
	}

	void UnitType::init()
	{
        sweetspot_cached = -1;

        not_used = false;
		commander = false;
		selfdestructcountdown = 5;

		last_click = -1;
		click_time = 0.0f;

		emitting_points_computed = false;
		soundcategory.clear();

		isfeature = false;
		antiweapons = false;

		weapon.clear();         // No weapons
		aim_data.clear();

		script = NULL;		// Aucun script

		page = 0;

		nb_pages = 0;
		nb_unit = 0;
		BuildList.clear();
		PicList.clear();
		Pic_x.clear();				// Coordinates
		Pic_y.clear();
		Pic_w.clear();				// Coordinates
		Pic_h.clear();
		Pic_p.clear();				// Page where the pic has to be shown

		dl_data = NULL;

		init_cloaked = false;
		mincloakdistance = 10;
		DefaultMissionType = MISSION_STANDBY;
		attackrunlength = 0;
		yardmap.clear();
		model = NULL;
		unitpic = NULL;
		glpic = 0;
		hoverattack = false;
		SortBias = 0;
		IsAirBase = false;
		AltFromSeaLevel = 0;
		TransportSize = 0;
		TransportCapacity = 0;
		ManeuverLeashLength = 640;
		CruiseAlt = 0;
		TEDclass = CLASS_UNDEF;
		WaterLine = 0.0f;
		StandingMoveOrder = 1;
		MobileStandOrders = 1;
		StandingFireOrder = 1;
		FireStandOrders = 1;
		ShootMe = false;
		ThreeD = true;
		Builder = false;
		Unitname.clear();
		name.clear();
		version = 0;
		side.clear();
		ObjectName.clear();
		FootprintX = 0;
		FootprintZ = 0;
		Category.clear();
		categories.clear();
		fastCategory = 0;
		MaxSlope = 255;
		BMcode = 0;
		norestrict = false;
		BuildAngle = 10;
		canresurrect = false;
		Designation_Name.clear();	// Nom visible de l'unité
		Description.clear();		// Description
		BuildCostEnergy = 0;		// Energie nécessaire pour la construire
		BuildCostMetal = 0;			// Metal nécessaire pour la construire
		MaxDamage = 10;				// Points de dégats maximum que l'unité peut encaisser
		EnergyUse = 0;				// Energie nécessaire pour faire quelque chose
		BuildTime = 0;				// Temps de construction
		WorkerTime = 1;				// Vitesse de construction
		AutoFire = false;			// Tire automatique
		SightDistance = 50;			// Distance maximale de vue de l'unité
		RadarDistance = 0;			// Distance maximale de detection radar
		RadarDistanceJam = 0;		// For Radar jammers
		EnergyStorage = 0;			// Quantité d'énergie stockable par l'unité
		MetalStorage = 0;			// Quantité de metal stockable par l'unité
		ExplodeAs.clear();			// Type d'explosion lorsque l'unité est détruite
		SelfDestructAs.clear();		// Type d'explosion lors de l'autodestruction
		Corpse.clear();				// Restes de l'unité
		UnitNumber = 0;				// ID de l'unité
		canmove = false;			// Indique si l'unité peut bouger
		canpatrol = false;			// si elle peut patrouiller
		canstop = false;			// si elle peut s'arrêter
		canguard = false;			// si elle peut garder une autre unité
		MaxVelocity = 1;			// Vitesse maximale
		BrakeRate = 1;				// Vitesse de freinage
		Acceleration = 1;			// Accélération
		TurnRate = 1;				// Vitesse de tournage
		SteeringMode = 0;
		canfly = false;				// si l'unité peut voler
		Scale = 1.0f;				// Echelle
		BankScale = 0;
		BuildDistance = 0.0f;		// Distance maximale de construction
		CanReclamate = false;		// si elle peut récupérer
		EnergyMake = 0;				// Production d'énergie de l'unité
		MetalMake = 0.0f;			// Production de métal de l'unité
		MovementClass.clear();		// Type de mouvement
		Upright = false;			// Si l'unité est debout
		w_badTargetCategory.clear();	// Type d'unité non ciblable par les armes
		BadTargetCategory.clear();	// Type d'unité non attacable
		DamageModifier = 1.0f;		// How much of the weapon damage it takes
		canattack = false;			// Si l'unité peut attaquer
		ActivateWhenBuilt = false;	// L'unité s'active lorsqu'elle est achevée
		onoffable = false;			// (Dés)activable
		MaxWaterDepth = 0;			// Profondeur maximale où l'unité peut aller
		MinWaterDepth = -0xFFF;		// Profondeur minimale où l'unité peut aller
		NoShadow = false;			// Si l'unité n'a pas d'ombre
		canload = false;			// Si elle peut charger d'autres unités
		WeaponID.clear();			// Arme 2
		Floater = false;			// Si l'unité flotte
		canhover = false;			// For hovercrafts
		NoChaseCategory.clear();	// Type d'unité non chassable
		SonarDistance = 0;			// Portée du sonar
		SonarDistanceJam = 0;		// For Sonar jammers
		candgun = false;			// si l'unité peut utiliser l'arme ravage
		CloakCost = 0;				// Coût en energie pour rendre l'unité invisible
		CloakCostMoving = 0;		// Idem mais quand l'unité bouge
		HealTime = 0;				// Temps nécessaire à l'unité pour se réparer
		CanCapture = false;			// Si elle peut capturer d'autres unités
		HideDamage = false;			// Cache la vie de l'unité aux autres joueurs
		ImmuneToParalyzer = false;	// Immunisation
		Stealth = false;
		MakesMetal = 0;				// production de métal de l'unité
		ExtractsMetal = 0.0f;		// métal extrait par l'unité
		TidalGenerator = false;		// Si l'unité est une centrale marée-motrice
		TransportMaxUnits = 0;		// Maximum d'unités transportables
		kamikaze = false;			// Unité kamikaze
		kamikazedistance = 0;
		WindGenerator = 0;			// Centrale de type Eolienne
	}


	void UnitType::show_info()
	{
        Gui::AREA::current()->caption("unit_info.tName", I18N::Translate("Name") + ": " + name);
        Gui::AREA::current()->caption("unit_info.tInternalName", I18N::Translate("Internal name") + ": " + Unitname);
		Gui::AREA::current()->caption("unit_info.tDescription", Description);
        Gui::AREA::current()->caption("unit_info.tHP", I18N::Translate("HP") + QString(": %1").arg(MaxDamage));
        Gui::AREA::current()->caption("unit_info.tCost", I18N::Translate("Cost") + QString(": E %1 M %2").arg(BuildCostEnergy).arg(BuildCostMetal));
        Gui::AREA::current()->caption("unit_info.tBuildTime", I18N::Translate("Build time") + QString(": %1").arg(BuildTime));

        QString tWeapons;
		for( std::vector<WeaponDef*>::iterator i = weapon.begin() ; i != weapon.end() ; ++i )
			if (*i)
                tWeapons += (*i)->name + QString(": %1").arg((*i)->damage) + "\n";
		Gui::AREA::current()->caption("unit_info.tWeaponList", tWeapons);
		Gui::GUIOBJ::Ptr image = Gui::AREA::current()->get_object("unit_info.unitpic");
		if (image)
		{
			if (unitpic)
			{
				gfx->set_texture_format(GL_RGB5);
				glpic = gfx->make_texture(unitpic, FILTER_LINEAR);
				SDL_FreeSurface(unitpic);
				unitpic = NULL;
			}
			image->x1 = 10;
			image->y1 = 40;
			image->x2 = 106;
			image->y2 = 136;
			image->Data = (uint32)glpic;
		}

		Gui::AREA::current()->msg("unit_info.show");
	}

#define parseStringDef(x,y)  (unitParser.pullAsString(x, unitParser_ci.pullAsString(x, y)))
#define parseIntDef(x, y)    (unitParser.pullAsInt(x, unitParser_ci.pullAsInt(x, y)))
#define parseBoolDef(x, y)   (unitParser.pullAsBool(x, unitParser_ci.pullAsBool(x, y)))
#define parseFloatDef(x, y)  (unitParser.pullAsFloat(x, unitParser_ci.pullAsFloat(x, y)))

#define parseString(x)  (unitParser.pullAsString(x, unitParser_ci.pullAsString(x)))
#define parseInt(x)     (unitParser.pullAsInt(x, unitParser_ci.pullAsInt(x)))
#define parseBool(x)    (unitParser.pullAsBool(x, unitParser_ci.pullAsBool(x)))
#define parseFloat(x)   (unitParser.pullAsFloat(x, unitParser_ci.pullAsFloat(x)))

    int UnitType::load(const QString &filename)
	{
		destroy();
		int nb_inconnu = 0;
        const QString lang_name = I18N::Translate("UNITTYPE_NAME", "UNITINFO.Name");
        const QString lang_desc = I18N::Translate("UNITTYPE_DESCRIPTION", "UNITINFO.Description");
        const QString lang_name_alt = I18N::Translate("UNITTYPE_NAME_ALT", "UNITINFO.Name");
        const QString lang_desc_alt = I18N::Translate("UNITTYPE_DESCRIPTION_ALT", "UNITINFO.Description");

		TDFParser unitParser( filename, true, true );         // FBI files are case sensitive (something related to variable priority)
		TDFParser unitParser_ci( filename, false, true );     // Case insensitive parser

        Unitname = parseString("UNITINFO.UnitName");
		version = byte(parseInt("UNITINFO.Version"));
        side = parseString("UNITINFO.Side");
        ObjectName = parseString("UNITINFO.Objectname");
        Designation_Name = parseString("UNITINFO.Designation");
        Description = parseStringDef( lang_desc, parseStringDef( lang_desc_alt, parseString("UNITINFO.Description") ) );
        name = parseStringDef( lang_name, parseStringDef(lang_name_alt, parseString("UNITINFO.Name") ) );

		FootprintX = byte(parseInt("UNITINFO.FootprintX"));
		FootprintZ = byte(parseInt("UNITINFO.FootprintZ"));
		BuildCostEnergy = parseInt("UNITINFO.BuildCostEnergy");
		BuildCostMetal = parseInt("UNITINFO.BuildCostMetal");
		MaxDamage = parseInt("UNITINFO.MaxDamage");
		MaxWaterDepth = short(parseIntDef("UNITINFO.MaxWaterDepth", 0));
		MinWaterDepth = short(parseIntDef("UNITINFO.MinWaterDepth", -0xFFF));
		if (MinWaterDepth != -0xFFF && MaxWaterDepth == 0) MaxWaterDepth = 255;
		EnergyUse = parseInt("UNITINFO.EnergyUse");
		BuildTime = parseInt("UNITINFO.BuildTime");
		WorkerTime = parseIntDef("UNITINFO.WorkerTime",1);
        Builder = parseBool("UNITINFO.Builder") | parseBool("UNITINFO.canbuild");
		ThreeD = parseBoolDef("UNITINFO.ThreeD",true);
		SightDistance = parseIntDef("UNITINFO.SightDistance",100) >> 1;
		RadarDistance = parseInt("UNITINFO.RadarDistance") >> 1;
		RadarDistanceJam = parseInt("UNITINFO.RadarDistanceJam") >> 1;
        soundcategory = parseString("UNITINFO.SoundCategory");
        if (!parseString("UNITINFO.wthi_badTargetCategory").isEmpty())
		{
            while (w_badTargetCategory.size() < 3)
                w_badTargetCategory.push_back(QString());
            w_badTargetCategory[2] = parseString("UNITINFO.wthi_badTargetCategory");
		}
        if (!parseString("UNITINFO.wsec_badTargetCategory").isEmpty())
		{
            while (w_badTargetCategory.size() < 2)
                w_badTargetCategory.push_back(QString());
            w_badTargetCategory[1] = parseString("UNITINFO.wsec_badTargetCategory");
		}
        if (!parseString("UNITINFO.wpri_badTargetCategory").isEmpty())
		{
			if (w_badTargetCategory.size() < 1)
                w_badTargetCategory.push_back(QString());
            w_badTargetCategory[0] = parseString("UNITINFO.wpri_badTargetCategory");
		}
        for (unsigned int i = 4 ; !parseString( QString("UNITINFO.w%1_badTargetCategory").arg(i)).isEmpty() ; ++i)
		{
            while (w_badTargetCategory.size() < i)
                w_badTargetCategory.push_back(QString());
            w_badTargetCategory[i-1] = parseString(QString("UNITINFO.w%1_badTargetCategory").arg(i));
		}
        NoChaseCategory = parseString("UNITINFO.NoChaseCategory");
        BadTargetCategory = parseString("UNITINFO.BadTargetCategory");

        const QString category = parseString("UNITINFO.Category").toLower();
		Category.clear();
        categories = category.split(' ', QString::SkipEmptyParts);
        for (const QString &i : categories)
            Category.insert(i);
		fastCategory = 0;
		if (checkCategory( "kamikaze" ) )	fastCategory |= CATEGORY_KAMIKAZE;
		if (checkCategory( "notair" ) )		fastCategory |= CATEGORY_NOTAIR;
		if (checkCategory( "notsub" ) )		fastCategory |= CATEGORY_NOTSUB;
		if (checkCategory( "jam" ) )		fastCategory |= CATEGORY_JAM;
		if (checkCategory( "commander" ) )	fastCategory |= CATEGORY_COMMANDER;
		if (checkCategory( "weapon" ) )		fastCategory |= CATEGORY_WEAPON;
		if (checkCategory( "level3" ) )		fastCategory |= CATEGORY_LEVEL3;

		UnitNumber = short(parseInt("UNITINFO.UnitNumber"));
		canmove = parseBool("UNITINFO.canmove");
		canpatrol = parseBool("UNITINFO.canpatrol");
		canstop = parseBool("UNITINFO.canstop");
		canguard = parseBool("UNITINFO.canguard");
		MaxVelocity = parseFloatDef("UNITINFO.MaxVelocity", 1.0f / 16.0f) * 16.0f;
		BrakeRate = parseFloatDef("UNITINFO.BrakeRate", 1.0f / 160.0f) * 160.0f;
		Acceleration = parseFloatDef("UNITINFO.Acceleration", 1.0f / 160.0f) * 160.0f;
		TurnRate = parseFloatDef("UNITINFO.TurnRate", 1.0f / (TA2DEG * 20.0f)) * TA2DEG * 20.0f;
		candgun = parseBool("UNITINFO.candgun");
		canattack = parseBool("UNITINFO.canattack");
		CanReclamate = parseBool("UNITINFO.CanReclamate");
		EnergyMake = short(parseInt("UNITINFO.EnergyMake"));
		MetalMake = parseFloat("UNITINFO.MetalMake");
		CanCapture = parseBool("UNITINFO.CanCapture");
		HideDamage = parseBool("UNITINFO.HideDamage");
		HealTime = parseInt("UNITINFO.HealTime") * 30;
		CloakCost = parseInt("UNITINFO.CloakCost");
		CloakCostMoving = parseInt("UNITINFO.CloakCostMoving");
		init_cloaked = parseBool("UNITINFO.init_cloaked");
		mincloakdistance = parseIntDef("UNITINFO.mincloakdistance", 20) >> 1;
		BuildDistance = parseFloat("UNITINFO.Builddistance");
		ActivateWhenBuilt = parseBool("UNITINFO.ActivateWhenBuilt");
		ImmuneToParalyzer = parseBool("UNITINFO.ImmuneToParalyzer");
		SonarDistance = parseInt("UNITINFO.SonarDistance")>>1;
		SonarDistanceJam = parseInt("UNITINFO.SonarDistanceJam")>>1;
		// copyright = ... not needed here :P
		MaxSlope = short(parseIntDef("UNITINFO.MaxSlope", 255));
		SteeringMode = byte(parseInt("UNITINFO.SteeringMode"));
		BMcode = byte(parseInt("UNITINFO.BMcode"));
		ShootMe = parseBool("UNITINFO.ShootMe");
		Upright = parseBool("UNITINFO.Upright");
		norestrict = parseBool("UNITINFO.norestrict");
		AutoFire = !parseBoolDef("UNITINFO.NoAutoFire", true);
		EnergyStorage = parseInt("UNITINFO.EnergyStorage");
		MetalStorage = parseInt("UNITINFO.MetalStorage");
		StandingMoveOrder = byte(parseIntDef("UNITINFO.StandingMoveOrder", 1));
		MobileStandOrders = byte(parseIntDef("UNITINFO.mobilestandorders", 1));
		StandingFireOrder = byte(parseIntDef("UNITINFO.StandingFireOrder", 1));
		FireStandOrders = byte(parseIntDef("UNITINFO.firestandorders", 1));
		WaterLine = parseFloat("UNITINFO.WaterLine");

        QString TEDclassString = ToLower( parseString("UNITINFO.TEDClass") );
        if (TEDclassString.contains("water"))           TEDclass = CLASS_WATER;
        else if (TEDclassString.contains("ship"))       TEDclass = CLASS_SHIP;
        else if (TEDclassString.contains("energy"))     TEDclass = CLASS_ENERGY;
        else if (TEDclassString.contains("vtol"))       TEDclass = CLASS_VTOL;
        else if (TEDclassString.contains("kbot"))       TEDclass = CLASS_KBOT;
        else if (TEDclassString.contains("plant"))      TEDclass = CLASS_PLANT;
        else if (TEDclassString.contains("tank"))       TEDclass = CLASS_TANK;
        else if (TEDclassString.contains("special"))    TEDclass = CLASS_SPECIAL;
        else if (TEDclassString.contains("fort"))       TEDclass = CLASS_FORT;
        else if (TEDclassString.contains("metal"))      TEDclass = CLASS_METAL;
        else if (TEDclassString.contains("cnstr"))      TEDclass = CLASS_CNSTR;
        else if (TEDclassString.contains("commander"))  TEDclass = CLASS_COMMANDER;
        else if (!TEDclassString.isEmpty())
		{
            LOG_DEBUG("unknown tedclass ID : " << TEDclassString);
			nb_inconnu++;
		}

		NoShadow = parseBool("UNITINFO.NoShadow");
		antiweapons = parseBool("UNITINFO.antiweapons");
		BuildAngle = parseIntDef("UNITINFO.buildangle",10);
		canfly = parseBool("UNITINFO.Canfly");
		canload = parseBool("UNITINFO.canload");
		Floater = parseBool("UNITINFO.Floater");
		canhover = parseBool("UNITINFO.canhover");
		if (canhover)           // Can go over water so let's say MinWaterDepth is negative and MaxWaterDepth is null, that way it's a standard hovercraft :)
		{
			MinWaterDepth = -0xFFF;
			MaxWaterDepth = 0;
		}
		BankScale = byte(parseInt("UNITINFO.BankScale"));
		TidalGenerator = parseBool("UNITINFO.TidalGenerator");
		Scale = 1.0f;//parseFloat("UNITINFO.Scale",1.0f);
        Corpse = parseString("UNITINFO.Corpse");
		WindGenerator = short(parseInt("UNITINFO.WindGenerator"));
		onoffable = parseBool("UNITINFO.onoffable");
		kamikaze = parseBool("UNITINFO.kamikaze");
		kamikazedistance = uint16(parseIntDef("UNITINFO.kamikazedistance", SightDistance << 1) >> 1);

		unsigned int i = 1;
        while (i <= 3 || !parseString( QString("UNITINFO.Weapon%1").arg(i) ).isEmpty())
		{
			if (WeaponID.size() < i)
				WeaponID.resize(i,-1);
            WeaponID[i-1] = weapon_manager.get_weapon_index( parseString( QString("UNITINFO.Weapon%1").arg(i) ) );
			++i;
		}
        yardmap = parseString("UNITINFO.YardMap").toLatin1();
        if (!yardmap.isEmpty())
		{
			i = 0;
			for (unsigned int e = 0 ; e < yardmap.size() ; e++)
				if (yardmap[e] == ' ')
					i++;
				else
					yardmap[e - i] = yardmap[e];
            yardmap.chop(i);
            if (!yardmap.isEmpty())
				while (yardmap.size() < FootprintX * FootprintZ)     // Complete the yardmap if needed
                    yardmap += *yardmap.rbegin();
		}

		CruiseAlt = short(parseInt("UNITINFO.cruisealt"));
        ExplodeAs = parseString("UNITINFO.ExplodeAs");
        SelfDestructAs = parseString("UNITINFO.SelfDestructAs");
		ManeuverLeashLength = short(parseIntDef("UNITINFO.maneuverleashlength", 640));

        const QString &DefaultMissionTypeString = parseString("UNITINFO.DefaultMissionType").toLower();
        if (DefaultMissionTypeString == "standby")				DefaultMissionType=MISSION_STANDBY;
        else if (DefaultMissionTypeString == "vtol_standby")		DefaultMissionType=MISSION_VTOL_STANDBY;
        else if (DefaultMissionTypeString == "guard_nomove")		DefaultMissionType=MISSION_GUARD_NOMOVE;
        else if (DefaultMissionTypeString == "standby_mine")		DefaultMissionType=MISSION_STANDBY_MINE;
        else if (!DefaultMissionTypeString.isEmpty())
		{
            LOG_ERROR("Unknown constant: `" << DefaultMissionTypeString << "`");
			++nb_inconnu;
		}

		TransportMaxUnits = parseInt("UNITINFO.TransMaxUnits");
		TransportMaxUnits = parseIntDef("UNITINFO.transportmaxunits", TransportMaxUnits);
		if (canload)
			TransportMaxUnits = Math::Max(1, TransportMaxUnits);
		TransportCapacity = parseInt("UNITINFO.transportcapacity");
		TransportSize = parseInt("UNITINFO.transportsize");
		AltFromSeaLevel = short(parseInt("UNITINFO.altfromsealevel"));
        MovementClass = parseString("UNITINFO.MovementClass");

		IsAirBase = parseBool("UNITINFO.IsAirBase");
		commander = parseBool("UNITINFO.Commander");
		DamageModifier = parseFloatDef("UNITINFO.DamageModifier",1.0f);
		MakesMetal = parseFloat("UNITINFO.MakesMetal");
		SortBias = byte(parseInt("UNITINFO.sortbias"));
		ExtractsMetal = parseFloat("UNITINFO.ExtractsMetal");
		hoverattack = parseBool("UNITINFO.HoverAttack");
		isfeature = parseBool("UNITINFO.IsFeature");
		Stealth = parseBool("UNITINFO.Stealth");
		attackrunlength = parseInt("UNITINFO.attackrunlength");
		selfdestructcountdown = uint8(parseIntDef("UNITINFO.selfdestructcountdown", 5));
		canresurrect = parseBool("UNITINFO.canresurrect") || parseBool("UNITINFO.resurrect");

		aim_data.resize(WeaponID.size());
		for (unsigned int i = 0 ; i < WeaponID.size(); i++)
		{
			aim_data[i].check = false;
			if (WeaponID[i] > -1)
			{
                const QString &aimdir = parseString( QString("UNITINFO.WeaponMainDir%1").arg(i) );
                if (!aimdir.isEmpty())
				{
                    const QStringList &vec = aimdir.split(' ');
					if (vec.size() == 3)
					{
						aim_data[i].check = true;
						aim_data[i].dir.x = vec[0].toFloat();
						aim_data[i].dir.y = vec[1].toFloat();
						aim_data[i].dir.z = vec[2].toFloat();
						// Should read almost every possible case
                        aim_data[i].Maxangledif = parseFloat( QString("UNITINFO.Maxangledif%1").arg(i) );
					}
					else
					{	LOG_DEBUG("FBI parser error: '" << aimdir << "' could not be parsed correctly");	}
				}
			}
		}

		if (canresurrect && Math::Zero(BuildDistance))
			BuildDistance = float(SightDistance);
		weapon.resize( WeaponID.size() );
        w_badTargetCategory.clear();
        w_badTargetCategory.reserve( WeaponID.size() );
		bomber = false;
		for (unsigned int i = 0; i < WeaponID.size(); ++i)
		{
            w_badTargetCategory.push_back(QString());
			if (WeaponID[i] > -1)
			{
				weapon[i] = &(weapon_manager.weapon[WeaponID[i]]);
				bomber |= weapon[i]->dropped;
			}
		}
        if (!Unitname.isEmpty())
		{
			model = model_manager.get_model(ObjectName);
			if (model == NULL)
				LOG_ERROR("`" << Unitname << "` without a 3D model");
		}
		else
			LOG_WARNING("The unit does not have a name");
		if (canfly == 1)
			TurnRate = TurnRate * 3; // A hack thanks to Doors
		// Build the repulsion grid
		gRepulsion.resize(FootprintX * 3, FootprintZ * 3);
		const float sigx = (float)FootprintX * 0.75f;
		const float sigz = (float)FootprintZ * 0.75f;
		const float sigx2 = -0.5f / (sigx * sigx);
		const float sigz2 = -0.5f / (sigz * sigz);
		for(int z = 0 ; z < gRepulsion.getHeight() ; ++z)
		{
			float dz = (float)z - (float)gRepulsion.getHeight() * 0.5f;
			// Distance to the unit, not its center of gravity if it's a building
			if (!BMcode)
				dz = (float)Math::Sgn(dz) * Math::Max(fabsf(dz) - (float)FootprintZ * 0.5f, 0.0f);
			dz *= dz;
			for(int x = 0 ; x < gRepulsion.getWidth() ; ++x)
			{
				float dx = (float)x - (float)gRepulsion.getWidth() * 0.5f;
				// Distance to the unit, not its center of gravity if it's a building
				if (!BMcode)
					dx = (float)Math::Sgn(dx) * Math::Max(fabsf(dx) - (float)FootprintX * 0.5f, 0.0f);
				dx *= dx;
				gRepulsion(x,z) = 2550.0f * std::exp(sigx2 * dx + sigz2 * dz);
			}
		}
		load_dl();
		return nb_inconnu;
	}

	void UnitType::load_dl()
	{
        if (side.isEmpty())
			return;
		dl_data = NULL;
		if (unit_manager.h_dl_data.count(ToLower(side)) != 0)
			dl_data = unit_manager.h_dl_data[ToLower(side)];

		if (dl_data)
			return;			// Ok it's already loaded

		int side_id = -1;
        for( int i = 0 ; i < ta3dSideData.nb_side && side_id == -1 ; ++i )
            if (ta3dSideData.side_name[ i ].toLower() == side.toLower())
				side_id = i;
		if (side_id == -1)
			return;


		TDFParser dl_parser;
        if (dl_parser.loadFromFile(ta3dSideData.guis_dir + ta3dSideData.side_pref[side_id] + "dl.gui", false, false, true))
		{
			dl_data = new DlData;
			int NbObj = dl_parser.pullAsInt( "gadget0.totalgadgets" );

			int x_offset = dl_parser.pullAsInt( "gadget0.common.xpos" );
			int y_offset = dl_parser.pullAsInt( "gadget0.common.ypos" );

			dl_data->clear();

			for (int i = 1; i <= NbObj; ++i)
			{
                if (dl_parser.pullAsInt( QString("gadget%1.common.attribs").arg(i) ) == 32 )
				{
					DlDataPic p;
                    p.x = short(dl_parser.pullAsInt(QString("gadget%1.common.xpos").arg(i)) + x_offset);
                    p.y = short(dl_parser.pullAsInt(QString("gadget%1.common.ypos").arg(i)) + y_offset);
                    p.w = short(dl_parser.pullAsInt(QString("gadget%1.common.width").arg(i)));
                    p.h = short(dl_parser.pullAsInt(QString("gadget%1.common.height").arg(i)));
					dl_data->push_back(p);
				}
			}

            unit_manager.h_dl_data[side.toLower()] = dl_data;
		}
		else
		{
			LOG_WARNING("`dl.gui` file is missing");
			dl_data = NULL;
		}
	}

    QString UnitType::getMoveStringID() const
	{
        return QString("%1,%2,%3,%4,%5,%6")
                .arg(int(FootprintX))
                .arg(int(FootprintZ))
                .arg(MaxSlope)
                .arg(canhover)
                .arg(MaxWaterDepth)
                .arg(MinWaterDepth);
	}

	void UnitManager::destroy()
	{
		unit_hashtable.clear();

		for (HashMap< DlData* >::Dense::iterator i = h_dl_data.begin() ; i != h_dl_data.end() ; ++i)
			delete *i;
		h_dl_data.clear();

		for (UnitList::iterator i = unit_type.begin(); i != unit_type.end(); ++i)
			delete *i;
		unit_type.clear();
		panel.destroy();
		paneltop.destroy();
		panelbottom.destroy();
		init();
	}

    void UnitManager::load_panel_texture( const QString &intgaf )
	{
		panel.destroy();

		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
		int w,h;
        panel.set(Gaf::ToTexture("anims/" + intgaf, "PANELSIDE2", &w, &h, true));
		panel.width = w;
		panel.height = h;

        paneltop.set(Gaf::ToTexture("anims/" + intgaf, "PANELTOP", &w, &h));
		paneltop.width = w;
		paneltop.height = h;
        panelbottom.set(Gaf::ToTexture("anims/" + intgaf, "PANELBOT", &w, &h));
		panelbottom.width = w;
		panelbottom.height = h;
	}



	int UnitManager::unit_build_menu(int index,int omb,float &dt, int scrolling, bool GUI)				// Affiche et gère le menu des unités
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		gfx->ReInitTexSys();
		glColor4ub(0xFF, 0xFF, 0xFF, byte(0xFF - int(lp_CONFIG->menuTransparency * 0xFF)));
		if (GUI)
		{
			if (panel.tex)
				gfx->drawtexture( panel.tex, 0.0f, 128.0f, 128.0f, 128.0f + float(panel.height) );

			if (paneltop.tex)
			{
				gfx->drawtexture( paneltop.tex, 128.0f, 0.0f, 128.0f + float(paneltop.width), float(paneltop.height) );
				for (int k = 0 ; 128 + paneltop.width + panelbottom.width * k < uint32(SCREEN_W); ++k)
				{
					gfx->drawtexture(panelbottom.tex, 128.0f + float(paneltop.width + k * panelbottom.width), 0.0f,
							128.0f + float(paneltop.width + panelbottom.width * (k + 1)), float(panelbottom.height) );
				}
			}

			if (panelbottom.tex)
			{
				for (int k = 0 ; 128 + panelbottom.width * k < uint32(SCREEN_W) ; ++k)
				{
					gfx->drawtexture( panelbottom.tex, 128.0f + float(k * panelbottom.width),
							float(SCREEN_H - panelbottom.height), 128.0f + float(panelbottom.width * (k + 1)), float(SCREEN_H) );
				}
			}

			glDisable(GL_TEXTURE_2D);
			glColor4ub(0x0, 0x0, 0x0, byte(0xFF - int(lp_CONFIG->menuTransparency * 0xFF)));
			glBegin(GL_QUADS);
			glVertex2i(0, 0);			// Barre latérale gauche
			glVertex2i(128, 0);
			glVertex2i(128, 128);
			glVertex2i(0, 128);

			glVertex2i(0, 128 + panel.height);			// Barre latérale gauche
			glVertex2i(128, 128 + panel.height);
			glVertex2i(128, SCREEN_H);
			glVertex2i(0, SCREEN_H);
			glEnd();
			glColor4ub(0xFF, 0xFF, 0xFF, byte(0xFF - int(lp_CONFIG->menuTransparency * 0xFF)));
			return 0;
		}

		glEnable(GL_TEXTURE_2D);

		if (index<0 || index>=nb_unit) return -1;		// L'indice est incorrect

		int page=unit_type[index]->page;

		int sel=-1;

		gfx->set_2D_clip_rectangle(0, 128, 128, SCREEN_H - 128);

		glDisable(GL_BLEND);
		for( int i = 0 ; i < unit_type[index]->nb_unit ; ++i) // Affiche les différentes images d'unités constructibles
		{
			if (unit_type[index]->Pic_p[i] != page)
				continue;
			const int px = unit_type[index]->Pic_x[ i ];
			const int py = unit_type[index]->Pic_y[ i ] - scrolling;
			const int pw = unit_type[index]->Pic_w[ i ];
			const int ph = unit_type[index]->Pic_h[ i ];
			const bool unused = unit_type[index]->BuildList[i] >= 0 && unit_type[unit_type[index]->BuildList[i]]->not_used;
			if (unused)
				glColor4ub(0x4C, 0x4C, 0x4C, 0xFF);		// Make it darker
			else
				glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

			if (unit_type[index]->PicList[i])							// If a texture is given use it
				gfx->drawtexture(unit_type[index]->PicList[i], float(px), float(py), float(px + pw), float(py + ph));
			else if (unit_type[index]->BuildList[i] >= 0)
				gfx->drawtexture(unit_type[unit_type[index]->BuildList[i]]->glpic, float(px), float(py), float(px + pw), float(py + ph));

			if (mouse_x >= px && mouse_x < px + pw && mouse_y >= py && mouse_y < py + ph && !unused)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE);
				glColor4ub(0xFF, 0xFF, 0xFF, 0xBF);
				if (unit_type[index]->PicList[i])							// If a texture is given use it
					gfx->drawtexture(unit_type[index]->PicList[i], float(px), float(py), float(px + pw), float(py + ph));
				else if (unit_type[index]->BuildList[i] >= 0)
					gfx->drawtexture(unit_type[unit_type[index]->BuildList[i]]->glpic, float(px), float(py), float(px + pw), float(py + ph));
				glDisable(GL_BLEND);
				sel = unit_type[index]->BuildList[i];
				if (sel == -1)
					sel = -2;
			}

			if (( unit_type[index]->BuildList[i] == unit_type[index]->last_click
				  || ( unit_type[index]->last_click == -2 && unit_type[index]->BuildList[i] == -1 ) )
				&& unit_type[index]->click_time > 0.0f )
			{
				glEnable(GL_BLEND);
				glDisable(GL_TEXTURE_2D);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4ub(0xFF, 0xFF, 0xFF, byte(255.0f * unit_type[index]->click_time));
				const float mx = float(px);
				const float my = float(py);
				const float mw = float(pw);
				const float mh = float(ph);
				gfx->rectfill( mx, my, mx + mw, my + mh );
				glColor4ub(0xFF, 0xFF, 0x00, 0xBF);
				gfx->line( mx, my + mh * unit_type[index]->click_time, mx + mw, my + mh * unit_type[index]->click_time );
				gfx->line( mx, my + mh * (1.0f - unit_type[index]->click_time), mx + mw, my + mh * (1.0f - unit_type[index]->click_time) );
				gfx->line( mx + mw * unit_type[index]->click_time, my, mx + mw * unit_type[index]->click_time, my + mh );
				gfx->line( mx + mw * (1.0f - unit_type[index]->click_time), my, mx + mw * (1.0f - unit_type[index]->click_time), my + mh );
				glColor4ub(0xFF, 0xFF, 0xFF, 0xBF);
				glEnable(GL_TEXTURE_2D);
				glDisable(GL_BLEND);
			}
		}
		gfx->set_2D_clip_rectangle();

		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		if (unit_type[index]->last_click != -1 )
			unit_type[index]->click_time -= dt;

		if (sel > -1)
		{
			if (lp_CONFIG->tooltips)
			{	// Tooltip code
                QString message = unit_type[sel]->name
                        + QString(" M:%1").arg(unit_type[sel]->BuildCostMetal)
                        + QString(" E:%1").arg(unit_type[sel]->BuildCostEnergy)
                        + QString(" HP:%1").arg(unit_type[sel]->MaxDamage);
                if (!unit_type[sel]->Description.isEmpty())
                    message += '\n' + unit_type[sel]->Description;
				Gui::AREA::current()->getSkin()->PopupMenu((float)mouse_x + 20.0f, (float)mouse_y + 20.0f, message);
			}
			else
			{	// Print info at the bottom of the screen
				const InterfaceData &side_data = ta3dSideData.side_int_data[ players.side_view ];
				gfx->print(gfx->normal_font,
						   (float)side_data.Name.x1,
						   (float)side_data.Name.y1,
						   0.0f, 0xFFFFFFFF,
                           unit_type[sel]->name
                           + QString(" M:%1").arg(unit_type[sel]->BuildCostMetal)
                           + QString(" E:%1").arg(unit_type[sel]->BuildCostEnergy)
                           + QString(" HP:%1").arg(unit_type[sel]->MaxDamage) );

                if (!unit_type[sel]->Description.isEmpty())
					gfx->print(gfx->normal_font,
							   (float)side_data.Description.x1,
							   (float)side_data.Description.y1,
							   0.0f,0xFFFFFFFF,unit_type[sel]->Description );
			}
			glDisable(GL_BLEND);
		}

		if (sel != -1 && mouse_b == 1 && omb != 1)		// Click !!
		{
			unit_type[index]->last_click = sint16(sel);
			unit_type[index]->click_time = 0.5f;		// One sec animation;
		}

		return sel;
	}

	int UnitManager::load_all_units(ProgressNotifier *progress)
	{
		init();
        QStringList file_list;
        VFS::Instance()->getFilelist( ta3dSideData.unit_dir + '*' + ta3dSideData.unit_ext, file_list);

        std::atomic<int> n(0), m(0);

        QThread * const root_thread = QThread::currentThread();
        parallel_for<size_t>(0, file_list.size(), [&](const size_t i)
		{
            if (QThread::currentThread() == root_thread)
            {
                if (progress != NULL && m >= 0xF)
                {
                    (*progress)((300.0f + float(n) * 50.0f / float(file_list.size() + 1)) / 7.0f, I18N::Translate("Loading units"));
                    m -= 0xF;
                }
            }
            ++m;
            const QString &nom = Paths::ExtractFileNameWithoutExtension(file_list[i]).toUpper();			// Vérifie si l'unité n'est pas déjà chargée
            ++n;

            mInternals.lock();
            if (unit_manager.get_unit_index(nom) == -1)
            {
                mInternals.unlock();
                LOG_DEBUG("Loading the unit `" << nom << "`...");
                UnitType *pUnitType = unit_manager.load_unit(file_list[i]);
                if (!pUnitType->Unitname.isEmpty())
                {
                    const QString &pcx_name = "unitpics/" + pUnitType->Unitname + ".pcx";
                    pUnitType->unitpic = gfx->load_image(pcx_name);
                }
            }
            else
                mInternals.unlock();
        });

		unit_manager.start_threaded_stuffs();

		unit_manager.gather_all_build_data();

		return 0;
	}


	bool UnitType::canBuild(const int index) const
	{
		for (int i = 0; i < nb_unit; ++i)
		{
			if (BuildList[i] == index)
				return true;
		}
		return false;
	}


	UnitManager::UnitManager()
	{
		init();
	}


	void UnitManager::init()
	{
		nb_unit = 0;
		panel.init();
		paneltop.init();
		panelbottom.init();
	}


	UnitType::UnitType()
	{
		init();
	}


	void UnitManager::waitUntilReady() const
	{
		while (!ready)
			rest(10);
	}


	UnitDataLoader::UnitDataLoader()
	{
		start();
	}


	void UnitDataLoader::proc(void *)
	{
		for (int i = 0;i < unit_manager.nb_unit; ++i)
			unit_manager.load_script_file(unit_manager.unit_type[i]->Unitname);

		unit_manager.Identify();

		// Correct some data given in the FBI file using data from the moveinfo.tdf file
        TDFParser parser(ta3dSideData.gamedata_dir + "moveinfo.tdf");
		int n = 0;
        while (!parser.pullAsString(QString("CLASS%1.name").arg(n)).isEmpty())
			++n;

		for (int i = 0; i < unit_manager.nb_unit; ++i)
		{
            if (!unit_manager.unit_type[i]->MovementClass.isEmpty())
			{
                const QString &movementclass = unit_manager.unit_type[i]->MovementClass.toUpper();
				for (int e = 0; e < n; ++e)
				{
                    if (parser.pullAsString(QString("CLASS%1.name").arg(e)) == movementclass)
					{
                        unit_manager.unit_type[i]->FootprintX = byte(parser.pullAsInt(QString("CLASS%1.footprintx").arg(e), unit_manager.unit_type[i]->FootprintX ));
                        unit_manager.unit_type[i]->FootprintZ = byte(parser.pullAsInt(QString("CLASS%1.footprintz").arg(e), unit_manager.unit_type[i]->FootprintZ ));
                        unit_manager.unit_type[i]->MinWaterDepth = short(parser.pullAsInt(QString("CLASS%1.minwaterdepth").arg(e), unit_manager.unit_type[i]->MinWaterDepth ));
                        unit_manager.unit_type[i]->MaxWaterDepth = short(parser.pullAsInt(QString("CLASS%1.maxwaterdepth").arg(e), unit_manager.unit_type[i]->MaxWaterDepth ));
                        unit_manager.unit_type[i]->MaxSlope = short(parser.pullAsInt(QString("CLASS%1.maxslope").arg(e), unit_manager.unit_type[i]->MaxSlope ));
						break;
					}
				}
			}
		}
		unit_manager.ready = true;
	}
} // namespace TA3D

