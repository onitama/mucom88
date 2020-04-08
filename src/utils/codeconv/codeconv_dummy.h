#pragma once

#include "codeconv_if.h"
#define CODECONVERT CodeConvertDummy


class CodeConvertDummy : public CodeConvert
{
public:
	CodeConvertDummy();
	~CodeConvertDummy();

	void Utf8ToSjis(const char *src, char *dest, int bufSize);
	void FromSjis(const char *src, char *dest, int bufSize);

private:

};

