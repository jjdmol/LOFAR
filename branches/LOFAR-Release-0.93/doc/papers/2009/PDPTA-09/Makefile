TEX_SOURCES =	fcnp.tex

BIB_SOURCES =	fcnp.bib

FIG_SOURCES =	FCNP-read.fig FCNP-write.fig fair.fig unfair.fig pset.fig\
		processing.fig

JGR_SOURCES =	cn-to-ion-performance.jgr ion-to-cn-performance.jgr

JPG_SOURCES =	station.jpg

PNG_SOURCES =	

SVG_SOURCES =	ionode-load.svg

STY_SOURCES =	

AUX_FILES =	$(TEX_SOURCES:%.tex=%.aux)
FIGURES =	$(FIG_SOURCES:%.fig=%.pdf) $(JGR_SOURCES:%.jgr=%.pdf)\
		$(JPG_SOURCES) $(SVG_SOURCES:%.svg=%.pdf)

GEN_EXT =	bbl blg dvi idx ilg ind lof log lot ps toc ps_pages
GEN_FILES =	$(AUX_FILES) $(FIGURES) $(GEN_EXT:%=fcnp.%) mfput.log\
		missfont.log texput.log

TEXINPUTS =	inputs:.:
TEXFONTS =	:

%.pdf:		%.jgr
		jgraph $< | epstopdf --filter >$@

%.pdf:		%.fig
		fig2dev -L pdf $< $@

%.pdf:		%.svg
		convert $< $@

%.ps:		%.pdf
		#pdftops -paper letter $<
		pdftops -paper A4 $<


fcnp.pdf:	$(TEX_SOURCES) $(STY_SOURCES) $(BIB_SOURCES) $(FIGURES)
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex fcnp
		bibtex fcnp
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex fcnp
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex fcnp

evince::	fcnp.pdf
		evince $<

lpr::		fcnp.ps
		lpr $<

clean::
		rm -f $(GEN_FILES:ionode-load.pdf=)
