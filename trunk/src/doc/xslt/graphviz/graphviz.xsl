<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">
<xsl:output method="text" indent="no" />

<xsl:variable name="v_clusters" select="0" />

<xsl:template match="/">
	digraph <xsl:value-of select="substring(/brick/header/brickname, 2)" /> {
		rankdir=LR;
		<xsl:apply-templates select="brick/instancelist" />
		
		<xsl:apply-templates select="brick/inputlist" />
		
		<xsl:apply-templates select="brick/outputlist" />
		
		<xsl:apply-templates select="brick/wirelist" />
	}
</xsl:template>

<xsl:template match="inputlist">
	{rank=source; 
	<xsl:apply-templates select="input" />
	
	<xsl:for-each select="/brick/aliaslist/alias">
		<xsl:if test="contains(@from, ':&lt;')">
			<xsl:value-of select="translate(substring-after(@from, ':&lt;'), '[]', '_')" />;
		</xsl:if>
	</xsl:for-each>
	
	Inputs[shape=box];
	};
</xsl:template>

<xsl:template match="input">
	<xsl:value-of select="translate(substring-after(@name, ':&lt;'), '[]', '_')" />
	<xsl:if test="@local = 'local'" > [style=filled, color=lightgrey] </xsl:if>;
</xsl:template>

<xsl:template match="outputlist">
	{rank=sink;
	<xsl:apply-templates select="output" />
	
	<xsl:for-each select="/brick/aliaslist/alias">
		<xsl:if test="contains(@from, ':&gt;')">
			<xsl:value-of select="translate(substring-after(@from, ':&gt;'), '[]', '_')" />;
		</xsl:if>
	</xsl:for-each>
	
	Outputs[shape=box];
	};
</xsl:template>

<xsl:template match="output">
	<xsl:value-of select="translate(substring-after(@name, ':&gt;'), '[]', '_')" />
	<xsl:if test="@local = 'local'" > [style=filled, color=lightgrey] </xsl:if>;
</xsl:template>

<xsl:template match="instancelist">
	<xsl:apply-templates select="instance" />
</xsl:template>

<xsl:template match="instance">
	<xsl:variable name="v_alias" select="@alias" />
	
	subgraph cluster_<xsl:value-of select="position()" /> {
		label="<xsl:value-of select="@alias" />"; 
		URL="<xsl:value-of select="substring(@type, 2)"/>.html"; 
		color=blue;
		
		{rank=source;
		<xsl:for-each select="/brick/aliaslist/alias">
			<xsl:if test="contains(@to, $v_alias) and contains(@to, ':&lt;')">
				<xsl:value-of select="translate(concat($v_alias, '_', substring-after(@to, ':&lt;')), '[]', '_')" />[style=filled, color=blue];
			</xsl:if>
		</xsl:for-each>
		
		<xsl:for-each select="/brick/wirelist/wire">
			<xsl:if test="contains(@to, $v_alias) and contains(@to, ':&lt;')">
				<xsl:value-of select="translate(concat($v_alias, '_', substring-after(@to, ':&lt;')), '[]', '_')" />;
			</xsl:if>
		</xsl:for-each>
		
		<xsl:for-each select="/brick/wirelist/wire">
			<xsl:if test="contains(@from, $v_alias) and contains(@from, ':&lt;')">
				<xsl:value-of select="translate(concat($v_alias, '_', substring-after(@from, ':&lt;')), '[]', '_')" />;
			</xsl:if>
		</xsl:for-each>
		}
		
		{rank=sink;
		<xsl:for-each select="/brick/aliaslist/alias">
			<xsl:if test="contains(@to, $v_alias) and contains(@to, ':&gt;')">
				<xsl:value-of select="translate(concat($v_alias, '_', substring-after(@to, ':&gt;')), '[]', '_')" />[style=filled, color=blue];
			</xsl:if>
		</xsl:for-each>
		
		<xsl:for-each select="/brick/wirelist/wire">
			<xsl:if test="contains(@to, $v_alias) and contains(@to, ':&gt;')">
				<xsl:value-of select="translate(concat($v_alias, '_', substring-after(@to, ':&gt;')), '[]', '_')" />;
			</xsl:if>
		</xsl:for-each>
		
		<xsl:for-each select="/brick/wirelist/wire">
			<xsl:if test="contains(@from, $v_alias) and contains(@from, ':&gt;')">
				<xsl:value-of select="translate(concat($v_alias, '_', substring-after(@from, ':&gt;')), '[]', '_')" />;
			</xsl:if>
		</xsl:for-each>
		}
	
	};
	<xsl:for-each select="/brick/aliaslist/alias">
		<xsl:if test="contains(@to, $v_alias) and contains(@to, ':&lt;')">
			<xsl:value-of select="translate(substring-after(@from, ':&lt;'), '[]', '_')" /> -> <xsl:value-of select="translate(concat($v_alias, '_', substring-after(@to, ':&lt;')), '[]', '_')" />;
		</xsl:if>
	</xsl:for-each>
	
	<xsl:for-each select="/brick/aliaslist/alias">
		<xsl:if test="contains(@to, $v_alias) and contains(@to, ':&gt;')">
			<xsl:value-of select="translate(concat($v_alias, '_', substring-after(@to, ':&gt;')), '[]', '_')" /> -> <xsl:value-of select="translate(substring-after(@from, ':&gt;'), '[]', '_')" />;
		</xsl:if>
	</xsl:for-each>
