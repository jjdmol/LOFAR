FO_XSL=./fo.xsl
HTML_XSL=./html.xsl

PDFS=bb.pdf bb-hierarchy.pdf database.pdf demo.pdf LogicalDB.pdf

%.fo:	%.xml
	xsltproc --docbook ${FO_XSL} $< > $@ 

%.pdf:	%.fo
	fop $< $@

%.html:	%.xml
	xsltproc --docbook ${HTML_XSL} $< > $@ 

pdfs:	$(PDFS)
