DOCS=bb-requirements
PDFS=bb-requirements.pdf

%.dvi:   %.tex
	latex $<

%.pdf:   %.dvi
	dvipdfm $<

bb-requirements.pdf: \
	bb-requirements.tex \
	../figures/blackboard-use_concept.png \
	../figures/demo.usecase.png \


clean:
	-@rm *~
	-@rm *.aux
	-@rm *.dvi
	-@rm *.log
	-@rm *.out
	-@rm *.pdf
