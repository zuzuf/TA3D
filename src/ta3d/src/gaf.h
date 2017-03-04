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
  |                                         gaf.h                                      |
  |  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers gaf de total annihilation qui sont les fichiers contenant les images  |
  | et les animations du jeu.                                                          |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#ifndef __TA3D_MISC_GAF_H__
# define __TA3D_MISC_GAF_H__

# include "stdafx.h"
# include "misc/string.h"
# include <vector>
# include "logs/logs.h"
# include <QOpenGLFunctions>
# include <QImage>
# include <gfx/texture.h>

# define TA3D_GAF_STANDARD      0x00010100
# define TA3D_GAF_TRUECOLOR     0x00010101

#if not defined FILTER_TRILINEAR
# define FILTER_TRILINEAR   0x3
#endif


class QIODevice;

namespace TA3D
{
	/*!
	** \brief Toolkit for the GAF file format
	**
	** Usefull classes :
	** <ul>
	**   <li> Gaf::Animation </li>
	**   <li> Gaf::AnimationList </li>
	** </ul>
	*/
	class Gaf
	{
	public:
		/*!
		** \brief Header of a Gaf file
		*/
		struct Header
		{
			//! \name Constructors
			//@{
			//! Default constructor
			Header() :IDVersion(0), Entries(0), Unknown1(0) {}
			/*!
			** \brief Constructor with RAW data from file
			*/
            Header(QIODevice* file);
			/*!
			** \brief Constructor
			*/
			Header(const sint32 v, const sint32 e, const sint32 u)
				:IDVersion(v), Entries(e), Unknown1(u)
			{}
			//@}

			//! Version stamp - always 0x00010100 */ // 0x00010101 is used for truecolor mode
			sint32 IDVersion;
			//! Number of items contained in this file
			sint32 Entries;
			//! Always equals to 0
			sint32 Unknown1;
		};

		/*!
		** \brief A single entry in a Gaf file
		*/
		struct Entry
		{
			//! \name Constructors
			//@{
			//! Default constructor
			Entry() :Frames(0), Unknown1(1), Unknown2(0) {}
			//@}

			//! Number of frames in this entry
			sint16 Frames;
			//! Unknown - always 1
			sint16 Unknown1;
			//! Unknown - always 0
			sint32 Unknown2;
			//! Name of the entry
			QString name;
		};

		struct Frame
		{
			/*!
			** \brief A single frame entry
			*/
			struct Entry
			{
				//! Pointer to frame data
				sint32 PtrFrameTable;
				//! Unknown - varies
				sint32 Unknown1;
			};


			/*!
			** \brief Data for a single frame
			*/
			struct Data
			{
				//! Constructors
				//@{
				//! Default constructor
				Data();
				//! Constructor from RAW file data
                Data(QIODevice* file);
				//@}

				//! Width of the frame
				sint16 Width;
				//! Height of the frame
				sint16 Height;
				//! X offset
				sint16 XPos;
				//! Y offset
				sint16 YPos;
				//! Transparency color for uncompressed images - always 9
				//! In truecolor mode : alpha channel present
				sint8 Transparency;
				//! Compression flag - Useless in truecolor mode
				sint8 Compressed;
				//! Count of subframes
				sint16 FramePointers;
				//! Unknown - always 0
				sint32  Unknown2;
				//! Pointer to pixels or subframes
				sint32  PtrFrameData;
				//! Unknown - value varies
				sint32  Unknown3;

			}; // class Data

		}; // class Frame



	public:
		/*!
		** \brief
		*/
        static TA3D::GfxTexture::Ptr ToTexture(QString filename, const QString& imgname, int* w = NULL, int* h = NULL, const bool truecolor = true, const int filter = FILTER_TRILINEAR);

		/*!
		** \brief Convert all Gaf images into OpenGL textures
		**
		** \param[out] out The list of OpenGL textures
		** \param filename The Gaf filename
		** \param imgname
		** \param[out] w The width of the image
		** \param[out] h The height of the image
		** \param truecolor
		*/
        static void ToTexturesList(std::vector<GfxTexture::Ptr>& out, const QString& filename, const QString &imgname,
								   int* w = NULL, int* h = NULL, const bool truecolor = true, const int filter = FILTER_TRILINEAR);

