TEX_SOURCES =	fcnp.tex

BIB_SOURCES =	fcnp.bib

FIG_SOURCES =	FCNP-read.fig FCNP-write.fig fair.fig unfair.fig pset.fig\
		processing.fig

JGR_SOURCES =	cn-to-ion-performance.jgr ion-to-cn-performance.jgr

JPG_SOURCES =	station.jpg

PNG_SOURCES =	

STY_SOURCES =	

AUX_FILES =	$(TEX_SOURCES:%.tex=%.aux)
GEN_EPS_FILES =	$(FIG_SOURCES:%.fig=%.eps) $(JGR_SOURCES:%.jgr=%.eps) $(JPG_SOURCES:%.jpg=%.eps) $(PNG_SOURCES:%.png=%.eps)
EPS_FILES =	$(GEN_EPS_FILES) ionode-load.eps

GEN_EXT =	bbl blg dvi idx ilg ind lof log lot ps toc ps_pages
GEN_FILES =	$(AUX_FILES) $(GEN_EPS_FILES) $(GEN_EXT:%=fcnp.%) mfput.log\
		missfont.log texput.log

TEXINPUTS =	inputs:.:
TEXFONTS =	:

%.eps:		%.jgr
		jgraph $< >$@

%.eps:		%.fig
		fig2dev -L eps $< $@

%.eps:		%.jpg
		convert $< $@

%.eps:		%.png
		convert $< $@

fcnp.dvi:	$(TEX_SOURCES) $(STY_SOURCES) $(BIB_SOURCES) $(EPS_FILES)
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) latex fcnp
		bibtex fcnp
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) latex fcnp
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) latex fcnp

fcnp.ps:	fcnp.dvi
		#dvips -t a4 -f $< >$@
		dvips -t letter -f $< >$@

fcnp.pdf:	fcnp.ps
		ps2pdf $< $@

xdvi::		fcnp.dvi
		MFINPUTS=`pwd`/mf: xdvi -thorough -s 7 -paper a4 $<

evince::	fcnp.pdf
		evince $<

lpr::		fcnp.ps
		lpr $<

clean::
		rm -f $(GEN_FILES)
