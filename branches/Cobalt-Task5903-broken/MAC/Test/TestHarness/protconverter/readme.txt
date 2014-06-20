2006-01-18, Converting a lofar.prot file into a TSE.prot file.

There are 2 steps to take for generating a template. Some handwork still needs to be done. 
Assumption: the lofar_autogen_protfile is a F_APL_PROTOCOL, enumerated '10'

Step one, type:
./converter lofar_autogen_protfile.prot 10

This results in 'lofar_autogen_protfile.tmp' 
The converter replaced the 'dir = xxx;' sections within each event into a 'SigNr = 0xYYYY;' section. The SigNr is the binary event header.

Step two, type:
autogen -T TSE-protocol.tpl lofar_autogen_protfile.tmp

This results in a 'lofar_autogen_protfile.tseprot' file.

Note: Currently TSE also uses the '.prot' file extension. This is confusing and should be changed in time.

Handwork to be done:
- Enumerated types
- specifiy the msgLength field if event/function has a static size 

This delivery contains:
- readme.txt       (this file)
- proconvert.c
- Makefile
- TSE-protocol.tpl (template for autogen)

This delivery is dependent on the following files installed at <LOFARBUILDHOME>/installed/gnu_debug/include:
GCF/TM/GCF_Protocols.h
GCF/TM/GCF_Events.h