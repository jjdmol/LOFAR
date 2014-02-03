include(LofarCTest)

lofar_add_test(tConverter tConverter.cc DEPENDS AMCServer)
lofar_add_test(tConverterStress tConverterStress.cc DEPENDS AMCServer)
