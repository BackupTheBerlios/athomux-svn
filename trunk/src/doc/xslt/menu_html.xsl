<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">
<xsl:output method="xml" indent="yes"
	doctype-system ="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
	doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" 
	omit-xml-declaration="no"
	encoding="utf-8" />
	
	<xsl:template match="menu">
		<ul>
			<xsl:apply-templates select="entry"/>
		</ul>
	</xsl:template>

	<xsl:template match="entry">
		<li>
			<a href="{@link}.html" >
				<xsl:value-of select="."/>
			</a>
		</li>
	</xsl:template>
	
</xsl:stylesheet>
