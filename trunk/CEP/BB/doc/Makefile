FO_XSL=./fo.xsl
HTML_XSL=./html.xsl

DOCS=bb-use selfcal-implementation
PDFS=bb-use.pdf selfcal-implementation.pdf

%.fo:	%.xml $(FO_XSL)
	xsltproc --docbook $(FO_XSL) $< > $@ 

%.pdf:	%.fo
	fop $< $@

%.html:	%.xml
	xsltproc --docbook $(HTML_XSL) $< > $@ 

pdfs:	$(PDFS)
	make $(PDFS)

selfcal-implementation.pdf: selfcal-implementation.xml local-_global-commit.png database.png nestedTransaction.png