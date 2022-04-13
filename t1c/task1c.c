#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct link link;

typedef struct virus {
    unsigned short SigSize;
    unsigned char *sig;
    char virusName[16];
} virus;

struct link {
    link *nextVirus;
    virus *vir;
};

link *virusList;
FILE *output;

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
    vir->sig = (unsigned char *) calloc(vir->SigSize, sizeof(unsigned char));
    fread(vir->sig, sizeof(unsigned char), vir->SigSize, input); //Fill the signature
    fread(vir->virusName, sizeof(unsigned char), 16, input); //Fill the virus name

}

void printVirus(virus *vir, FILE *output) {
    fprintf(output, "Virus name: %s \n", vir->virusName);
    fprintf(output, "Virus size: %d \n", vir->SigSize);
    fprintf(output, "signature:\n");

    for (int i = 0; i < vir->SigSize; i++) {
        fprintf(output, "%X ", vir->sig[i]);
    }
    fprintf(output, "\n\n");

}

void listPrint(link *virus_list, FILE *output) {
    /* Print the data of every link in list to the given stream. Each item followed by a newline character. */
    link *curr = virus_list;
    while (curr->nextVirus != NULL) {
        printVirus(curr->vir, output);
        curr = curr->nextVirus;
    }

}

int findSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);
    return size;
}

link *listAppend(link *virus_list, link *to_add) {
    if (virus_list == NULL) {
        virus_list = to_add;
        return virus_list;
    }

    link *curr = virus_list;
    link *first = virus_list;
    while (curr->nextVirus != NULL)
        curr = curr->nextVirus;
    curr->nextVirus = to_add;

    return first;
}

void listFree(link *virus_list) {
    /* Free the memory allocated by the list. */
    link *curr = virus_list;
    while (curr != NULL) {
        virus *temp_vir = curr->vir;
        link *temp_link = curr;
        curr = curr->nextVirus;
        virusDistrctor(temp_vir);
        free(temp_link);
    }
}


void quit() {
    listFree(virusList);
    fclose(output);
    exit(0);
}

struct fun_desc {
    char *name;
    void(*fun);
};

void printSignatures() {
    if (virusList == NULL) {
        return;
    }
    link *curr = virusList;
    while (curr != NULL) {
        if (curr->vir->SigSize > 50)
            printVirus(curr->vir, stdout);
        curr = curr->nextVirus;
    }
}

void loadSignatures() {
    char inp[15];
    scanf("%s", inp);
    FILE *input = fopen(inp, "rb");

    if (input == NULL) {
        printf("No valid file");
        return;
    }

    link *virus_list = (link *) calloc(1, sizeof(link));
    virus *vir = (virus *) calloc(1, sizeof(virus));
    readVirus(vir, input);
    virus_list->vir = vir;
    link *curr = virus_list;

    while (!feof(input)) {
        //Create next virus
        link *next = (link *) calloc(1, sizeof(link));
        virus *vir = (virus *) calloc(1, sizeof(virus));
        readVirus(vir, input);
        if (vir->SigSize >= 60000 || vir->SigSize == 0) {
            virusDistrctor(vir);
            free(next);
            break;
        }

        next->vir = vir;
        curr = listAppend(curr, next);
        curr = curr->nextVirus;
    }


    virusList = virus_list;
    fclose(input);
}

void detectVirus(char *buffer, unsigned int size, link *virus_list, FILE *input) {

    if (virus_list == NULL) {
        printf("No viruses has been loaded first");
        return;
    }
    fread(buffer, sizeof(unsigned char), size, input);

    for (int i = 0; i < size; i++) {
        link *curr = virus_list;
        while (curr != NULL) {
            if (curr->vir->SigSize > size - i) { // if the virus signatures is longer than what is left to read
                curr = curr->nextVirus;
                continue;
            }
            int indicator = memcmp(buffer + i, curr->vir->sig, curr->vir->SigSize);
            if (!indicator) {
                printf("Starting byte: %d\n", i);
                printf("Virus name: %s\n", curr->vir->virusName);
                printf("Size of Virus: %d\n", curr->vir->SigSize);
            }
            curr = curr->nextVirus;
        }
    }
}

void detect() {
    char *buffer = (char *) malloc(10000 * sizeof(unsigned char));
    char fileName[15];
    printf("Enter name of file to be scanned \n");
    scanf("%s", fileName);
    FILE *input = fopen(fileName, "rb");

    if (input == NULL) {
        printf("No valid file");
        return;
    }
    int fileSize = findSize(input);
    int size = fileSize > 10000 ? 10000 : fileSize;
    detectVirus(buffer, size, virusList, input);
    free(buffer);
    fclose(input);
}

void killVirus(char *fileName, int signatureOffset, int signatureSize) {
    FILE *input = fopen(fileName, "rb+");
    fseek(input, signatureOffset, SEEK_SET);
    printf("%ld \n", ftell(input));
    printf("%d\n", signatureOffset);
    char nopArr[signatureSize];

    for (int i = 0; i < signatureSize; i++)
        nopArr[i] = 0x90;

    fwrite(nopArr, sizeof(unsigned char), signatureSize, input);
    fclose(input);

}

void fixFile() {
    int startingByte;
    int signatureSize;
    char fileName[15];
    printf("Enter starting byte location in the suspected file \n");
    scanf("%d", &startingByte);
    printf("Enter the size of the virus signature \n");
    scanf("%d", &signatureSize);
    printf("Enter the file name \n");
    scanf("%s", fileName);
    killVirus(fileName, startingByte, signatureSize);
}

struct fun_desc menu[] =
        {{"Load signatures",  loadSignatures},
         {"Print signatures", printSignatures},
         {"Detect Viruses",   detect},
         {"Fix file",         fixFile},
         {"Quit",             quit},
         {NULL, NULL}};

void printMenu(int length) {
    printf("#> menu \n");
    printf("Please choose a function: \n");

    for (int i = 1; i <= length; i++) {
        printf("%d) %s \n", i, menu[i - 1].name);
    }
    printf("Option: ");
    fflush(stdin);
}


int main(int argc, char **argv) {
    output = fopen("output", "w+");
    while (1) {
        int functionIndex;
        printMenu(5);
        scanf("%d", &functionIndex);
        fgetc(stdin); // skip '\n'
        if (functionIndex < 1 || functionIndex > 5) {
            printf("Index isn't valid, please try again  \n");
            continue;
        }
        void (*func_ptr)() = menu[functionIndex - 1].fun;
        (*func_ptr)();
    }

}

