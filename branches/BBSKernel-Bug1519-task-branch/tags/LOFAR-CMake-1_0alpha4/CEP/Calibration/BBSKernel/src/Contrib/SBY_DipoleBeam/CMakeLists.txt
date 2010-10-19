# $Id$

set(lba_beam_phi_SRCS   beam_dr_phi.c)
set(lba_beam_theta_SRCS beam_dr_theta.c)
set(hba_beam_phi_SRCS   hba_beam_phi.c)
set(hba_beam_theta_SRCS hba_beam_theta.c)

foreach(lib lba_beam_phi lba_beam_theta hba_beam_phi hba_beam_theta)
  add_library(${lib} MODULE ${${lib}_SRCS})
  set_target_properties(${lib} PROPERTIES PREFIX "")
  add_dependencies(BBSKernel ${lib})
  install(TARGETS ${lib} DESTINATION ${LOFAR_LIBDIR})
endforeach(lib)
