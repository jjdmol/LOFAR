# $Id$

install(FILES
  QPIDWebserverJSON
  DESTINATION bin)

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
