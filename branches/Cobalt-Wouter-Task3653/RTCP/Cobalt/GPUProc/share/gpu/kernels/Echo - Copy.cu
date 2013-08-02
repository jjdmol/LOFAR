//# Correlator.cu
//#
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: Echo.cu 25342 2013-06-14 12:11:09Z klijn $

// \file
// This file contains a CUDA implementation of the GPU kernel for the
// correlator. It computes correlations between all pairs of stations
// (baselines) and X,Y polarizations, including auto-correlations.

#include <cuda.h>

#include "gpu_math.cuh"
#include "complex.cuh"

typedef LOFAR::Cobalt::gpu::complex<float> fcomplex;

// The external input: Currently input is connected to all neurons,
typedef fcomplex(*ExternalInputsDataType)[NR_INPUTS][NR_TIMESTEPS];
typedef fcomplex(*ExternalInputsWeightsDataType)[NR_INPUTS][NR_NEURONS];
typedef fcomplex(*BaseWeightsDataType)[NR_BASENETWORKS][NR_NEURONS][NR_NEURONS];

// The output: only the activations: The output can be calculated (cheaply??)
typedef fcomplex(*NeuronActivationsDataType)[NR_BASENETWORKS][NR_TIMESTEPS][NR_NEURONS];

extern "C"  __global__ void echo(void *neuronActivationsPtr,
                     const void *externalInputsPtr, 
                     const void *externalInputsWeightsPtr,
                     const void *baseWeightsTypePtr) 
{
  NeuronActivationsDataType neuronActivations = (NeuronActivationsDataType) neuronActivationsPtr;
  ExternalInputsDataType externalInputs = (ExternalInputsDataType) externalInputsPtr;
  ExternalInputsWeightsDataType externalInputsWeights = (ExternalInputsWeightsDataType) externalInputsWeightsPtr;
  BaseWeightsDataType baseWeights = (BaseWeightsDataType) baseWeightsTypePtr;
  // Local array for activations
  fcomplex neuron_activations[NR_NEURONS]; 
  fcomplex neuron_activations_pref[NR_NEURONS]; 

  for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  {
    neuron_activations[idx_neuron] = fcomplex(0,0);
  }

  for (unsigned idx_timestep = 1; idx_timestep < NR_TIMESTEPS; ++ idx_timestep)
  {
    for (unsigned idx_synaps = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
    {
      // Get the correct neuron activation, this is candidate for the y dimension block
      //fcomplex activation = neuron_activations_pref[idx_neuron]; 
      // external input
      // NEXT STEP: Add the inputs and validate the activations
      fcomplex neuron_activation_deltasum = 0;
      fcomplex neuron_activation_pref = =0;
      for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
      {
        //neuron_activation_pref = neuron_activation[idx_neuron]; // deze bevat dus 
        neuron_activation_delta = (*baseWeights)[0][idx_neuron][idx_synaps] * 
                                    neuron_activations[idx_synaps];
        ////neuron_activation_pref = neuron_activation[idx_neuron]; // deze bevat dus 
        neuron_activation_deltasum += neuron_activation_delta
        neuron_activation[idx_synaps] += neuron_activation_delta;

        // kan ik hier 

        //neuron_activation[idx_neuron] -= neuron_activation_pref; // kan ik met deze de huidige activatie uitrekenen?
        //// Wat als ik hier loop: Dit heb ik ooit eens op papier getekend.
      } 
        // ******************************************************************
        fcomplex neuron_activation_delta = (*baseWeights)[0][idx_neuron][idx_synaps] * 
                                    neuron_activations[idx_synaps];
        //neuron_activation_pref = neuron_activation[idx_neuron]; // deze bevat dus 
        neuron_activation[idx_synaps] += neuron_activation_delta;


        neuron_activation[idx_neuron] -= neuron_activation_pref; // kan ik met deze de huidige activatie uitrekenen?
      }

      // No output function!!
      neuron_activations[idx_neuron] =
          neuron_activation;
    }
    //michien diagonaal over de neuron input heen om de copy van de activiteit te voorkomen?
    for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
    {
      neuron_activations_pref[idx_neuron] = neuron_activations[idx_neuron];
    }
    for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
    {
      (*neuronActivations)[0][idx_timestep][idx_neuron] = neuron_activations[idx_neuron];
    }
    __syncthreads();
  //// Local array for activations
  //fcomplex neuron_activations[NR_NEURONS]; 
  //fcomplex neuron_activations_pref[NR_NEURONS]; 

  //for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  //{
  //  neuron_activations_pref[idx_neuron] = fcomplex(0,0);
  //}

  //for (unsigned idx_timestep = 1; idx_timestep < NR_TIMESTEPS; ++ idx_timestep)
  //{
  //  for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  //  {
  //    // Get the correct neuron activation, this is candidate for the y dimension block
  //    fcomplex neuron_activation = neuron_activations_pref[idx_neuron]; 
  //    // external input
  //    // NEXT STEP: Add the inputs and validate the activations
  //    for (unsigned idx_synaps = 0; idx_synaps < NR_NEURONS; ++ idx_synaps)
  //    {
  //      neuron_activation += (*baseWeights)[0][idx_neuron][idx_synaps] * 
  //                                  neuron_activations_pref[idx_synaps];

  //    }
  //    // No output function!!
  //    neuron_activations[idx_neuron] =
  //        neuron_activation;
  //  }
  //  //michien diagonaal over de neuron input heen om de copy van de activiteit te voorkomen?
  //  for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  //  {
  //    neuron_activations_pref[idx_neuron] = neuron_activations[idx_neuron];
  //  }
  //  for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  //  {
  //    (*neuronActivations)[0][idx_timestep][idx_neuron] = neuron_activations[idx_neuron];
  //  }
  //  __syncthreads();
  }
  //(*neuronActivations)[0][0][0] = fcomplex(20,20);
  //(*neuronActivations)[0][0][0] = (*externalInputs)[0][0];
  return;
}
