
_gsmtable = gsm.tab

_gsmtablesrc = $(srcdir)/$(_gsmtable)
_tablefiles = table.dat table.f0 table.info table.lock
_gsmfiles = $(addprefix $(_gsmtablesrc)/,$(_tablefiles))

# # OMS 19/08/03: commented this target out for now -- I'll just check in 
# # the table statically into CVS 
# # 
# $(_gsmfiles): 4C-Vizier.txt ../src/gsm_definitions.g
#	  glish -l $(srcdir)/4c2gsm.g

  
gsmdir = $(datadir)/GSM/gsm.tab
gsm_DATA = $(_gsmfiles)