		/*!
        ** \brief Load a GAF image into a QImage
		*/
        static QImage RawDataToBitmap(QIODevice* file, const sint32 entry_idx, const sint32 img_idx, short* ofs_x = NULL, short* ofs_y = NULL, const bool truecolor = true);			// Lit une image d'un fichier gaf en mémoire

		/*!
		** \brief Get the number of entries from raw data
		** \see Gaf::Header
		*/
        static sint32 RawDataEntriesCount(QIODevice* file);

        static QString RawDataGetEntryName(QIODevice* file, int entry_idx);

        static sint32 RawDataGetEntryIndex(QIODevice *file, const QString& name);

        static sint32 RawDataImageCount(QIODevice *file, const int entry_idx);



	public:
		/*!
		** \brief Read animated GAF files
		*/
		class Animation
		{
		public:
			//! \name Constructor & Destructor
			//@{
			//! Default constructor
			Animation() {init();}
			//! Destructor
			~Animation() {destroy();}
			//@}

			void init();
			void destroy();

            void loadGAFFromRawData(QIODevice *file, const int entry_idx = 0, const bool truecolor = true, const QString& fname = QString());

			void loadGAFFromDirectory(const QString &folderName, const QString &entryName);

			void convert(bool NO_FILTER = false, bool COMPRESSED = false);

			void clean();

		public:
			//!
			sint32 nb_bmp;
			//!
			std::vector<QImage> bmp;
			//!
			std::vector<short> ofs_x;
			//!
			std::vector<short> ofs_y;
			//!
			std::vector<short> w;
			//!
			std::vector<short> h;
			//!
            std::vector<GfxTexture::Ptr> glbmp;
			//!
			QString name;
			//!
			QString  filename;
			//!
			bool    logo;       // Logo flag, used to prevent animated logos

		private:
			//! True when convert() has already been called
			bool pAnimationConverted;

		}; // class Animation


		/*! \class AnimationList
		**
		** \brief Container for all animations in a GAF file
		*/
		class AnimationList
		{
		public:
			//! \name Constructor & Destructor
			//@{
			//! Default constructor
			AnimationList() :pList() {}
			//! Destructor
			~AnimationList();
			//@}

			/*!
			** \brief Clear all animation
			** \todo Must be removed in cTA3D_Engine::~cTA3D_Engine: bus error when the program exits
			*/
			void clear();

			/*!
			** \brief Load all animation from a GAF file (Raw Data)
			**
			** \param data The Raw data
			** \param doConvert
			** \param fname
			** \return The number of animation found
			*/
            sint32 loadGAFFromRawData(QIODevice* file, const bool doConvert = false, const QString& fname = "");

			/*!
			** \brief Load all animation from a GAF-like directory
			**
			** \param data The Raw data
			** \param doConvert
			** \param fname
			** \return The number of animation found
			*/
			sint32 loadGAFFromDirectory(const QString &folderName, const bool doConvert = false);

			/*!
			** \brief
			*/
			void convert(const bool no_filter = false, const bool compressed = false);

			/*!
			** \deprecated Use clear instead
			*/
			void clean();

			/*!
			** \brief Find the first animation wit hthe given name
			**
			** \param name The name of the animation to find
			** \return The index of the animation. -1 if not found
			*/
			sint32 findByName(const QString& name) const;

			/*!
			** \brief The number of Animation in the list
			*/
			sint32 size() const {return sint32(pList.size());}

			/*!
			** \brief Get an animation given its index
			*/
			const Gaf::Animation& operator[] (const sint32 indx) const
			{ LOG_ASSERT((unsigned int)indx < pList.size()); return pList[indx]; }

			/*!
			** \brief Get an animation given its index
			*/
			Gaf::Animation& operator[] (const sint32 indx)
			{ LOG_ASSERT((unsigned int)indx < pList.size()); return pList[indx]; }

		private:
			typedef std::vector<Gaf::Animation> AnimationVector;
			//! All animation
			AnimationVector pList;
		}; // class AnimationList


	}; // class Gaf




} // namespace TA3D


#endif // __TA3D_MISC_GAF_H__
