#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _D3DVertexShaderFile {
    uint16_t    Magic;
    uint16_t    Len;
    uint32_t    Inst[];
} D3DVERTEXSHADERFILE;

#define D3DVERTEXSHADERFILE_MAGIC_NORMAL    0x2078 // "x "
#define D3DVERTEXSHADERFILE_MAGIC_STATE     0x7378 // "xs"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

bool is_big_endian = false;

uint32_t u32tole(uint32_t a) {
    if (is_big_endian)
        return __builtin_bswap32(a);

    return a;
}

uint16_t u16tole(uint16_t a) {
    if (is_big_endian)
        return __builtin_bswap16(a);

    return a;
}

typedef union endian_test {
    uint16_t u16;
    uint8_t  u8[2];
} endian_test;

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("USAGE: vshc [-o output] <filename>\n");
        return 1;
    }

    endian_test test;
    test.u16 = 0x00FF;
    if (test.u8[0] == 0)
        is_big_endian = true;

    const char* output = "a.vsh";
    int i = 1;
    if (strncmp(argv[1], "-o", 2) == 0) {
        output = argv[2];
        i = 3;
    }

    FILE* f = fopen(argv[i], "r");
    if (f == NULL) {
        printf("Invalid path: '%s'\n", argv[i]);
        return 2;
    }

    D3DVERTEXSHADERFILE* prog = malloc(sizeof(*prog) + (136 * 4 * sizeof(uint32_t)));
    if (prog == NULL) {
        printf("Allocation failed (OOM?)\n");
        return 3;
    }
    prog->Magic = D3DVERTEXSHADERFILE_MAGIC_NORMAL;
    memset(prog->Inst, 0, (136 * 4 * sizeof(uint32_t)));

    char line[256];
    i = 0;
    while (!feof(f) && fgets(line, sizeof(line), f) != NULL && i < 136) {
        if (line[0] == '/' && line[1] == '/')
            continue;
        uint32_t* inst = (uint32_t*)&prog->Inst[i * 4];
        char comma;
        sscanf(line, "%010x%c %010x%c %010x%c %010x%c",
               &inst[0], &comma, &inst[1], &comma,
               &inst[2], &comma, &inst[3], &comma);
        i++;
    }
    // TODO: should this be in DWORDs or bytes?
    prog->Len = i * 4;
    fclose(f);
    size_t len = prog->Len;

    prog->Magic     = u16tole(prog->Magic);
    prog->Len       = u16tole(prog->Len);
    for (i = 0; i < len; i++) {
        prog->Inst[i] = u32tole(prog->Inst[i]);
    }

    f = fopen(output, "wb");
    if (f == NULL) {
        printf("Invalid output path: '%s'\n", output);
        return 3;
    }
    fwrite(prog, sizeof(*prog) + (len * sizeof(*prog->Inst)), 1, f);
    fclose(f);
}
