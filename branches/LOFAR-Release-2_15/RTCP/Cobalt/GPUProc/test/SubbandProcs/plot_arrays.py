from pylab import *
from numpy import *

###################################################################
s_complex = 2
s_blocksize = 65536
nPol = 2
nStation = 2
s_totl = 8 * 65536

# Here plots based on pipeline output
# Should display a sinus
range_min = 0
range_max = s_totl #8184
#range_max = 256


npol = 2
nSample = 1024
delayCompChannels = 64
nrHighResolutionChannels = 4096
nrSamplesAtBeamformer = s_blocksize / 4096

# PLotting settings
# Set the collors and alpha value of the two plots
colorCobalt = 'red'
alphaCobalt = 1.0

plot_python = True
colorPython = 'blue'
alphaPython = 0.5

# number visualizatio
nFigVer = 3
nFigHor = 8

#####################################################
# First the raw input
try: 
  # Convert the raw input data to correct arrays
  intToFloatinRaw=fromfile('intToFloatBuffers.input.dat', int16)
  subplot(nFigVer,nFigHor,1)
  intToFloatinRaw = intToFloatinRaw[:intToFloatinRaw.size / 2]
  # separate the imag and real
  intToFloatinReal=intToFloatinRaw[0::2]
  intToFloatinImag=intToFloatinRaw[1::2]

# seperate the two polarisation (step 2)
  plot(intToFloatinReal[range_min:range_max:2], color=colorCobalt, alpha=alphaCobalt)
  plot(intToFloatinImag[range_min:range_max:2], color=colorPython, alpha=alphaPython)
  title("Raw input (integer)")
except:
  pass
########################################################

############################################################
try: 
  intToFloatOut=fromfile('L0_SB000_BL000_IntToFloatKernel.dat', complex64)
  subplot(nFigVer,nFigHor,2)
  plot(intToFloatOut[range_min:range_max].real,color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):
    # Convert to complex64
    intToFloatOutPython = intToFloatinReal + complex64(1j) * intToFloatinImag

    # Transpose
    intToFloatOutPython = reshape(intToFloatOutPython,
                              (nStation, s_blocksize, nPol))
    intToFloatOutPython = intToFloatOutPython.transpose(0,2,1)
    intToFloatOutPython = reshape(intToFloatOutPython, (nStation * s_blocksize * nPol))

    plot(intToFloatOutPython[range_min:range_max].real, color=colorPython, alpha=alphaPython)

  title("input to float")
except:
  pass
##########################################################

