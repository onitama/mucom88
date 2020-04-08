#include "codeconv.h"

#include <stdio.h>
#include <string.h>

// こんにちは
// utf8: E3-81-93-E3-82-93-E3-81-AB-E3-81-A1-E3-81-AF
// sjis: 82-B1-82-F1-82-C9-82-BF-82-CD

const unsigned char msg_utf8[] = { 0xE3,0x81,0x93,0xE3,0x82,0x93,0xE3,0x81,0xAB,0xE3,0x81,0xA1,0xE3,0x81,0xAF, 0x00 };
const unsigned char msg_sjis[] = { 0x82,0xB1,0x82,0xF1,0x82,0xC9,0x82,0xBF,0x82,0xCD, 0x00 };

void ShowHexDump(const char *msg) {
    int i = 0; 
    int len = strlen(msg);
    for(i=0; i < strlen(msg); i++) {
        printf("%02d : %02x\n", i, (msg[i] & 0xff));
    }

}

void UtfToSjis() {
    char outmsg[512];
    CodeConvert *o = new CODECONVERT();
    o->Utf8ToSjis((char*)msg_utf8, outmsg, 512);
    printf("sjis:%s\n", outmsg);

    strcpy(outmsg, (char *)msg_sjis);
    printf("sjis:%s\n", outmsg);

    ShowHexDump((char *)msg_sjis);

    delete o;
}

void SjisToUtf8() {
    char outmsg[512];
    CodeConvert *o = new CODECONVERT();
    o->FromSjis((char*)msg_sjis, outmsg, 512);
    printf("sjis -> utf8:%s\n", outmsg);
    ShowHexDump((char *)outmsg);

    strcpy(outmsg, (char *)msg_utf8);
    printf("utf8:%s\n", outmsg);

    ShowHexDump((char *)outmsg);

    delete o;
}

int main(int argc,char *argv[]) {
    // UtfToSjis();
    SjisToUtf8();
    return 0;
}