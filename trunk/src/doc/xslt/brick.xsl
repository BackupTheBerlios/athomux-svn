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
		    	
		    	<title>Athomux - <xsl:value-of select="brick/header/brickname"/></title>
		    	<!--<link rel="stylesheet" type="text/css" href="mystyle.css" />-->
		  </head>
			
			<body>
				<xsl:apply-templates select="brick"/>
			</body>
		</html>
	</xsl:template>
	
	
	<xsl:template match="brick">
		<h1>Header</h1>
		<xsl:apply-templates select="header"/>
		
		<h1>Inputs</h1>
		<xsl:apply-templates select="inputlist"/>
		
		<h1>Outputs</h1>
		<xsl:apply-templates select="outputlist"/>
	</xsl:template>
	
	
	<xsl:template match="header">
		<h2>Brick Name</h2>
		<p><xsl:value-of select="brickname"/></p>
		
		<h2>Author</h2>
		<p><xsl:value-of select="author"/></p>
		
		<h2>Copyright</h2>
		<p><xsl:value-of select="copyright"/></p>
		
		<h2>License</h2>
		<ul>
			<xsl:apply-templates select="license"/>
		</ul>
		
		<h2>Contexts</h2>
		<ul>
			<xsl:apply-templates select="contextlist"/>
		</ul>
		
		<h2>Purpose</h2>
		<p><xsl:value-of select="purpose"/></p>
		
		<h2>Description</h2>
		<p><xsl:value-of select="description"/></p>
		
		<h2>Example</h2>
		<p><xsl:value-of select="example"/></p>
		
		<h2>Attributs</h2>
		<xsl:apply-templates select="attributelist"/>
		
		<h2>Tags</h2>
		<xsl:apply-templates select="taglist"/>
	</xsl:template>
	
	
	<xsl:template match="license|context">
			<xsl:for-each select="file">
				<li><xsl:value-of select="."/></li>
			</xsl:for-each>
	</xsl:template>
	
	
	<xsl:template match="contextlist">
			<xsl:apply-templates select="context"/>
	</xsl:template>
	
	
	<xsl:template match="attributelist|taglist">
			<table>
			<xsl:for-each select="attribute">
				<tr>
					<td><xsl:value-of select="@name"/></td>
					<td><xsl:value-of select="."/></td>
				</tr>
			</xsl:for-each>
			</table>
	</xsl:template>	
	
	
	<xsl:template match="inputlist">
			<xsl:apply-templates select="input"/>
	</xsl:template>
	
	
	<xsl:template match="outputlist">
			<xsl:apply-templates select="output"/>
	</xsl:template>
	
	
	<xsl:template match="input|output">
			<h3><xsl:value-of select="name"/></h3>
			<xsl:apply-templates select="attributelist"/>
	</xsl:template>
	
</xsl:stylesheet>
