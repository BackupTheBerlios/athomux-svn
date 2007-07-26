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
		<table>
			<xsl:apply-templates select="header"/>
		</table>
		
		<h1>Inputs</h1>
		<table>
			<xsl:apply-templates select="inputlist"/>
		</table>
		
		<h1>Outputs</h1>
		<table>
			<xsl:apply-templates select="outputlist"/>
		</table>
		
		<h1>Instances</h1>
		<table>
			<xsl:apply-templates select="instancelist"/>
		</table>
		
		<h1>Alias</h1>
		<table>
			<xsl:apply-templates select="aliaslist"/>
		</table>
		
		<h1>Wires</h1>
		<table>
			<xsl:apply-templates select="wirelist"/>
		</table>
		
		<h1>Operations</h1>
		<table>
			<xsl:apply-templates select="operationlist"/>
		</table>
	</xsl:template>
	
	
	<xsl:template match="header">
		<tr>
			<td>Brick Name</td>
			<td><xsl:value-of select="brickname" /></td>
		</tr>
	
		
		<tr>
			<td>Author</td>
			<td><xsl:value-of select="author"/></td>
		</tr>
		
		<tr>
			<td>Copyright</td>
			<td><xsl:value-of select="copyright"/></td>
		</tr>
		
		<tr>
			<td>License</td>
			<td><xsl:apply-templates select="license"/></td>
		</tr>
		
		<tr>
			<td>Contexts</td>
			<td><xsl:apply-templates select="contextlist"/></td>
		</tr>
		
		<tr>
			<td>Purpose</td>
			<td><xsl:value-of select="purpose" disable-output-escaping="yes"/></td>
		</tr>
		
		<tr>
			<td>Description</td>
			<td><xsl:value-of select="description" disable-output-escaping="yes"/></td>
		</tr>
		
		<tr>
			<td>Example</td>
			<td><xsl:value-of select="example" disable-output-escaping="yes"/></td>
		</tr>
		
		<tr>
			<td>Categorys</td>
			<td><xsl:apply-templates select="categorylist"/></td>
		</tr>
		
		<tr>
			<td>Tags</td>
			<td><xsl:apply-templates select="taglist"/></td>
		</tr>
		
		<tr>
			<td>Attributs</td>
			<td><xsl:apply-templates select="attributelist"/></td>
		</tr>
	</xsl:template>
	
	
	<xsl:template match="license|context">
			<ul>
				<xsl:for-each select="file">
					<li><xsl:value-of select="."/></li>
				</xsl:for-each>
			</ul>
	</xsl:template>
	
	
	<xsl:template match="contextlist">
			<h3><xsl:value-of select="@key"/></h3>
			<xsl:apply-templates select="context"/>
	</xsl:template>
	
	
	<xsl:template match="attributelist|taglist|categorylist">
			<table>
			<xsl:for-each select="attribute|tag|category">
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
			<tr>
				<td><xsl:value-of select="name"/></td>
				<td><xsl:apply-templates select="categorylist"/></td>
				<td><xsl:apply-templates select="taglist"/></td>
				<td><xsl:apply-templates select="attributelist"/></td>
			</tr>
	</xsl:template>
	
	
	<xsl:template match="instancelist">
			<xsl:apply-templates select="instance"/>
	</xsl:template>
	
	<xsl:template match="aliaslist|wirelist">
			<xsl:apply-templates select="alias|wire"/>
	</xsl:template>
	
	<xsl:template match="operationlist">
			<xsl:apply-templates select="operation"/>
	</xsl:template>
	
	<xsl:template match="instance">
			<tr>
				<td><xsl:value-of select="@type"/></td>
				<td><xsl:value-of select="@alias"/></td>
			</tr>
	</xsl:template>
	
	
	<xsl:template match="alias|wire">
			<tr>
				<td><xsl:value-of select="@from"/></td>
				<td><xsl:value-of select="@to"/></td>
			</tr>
	</xsl:template>
	
	<xsl:template match="operation">
			<tr>
				<td><xsl:value-of select="@name"/></td>
				<td><xsl:value-of select="@parent"/></td>
				<td><xsl:value-of select="@section"/></td>
			</tr>
	</xsl:template>
	
</xsl:stylesheet>
