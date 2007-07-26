<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="yes"
	omit-xml-declaration="no"
	encoding="utf-8" />

<xsl:param name="title" /> 

<xsl:template match="/">
  <xsl:apply-templates select="toc"/>
</xsl:template>

<xsl:template match="toc">
  <xsl:copy>
  	<xsl:copy-of select="node()" />
  	<xsl:copy-of select="document($title)/brick/header/brickname"/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>