</xsl:template>

<xsl:template match="wirelist">
	<xsl:for-each select="wire">
		<xsl:if test="contains(@from, ':&gt;')">
			<xsl:variable name="v_from" select="translate(substring-before(substring-after(substring-after(@from, substring(/brick/header/brickname, 2)), '#'), ':&gt;'), '#', '_')" />
			<xsl:variable name="v_to" select="translate(substring-before(substring-after(substring-after(@to, substring(/brick/header/brickname, 2)), '#'), ':&lt;'), '#', '_')" />
			
			<xsl:if test="string-length($v_from) = 0">
				<xsl:value-of select="translate(substring-after(@from, ':&gt;'), '[]', '_')" /> ->
			</xsl:if>
			<xsl:if test="string-length($v_from) != 0">
				<xsl:value-of select="translate(concat($v_from, '_', substring-after(@from, ':&gt;')), '[]', '_')" /> ->
			</xsl:if>
			
			<xsl:if test="string-length($v_to) = 0">
				<xsl:value-of select="translate(substring-after(@to, ':&lt;'), '[]', '_')" />;
			</xsl:if>
			<xsl:if test="string-length($v_to) != 0">
				<xsl:value-of select="translate(concat($v_to, '_', substring-after(@to, ':&lt;')), '[]', '_')" />;
			</xsl:if>
		</xsl:if>
		
		<xsl:if test="contains(@from, ':&lt;')">
			<xsl:variable name="v_from" select="translate(substring-before(substring-after(substring-after(@from, substring(/brick/header/brickname, 2)), '#'), ':&lt;'), '#', '_')" />
			<xsl:variable name="v_to" select="translate(substring-before(substring-after(substring-after(@to, substring(/brick/header/brickname, 2)), '#'), ':&gt;'), '#', '_')" />
			
			<xsl:if test="string-length($v_from) = 0">
				<xsl:value-of select="translate(substring-after(@from, ':&lt;'), '[]', '_')" /> ->
			</xsl:if>
			<xsl:if test="string-length($v_from) != 0">
				<xsl:value-of select="translate(concat($v_from, '_', substring-after(@from, ':&lt;')), '[]', '_')" /> ->
			</xsl:if>
			
			<xsl:if test="string-length($v_to) = 0">
				<xsl:value-of select="translate(substring-after(@to, ':&gt;'), '[]', '_')" />;
			</xsl:if>
			<xsl:if test="string-length($v_to) != 0">
				<xsl:value-of select="translate(concat($v_to, '_', substring-after(@to, ':&gt;')), '[]', '_')" />;
			</xsl:if>
		</xsl:if>
	</xsl:for-each>
</xsl:template>

<xsl:template match="wire">

</xsl:template>

</xsl:stylesheet>
