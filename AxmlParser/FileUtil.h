#include <stdio.h>
#include <iostream>

#include "AxmlParser.h"
#include "ParseUtil.h"

using namespace std;

#pragma once // compile only once

Parser * AxmlOpen(char *buffer, size_t size);

int AxmlClose(Parser* parser);

int AxmlToXml(char *inbuf, size_t insize);


