<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">
<xsl:output method="xml" indent="yes"
	doctype-system ="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
	doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" 
	omit-xml-declaration="no"
	encoding="utf-8" />
	
	<xsl:template match="/">
				
		<html xmlns="http://www.w3.org/1999/xhtml">
			<head>
		    		<title>Athomux - Index</title>
		    		<link rel="stylesheet" type="text/css" href="style.css" />
			</head>
			
			<body>
				<h1>Table Of Contents</h1>
				<ul>
					<xsl:apply-templates select="toc"/>
				</ul>
			</body>
		</html>
	</xsl:template>
	
	
	<xsl:template match="toc">
		<xsl:apply-templates select="brickname">
			<xsl:sort select="."/>
		</xsl:apply-templates>
	</xsl:template>

	<xsl:template match="brickname">
		<xsl:variable name="brickname" select="substring-after(.,'#')"/>
		<li>
			<a href="{$brickname}.html" >
				<xsl:value-of select="$brickname"/>
			</a>
		</li>
	</xsl:template>
	
</xsl:stylesheet>
