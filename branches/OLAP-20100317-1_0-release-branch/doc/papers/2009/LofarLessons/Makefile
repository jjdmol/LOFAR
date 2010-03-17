TEX_SOURCES =	lofar_lessons.tex

BIB_SOURCES =	lofar_lessons.bib

FIG_SOURCES =	

JGR_SOURCES =	

JPG_SOURCES =	

PNG_SOURCES =	

STY_SOURCES =	

AUX_FILES =	$(TEX_SOURCES:%.tex=%.aux)
FIGURES =	$(FIG_SOURCES:%.fig=%.pdf) $(JGR_SOURCES:%.jgr=%.pdf)

GEN_EXT =	bbl blg dvi idx ilg ind lof log lot ps toc ps_pages
GEN_FILES =	$(AUX_FILES) $(FIGURES) lofar_lessons.pdf $(GEN_EXT:%=lofar_lessons.%)\
		mfput.log missfont.log texput.log

TEXINPUTS =	inputs:.:
TEXFONTS =	:

%.pdf:		%.jgr
		jgraph $< | epstopdf --filter > $@

%.pdf:		%.fig
		fig2dev -L pdf $< $@

lofar_lessons.pdf:	$(TEX_SOURCES) $(STY_SOURCES) $(BIB_SOURCES) $(FIGURES)
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar_lessons
		bibtex lofar_lessons
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar_lessons
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar_lessons

lofar_lessons.ps:	lofar_lessons.pdf
		#pdftops -paper letter lofar_lessons.pdf
		pdftops -paper A4 lofar_lessons.pdf

evince::	lofar_lessons.pdf
		evince $<

xpdf::		lofar_lessons.pdf
		xpdf -g 900x1200 $<

lpr::		lofar_lessons.ps
		lpr $<

clean::
		rm -f $(GEN_FILES)
