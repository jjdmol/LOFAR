<?xml version="1.0"?>
<xsl:stylesheet version='1.0'
        xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="/usr/share/xsl/docbook/latex/docbook.xsl"/>
  <xsl:output method="text" encoding="ISO-8859-1" indent="yes"/>
  <xsl:variable name="latex.override">
% -----------------------  Define your Preamble Here 
\documentclass[]{lofar}
\usepackage{layout}
%
\include{definitions}
%\usepackage{amsmath,amsthm, amsfonts, amssymb, amsxtra,amsopn}
%\usepackage{graphicx}
%\usepackage{float}
%\usepackage{times}
%\usepackage{algorithmic}
%\usepackage[dvips]{hyperref}
%\DeclareGraphicsExtensions{.eps}
%\DeclareGraphicsExtensions{.png}
% ------------------------  End of you preamble.
  </xsl:variable>
</xsl:stylesheet>