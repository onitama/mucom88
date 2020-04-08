#pragma once

#include "codeconv_if.h"

#ifdef USE_ICONV
#include "codeconv_iconv.h"
#else
#include "codeconv_dummy.h"
#endif
