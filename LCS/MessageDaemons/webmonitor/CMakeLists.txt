# $Id$

install(FILES
  QPIDWebserverJSON
  DESTINATION bin)

install(FILES
  start_QPIDWebMonitor
  stop_QPIDWebMonitor
  DESTINATION sbin)

file(GLOB StaticFiles
  lib/*
  Schema.svg.*
  *.html
  )

file(GLOB LibFiles
  lib/*
  )

install(FILES
  ${StaticFiles}
  DESTINATION srv/www)

install(FILES
  ${LibFiles}
  DESTINATION srv/www/lib)
