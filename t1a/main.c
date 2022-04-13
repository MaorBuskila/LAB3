#include <stdio.h>
#include <stdlib.h>

typedef struct virus {
    unsigned short SigSize;
    unsigned char *sig;
    char virusName[16];
} virus;

void virusDistrctor(virus *vir) {
    free(vir->sig);
    free(vir);
}

int getVirusLen(FILE *input) {
    char len[2] = {0, 0};
    fread(len, sizeof(unsigned char), 2, input);
    int result = (unsigned char) len[1] * 256 + (unsigned char) len[0];
    return result;
}

void readVirus(virus *vir, FILE *input) {
    vir->SigSize = getVirusLen(input);
    vir->sig = (unsigned char *) calloc(vir->SigSize, sizeof(char) * vir->SigSize);
    fread(vir->sig, sizeof(unsigned char), vir->SigSize, input); //Fill the signature
    fread(vir->virusName, sizeof(unsigned char), 16, input); //Fill the virus name
}

void printVirus(virus *vir, FILE *output) {
    fprintf(output, "Virus name: %s \n", vir->virusName);
    fprintf(output, "Virus size: %d \n", vir->SigSize);
    fprintf(output, "signature:\n");
    for (int i = 0; i < vir->SigSize; i++) {
        fprintf(output, "%02X ", vir->sig[i]);
    }
    fprintf(output, "\n\n");
}

int main(int argc, char **argv) {
    FILE *input = fopen(argv[1], "rb");
    FILE *output = fopen("output", "w+");
    while (!feof(input)) {
        virus *vir = calloc(1, sizeof(virus));
        readVirus(vir, input);
        if (vir->SigSize == 0) {
            virusDistrctor(vir);
            break;
        }
        printVirus(vir, output);
        virusDistrctor(vir);
    }
    fclose(input);
    fclose(output);
    return 0;
}

