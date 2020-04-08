
#include "codeconv_dummy.h"
#include <string.h>

CodeConvertDummy::CodeConvertDummy() {
}

CodeConvertDummy::~CodeConvertDummy() {
}

void CodeConvertDummy::Utf8ToSjis(const char *src, char *dest, int bufSize) {
    strcpy(dest, src);
}

void CodeConvertDummy::FromSjis(const char *src, char *dest, int bufSize) {
    strcpy(dest, src);
}
