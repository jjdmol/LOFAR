FO_XSL=./fo.xsl
HTML_XSL=./html.xsl
TEX_XSL=./tex.xsl

DOCS=bb-use selfcal-implementation
PDFS=bb-use.pdf selfcal-implementation.pdf

%.fo:	%.xml $(FO_XSL)
	xsltproc --docbook $(FO_XSL) $< > $@ 

%.pdf:	%.fo
	fop $< $@

%.html:	%.xml
	xsltproc --docbook $(HTML_XSL) $< > $@ 

%.tex:	%.xml
	xsltproc --docbook $(TEX_XSL) $< > $@ 

pdfs:	$(PDFS)
	make $(PDFS)

selfcal-implementation.pdf: \
	selfcal-implementation.xml \
	local-_global-commit.png \
	database.png \
	nestedTransaction.png \
	strategy_interpreter.png \
	start.usecase.png \
	bb.subcluster.png \


bb-use.pdf: \
	bb-use.xml \
	blackboard-use_concept.png \
	demo.usecase.png \


# should use latex: Astron standard. doesn't work yet.
#
selfcal-implementation/selfcal-implementation.tex:	selfcal-implementation.xml
	cd selfcal-implementation; xsltproc --docbook $(TEX_XSL) $< > $@ 

clean:
	-@rm *~
	-@rm *.aux
	-@rm *.dvi
	-@rm *.log
	-@rm *.out
	-@rm *.pdf
