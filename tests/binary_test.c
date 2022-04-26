#include "../src/binary.h"

int main() {
    char buf[8];
    putle16(buf, 1);
    if(buf[0] != 1) return 1;
    putle32(buf, 2);
    if(buf[0] != 2) return 2;
    putle64(buf, 3);
    if(buf[0] != 3) return 3;
    return 0;
}
