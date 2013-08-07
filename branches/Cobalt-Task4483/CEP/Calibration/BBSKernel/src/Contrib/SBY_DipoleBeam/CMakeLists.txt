# $Id$

set(element_beam_YATAWATTA_LBA_phi_SRCS   beam_dr_phi.c)
set(element_beam_YATAWATTA_LBA_theta_SRCS beam_dr_theta.c)
set(element_beam_YATAWATTA_HBA_phi_SRCS   hba_beam_phi.c)
set(element_beam_YATAWATTA_HBA_theta_SRCS hba_beam_theta.c)

foreach(lib element_beam_YATAWATTA_LBA_phi element_beam_YATAWATTA_LBA_theta element_beam_YATAWATTA_HBA_phi element_beam_YATAWATTA_HBA_theta)
  add_library(${lib} MODULE ${${lib}_SRCS})
  set_target_properties(${lib} PROPERTIES PREFIX "")
  add_dependencies(BBSKernel ${lib})
  install(TARGETS ${lib} DESTINATION ${LOFAR_LIBDIR})
endforeach(lib)
