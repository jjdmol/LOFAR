DOCS=bb-requirements
PDFS=bb-requirements.pdf

%.dvi:   %.tex
	latex $<

%.pdf:   %.dvi
	dvipdfm $<

bb-requirements.pdf: \
	bb-requirements.tex \
	definitions.tex \
	../figures/astronpro.eps \
	../figures/astronple.eps \
	../figures/llofar.eps \
	../figures/blackboard-use_concept.eps \
	../figures/demo.usecase.eps \


clean:
	-@rm *~
	-@rm *.ass
	-@rm *.aux
	-@rm *.dvi
	-@rm *.lof
	-@rm *.log
	-@rm *.lot
	-@rm *.out
	-@rm *.pdf
	-@rm *.ref
	-@rm *.req
	-@rm *.toc
