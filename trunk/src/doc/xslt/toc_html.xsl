<?xml version="1.0" encoding="utf-8"?>

<!--

# Author: Alexander Herbig
# Copyright: Alexander Herbig
# License: see files SOFTWARE-LICENSE, PATENT-LICENSE

-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">
	
<xsl:import href="menu_html.xsl"/>
	
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
				<xsl:apply-templates select="document('data.menu')/menu"/>
				<h1>Table Of Contents</h1>
				<xsl:apply-templates select="toc"/>
			</body>
		</html>
	</xsl:template>
	
	<xsl:template match="toc">
		<table>
			<tr>
				<th>Brick</th>
				<th>Pupose</th>
			</tr>
			<xsl:apply-templates select="brick">
				<xsl:sort select="@name"/>
			</xsl:apply-templates>
		</table>
	</xsl:template>

	<xsl:template match="brick">
		<xsl:variable name="brickname" select="substring-after(@name,'#')"/>
		<tr>
			<td><a href="{$brickname}.html" ><xsl:value-of select="$brickname"/></a></td>
			<td><xsl:value-of select="purpose"/></td>
		</tr>
	</xsl:template>
	
</xsl:stylesheet>
