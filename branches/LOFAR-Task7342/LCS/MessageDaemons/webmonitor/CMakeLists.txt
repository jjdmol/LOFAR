# $Id$

install(FILES
  QPIDWebserverJSON
  DESTINATION bin)

file(GLOB WebMonitorFiles
  lib/*
  Schema.svg.*
  *.html
  )

install(FILES
  ${WebMonitorFiles}
  DESTINATION srv/www)
