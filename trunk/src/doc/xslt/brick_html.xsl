<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
<xsl:output method="xml" indent="yes"
	doctype-system ="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
	doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" 
	omit-xml-declaration="no"
	encoding="utf-8" />
	
	<xsl:template match="/">
			
		<html xmlns="http://www.w3.org/1999/xhtml">
			<head>
		    	
		    	<title>Athomux - <xsl:value-of select="brick/header/brickname"/></title>
		    	<link rel="stylesheet" type="text/css" href="style.css" />
		  </head>
			
			<body>
				<xsl:apply-templates select="brick"/>
			</body>
		</html>
	</xsl:template>
	
	<xsl:template match="brick">
		<p><a href="__toc.html">Index</a></p>
		
		<h1>Header: <xsl:value-of select="header/brickname" /></h1>
		<table>
			<xsl:apply-templates select="header"/>
		</table>
		
		<xsl:choose>
			<xsl:when test="count(inputlist/child::*) > 0">
				<h1>Inputs</h1>
				<table>
					<xsl:apply-templates select="inputlist"/>
				</table>
			</xsl:when>
			<xsl:otherwise>
				<h1>Inputs: None</h1>
			</xsl:otherwise>
		</xsl:choose>
		
		<xsl:choose>
			<xsl:when test="count(outputlist/child::*) > 0">
				<h1>Outputs</h1>
				<table>
					<xsl:apply-templates select="outputlist"/>
				</table>
			</xsl:when>
			<xsl:otherwise>
				<h1>Outputs: None</h1>
			</xsl:otherwise>
		</xsl:choose>
		
		<xsl:choose>
			<xsl:when test="count(instancelist/child::*) > 0">
				<h1>Internal Structure</h1>
				
				<img src="image.png" alt="image" />
				
				<h2>Instances</h2>
				<table>
					<xsl:apply-templates select="instancelist"/>
				</table>
				
				<xsl:if test="count(aliaslist/child::*) > 0">
					<h2>Alias</h2>
					<table>
						<xsl:apply-templates select="aliaslist"/>
					</table>
				</xsl:if>
				
				<xsl:if test="count(wirelist/child::*) > 0">
					<h2>Wires</h2>
					<table>
						<xsl:apply-templates select="wirelist"/>
					</table>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<h1>Internal Structure: None</h1>
			</xsl:otherwise>
		</xsl:choose>
		
		<xsl:choose>
			<xsl:when test="count(operationlist/child::*) > 0">
				<h1>Operations</h1>
				<table>
					<xsl:apply-templates select="operationlist"/>
				</table>
			</xsl:when>
			<xsl:otherwise>
				<h1>Operations: None</h1>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	
	<xsl:template match="header">

		<tr>
			<td class="header">Brick Name</td>
			<td><xsl:value-of select="brickname" /></td>
		</tr>
	
		
		<tr>
			<td class="header">Author</td>
			<td><xsl:value-of select="author"/></td>
		</tr>
		
		<tr>
			<td class="header">Copyright</td>
			<td><xsl:value-of select="copyright"/></td>
		</tr>
		
		<tr>
			<td class="header">License</td>
			<td><xsl:apply-templates select="licenselist"/></td>
		</tr>
		
		<tr>
			<td class="header">Context</td>
			<td><xsl:apply-templates select="contextlist"/></td>
		</tr>
		
		<tr>
			<td class="header">Purpose</td>
			<td><xsl:value-of select="purpose" disable-output-escaping="yes"/></td>
		</tr>
		
		<tr>
			<td class="header">Description</td>
			<td>
				<pre><xsl:value-of select="description" disable-output-escaping="yes"/></pre>
			</td>
		</tr>
		
		<tr>
			<td class="header">Example</td>
			<td>
				<pre><xsl:value-of select="example" disable-output-escaping="yes"/></pre>
			</td>
		</tr>
		
		<tr>
			<td class="header">Categories</td>
			<td><xsl:apply-templates select="categorylist"/></td>
		</tr>
		
		<tr>
			<td class="header">Tags</td>
			<td><xsl:apply-templates select="taglist"/></td>
		</tr>
		
		<tr>
			<td class="header">Attributes</td>
			<td><xsl:apply-templates select="attributelist"/></td>
		</tr>
	</xsl:template>
	
	<xsl:template match="licenselist">
			<xsl:apply-templates select="file"/>	
	</xsl:template>
	
	<xsl:template match="context">
			<tr>
				<td class="subheader"><xsl:value-of select="@key"/></td>
				<td><xsl:apply-templates select="file"/></td>
			</tr>
	</xsl:template>
	
	<xsl:template match="file">
			<xsl:value-of select="."/>
			<xsl:if test="position()!=last()">
				<xsl:text>, </xsl:text>
			</xsl:if>
	</xsl:template>
	
	<xsl:template match="contextlist">
			<table class="sub">
				<xsl:apply-templates select="context"/>
			</table>
	</xsl:template>
	
	
	<xsl:template match="attributelist|taglist|categorylist">
			<table class="sub">
				<xsl:for-each select="attribute|tag|category">
					<tr>
						<td class="subheader"><xsl:value-of select="@name"/></td>
						<td><xsl:value-of select="."/></td>
					</tr>
				</xsl:for-each>
			</table>
	</xsl:template>	
	
	<xsl:template match="inputlist|outputlist">
			<tr>
				<th class="header">Name</th>
				<th>Max Sections</th>
				<th>Categories</th>
				<th>Tags</th>
				<th>Attributes</th>
			</tr>
			<xsl:apply-templates select="input|output"/>
	</xsl:template>
	
	
	<xsl:template match="input|output">
			<tr>
				<td class="header">
					<xsl:value-of select="@name"/>
				</td>
				<td><xsl:value-of select="@maxsections"/></td>
				<td><xsl:apply-templates select="categorylist"/></td>
				<td><xsl:apply-templates select="taglist"/></td>
				<td><xsl:apply-templates select="attributelist"/></td>
			</tr>
	</xsl:template>
	
	
	<xsl:template match="instancelist">
			<tr>
				<th class="header">Type</th>
				<th>Alias</th>
			</tr>
			<xsl:apply-templates select="instance"/>
	</xsl:template>
	
	<xsl:template match="aliaslist|wirelist">
			<tr>
				<th>From</th>
				<th>To</th>
			</tr>
			<xsl:apply-templates select="alias|wire"/>
	</xsl:template>
	
	<xsl:template match="operationlist">
			<tr>
				<th class="header">Name</th>
				<th>Parent</th>
				<th>Section</th>
			</tr>
			<xsl:apply-templates select="operation"/>
	</xsl:template>
	
	<xsl:template match="instance">
			<tr>
				<td class="header"><xsl:value-of select="@type"/></td>
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
				<td class="header"><xsl:value-of select="@name"/></td>
				<td><xsl:value-of select="@parent"/></td>
				<td><xsl:value-of select="@section"/></td>
			</tr>
	</xsl:template>
	
</xsl:stylesheet>
