TEX_SOURCES =	lofar.tex

BIB_SOURCES =	lofar.bib lofarRJN.bib

FIG_SOURCES =	flow.fig processing.fig

JGR_SOURCES =	

PNG_SOURCES =	LBA_observed.png LBA_calibrated.png LBA_peeled.png telescope_overview.png stationoverview.png

STY_SOURCES =	

AUX_FILES =	$(TEX_SOURCES:%.tex=%.aux)
GEN_EPS_FILES =	$(FIG_SOURCES:%.fig=%.eps) $(JGR_SOURCES:%.jgr=%.eps) $(PNG_SOURCES:%.png=%.eps)
EPS_FILES =	$(GEN_EPS_FILES)

GEN_EXT =	bbl blg dvi idx ilg ind lof log lot ps toc ps_pages
GEN_FILES =	$(AUX_FILES) $(GEN_EPS_FILES) $(GEN_EXT:%=lofar.%) mfput.log\
		missfont.log texput.log

TEXINPUTS =	inputs:.:
TEXFONTS =	:

%.eps:		%.jgr
		jgraph $< >$@

%.eps:		%.fig
		fig2dev -L eps $< $@

%.eps:		%.png
		convert $< $@

lofar.dvi:	$(TEX_SOURCES) $(STY_SOURCES) $(BIB_SOURCES) $(EPS_FILES)
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) latex lofar
		bibtex lofar
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) latex lofar
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) latex lofar

lofar.ps:	lofar.dvi
		dvips -t a4 -f $< >$@
		#dvips -t letter -f $< >$@


xdvi::		lofar.dvi
		MFINPUTS=`pwd`/mf: xdvi -thorough -s 7 -paper a4 $<

evince::	lofar.ps
		evince $<

lpr::		lofar.ps
		lpr $<

clean::
		rm -f $(GEN_FILES)
