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

/*-----------------------------------------------------------------------------\
|                                      ai.h                                    |
|       Ce module est responsable de l'intelligence artificielle               |
|                                                                              |
\-----------------------------------------------------------------------------*/

#ifndef TA3D_XX_AI_H__
# define TA3D_XX_AI_H__

# include "../stdafx.h"
# include "../threads/thread.h"
# include <list>
# include "../tdf.h"

# define TA3D_AI_FILE_EXTENSION  ".ai"


namespace TA3D
{


    struct NEURON
    {
        float	var;			// Variable pour les opérations
        float	*weight;		// Poids des différents neurones sources
    };

    class BRAIN		// NEURON network with n NEURON in input layer and p in output layer
    {
    public:
        int		nb_neuron;		// Number of NEURONs
        NEURON	*neuron;		// Array of NEURONs
        int		n;				// Number of inputs
        int		p;				// Number of outputs
        int		q;				// Size of middle layers
        float	*n_out;			// Result array

        inline void init()
        {
            nb_neuron=0;
            neuron=NULL;
            n=p=q=0;
            n_out=NULL;
        }

        inline void destroy()
        {
            if(nb_neuron>0 && neuron) {
                for(int i=0;i<nb_neuron;i++)
                    if(neuron[i].weight!=NULL)
                        free(neuron[i].weight);
                free(neuron);
            }
            if(n_out)	free(n_out);
            init();
        }

        BRAIN()
        {
            init();
        }

        ~BRAIN()
        {
            destroy();
        }

        void build(int nb_in,int nb_out,int rg);			// Create the neural network

        void active_neuron(int i);

        float *work(float entry[],bool seuil=false);			// Make NEURONs work and return the network results

        void mutation();			// Make some changes to the neural network

        void learn(float *result,float coef=1.0f);		// Make it learn

        void save(FILE *file);		// Save the network

        int load(FILE *file);		// Load the network
    };

    BRAIN *copy_brain(BRAIN *brain,BRAIN *dst=NULL);		// Make a copy

#define	ORDER_ARMY			0x00			// Order to build an army
#define	ORDER_METAL_P		0x01			// Order to gather metal
#define	ORDER_ENERGY_P		0x02			// Order to gather energy
#define	ORDER_DEFENSE		0x03			// Order to build defensive units ( AA towers, but also atomic weapons ...)
#define	ORDER_FACTORY		0x04			// Order to build factories
#define	ORDER_BUILDER		0x05			// Order to build construction units
#define	ORDER_METAL_S		0x06			// Order to store metal
#define	ORDER_ENERGY_S		0x07			// Order to store energy

#define NB_ORDERS			0x8

#define BRAIN_VALUE_NULL	0x0
#define BRAIN_VALUE_LOW		0x1
#define BRAIN_VALUE_MEDIUM	0x2
#define BRAIN_VALUE_HIGH	0x4
#define BRAIN_VALUE_MAX		0x8

#define BRAIN_VALUE_BITS	0x4			// How many bits are needed to store a value in a neural network ?

#define AI_UNIT_TYPE_BUILDER	0x0
#define AI_UNIT_TYPE_FACTORY	0x1
#define AI_UNIT_TYPE_ARMY		0x2
#define AI_UNIT_TYPE_DEFENSE	0x3
#define AI_UNIT_TYPE_ENEMY		0x4
#define AI_UNIT_TYPE_METAL		0x5
#define AI_UNIT_TYPE_ENERGY		0x6

#define NB_AI_UNIT_TYPE			0x7

#define AI_FLAG_BUILDER		0x01
#define AI_FLAG_FACTORY		0x02
#define AI_FLAG_ARMY		0x04
#define AI_FLAG_DEFENSE		0x08
#define AI_FLAG_METAL_P		0x10			// Producers
#define AI_FLAG_ENERGY_P	0x20
#define AI_FLAG_METAL_S		0x40			// Storage
#define AI_FLAG_ENERGY_S	0x80

    class AI_WEIGHT
    {
    public:
        uint16			nb;						// Number of units of this type the AI has
        float			w;						// Weight given to this unit
        float			o_w;					// Remember w for possible changes
        byte			type;					// Builder, factory, army, defense, ...
        std::list<uint16> built_by;				// Who can build it
        float			army;
        float			defense;
        float			metal_p;
        float			energy_p;
        float			metal_s;
        float			energy_s;

        AI_WEIGHT() : built_by()
        {
            nb = 0;
            w = 0.0f;
            o_w = 0.0f;
            type = 0;
            army = 0.0f;
            defense = 0.0f;
            metal_p = 0.0f;
            energy_p = 0.0f;
            metal_s = 0.0f;
            energy_s = 0.0f;
        }

        ~AI_WEIGHT()
        {
            built_by.clear();
        }
    };

    class WEIGHT_COEF
    {
    public:
        uint16	idx;
        uint32	c;

        WEIGHT_COEF()	{	idx = 0;	c = 0;	}
        WEIGHT_COEF( uint16 a, uint32 b )	{	idx = a;	c = b;	}
    };

    inline bool operator<( WEIGHT_COEF &a, WEIGHT_COEF &b )		{	return a.c > b.c;	}

#define	AI_TYPE_EASY		0x0
#define AI_TYPE_MEDIUM		0x1
#define AI_TYPE_HARD		0x2
#define AI_TYPE_BLOODY		0x3

    class AI_PLAYER :	public ObjectSync,			// Class to manage players controled by AI
    public cThread
    {
    public:
        String			name;			// Attention faudrait pas qu'il se prenne pour quelqu'un!! -> indique aussi le fichier correspondant à l'IA (faut sauvegarder les cervelles)
        BRAIN			decide;			// Neural network to take decision
        BRAIN			anticipate;		// Neural network to make it anticipate
        byte			player_id;		// Identifiant du joueur / all is in the name :)
        uint16			unit_id;		// Unit index to run throught the unit array
        uint16			total_unit;

        byte			AI_type;		// Which AI do we have to use?

        AI_WEIGHT		*weights;		// Vector of weights used to decide what to build
        uint16			nb_units[ NB_AI_UNIT_TYPE ];
        uint16			nb_enemy[ 10 ];				// Hom many units has each enemy ?
        float			order_weight[NB_ORDERS];	// weights of orders
        float			order_attack[ 10 ];			// weights of attack order per enemy player
        std::list<uint16>	builder_list;
        std::list<uint16>	factory_list;
        std::list<uint16>	army_list;
        std::vector< std::list<WEIGHT_COEF> > enemy_list;

    private:
        void (*fp_scan_unit)( AI_PLAYER* );

        void (*fp_refresh_unit_weights)( AI_PLAYER* );

        void (*fp_think)( AI_PLAYER*, MAP* );

    protected:
        int			Run();
        void		SignalExitThread();

        bool		thread_running;
        bool		thread_ask_to_stop;

    public:

        void init();

        void destroy();

        AI_PLAYER() : builder_list(), factory_list(), army_list(), enemy_list()
        {
            InitThread();
            init();
        }

        ~AI_PLAYER()
        {
            destroy();
        }

        void monitor();

        void change_name(const String& newName);		// Change AI's name (-> creates a new file)

        void save();

        void load(const String& filename, const int id = 0);

        void scan_unit();

        void refresh_unit_weights();

        void think( MAP *map );
    };


} // namespace TA3D

#endif // TA3D_XX_AI_H__
