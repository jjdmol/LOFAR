<?xml version="1.0"?>
<xsl:stylesheet version='1.0'
        xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="/usr/share/xsl/docbook/fo/docbook.xsl"/>

  <xsl:param name="paper.type">A4</xsl:param>

  <xsl:param name="body.font.family">Times</xsl:param>
  <xsl:param name="title.font.family">Courier</xsl:param>
  <xsl:param name="monospace.font.family">Helvetica</xsl:param>

  <xsl:param name="body.font.size">10pt</xsl:param>

  <xsl:param name="page.margin.top">33mm</xsl:param> <!-- comes out as 60mm -->
  <xsl:param name="page.margin.bottom">15mm</xsl:param> <!-- comes out as 30mm -->
  <xsl:param name="page.margin.inner">20mm</xsl:param> <!-- comes out as 24mm on printer -->
  <xsl:param name="page.margin.outer">53mm</xsl:param> <!-- comes out as 54mm on printer -->

  <xsl:param name="title.margin.left">-1pc</xsl:param> 

</xsl:stylesheet>
