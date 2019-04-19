// Callback 
// BouKiCHi 2019

#include <stdio.h>
#include "callback.h"

// 繧ｳ繝ｼ繝ｫ繝舌ャ繧ｯ
Callback::Callback() {
    Instance = NULL;
    Method = NULL;
}

void Callback::Set(void *MethodInstance, CALLBACK_PTR MethodPointer) {
    Instance = MethodInstance;
    Method = MethodPointer;
}
void Callback::Run() {
    if (!Method) return;
    Method(this, Instance);
}
