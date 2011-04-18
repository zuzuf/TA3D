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

#include <stdafx.h>
#include "brain.h"
#include <misc/math.h>
#include <vfs/file.h>

using namespace TA3D::UTILS;

namespace TA3D
{



	void BRAIN::init()
	{
		nb_neuron = 0;
		neuron = NULL;
		n = p = q = 0;
		n_out = NULL;
	}

	void BRAIN::destroy()
	{
		if (nb_neuron > 0 && neuron)
		{
			for(int i = 0 ; i < nb_neuron ; i++)
				if(neuron[i].weight != NULL)
					DELETE_ARRAY(neuron[i].weight);
			DELETE_ARRAY(neuron);
		}
		DELETE_ARRAY(n_out);
		init();
	}

	BRAIN::BRAIN()
	{
		init();
	}

	BRAIN::~BRAIN()
	{
		destroy();
	}

	void BRAIN::build(int nb_in,int nb_out,int rg)				// Create the neural network
	{
		destroy();
		q = rg;
		nb_neuron = q + nb_in + nb_out;		// Number of layers * number of input NEURONs + number of output NEURONs
		n = nb_in;
		p = nb_out;
		neuron = new NEURON[nb_neuron];
		n_out = new float[nb_out];
		for(int i = 0; i < nb_neuron; ++i)
		{
			neuron[i].var = 0.0f;
			neuron[i].weight = NULL;
			if (i < nb_out)
				n_out[i] = 0.0f;
			if (i >= n && i < nb_neuron - p)
			{
				neuron[i].weight = new float[n];
				for(int e = 0 ; e < n ; ++e)
					neuron[i].weight[e] = float(TA3D_RAND() % 2001) * 0.001f - 1.0f;
			}
			else
			{
				if (i >= n)
				{
					neuron[i].weight = new float[q];
					for(int e = 0 ; e < q ; ++e)
						neuron[i].weight[e] = float(TA3D_RAND() % 2001) * 0.001f - 1.0f;
				}
			}
		}
	}



	void BRAIN::active_neuron(int i)
	{
		if (neuron[i].weight == NULL || i < n)
			return;
		neuron[i].var=0.0f;
		if (i < nb_neuron - p)
		{
			for(int e=0;e<n; ++e)
				neuron[i].var += neuron[e].var * neuron[i].weight[e];
		}
		else
		{
			for(int e = 0; e < q; ++e)
				neuron[i].var += neuron[n+e].var * neuron[i].weight[e];
		}
		neuron[i].var = 1.0f / (1.0f + expf(-neuron[i].var));
	}


	float *BRAIN::work(float entry[],bool seuil)			// Make NEURONs work and return the network results
	{
		if (nb_neuron < 0)
			return NULL;
		int i;
		for(i=0; i < n; ++i) // Prepare the network
			neuron[i].var=entry[i];
		for(i = n; i < nb_neuron; ++i)
			active_neuron(i);
		if (!seuil)
		{
			for(i = 0; i < p; ++i) // get the result
				n_out[i] = neuron[n+q+i].var;
		}
		else
		{
			for(i=0;i<p; ++i) // get the result
				n_out[i] = (neuron[n+q+i].var >= 0.5f) ? 1.0f : 0.0f;
		}
		return n_out;
	}


	void BRAIN::mutation()			// Make some changes to the neural network
	{
		int index = (TA3D_RAND() % (nb_neuron-n)) + n;
		int mod_w = 0;
		if (index < nb_neuron - p)
			mod_w=TA3D_RAND() % n;
		else
			mod_w = TA3D_RAND() % q;
		neuron[index].weight[mod_w] += float((TA3D_RAND() % 200001) - 100000) * 0.00001f;
	}


