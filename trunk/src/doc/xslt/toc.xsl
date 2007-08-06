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
  
		<xsl:element name="brick">
			<xsl:attribute name="name">
				<xsl:value-of select="document(concat('../', $title))/brick/header/brickname" />
			</xsl:attribute>
			<xsl:element name="purpose">
				<xsl:value-of select="document(concat('../', $title))/brick/header/purpose" />
			</xsl:element>
		</xsl:element>
  </xsl:copy>
  
</xsl:template>

</xsl:stylesheet>
