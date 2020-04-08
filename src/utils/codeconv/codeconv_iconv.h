#pragma once

#include <iconv.h>
#include "codeconv_if.h"

#define CODECONVERT CodeConvertIconv


class CodeConvertIconv : public CodeConvert
{
public:
	CodeConvertIconv();
	~CodeConvertIconv();

	void Utf8ToSjis(const char *src, char *dest, int bufSize);
	void FromSjis(const char *src, char *dest, int bufSize);

	iconv_t ToUtf8;
	iconv_t ToSjis;

private:

};

