DOCS=bb-requirements
PDFS=bb-requirements.pdf

%.dvi:   %.tex
	latex $< && makeindex $*.idx && latex $< && latex $<

%.pdf:   %.dvi
	dvipdfm $<

bb-implementation.pdf: \
	bb-implementation.tex \
	definitions.tex \
	../figures/astronpro.eps \
	../figures/astronple.eps \
	../figures/llofar.eps \

view:	bb-implementation.pdf
	acroread bb-implementation.pdf

clean:
	-@rm *~
	-@rm *.aux
	-@rm *.dvi
	-@rm *.log
	-@rm *.out
	-@rm *.pdf
