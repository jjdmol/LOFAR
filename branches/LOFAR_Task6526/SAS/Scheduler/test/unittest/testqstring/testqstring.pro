    SOURCES = testqstring.cpp
    CONFIG  += qtestlib
     
    # install
	QT -= gui
    target.path = ../
    sources.files = $$SOURCES *.pro
    sources.path = ../
    INSTALLS += target sources



