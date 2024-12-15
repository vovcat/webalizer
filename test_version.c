#include "version.c"

const struct { const char *k, *v; } version_info[] = {
    {0, 0}
};

#include <stdio.h>

int main(void)
{
    const char *k = FULL_VERSION_key;
    while (k < VERSION_TXT_key) {
        const char *v = k;
        while (*v) { v++; } while (!*v) { v++; }
        printf("%s = %s\n", k, v);
        k = v;
        while (*k) { k++; } while (!*k) { k++; }
    }
}
