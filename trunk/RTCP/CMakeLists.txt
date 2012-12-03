# $Id$

lofar_add_package(Interface) # Interfaces between RTCP product components 
lofar_add_package(FCNP)      # BlueGene: Fast Collective Network Protocol
lofar_add_package(CNProc)    # BlueGene: Compute-Node Processing applications 
lofar_add_package(IONProc)   # BlueGene: I/O Node Processing applications 
lofar_add_package(InputProc) # GPU cluster: Station Input Retrieval and Redistribution applications 
lofar_add_package(Storage)   # Stores correlator output in MeasurmentSets
lofar_add_package(Run)       # Run scripts to start RTCP components
lofar_add_package(LofarStMan)# Storage Manager for the main table of a LOFAR MS
lofar_add_package(MetaDataGatherer)# Pulls in meta data from OTDB etc
