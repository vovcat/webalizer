// xcode.C (formerly auto2unix.cc)
// This program tries to determine input document encoding
// and to convert it to koi8, CP-1251 or cp866.

// Written  by Andrey V. Lukyanov on May 14, 1997
// Last modified on May 18, 1997

// Updated to convert to /don't kill me/ cp1251 (oh, god! I hate it!)
// instead of KOI-8 (original)
// by Cyril Rotmistrovsky
// Updated to make conversions to cp1251, cp866 and koi8-r (default)
// depending on flags
// by Cyril Rotmistrovsky
// Modified on Oct 19, 1997
// Modified by Cyril Rotmistrovsky to be compiled by Watcom C++ 10.0
// (Oh, God! what a non-standard compiler!)
// Last modified on Jun 18 1998

// Modified by Igor V. Krassikov (KIV without Co)
// for quoted-printable decodeing

// Name changed to xcode


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef KOI8_WIN
#define KOI8_WIN \
    "\x94\x83\xaa\x8f\x90\xa9\x93\x84\x92\x91\x95\xaf\xac\xab\xad\xae" \
    "\x80\x81\x82\xb4\xbe\xb9\xbb\xb7\xb3\xb2\xbf\xb5\xb8\xbd\xba\xb6" \
    "\x9d\x8a\xa5\xb1\xa6\x99\x88\x87\x8b\xa4\xa3\x98\x8e\x8d\x8c\x96" \
    "\x97\x9c\x85\xb0\x86\x89\xa1\xa2\x9b\x9f\xa0\x9a\xa8\xa7\x9e\xbc" \
    "\xfe\xe0\xe1\xf6\xe4\xe5\xf4\xe3\xf5\xe8\xe9\xea\xeb\xec\xed\xee" \
    "\xef\xff\xf0\xf1\xf2\xf3\xe6\xe2\xfc\xfb\xe7\xf8\xfd\xf9\xf7\xfa" \
    "\xde\xc0\xc1\xd6\xc4\xc5\xd4\xc3\xd5\xc8\xc9\xca\xcb\xcc\xcd\xce" \
    "\xcf\xdf\xd0\xd1\xd2\xd3\xc6\xc2\xdc\xdb\xc7\xd8\xdd\xd9\xd7\xda"
#endif

#ifndef KOI8_ALT
#define KOI8_ALT \
    "\xc4\xb3\xda\xbf\xc0\xd9\xc3\xb4\xc2\xc1\xc5\xdf\xdc\xdb\xdd\xde" \
    "\xb0\xb1\xb2\xf4\xfe\xf9\xfb\xf7\xf3\xf2\xff\xf5\xf8\xfd\xfa\xf6" \
    "\xcd\xba\xd5\xf1\xd6\xc9\xb8\xb7\xbb\xd4\xd3\xc8\xbe\xbd\xbc\xc6" \
    "\xc7\xcc\xb5\xf0\xb6\xb9\xd1\xd2\xcb\xcf\xd0\xca\xd8\xd7\xce\xfc" \
    "\xee\xa0\xa1\xe6\xa4\xa5\xe4\xa3\xe5\xa8\xa9\xaa\xab\xac\xad\xae" \
    "\xaf\xef\xe0\xe1\xe2\xe3\xa6\xa2\xec\xeb\xa7\xe8\xed\xe9\xe7\xea" \
    "\x9e\x80\x81\x96\x84\x85\x94\x83\x95\x88\x89\x8a\x8b\x8c\x48\x8e" \
    "\x8f\x9f\x90\x91\x92\x93\x86\x82\x9c\x9b\x87\x98\x9d\x99\x97\x9a"
#endif

#define NUMCOD 5

unsigned char *destTab[] = {
    (unsigned char *)KOI8_ALT,
    (unsigned char *)KOI8_WIN
};

char *encName[] = { "koi8", "cp866", "cp1251", "iso8859-5", "mac" };

