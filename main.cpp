#include <stdlib.h>

#include "gaterecorder.h"

using namespace kfr;

int main(int argc, char ** argv)
{
    float l = -24;
    if (argc > 1)
        l = atof(argv[1]);
    GateRecorder gr(l);

    while(1)
        sleep(100500);
    return 0;
}
