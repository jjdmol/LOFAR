TEX_SOURCES =	lofar.tex

BIB_SOURCES =	lofar.bib

FIG_SOURCES =	colinear.fig correlation-triangle.fig delay.fig processing.fig\
		pset.fig

JGR_SOURCES =	bandpass.jgr cn-performance.jgr ion-performance.jgr

JPG_SOURCES =	fringe.jpg LBA-field.jpg

PNG_SOURCES =	lofar-stations.png

STY_SOURCES =	

AUX_FILES =	$(TEX_SOURCES:%.tex=%.aux)
FIGURES =	$(FIG_SOURCES:%.fig=%.pdf) $(JGR_SOURCES:%.jgr=%.pdf)

GEN_EXT =	bbl blg dvi idx ilg ind lof log lot ps toc ps_pages
GEN_FILES =	$(AUX_FILES) $(FIGURES) lofar.pdf $(GEN_EXT:%=lofar.%)\
		mfput.log missfont.log texput.log

TEXINPUTS =	inputs:.:
TEXFONTS =	:

%.pdf:		%.jgr
		jgraph $< | epstopdf --filter > $@

%.pdf:		%.fig
		fig2dev -L pdf $< $@

lofar.pdf:	$(TEX_SOURCES) $(STY_SOURCES) $(BIB_SOURCES) $(FIGURES)
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar
		bibtex lofar
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar

lofar.ps:	lofar.pdf
		#pdftops -paper letter lofar.pdf
		pdftops -paper A4 lofar.pdf

evince::	lofar.pdf
		evince $<

xpdf::		lofar.pdf
		xpdf -g 900x1200 $<

lpr::		lofar.ps
		lpr $<

clean::
		rm -f $(GEN_FILES)
