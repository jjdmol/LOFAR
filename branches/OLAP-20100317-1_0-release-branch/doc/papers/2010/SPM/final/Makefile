TEX_SOURCES =	spm.tex

BIB_SOURCES =	spm.bib

FIG_SOURCES =	

JGR_SOURCES =	

JPG_SOURCES =	

PNG_SOURCES =	

STY_SOURCES =	

AUX_FILES =	$(TEX_SOURCES:%.tex=%.aux)
FIGURES =	$(FIG_SOURCES:%.fig=%.pdf) $(JGR_SOURCES:%.jgr=%.pdf)

GEN_EXT =	bbl blg dvi idx ilg ind lof log lot ps toc ps_pages
GEN_FILES =	$(AUX_FILES) $(FIGURES) $(GEN_EXT:%=spm.%)\
		mfput.log missfont.log texput.log

TEXINPUTS =	inputs:.:
TEXFONTS =	:

%.pdf:		%.jgr
		jgraph $< | epstopdf --filter > $@

%.pdf:		%.fig
		fig2dev -L pdf $< $@

spm.pdf:	$(TEX_SOURCES) $(STY_SOURCES) $(BIB_SOURCES) $(FIGURES)
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex spm
		bibtex spm
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex spm
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex spm

spm.ps:		spm.pdf
		#pdftops -paper letter spm.pdf
		pdftops -paper A4 spm.pdf

evince::	spm.pdf
		evince $<

xpdf::		spm.pdf
		xpdf -g 900x1200 $<

lpr::		spm.ps
		lpr $<

clean::
		rm -f $(GEN_FILES)
