/**

\page MAC RTC Calibration Server Documentation

\section intro Introduction

The MAC RTC calibration server software runs on the LCU of a LOFAR
remote station and executes the background calibration algorithm to
calibrate the gains of the LOFAR dual polarized antennas, consisting
of two dipoles, one for x- and one for y-polarization.

This document describes the Calibration server from the viewpoint of
the calibration algorithm designer/developer. After reading this
document you should be able to write your own calibration algorithm
and know how to obtain the input data (parameters) required for the
calibration algorithm.

\section inputdata Input data

The calibration algorithm for the LOFAR remote station needs the
following input data:

\li <b>The Array Correlation Matrix (ACC) for all relevant subbands.</b>\n This
is called the ACC (Array Correlation Cube) and it contains the
correlation matrix for all relevant subbands. The CAL::ACC class represents
this matrix.
\li <b>The subband frequencies.</b>\n In the calibration software
this is represented by the CAL::SpectralWindow class which can be queried to
obtain the actual frequency for a specific subband.
\li <b>A sky model to make an accurate prediction of the input
signal.</b>\n The CAL::Sources class represents the sky model.
\li <b>A model of the beam pattern of the individual antenna
elements.</b>\n The CAL::DipoleModel class represents the beam pattern of a
dipole.
\li <b>The array configuration; the location and orientation of the
antennas in the field.</b>\n The CAL::AntennaArray class represents the array
configuration.

\section outputdata Output data

The goal of a LOFAR remote station calibration algorithm is to
determine the complex electronic gains of the individual dipoles for
each subband.

The result consists of the following data:

\li The complex gain for each dipole (x-, and y-polarization of each
antenna) for each subband.
\li The quality of the calibration for each subband. The quality may
be degraded when interpolation between neighbouring subbands is
required to estimate the gain in a subband that was occupied by RFI.

The CAL::AntennaGains class contains these two result arrays.

\section subarray Sub arrays

Because it should be possible to subdivide the remote station
antenna array into upto four sub arrays the calibration server must
support sub arrays. A sub array is a selection of the antenna elements
from a CAL::AntennaArray for a specific CAL::SpectralWindow. It is
represented by the CAL::SubArray class. The CAL::SubArray is the unit of
calibration. The gains of all antennas of a CAL::SubArray are
calibrated for all subbands of the spectral window associated with the
sub array.

\section calibrationinterface The Calibration Interface class

To implement a calibration algorithm within the calibration server the
developer has to implement a single method:
CAL::CalibrationInterface::calibrate().

The calibrate() method has two input parameters:

\li The sub array to calibrate (CAL::SubArray)
\li The auto correlation cube to use for calibration (CAL::ACC).

..and one output parameter:
\li The calibration result (CAL::AntennaGains)

Access to the other input parameters is done through the methods of
the CAL::CalibrationAlgorithm class:

\li CAL::CalibrationAlgorithm::getSources()
\li CAL::CalibrationAlgorithm::getDipoleModel()

The CAL::CalibrationAlgorithm class is a sub class of
CAL::CalibrationInterface. The developer should create a sub class of
CAL::CalibrationAlgorithm and implement the
CAL::CalibrationInterface::calibrate() method and use the methods of
CAL::CalibrationAlgorithm to get access to the source catalog and the
dipole model.

\section rscal Implementation template: CAL::RemoteStationCalibration

To provide a running start to the implementation of a remote station
calibration algorithm the CAL::RemoteStationCalibration class has been
written which derives from CAL::CalibrationAlgorithm with a template
implementation of the CAL::CalibrationInterface::calibrate() method.

The class implementation is contained in two files:

\li RemoteStationCalibration.h
\li RemoteStationCalibration.cc

The calibrate method demonstrates access to parameters through the
CAL::CalibrationAlgorithm methods and shows an example of what the
main loop over the subbands might look like.

\code

void RemoteStationCalibration::calibrate(const SubArray&    subarray,
                                         const ACC&         acc,
                                         AntennaGains& gains)
{
  const SpectralWindow&   spw = subarray.getSPW();        // get spectral window
  const Array<double, 3>& pos = subarray.getAntennaPos(); // get antenna positions

  const DipoleModel&   dipolemodel = getDipoleModel();    // get dipole model
  const Sources&       sources     = getSources();        // get sky model

  cout << "calibrate: spectral window name=" << spw.getName() << endl;
  cout << "calibrate: subband width=" << spw.getSubbandWidth() << " Hz" << endl;
  cout << "calibrate: num_subbnads=" << spw.getNumSubbands() << endl;
  cout << "calibrate: subarray name=" << subarray.getName() << endl;
  cout << "calibrate: num_antennas=" << subarray.getNumAntennas() << endl;

  find_rfi_free_channels();
  for (int sb = 0; sb < spw.getNumSubbands(); sb++)
    {
      Timestamp acmtime;
      const Array<complex<double>, 4> acm = acc.getACM(sb, acmtime);

      localsource = make_local_sky_model(sources, acmtime);
      R0 = make_model_ACM(localsources, dipolemodel, );
      compute_gains(acm, R0, pos, spw.getSubbandFreq(sb), Rtest, result);
      compute_quality(Rtest, sb, result);
    }
  interpolate_bad_subbands();
   
  result.setDone(true); // when finished
}

\endcode

\section blitz Blitz++ Arrays

Various classes contain array classes based on the Blitz++ array
template package. A Blitz++ array is declared using the element type
and number of dimensions. The following example declares a four
dimensional array of complex doubles with dimensions 96 x 96 x 2 x 2.

\code
Array<complex<double>, 4> acm(96, 96, 2, 2);
\endcode

For more specific information on how to use Blitz++ the reader is
referred to the Blitz++ documentation:
http://www.oonumerics.org/blitz/manual .

The following arrays are of concern to the calibration algorithm
developer. For the dimensions of the arrays we assume a scenario with
96 dual polarized (2) antennas, 512 subbands and a dipole model with
50 x 50 (l,m) grid and 24 frequency points. There are 300 sources in
the source catalog.

\li Array of antenna positions obtained using the
CAL:AntennaArray::getAntennaPos() method. This is a 3 dimensional
array of doubles. Dimensions are 96 antennas x 2 polarizations x
3d-position vector.
\code
Array<double, 3> pos(96, 2, 3);
\endcode
\li Array correlation cube returned by CAL::ACC::getACC(). The ACC
array is a 5 dimensional array of complex doubles. Dimensions are 96
antennas x 96 antennas x 512 subbands x 2 polarizations x 2
polarizations.
\code
Array<complex<double>, 5> acc(96, 96, 512, 2, 2);
\endcode
\li Timestamps for each subband slices of the array correlation
cube. Obtained with the CAL::ACC::getACM() method. This is a
1-dimensional array of Timestamps.
\code
Array<Timestamp, 1> subband_time(512);
\endcode
\li Dipole model returned by CAL::DipoleModel::getModel(). This is a
four dimensional array of complex doubles. Dimensions are 2 (phi,
theta) x 50 (l) x 50 (m) x 24 (frequency points).
\code
Array<complex<double>, 4> sens(2, 50, 50, 24);
\endcode
\li The resulting gains for each antenna element and subband. This is
2-dimensional array of complex doubles. Dimensions are 96 antennas x 2
polarizations x 512 subbands. This array is returned by the
CAL::AntennaGains::getGains() method.
\code
Array<complex<double>, 3> gains(96, 2, 512);
\endcode
\li Quality measure for the calibration of each subband. This is a
1-dimensional array of doubles. Extent of the array is 512
subbands. This array is returned by the
CAL::AntennaGains::getQuality() method.
\code
Array<double, 1> quality(512);
\endcode
\li The source positions. This is a 2-dimensional array of
doubles. The dimensions are 300 sources x 3d position vector. This
array is returned by the CAL::Sources::getSourcePositions()
method.
\code
Array<double, 2> skymodel(300, 3);
\endcode

\section files Input data files

The current implementation of the calibration server loads all input
data arrays (see \ref blitz) for the calibration algorithm from input
files.

There are two types of files:

\li ParameterSet file. These files contains key value pairs separated
by the equal sign (=). 
\li Plain ASCII format containing an array in Blitz++ format and
possibly other values. Line-breaks are allowed to spread the Blitz++
array over multiple lines.

The following input files are required to run a calibration algorithm:

\li <b>\c DipoleModel.conf </b>: Dipole sensitivity model as Blitz++ array.
\li <b>\c Antennas.conf </b>: Array configuration (antenna locations) as
two Blitz++ arrays. One for low band antennas and one for high band
antennas.
\li <b>\c ACC.conf </b>: Full array correlation cube.
\li <b>\c SourceCatalog.conf </b>: Source catalog. Key-value file with
astronomical source positions and fluxes for one or more frequencies.

The format of each of these files will be described individually in
the following sections.

\subsection dipolemodel DipoleModel.conf format

This file contains the model of the dipole sensitivity. These are two
3-dimensional arrays (modelled as one 4-dimensional array). One array
for E_phi and one for E_theta. The dimensions are 2 (E_phi,E_theta) x
l x m x freq. The current idea is to have a frequency plane every
10MHz. This results in 240MHz / 10MHz = 24 frequency planes. For (l,m)
resultion a size of 50 x 50 seems appropriate. The actual dimensions
of this array will therefor be: 2 x 50 x 50 x 24 of complex doubles.

<em>Note that complex numbers in Blitz++ array are written as "(Re,Im)".</em>

\code
DipoleModel.sens= 2 x 50 x 50 x 24 [ (0,0) (0,0) ... 120000 complex numbers ... (0,0) ]
\endcode

\subsection antennas Antennas.conf format

This file contains the 3-d positions of the antennas. Although the
position of the x- and y-polarization will most likely be the same,
in this file they are specified separately.

The positions of the low-band antennas are specified with the
LBA_POSITIONS key and for the high-band antennas with the
HBA_POSITIONS key. Each is specified with a Blitz++ array with
dimensions: 96 antennas x 2 polarizations x 3-dimensional
coordinates.
\code
LBA_POSITIONS= 96 x 2 x 3 [ 0 0 0  0 0 0  3    0 0  3    0 0 ... ]
HBA_POSITIONS= 96 x 2 x 3 [ 0 0 0  0 0 0  1.25 0 0  1.25 0 0 ... ]
\endcode

\subsection acc ACC.conf format

The array correlation matrices for all subbands are contained in the
Array Correlation Cube. The ACC.conf file contains this cube as a
Blitz++ array of complex doubles. The dimensions of the cube are:
96 antennas x 96 antennas x 512 subbands x 2 polarizations x 2
polarizations.

\code
96 x 96 x 512 x 2 x 2 [ (0,0) (0,0) ... (0,0 ]
\endcode

\subsection skymodel SourceCatalog.conf format

The source catalog is defined in the SourceCatalog.conf file. This
file contains 4 lines per source. The four lines are:
\li Line 1: The name of the source (type string)
\li Line 2: The right-ascensions (RA) of the source (type double)
\li Line 3: The declination (DEC) of the source (type double)
\li Line 4: The frequency flux at various frequencies (type Blitz++
array)

The dimensions of the Blitz++ array are: nfrequencies x 2. The array
contains pairs of doubles. The first value is the frequency, the
second value the flux at that frequency.

<em>Note: The SourceCatalog.conf file must have exactly four
lines per source (with a total of 4 x #sources) and must not contain any
empty lines or comment lines.</em>

\code
source 1
0
1
10 x 2 [ 100e6 20.0 110e6 20.0 ]
source 2
1
2
10 x 2 [ 100e6 10 105e6 12 ]
\endcode

\section development Developing a calibration algorithm

To develop a new calibration algorithm the
CAL::RemoteStationCalibration class implementation must completed. A
skeleton implementation is provided in the source code. To build the
source the developer needs to have a LOFAR build environment.

\subsection buildenv Setting up the LOFAR build environment

The calibration server software requires the following LOFAR
packages. The corresponding CVS tag is mentioned in brackets.

\li LOFAR/autoconf_share  (autoconf_share-2_1)
\li LOFAR/LCS/Common      (LCS-Common-2_1)
\li LOFAR/LCS/Transport   (LCS-Transport-1_1)
\li LOFAR/MAC             (MAC-GCF-5_0)
\li LOFAR/MAC/APL         (HEAD)

To check out these packages from CVS setup CVSROOT for example in
~/.bashrc.
\code
export CVSROOT=:pserver:wierenga@cvs.astron.nl:/cvs/cvsroot
\endcode

Issue the \c cvs \c login command to login to CVS. Then check out the
packages using the following command:
\code
# general
cvs co -r <tag> <package>
# for example
cvs co -r LCS-Common-2_1 LOFAR/LCS/Common
\endcode

\subsection compile Compiling the source

Each package is configured and compiled with the same set of
commands. The following fragment shows how the \c LCS/Common package
is configured and compiled. The directory in which the command should
be executed is shown in the prompt.

\code
# build and install LCS/Common
LOFAR/LCS/Common > ./bootstrap
LOFAR/LCS/Common > mkdir -p build/gnu_debug
LOFAR/LCS/Common/build/gnu_debug > ../../lofarconf && make install
\endcode

Follow the same pattern to configure and build the all packages in the
order shown below.

\code
LOFAR/LCS/Common
LOFAR/LCS/Transport
LOFAR/MAC/APL/TestSuite
LOFAR/MAC/GCF/GCFCommon
LOFAR/MAC/GCF/TM
LOFAR/MAC/GCF/Protocols
LOFAR/MAC/GCF/PALlight
LOFAR/MAC/APL/PAC/RSPDriver
LOFAR/MAC/APL/PAC/Calibration
\endcode

\subsection running Running the code

The \c make \c install command installs the compiled packages in the
\c LOFAR/installed/gnu_debug directory. The input configuration files
are installed in the \c LOFAR/installed/gnu_debug/etc directory. The
\c CalServer executable is installed in \c
LOFAR/installed/gnu_debug/bin. To run the CalServer it needs to have
the \c *.conf files to be in the \c bin directory. The simplest way to
achieve this is to create symbolic links from the \c etc directory to
the \c bin directory like this:

\code
LOFAR/installed/gnu_debug/bin > ln -s ../etc/\*.conf .
\endcode

An example output of running the CalServer is show below.
\code
> cd LOFAR/installed/gnu_debug/bin
> ./CalServer
050413 094943,723 [single] TRACE  TRC - TRACE module activated
13-04-05 09:49:43 INFO  APL.PAC.Calibration - Program ./CalServer has started [tion/src/CalServer.cc:372]
calibrate: spectral window name="80"
calibrate: subband width=156250 Hz
calibrate: num_subbnads=512
calibrate: subarray name=FTS-1
calibrate: num_antennas=96
13-04-05 09:51:08 INFO  APL.PAC.Calibration - CalServer execution time: 85 seconds [tion/src/CalServer.cc:381]
13-04-05 09:51:08 INFO  APL.PAC.Calibration - Normal termination of program [tion/src/CalServer.cc:399]
\endcode

\subsection impl Implementing CAL::RemoteStationCalibration::calibrate()

It is now up to you as developer to implement the
CAL::RemoteStationCalibration::calibrate() method to perform the
calibration. The source for this class can be found in \c
LOFAR/MAC/APL/PAC/Calibration/src. To re-build the CalServer go to \c
LOFAR/MAC/APL/PAC/Calibration/build/gnu_debug and type \c make \c
install.

That's all!

*/
