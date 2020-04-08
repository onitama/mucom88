#include <stdio.h>
#include <iconv.h>

#include "codeconv_iconv.h"
#include <string.h>

CodeConvertIconv::CodeConvertIconv() {
  ToUtf8 = iconv_open("UTF-8", "Shift_JIS");
  ToSjis = iconv_open("Shift_JIS", "UTF-8");
}

CodeConvertIconv::~CodeConvertIconv() {
    iconv_close(ToSjis);
    iconv_close(ToUtf8);
}

void CodeConvertIconv::Utf8ToSjis(const char *src, char *dest, int bufSize) {
    int srclen = strlen(src);
    char *srcbuf = new char[srclen + 1];
    strcpy(srcbuf, src);
    char *srctext = srcbuf;
    size_t srcsize = srclen;
    size_t destlen = bufSize;

    iconv(ToSjis, &srctext, &srcsize, &dest, &destlen);
    *dest = '\0';
    delete[] srcbuf;
}

void CodeConvertIconv::FromSjis(const char *src, char *dest, int bufSize) {
    int srclen = strlen(src);
    char *srcbuf = new char[srclen + 1];
    strcpy(srcbuf, src);
    char *srctext = srcbuf;
    size_t srcsize = srclen;
    size_t destlen = bufSize;

    iconv(ToUtf8, &srctext, &srcsize, &dest, &destlen);
    *dest = '\0';
    delete[] srcbuf;
}
