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

#ifndef __BRAIN_H__
#define __BRAIN_H__

#include <vfs/vfs.h>

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

        void init();
        void destroy();
        BRAIN();
        ~BRAIN();

        void build(int nb_in,int nb_out,int rg);			// Create the neural network

        void active_neuron(int i);

        float *work(float entry[],bool seuil = false);			// Make NEURONs work and return the network results

        void mutation();			// Make some changes to the neural network

        void learn(float *result,float coef = 1.0f);		// Make it learn

        void save(QIODevice *file);		// Save the network

        int load(QIODevice *file);		// Load the network
    };

    BRAIN *copy_brain(BRAIN *brain, BRAIN *dst = NULL);		// Make a copy
}

#endif
