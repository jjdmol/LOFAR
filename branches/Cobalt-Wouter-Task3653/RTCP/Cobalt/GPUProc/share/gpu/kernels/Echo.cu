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
//# $Id$

// \file
// This file contains a CUDA implementation of the GPU kernel for the
// correlator. It computes correlations between all pairs of stations
// (baselines) and X,Y polarizations, including auto-correlations.

#include <cuda.h>

#include "gpu_math.cuh"
#include "complex.cuh"
#include "curand_kernel.h"

typedef LOFAR::Cobalt::gpu::complex<float> fcomplex;

// The external input: Currently input is connected to all neurons,
typedef fcomplex(*ExternalInputsDataType)[NR_INPUTS][NR_TIMESTEPS];
typedef fcomplex(*ExternalInputsWeightsDataType)[NR_INPUTS][NR_NEURONS];
typedef fcomplex(*BaseWeightsDataType)[NR_BASENETWORKS][NR_NEURONS][NR_NEURONS];

// The output: only the activations: The output can be calculated (cheaply??)
typedef fcomplex(*NeuronActivationsDataType)[NR_BASENETWORKS][NR_TIMESTEPS][NR_SOLUTION_PARALEL][NR_NEURONS];

extern "C"  __global__ void echo(void *neuronActivationsPtr,
                     const void *externalInputsPtr, 
                     const void *externalInputsWeightsPtr,
                     const void *baseWeightsTypePtr,
                     void * globalStatePtr
                     ) 
{

  NeuronActivationsDataType neuronActivations = (NeuronActivationsDataType) neuronActivationsPtr;
  ExternalInputsDataType externalInputs = (ExternalInputsDataType) externalInputsPtr;
  ExternalInputsWeightsDataType externalInputsWeights = (ExternalInputsWeightsDataType) externalInputsWeightsPtr;
  BaseWeightsDataType baseWeights = (BaseWeightsDataType) baseWeightsTypePtr;
  curandState* globalState = (curandState*)globalStatePtr;

  

  // **************************************************************************
  // Get the weights into local memory
  unsigned tid = blockIdx.x * blockDim.x + threadIdx.x;
  __shared__ fcomplex shared_baseWeights[NR_NEURONS][NR_NEURONS];
  // Candidate for loop unrolling:
  for (unsigned idx_copy = 0;
                idx_copy < (NR_NEURONS * NR_NEURONS) / NR_SOLUTION_PARALEL;
                ++idx_copy)
  {
    unsigned idx = idx_copy * NR_SOLUTION_PARALEL + tid;
    
    shared_baseWeights[idx /  NR_NEURONS][idx % NR_NEURONS] = 
       (*baseWeights)[0][idx /  NR_NEURONS][idx % NR_NEURONS];
  }

  //// *******************************************************************************
  // Local array for activations
  fcomplex neuron_activations_pref[NR_NEURONS];   // Candidate for register variable!!
  for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  {
    neuron_activations_pref[idx_neuron] = fcomplex(0,0);
  }

  // *******************************************************************************
  // Random numbers to be added to the default weight matrix
  // Will contain the random numbers to added to weights
  // Maybee use a const array to shift through the weigths
  // depending on the threads idx.
  // 256 weights and 256 threads //  misschien plus NR_NEURONS om deling in the most inner loop te ontweiken.
  curandState_t state = globalState[threadIdx.x];
  unsigned salt = 12;
  curand_init(salt, threadIdx.x,0, &state);  //maybee this can be done better: salt in the randomization

  __shared__ fcomplex weightRandom[NR_NEURONS * NR_NEURONS]; // use 512 complex and shift through them. this alles omition of 
  ////candidate unrolling:
  for (unsigned idx_copy = 0; idx_copy < NR_NEURONS * NR_NEURONS / NR_SOLUTION_PARALEL; ++ idx_copy)
  {
    unsigned idx = idx_copy * NR_SOLUTION_PARALEL + tid;
    // Do not go over the bounds of the random weight matrix
    if ( idx > NR_NEURONS * NR_NEURONS )
      weightRandom[idx] = 
          fcomplex(curand_normal(&state), curand_normal(&state));  //lognormal aslo candidate?
  }
  __syncthreads();

  for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  {
    neuron_activations_pref[idx_neuron] = (*neuronActivations)[0][0][threadIdx.x][idx_neuron];
  }

  // ************************************************************************
  // From here all code is fully parallel and no sync between threads is needed
  for (unsigned idx_timestep = 1; idx_timestep < NR_TIMESTEPS; ++ idx_timestep)
  {
    // Under the assumption that we have 128 threads running
    // either have the first wave create the random numbers or
    // have each thread create a random number.
  
    for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
    {
      // Get the correct neuron activation, including decay
      fcomplex neuron_activation =  neuron_activations_pref[idx_neuron]; 
      neuron_activation *= ACTIVATION_DECAY;
      // external input
      // NEXT STEP: Add the inputs and validate the activations
      unsigned saved_neuron_idx = (idx_neuron + threadIdx.x) * NR_NEURONS ;
      for (unsigned idx_synaps = 0; idx_synaps < NR_NEURONS; ++ idx_synaps)
      {
        neuron_activation += (shared_baseWeights[idx_neuron][idx_synaps] + 
           weightRandom[(saved_neuron_idx + idx_synaps) % (NR_NEURONS * NR_NEURONS)]) *
                             neuron_activations_pref[idx_synaps];
      }


      // No output function!!
      (*neuronActivations)[0][idx_timestep][threadIdx.x][idx_neuron] = neuron_activation;
    }
    for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
    {
      neuron_activations_pref[idx_neuron] = (*neuronActivations)[0][idx_timestep][threadIdx.x][idx_neuron];
    }
  }
  __syncthreads();
  
  return;
}
