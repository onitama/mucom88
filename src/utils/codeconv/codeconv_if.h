#pragma once

class CodeConvert
{
public:

	virtual void Utf8ToSjis(const char *src, char *dest, int bufSize) = 0;
	virtual void FromSjis(const char *src, char *dest, int bufSize) = 0;

private:

};
