<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="yes"
	omit-xml-declaration="no"
	encoding="utf-8" />

<xsl:param name="p_title" /> 

<xsl:template match="/">
  <xsl:apply-templates select="menu"/>
</xsl:template>

<xsl:template match="menu">
	
	<xsl:copy>
	  	<xsl:copy-of select="node()" />
  
		<xsl:element name="entry">
			<xsl:attribute name="link">
				<xsl:value-of select="concat('__toc_', $p_title)" />
			</xsl:attribute>			<xsl:value-of select="concat('Group by ', $p_title)" />
		</xsl:element>
	</xsl:copy>
  
</xsl:template>

</xsl:stylesheet>
