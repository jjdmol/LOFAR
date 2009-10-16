# $Id$

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(LofarInit)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project(RTCP)
include(LofarGeneral)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------

# Defines interfaces between the different components within the RTCP product
lofar_add_package(Interface)

# Fast Collective Network Protocol; used on BlueGene only.
lofar_add_package(FCNP)

# Compute-Node Processing: application running on the BlueGene compute nodes
lofar_add_package(CNProc)

# I/O Node Processing: application running on the BlueGene I/O nodes
lofar_add_package(IONProc)

# Stores output from the correlator in one or more MeasurmentSet files.
lofar_add_package(Storage)

# Several RTCP tools
lofar_add_package(RTCPTools)

# Several run scripts to start the RTCP components
lofar_add_package(Run)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)
