
gsmtable = gsm.tab

gsmtablesrc = $(srcdir)/$(gsmtable)
aipspp_tablefiles = table.dat table.f0 table.info table.lock

gsmfiles := $(addprefix $(gsmtable)/,$(aipspp_tablefiles))

# # OMS 19/08/03: commented this target out for now -- I'll just check in 
# # the table statically into CVS 
# # 
# $(_gsmfiles): 4C-Vizier.txt ../src/gsm_definitions.g
#	  glish -l $(srcdir)/4c2gsm.g

gsmdir = $(datadir)/GSM/$(gsmtable)
dist_gsm_DATA = $(gsmfiles)

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
