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
|                                      ia.h                                    |
|       Ce module est responsable de l'intelligence artificielle               |
|                                                                              |
\-----------------------------------------------------------------------------*/

#ifndef ARTIFICIAL_INTELLIGENCE
#define ARTIFICIAL_INTELLIGENCE

struct NEURON
{
	float	var;			// Variable pour les opérations
	float	*weight;		// Poids des différents neurones sources
};

class BRAIN		// Réseau de NEURON à n entrées et p sorties
{
public:
	int		nb_neuron;		// Nombre de NEURON
	NEURON	*neuron;		// Tableau de NEURON
	int		n;				// Nombre d'entrées dans le réseau
	int		p;				// Nombre de sorties du réseau
	int		q;				// Taille des rangs
	float	*n_out;			// Tableau de sortie

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

	void build(int nb_in,int nb_out,int rg)				// Crée le réseau de neurones
	{
		destroy();
		q=rg;
		nb_neuron=q+nb_in+nb_out;		// Nombre de couches x nombre d'entrées + nombre de sorties
		n=nb_in;
		p=nb_out;
		neuron=(NEURON*) malloc(sizeof(NEURON)*nb_neuron);
		n_out=(float*) malloc(sizeof(float)*nb_out);
		for(int i=0;i<nb_neuron;i++) {
			neuron[i].var=0.0f;
			neuron[i].weight=NULL;
			if(i<nb_out)
				n_out[i]=0.0f;
			if(i>=n && i<nb_neuron-p) {
				neuron[i].weight=(float*) malloc(sizeof(float)*n);
				for(int e=0;e<n;e++)
					neuron[i].weight[e]=(TA3D_RAND()%2001)*0.001f-1.0f;
				}
			else if(i>=n) {
				neuron[i].weight=(float*) malloc(sizeof(float)*q);
				for(int e=0;e<q;e++)
					neuron[i].weight[e]=(TA3D_RAND()%2001)*0.001f-1.0f;
				}
			}
	}

	inline void active_neuron(int i)
	{
		if(neuron[i].weight==NULL)	return;
		if(i<n)
			return;
		neuron[i].var=0.0f;
		if(i<nb_neuron-p)
			for(int e=0;e<n;e++)
				neuron[i].var+=neuron[e].var*neuron[i].weight[e];
		else
			for(int e=0;e<q;e++)
				neuron[i].var+=neuron[n+e].var*neuron[i].weight[e];
		neuron[i].var=1.0f/(1.0f+exp(-neuron[i].var));
	}

	inline float *work(float entry[],bool seuil=false)			// Fait bosser un peu le réseau de NEURON et renvoie les valeurs calculées par un NEURON
	{
		if(nb_neuron<0)	return NULL;		// Pas de NEURON à faire bosser
		int i;
		for(i=0;i<n;i++)		// Prépare le réseau au calcul
			neuron[i].var=entry[i];
		for(i=n;i<nb_neuron;i++)
			active_neuron(i);
		if(!seuil)
			for(i=0;i<p;i++)		// Récupère le résultat du calcul
				n_out[i]=neuron[n+q+i].var;
		else
			for(i=0;i<p;i++)		// Récupère le résultat du calcul
				n_out[i]=neuron[n+q+i].var>=0.5f ? 1.0f : 0.0f;
		return n_out;
	}

	inline void mutation()			// Déclenche une mutation dans le réseau de neurones
	{
		int index=(TA3D_RAND()%(nb_neuron-n))+n;
		int mod_w=0;
		if(index<nb_neuron-p)	mod_w=TA3D_RAND()%n;
		else mod_w=TA3D_RAND()%q;
		neuron[index].weight[mod_w]+=((TA3D_RAND()%200001)-100000)*0.00001f;
	}

	inline void learn(float *result,float coef=1.0f)		// Corrige les défauts
	{
		for(int i=0;i<p;i++)
			n_out[i]=(result[i]-n_out[i])*(n_out[i]+0.01f)*(1.01f-n_out[i]);

		float *diff = new float[q];
		for(int i=0;i<q;i++)
			diff[i]=0.0f;

		for(int i=0;i<p;i++)		// Neurones de sortie
			for(int e=0;e<q;e++) {
				diff[e]+=(n_out[i]+0.01f)*(1.01f-n_out[i])*neuron[n+q+i].weight[e]*neuron[n+e].var;
				neuron[n+q+i].weight[e]+=coef*n_out[i]*neuron[n+e].var;
				}

									// Neurones des couches intermédiaires
		for(int i=0;i<q;i++)
			for(int e=0;e<n;e++)
				neuron[n+i].weight[e]+=coef*diff[i]*neuron[e].var;
		delete[] diff;
	}

	void save(FILE *file);		// Enregistre le réseau de neurones

	int load(FILE *file);		// Charge le réseau de neurones
};

BRAIN *copy_brain(BRAIN *brain,BRAIN *dst=NULL);		// Copie un réseau de neurones

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

#define BRAIN_VALUE_BITS	0x4			// Nombre de bits nécessaires pour coder une valeur pour un réseau de neurones

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
	List<uint16>	built_by;				// Who can build it
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

class AI_PLAYER :	protected cCriticalSection,			// Class to manage players controled by AI
		            public cThread
{
public:
	char			*name;			// Attention faudrait pas qu'il se prenne pour quelqu'un!! -> indique aussi le fichier correspondant à l'IA (faut sauvegarder les cervelles)
	BRAIN			decider;		// Réseau de neurones d'analyse de la partie et de décision
	BRAIN			anticiper;		// Réseau de neurones voué à l'analyse et à l'anticipation des mouvements ennemis
	byte			player_id;		// Identifiant du joueur
	uint16			unit_id;		// Identifiant d'unité pour parcourir les unités
	uint16			total_unit;

	byte			AI_type;		// Which AI do we have to use?

private:
	AI_WEIGHT		*weights;	// Vector of weights used to decide what to build
	uint16			nb_units[ NB_AI_UNIT_TYPE ];
	uint16			nb_enemy[ 10 ];				// Hom many units has each enemy ?
	float			order_weight[NB_ORDERS];	// weights of orders
	float			order_attack[ 10 ];			// weights of attack order per enemy player
	List<uint16>	builder_list;
	List<uint16>	factory_list;
	List<uint16>	army_list;
	Vector< List<WEIGHT_COEF> >	enemy_list;

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
		CreateCS();				// Thread safe model

		InitThread();

		init();
	}

	~AI_PLAYER()
	{
		destroy();

		DeleteCS();
	}

	void monitor();

	void change_name(char *new_name);		// Change AI's name (-> creates a new file)

	void save();

	void load(char *filename,int id=0);

	void scan_unit();

	void refresh_unit_weights();

	void think(MAP *map);
};

#endif
