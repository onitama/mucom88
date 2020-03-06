#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pcmtool.h"

int main(int argc, char *argv[])
{
    printf("MUCOM88 pcmtool\n");
    if (argc < 2)
    {
        printf("usage pcmtool <pcmlist.txt>\n");
        return 0;
    }
    PcmTool *p = new PcmTool();
    p->Convert(argv[1]);
    delete p;
    return 0;
}
