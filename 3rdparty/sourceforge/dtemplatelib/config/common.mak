# Written for GNU -*- makefile -*-.

# Customization for all directories goes here:
include sources.inc
MAKEFILEDEPS += sources.inc
include ../config/common.inc
MAKEFILEDEPS += ../config/common.inc
include ../config/$(PLATFORM).inc
MAKEFILEDEPS += ../config/$(PLATFORM).inc


# Customization for this directory goes here:
include project.inc
MAKEFILEDEPS += project.inc
include ../config/post.inc
MAKEFILEDEPS += ../config/post.inc
