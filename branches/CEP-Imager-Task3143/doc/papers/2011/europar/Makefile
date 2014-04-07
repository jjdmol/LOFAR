TEX_SOURCES =	lofar.tex

BIB_SOURCES =

FIG_SOURCES =	lofar-stations.fig processing.fig

JGR_SOURCES =	stations-beams.jgr execution_times.jgr coherent-dedispersion.jgr dispersed-signal.jgr ionperf.jgr

JPG_SOURCES =	

PDF_SOURCES =	

PNG_SOURCES =	

STY_SOURCES =	

SVG_SOURCES =	pencilbeams.svg LBAfield-delay.svg dataflow-2.svg

AUX_FILES =	$(TEX_SOURCES:%.tex=%.aux)
GEN_FIGURES =	$(FIG_SOURCES:%.fig=%.pdf) $(JGR_SOURCES:%.jgr=%.pdf) $(SVG_SOURCES:%.svg=%.pdf)

FIGURES =	$(GEN_FIGURES) $(JPG_SOURCES) $(PNG_SOURCES) $(PDF_SOURCES)

GEN_EXT =	bbl blg dvi idx ilg ind lof log lot ps toc ps_pages
GEN_FILES =	$(AUX_FILES) $(GEN_FIGURES) lofar.pdf $(GEN_EXT:%=lofar.%)\
		mfput.log missfont.log texput.log

TEXINPUTS =	inputs:.:
TEXFONTS =	:

%.eps:		%.jgr
		jgraph $< > $@

%.pdf:		%.eps
		epstopdf --outfile=$@ $<

%.pdf:		%.fig
		fig2dev -L pdf $< $@

%.pdf:		%.svg
		inkscape $< -A $@


%-bw.pdf:	%.pdf
		gs -sOutputFile=$@ -sDEVICE=pdfwrite -sColorConversionStrategy=Gray -dProcessColorModel=/DeviceGray -dCompatibilityLevel=1.4 -dGrayImageFilter=/FlateEncode -dNOPAUSE -dBATCH $<

%-bw.jpg:	%.jpg
		convert $< -colorspace Gray $@

%-bw.png:	%.png
		convert $< -colorspace Gray $@

lofar.pdf:	$(TEX_SOURCES) $(STY_SOURCES) $(FIGURES)
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar
		TEXINPUTS=$(TEXINPUTS) TEXFONTS=$(TEXFONTS) pdflatex lofar

lofar.ps:	lofar.pdf
		#pdftops -paper letter lofar.pdf
		pdftops -paper A4 lofar.pdf

lofar-a4.pdf:	lofar.ps
		ps2pdf $< $@

evince::	lofar.pdf
		evince $<

xpdf::		lofar.pdf
		xpdf -g 900x1200 $<

lpr::		lofar.ps
		lpr $<

clean::
		rm -f $(GEN_FILES)

dispersed-signal.pdf:	dispersed-signal-data-2.sh