##########################################################
try:
  firstFFTShift=fromfile('firstFFTShiftBuffers.output.dat', complex64)
  subplot(nFigVer,nFigHor,3)
  plot(firstFFTShift[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):
    firstFFTShiftpython = copy(intToFloatOutPython)
    for x in range(1,firstFFTShiftpython.size,2):
      firstFFTShiftpython[x] = -1 * firstFFTShiftpython[x]
  
    plot(firstFFTShiftpython[range_min:range_max].real,
       color=colorPython, alpha=alphaPython)

  title("firstFFTShift")
except:
  pass
#############################################################

##########################################################
try:
  firstFFT=fromfile('firstFFT.output.dat', complex64)
  subplot(nFigVer,nFigHor,4)
  plot(firstFFT[range_min:range_max], color=colorCobalt, alpha=alphaCobalt)


  # Perform a repeated fft over the complete input
  if (plot_python):
    firstFFTPython = zeros(0, dtype=complex64)
    for idx in range(firstFFTShiftpython.size / delayCompChannels): # deze stap gaat nog verkeerd
      firstFFTPython = append(firstFFTPython, 
          fft.fft(firstFFTShiftpython[idx*delayCompChannels:idx*delayCompChannels + delayCompChannels]))


    plot(firstFFTPython[range_min:range_max], color=colorPython, alpha=alphaPython)
  
  title("first FFT")
except:
  pass
##########################################################

##########################################################
try:
  delayCompensationBuffers=fromfile('delayCompensationBuffers.output.dat', complex64)
  subplot(nFigVer,nFigHor,5)
  plot(delayCompensationBuffers[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)

  delayCompensationBuffersPython = copy(firstFFTPython)

  if (plot_python):
  
    delayCompensationBuffersPython = reshape(firstFFTPython, 
                                           (nStation, nPol, 
                                            s_blocksize / delayCompChannels,
                                            delayCompChannels))
    delayCompensationBuffersPython = delayCompensationBuffersPython.transpose(
                        (0,1,3,2))

    delayCompensationBuffersPython = reshape(delayCompensationBuffersPython,
                                           (nStation * nPol * s_blocksize))

  plot(delayCompensationBuffersPython[range_min:range_max].real, color=colorPython, alpha=alphaPython)

  title("delaycompensation")
except:
  pass
##########################################################

##########################################################
try:
  secondFFTShift=fromfile('secondFFTShiftBuffers.output.dat', complex64)
  subplot(nFigVer,nFigHor,6)
  plot(secondFFTShift[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):
    secondFFTShiftpython = copy(delayCompensationBuffersPython)
    for x in range(1,secondFFTShiftpython.size,2):
      secondFFTShiftpython[x] = -1 * secondFFTShiftpython[x]
  
    plot(secondFFTShiftpython[range_min:range_max].real,
       color=colorPython, alpha=alphaPython)
           
  title("secondFFTShift")
except:
  pass
##########################################################

##########################################################
try:
  secondFFT=fromfile('secondFFT.output.dat', complex64)
  subplot(nFigVer,nFigHor,7)
  plot(secondFFT[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)

  # Perform a repeated fft over the complete input

  channelsSecond = nrHighResolutionChannels / delayCompChannels
  if (plot_python):
    secondFFTPython = zeros(0, dtype=complex64)
    for idx in range(secondFFTShiftpython.size / channelsSecond):
      secondFFTPython = append(secondFFTPython, 
          fft.fft(secondFFTShiftpython[idx*channelsSecond:idx*channelsSecond + channelsSecond]))

    plot(secondFFTPython[range_min:range_max], color=colorPython, alpha=alphaPython)      
      
  title("secondFFT")
except:
  pass
##########################################################

##########################################################
try:
  delayAndBandPass=fromfile('L0_SB000_BL000_BandPassCorrectionKernel.dat', complex64)
  subplot(nFigVer,nFigHor,8)
  plot(delayAndBandPass[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)
  if (plot_python):
    nchan2 = (nrHighResolutionChannels / delayCompChannels)
    nlocalsamples = nSample / nchan2
    delayAndBandPassPython = reshape(secondFFTPython, 
                     (nStation, nPol, delayCompChannels, nlocalsamples,
                      nchan2))

    delayAndBandPassPython = delayAndBandPassPython.transpose(
                             (0,2,4,3,1))
    delayAndBandPassPython = reshape(delayAndBandPassPython, 
                     (nStation * nPol * delayCompChannels * nlocalsamples * nchan2))
 
    plot(delayAndBandPassPython[range_min:range_max], color=colorPython, alpha=alphaPython)                

  title("BandPassCorrection")
except:
  pass
##########################################################

########################################################
# Coherent stokes buffers
##########################################################
try:
  beamFormer=fromfile('L0_SB000_BL000_BeamFormerKernel.dat', complex64)
  subplot(nFigVer,nFigHor,10)
  plot(beamFormer[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):
    beamFormerPython = add(delayAndBandPassPython[:delayAndBandPassPython.size/2], 
                        delayAndBandPassPython[delayAndBandPassPython.size/2:])

    beamFormerPython = append(beamFormerPython, beamFormerPython)
    beamFormerPython = reshape(beamFormerPython,(nStation, nrHighResolutionChannels , nrSamplesAtBeamformer ,nPol))
    beamFormerPython = beamFormerPython.transpose((1,2,0,3))
    beamFormerPython = reshape(beamFormerPython, (nStation * nrHighResolutionChannels* nrSamplesAtBeamformer * nPol))

    plot(beamFormerPython[range_min:range_max].real, color=colorPython, alpha=alphaPython)
    
    title("beamFormer")
except:
  pass
##########################################################

 
##########################################################

try:
  coherenttranspose=fromfile('coherentTransposeBuffers.output.dat', complex64)
  subplot(nFigVer,nFigHor,11)
  plot(coherenttranspose[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):
    coherenttransposePython = reshape(beamFormerPython,(nrHighResolutionChannels , nrSamplesAtBeamformer ,2 , 2))
    coherenttransposePython = coherenttransposePython.transpose((2,3,1,0))
    coherenttransposePython = reshape(coherenttransposePython, (2 * nrHighResolutionChannels* nrSamplesAtBeamformer * 2))
    plot(coherenttransposePython[range_min:range_max].real, color=colorPython, alpha=alphaPython)

  title("coherenttransposebuffers")
except:
  pass
##########################################################

##########################################################

try:
  inverseFFTBuffers=fromfile('inverseFFTBuffers.output.dat', complex64)
  subplot(nFigVer,nFigHor,12)
  plot(inverseFFTBuffers[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)


  if (plot_python):
    inverseFFTPython = zeros(0, dtype=complex64)
    for idx in range(coherenttransposePython.size / nrHighResolutionChannels):
      inverseFFTPython = append(inverseFFTPython, 
          fft.ifft(coherenttransposePython[
             idx*nrHighResolutionChannels:idx*nrHighResolutionChannels + nrHighResolutionChannels]))

    inverseFFTPython *= nrHighResolutionChannels  # undo nupy scaling 
    plot(inverseFFTPython[range_min:range_max].real, color=colorPython, alpha=alphaPython)

  title("inverseFFTBuffers")
except:
  pass
##########################################################

##########################################################
try:
  inverseFFTShift=fromfile('inverseFFTShift.output.dat', complex64)
  subplot(nFigVer,nFigHor,13)
  plot(inverseFFTShift[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)


  if (plot_python):
    inverseFFTShiftPython = copy(inverseFFTPython)
    for x in range(1,inverseFFTShiftPython.size,2):
      inverseFFTShiftPython[x] = -1 * inverseFFTShiftPython[x]
  
    plot(inverseFFTShiftPython[range_min:range_max].real,
         color=colorPython, alpha=alphaPython)

  title("inverseFFTShift")
except:
  pass
##########################################################


##########################################################
try:
  CoherentStokesKernel=fromfile('L0_SB000_BL000_CoherentStokesKernel.dat', float32)
  subplot(nFigVer,nFigHor,14)
  plot(CoherentStokesKernel[range_min:range_max/4], color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):
    CoherentStokesKernelInputPython = reshape(inverseFFTShiftPython, (2,2,65536,1))
    nTabs = 2
    CoherentStokesKernelPython = zeros((2,1,65536,1), dtype=float32)
    for idxTab in range(nTabs):
      for idxSample in range(65536):
        x = CoherentStokesKernelInputPython[idxTab][0][idxSample][0]
        y = CoherentStokesKernelInputPython[idxTab][1][idxSample][0]

        Px = x.real * x.real + x.imag * x.imag
        Py = y.real * y.real + y.imag * y.imag
        CoherentStokesKernelPython[idxTab][0][idxSample][0] = Px + Py

    CoherentStokesKernelPython = reshape(CoherentStokesKernelPython, (2 * 65536))
    plot(CoherentStokesKernelPython[range_min:range_max], color=colorPython, alpha=alphaPython)

  title("CohSt")
except:
  pass
##########################################################

##########################################################
# Inchoherent buffers
##########################################################
try:
  incTranspose=fromfile(
               'L0_SB000_BL000_IncoherentStokesTransposeKernel.dat',
               complex64)
  subplot(nFigVer,nFigHor,18)
  plot(incTranspose[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):

    incTransposePython = reshape(delayAndBandPassPython,(nStation, nrHighResolutionChannels , nrSamplesAtBeamformer ,nPol))
    incTransposePython = incTransposePython.transpose((0,3,2,1))
    incTransposePython = reshape(incTransposePython, (nStation * nrHighResolutionChannels* nrSamplesAtBeamformer * nPol))

    plot(incTransposePython[range_min:range_max].real, color=colorPython, alpha=alphaPython)
  

  title("Transpose")
except:
  pass
##########################################################

##########################################################
try:
  IncInverseFFT=fromfile('incoherentInverseFFTShiftBuffers.input.dat', complex64)
  subplot(nFigVer,nFigHor,19)
  plot(IncInverseFFT[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)


  if (plot_python):
    IncInverseFFTPython = zeros(0, dtype=complex64)
    for idx in range(incTransposePython.size / nrHighResolutionChannels):
      IncInverseFFTPython = append(IncInverseFFTPython, 
          fft.ifft(incTransposePython[
             idx*nrHighResolutionChannels:idx*nrHighResolutionChannels + nrHighResolutionChannels]))

    IncInverseFFTPython *= nrHighResolutionChannels  # undo nupy scaling 
    plot(IncInverseFFTPython[range_min:range_max].real, color=colorPython, alpha=alphaPython)

  title("IncInverseFFT")
except:
  pass
##########################################################

##########################################################
try:
  incInverseFFTShift=fromfile('incoherentInverseFFTShiftBuffers.output.dat', complex64)
  subplot(nFigVer,nFigHor,20)
  plot(incInverseFFTShift[range_min:range_max].real, color=colorCobalt, alpha=alphaCobalt)


  if (plot_python):
    incInverseFFTShiftPython = copy(IncInverseFFTPython)
    for x in range(1,incInverseFFTShiftPython.size,2):
      incInverseFFTShiftPython[x] = -1 * incInverseFFTShiftPython[x]
  
    plot(incInverseFFTShiftPython[range_min:range_max].real,
         color=colorPython, alpha=alphaPython)

  title("incIFFTShift")
except:
  pass
##########################################################

##########################################################
try:
  InCoherentStokesKernel=fromfile('L0_SB000_BL000_IncoherentStokesKernel.dat', float32)
  subplot(nFigVer,nFigHor,21)
  plot(InCoherentStokesKernel[range_min:range_max], color=colorCobalt, alpha=alphaCobalt)

  if (plot_python):
    InCoherentStokesKernelInputPython = reshape(incInverseFFTShiftPython,
                                                (nStation,nPol,65536,1))
    nTabs = 1
    InCoherentStokesKernelPython = zeros((1,65536,1), dtype=float32)
    for idxStat in range(nStation):
      for idxSample in range(65536):
        x = InCoherentStokesKernelInputPython[idxStat][0][idxSample][0]
        y = InCoherentStokesKernelInputPython[idxStat][1][idxSample][0]

        Px = x.real * x.real + x.imag * x.imag
        Py = y.real * y.real + y.imag * y.imag
        InCoherentStokesKernelPython[0][idxSample][0] += Px + Py

    InCoherentStokesKernelPython = reshape(InCoherentStokesKernelPython, (65536))
    plot(InCoherentStokesKernelPython[range_min:range_max], color=colorPython, alpha=alphaPython)

  title("InCohSt")
except:
  pass
##########################################################

show()

 