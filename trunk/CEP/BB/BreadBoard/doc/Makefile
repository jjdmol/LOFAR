FO_XSL=./fo.xsl
HTML_XSL=./html.xsl

%.fo:	%.xml
	xsltproc --docbook ${FO_XSL} $< > $@ 

%.pdf:	%.fo
	fop $< $@

%.html:	%.xml
	xsltproc --docbook ${HTML_XSL} $< > $@ 
