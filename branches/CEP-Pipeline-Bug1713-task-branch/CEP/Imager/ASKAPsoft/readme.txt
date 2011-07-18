ASKAP doesn't use CMake to build their software. So, in order to integrate
the ASKAP software into the LOFAR build environment, it was necessary to
write a number of CMakeLists.txt files. Because it is nearly impossible to
keep the list of source files up-to-date, it was decided to use file glob
patterns in this case. Even though the use of glob patterns is strongly
discouraged by the CMake community, it was considered safe in this specific
case. Note that the ASKAP build environment (based on Scons) also uses glob
patterns instead of explicity listing the source files!
