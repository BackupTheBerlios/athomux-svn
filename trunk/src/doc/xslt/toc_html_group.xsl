<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">
	
<xsl:import href="menu_html.xsl"/>

<xsl:output method="xml" indent="yes"
	doctype-system ="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
	doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" 
	omit-xml-declaration="no"
	encoding="utf-8" />
	
	<xsl:param name="p_type" />
	
	<xsl:key name="pCategory" match="category" use="." />
	
	<xsl:template match="/">
				
		<html xmlns="http://www.w3.org/1999/xhtml">
			<head>
		    		<title>Athomux - Group by <xsl:value-of select="$p_type" /></title>
		    		<link rel="stylesheet" type="text/css" href="style.css" />
			</head>
			
			<body>
				<xsl:apply-templates select="document('data.menu')/menu"/>
				<h1>TOC - Group by <xsl:value-of select="$p_type" /></h1>
				<xsl:apply-templates select="toc"/>
			</body>
		</html>
	</xsl:template>
	
	<xsl:template match="toc">
		<xsl:for-each select="//category[generate-id(.)=generate-id(key('pCategory',.))][@name=$p_type]">
			
			<xsl:sort select="." order="ascending"/>
			
			<xsl:element name="{concat('h', 1+string-length(.)-string-length(translate(.,'/','')))}">
				<xsl:value-of select="."/>
			</xsl:element>
			
			<table>
				<xsl:for-each select="key('pCategory',.)">
					<xsl:sort select="../../@name"/>
					
					<xsl:variable name="brickname" select="substring-after(../../@name, '#')" />
					
					<tr>
						<td>
							<a href="{$brickname}.html"><xsl:value-of select="$brickname"/></a>
						</td>
						<td>
							<xsl:value-of select="../../purpose"/>
						</td>
					</tr>
				</xsl:for-each>
			</table>
		</xsl:for-each>
	</xsl:template>
	
</xsl:stylesheet>
