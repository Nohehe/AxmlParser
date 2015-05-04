#include "FileUtil.h"

Parser * AxmlOpen(char *buffer, size_t size)
{
	Parser *parser = new Parser();

	if (buffer == NULL)
	{
		fprintf(stderr, "Error: AxmlOpen get an invalid parameter.\n");
		return NULL;
	}

	//parser = (Parser *)malloc(sizeof(Parser));
	if (parser == NULL)
	{
		fprintf(stderr, "Error: init parser.\n");
		return NULL;
	}

	/* init parser */
	parser->_buf = (unsigned char *)buffer;
	parser->_fileSize = size;
	parser->_curPostion = 0;

	parser->text = NULL;

	parser->nsStack;

	parser->sp = (StringPool *)malloc(sizeof(StringPool));
	if (parser->sp == NULL)
	{
		fprintf(stderr, "Error: init string table struct.\n");
		free(parser);
		return NULL;
	}

	parser->rp = (ResourcePool *)malloc(sizeof(ResourcePool));
	if (parser->rp == NULL)
	{
		fprintf(stderr, "Error: init resource table struct.\n");
		free(parser);
		return NULL;
	}


	/* parse first three chunks */
	if (ParseHeadChunk(parser) != 0 ||
		ParseStringChunk(parser) != 0 ||
		ParseResourceChunk(parser) != 0)
	{
		free(parser->sp);
		free(parser);
		return NULL;
	}

	return (Parser *)parser;
}

int AxmlClose(Parser* parser)
{
	
	if (parser == NULL)
	{
		printf("Error: AxmlClose get an invalid parameter.\n");
		return UNKNOWN_ERROR;
	}

	if (parser->sp->data)
		free(parser->sp->data);


	if (parser->sp->offsets)
		free(parser->sp->offsets);

	if (parser->sp->stringTable)
	{
		for (uint32_t i = 0; i < parser->sp->count; i++)
		{
			if (parser->sp->stringTable[i])
			{
				free(parser->sp->stringTable[i]);

			}
		}
		free(parser->sp->stringTable);
	}



	if (parser->sp)
		free(parser->sp);

	if (!parser->attrStack.empty())
	{
		parser->attrStack.clear();
	}

	if (!parser->nsStack.empty())
	{
		parser->nsStack.clear();
	}

	if (!parser->tagStack.empty())
	{
		parser->tagStack.clear();
	}

	if (parser)
		free(parser);

	return PARSE_OK;
}


Axml_Event AxmlNext(Parser *parser)
{

	uint32_t chunkType;
	Axml_Event event;

	/* when buffer ends */
	if (FileEnd(parser))
	{
		event = END_DOCUMENT;
		return event;
	}


	/* common chunk head */
	chunkType = GetIntFrom4LeBytes(parser);


	if (chunkType == AXML_START_TAG)
	{
		event = START_TAG;
	}
	else if (chunkType == AXML_END_TAG)
	{
		event = END_TAG;
	}
	else if (chunkType == AXML_START_NAMESPACE)
	{
		event = START_NAMESPACE;
	}
	else if (chunkType == AXML_END_NAMESPACE)
	{
		event = END_NAMESPACE;
	}
	else if (chunkType == AXML_CDATA)
	{
		event = TEXT;
	}
	else if (chunkType == AXML_LAST_CHUNK)
	{
		event = END_DOCUMENT;
	}
	else
	{
		event = BAD_DOCUMENT;
	}

	return event;
}

int AxmlToXml(char *inbuf, size_t insize)
{
	Parser *parser;
	Axml_Event event;

	int indent = 0;

	parser = AxmlOpen(inbuf, insize);
	if (parser == NULL)
		return WRONG_FILE;

	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	
	while ((event = AxmlNext(parser)) != END_DOCUMENT
		&& event != BAD_DOCUMENT)
	{
		char *prefix;
		char *name;
		char *value;

		switch (event){

		case START_TAG:
			indent++;
			printf("\n%*s", indent * 4, "");
			ParseTagChunk(parser);
			prefix = GetUriPrefix(parser, parser->tagStack.back().tagUri);
			name = GetString(parser, parser->tagStack.back().tagName);
			if (strlen(prefix) != 0)
				printf("<%s:%s", prefix, name);
			else
				printf("<%s", name);

			if (parser->nsStack.back().newNs)
			{
				prefix = GetString(parser, parser->nsStack.back().prefix);
				name = GetString(parser, parser->nsStack.back().uri );
				
				printf("\n%*sxmlns:%s=\"%s\" ", indent * 4 + 4, "", prefix, name);
				parser->nsStack.back().newNs = false;

			}
			
			/* for file format */
			if (!parser->attrStack.empty())
			{
				printf("\n");
			}
			

			for (uint32_t i  = 0; i < parser->attrStack.size();i++)
			{
				prefix = GetUriPrefix(parser, parser->attrStack[i].uri);
				name = GetString(parser, parser->attrStack[i].name);
				value = parser->attrStack[i].value;
				if (strlen(prefix) != 0)
					printf("%*s%s:%s=\"%s\" ", indent * 4 + 4, "", prefix, name, value);
				else
					printf("%*s%s=\"%s\" ", indent * 4 + 4, "", name, value);
				if (i < parser->attrStack.size() - 1)
					printf("\n");

			}
			
			parser->attrStack.clear();
			printf(">\n");
			break;

		case END_TAG:
			
			ParseTagEnd(parser);
			printf("%*s", indent * 4, "");
			prefix = GetUriPrefix(parser, parser->tagStack.back().tagUri);
			name = GetString(parser, parser->tagStack.back().tagName);
			if (strlen(prefix) != 0)
				printf("</%s:%s>\n", prefix, name);
			else
				printf("</%s>\n", name);	
			parser->tagStack.pop_back();		/* remove last tag */
			--indent;
			break;

		case TEXT:
			ParseTextChunk(parser);
			break;

		case START_NAMESPACE:
			ParseNamespaceChunk(parser);
			break;

		case  END_NAMESPACE:
			ParseNamespaceEnd(parser);
			break;

		case BAD_DOCUMENT:
			printf("Error: AxmlNext() returns a AE_ERROR event.\n");
			break;

		default:
			break;
		}
	}

	AxmlClose(parser);

	return PARSE_OK;
}