unsigned char recode_table[NUMCOD][128] = {
    { //[1] koi8
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
    160,161,162,163,164,165,166,167,168,169,170,171,172,'-',174,175,
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
    192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
    208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
    },
    { //[2] dos
    225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
    242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
    193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
    128,129,130,131,132,133,134,135,136,137,186,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,191,156,157,158,159,
    160,161,162,176,164,165,166,167,168,169,170,171,172,173,174,175,
    210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
    179,163,'+','+','+','+','+','+',184,'+','+','+','+','+',190,' '
    },
    { //[3] win
    '+','+', 39,'+', 34,'+','+','+','+','+','+', 39,'+','+','+','+',
    '+', 39, 39, 34, 34,'+','+','-','-','*','+', 39,'+','+','+','+',
    ' ','+','I','+','+','+','+','+',179,188,'E', 34,'+','+','*','I',
    184,'+','i',199,'*','*','*','*',163,'N','e', 34,'j','S','s','i',
    225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
    242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
    193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
    210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209
    },
    { //[4] iso
    '+','+','+','+','+','+','+','+','+','+','+','+','+','+','+','+',
    '+','+','+','+','+','+','+','+','+','+','+','+','+','+','+','+',
    '*',179,'*','*','*','*','*','*','*','*','*','*','*','*','*','*',
    225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
    242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
    193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
    210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
    '*',163,'*','*','*','*','*','*','*','*','*','*','*','*','*','*'
    },
    { //[5] mac
    225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
    242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
    '*',184,'*','*','*','*','*','I','*','*','*','*','*','*','*','*',
    '*',179,177,178,'i','*',199,'J','E','e','I','i','*','*','*','*',
    'j','S','*','*','f','*','*','*','*','*','*','*','*','*','*','s',
    '-','-', 34, 34, 39, 39,'*', 39,'*','*','*','*','N',163,179,209,
    193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
    210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,'*'
    }
};

int letter_frequency[32] = {
    125,1606,331,65,598,1714,22,356,168,1312,206,636,1050,658,1295,2259,
    544,447,887,1049,1217,572,200,823,398,400,355,168,67,78,285,4
};

static unsigned char temp[16384];

void xdecode(unsigned char *str)
{
    unsigned int c = 0, c_old;
    int decide[NUMCOD];
    double rating[NUMCOD];
    int a = 0, enc = 2;
    int tpos = 0, opos = 0, ipos = 0;

    for (int i = 0; i < NUMCOD; i++) {
        decide[i] = 1;
        rating[i] = 0;
    }

    while (1) {
        c_old = c;
        c = (unsigned char)str[ipos];
        ipos++;

        if (c == '\r') continue;
        if (c_old == '\r' && c != '\n') { temp[tpos] = '\n'; tpos++; }
        if (c == 0) break;

        if (c >= 128)
            for (int i = 0; i < NUMCOD; i++)
                if (recode_table[i][c-128] >= 192)
                    rating[i] += letter_frequency[
                        ((int)recode_table[i][c-128] - 192) % 32
                    ];
        temp[tpos] = c;
        tpos++;
    }
    temp[tpos] = 0;

    for (int i = 0; i < NUMCOD; i++)
        for (int j = 0; j < NUMCOD; j++)
            if (i != j && rating[i] < rating[j])
                decide[i] = 0;

    for (int i = NUMCOD-1; i >= 0; i--)
        if (decide[i])
            a = i;

    //fprintf(stderr,"Guessed input encoding: %s\n",encName[a]);
    //fprintf(stderr,"Output encoding: %s\n",encName[enc]);

    tpos = 0;
    while (1) {
        c = temp[tpos++];
        if (c == 0) break;
        if (c >= 128) c = recode_table[a][c-128];
        if (enc && c >= 128) c = destTab[enc-1][c-128];
        str[opos] = (char) c;
        opos++;
    }

    //for(i=0; i<NUMCOD; i++) printf("\n%d\n", rating[i]);
}