	void BRAIN::learn(float *result,float coef)		// Make it learn
	{
		for(int i=0;i<p; ++i)
			n_out[i] = (result[i]-n_out[i]) * (n_out[i]+0.01f) * (1.01f-n_out[i]);

		float *diff = new float[q];
		for(int i=0;i<q;++i)
			diff[i]=0.0f;

		for(int i=0;i<p;++i)		// Output layer
		{
			for(int e=0;e<q;e++)
			{
				diff[e]+=(n_out[i]+0.01f)*(1.01f-n_out[i])*neuron[n+q+i].weight[e]*neuron[n+e].var;
				neuron[n+q+i].weight[e]+=coef*n_out[i]*neuron[n+e].var;
			}
		}

		// Middle layers
		for(int i=0;i<q;++i)
		{
			for(int e=0;e<n;e++)
				neuron[n+i].weight[e]+=coef*diff[i]*neuron[e].var;
		}
		DELETE_ARRAY(diff);
	}

	void BRAIN::save(Yuni::Core::IO::File::Stream *file)		// Save the neural network
	{
		file->write("BRAIN",5);			// File format ID
		file->write((char*)&n,sizeof(int));		// Inputs
		file->write((char*)&p,sizeof(int));		// Outputs
		file->write((char*)&q,sizeof(int));		// Size of middle layer

		for(int i = n ; i < nb_neuron ; ++i)		// Save weights
		{
			if (i < n + q)
				file->write((char*)neuron[i].weight, sizeof(float) * n);
			else
				file->write((char*)neuron[i].weight, sizeof(float) * q);
		}
	}

	int BRAIN::load(File *file)		// Load the neural network
	{
		char tmp[6];

		file->read(tmp, 5); // File format ID
        tmp[5] = 0;
		if (strcmp(tmp,"BRAIN") != 0)	// Check if it is what is expected
			return 1;

		destroy();		// clean the object

		file->read(n);		// Inputs
		file->read(p);		// Outputs
		file->read(q);		// Size of middle layer
        nb_neuron = p + q + n;

		neuron = new NEURON[nb_neuron];
		n_out = new float[p];

        for(int i = 0 ; i < p ; ++i)
            n_out[i] = 0.0f;

        for(int i = 0 ; i < n ; ++i)
			neuron[i].weight = NULL;

        for(int i = n ; i < nb_neuron ; ++i)		// Read weights
		{
			if (i<n+q)
			{
				neuron[i].weight = new float[n];
				file->read(neuron[i].weight, (int)sizeof(float) * n);
			}
			else
			{
				neuron[i].weight = new float[q];
				file->read(neuron[i].weight, (int)sizeof(float) * q);
			}
		}
		return 0;
	}



	BRAIN *copy_brain(BRAIN *brain,BRAIN *dst)			// Make a copy
	{
		BRAIN *copy=dst;
		if (copy==NULL)
			copy = new BRAIN;
		copy->init();
		copy->nb_neuron=brain->nb_neuron;
		copy->n=brain->n;
		copy->p=brain->p;
		copy->q=brain->q;
		copy->neuron = new NEURON[copy->nb_neuron];
		copy->n_out = new float[copy->p];
		for(int i=0;i<brain->nb_neuron; ++i)
		{
			if (i<brain->p)
				copy->n_out[i]=0.0f;
			if (brain->neuron[i].weight==NULL)
				copy->neuron[i].weight=NULL;
			else
			{
				if (i>=copy->n && i<copy->nb_neuron-copy->p)
				{
					copy->neuron[i].weight = new float[brain->q];
					for(int e=0;e<brain->q;e++)
						copy->neuron[i].weight[e]=brain->neuron[i].weight[e];
				}
				else
				{
					if (i>=copy->n)
					{
						copy->neuron[i].weight = new float[brain->n];
						for(int e=0;e<brain->n;e++)
							copy->neuron[i].weight[e]=brain->neuron[i].weight[e];
					}
					else
						copy->neuron[i].weight=NULL;
				}
			}
		}
		return copy;
	}
}
