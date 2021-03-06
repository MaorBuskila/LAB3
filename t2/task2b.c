#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
    unsigned short SigSize;
    unsigned char *sig;
    char virusName[16];
} virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};

link *virusList;
FILE *output;

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
    while (curr!= NULL) {
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

link *listAppend(link *virus_list, link *add_to_next) {
    if (virus_list == NULL) {
        virus_list = add_to_next;
        return virus_list;
    }
    link *curr = virus_list;
    link *first = virus_list;
    while (curr->nextVirus != NULL)
        curr = curr->nextVirus;
    curr->nextVirus = add_to_next;

    return first;
}
void virusDistrctor(virus *vir) {
    free(vir->sig);
    free(vir);
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
    listPrint(virusList, stdout);
}

void loadSignatures() {
    char filename[256];
    scanf("%s", filename);
//    FILE *input = fopen("/home/spl211/Desktop/LAB3/t2/signatures", "rb");
    FILE *input = fopen(filename, "rb");
    if (input == NULL) {
        printf("File is not valid!\n");
        return;
    }
    link *virus_list = (link *) calloc(1, sizeof(link));
    virus *vir = (virus *) calloc(1, sizeof(virus));
    readVirus(vir, input);
    virus_list->vir = vir;
    link *curr = virus_list;

    while (!feof(input)) {
        link *next = (link *) calloc(1, sizeof(link));
        virus *vir = (virus *) calloc(1, sizeof(virus));
        readVirus(vir, input);
        next->vir = vir;
        if (vir->SigSize == 0) {
          virusDistrctor(vir);
          free(next);
          break;
        }
        curr = listAppend(curr, next);
        curr = curr->nextVirus;
    }
    virusList = virus_list;
    fclose(input);
}

void detectVirus(char *buffer, unsigned int size, link *virus_list, FILE *input) {
    if (virus_list == NULL) {
        printf("Virus list is empty");
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
            int cmp_flag = memcmp(buffer + i, curr->vir->sig, curr->vir->SigSize);
            if (!cmp_flag) {
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
    char fileName[256];
    printf("Enter name of file \n");
    scanf("%s", fileName);
    FILE *input = fopen("/home/spl211/Desktop/LAB3/t2/infected", "rb");
//    FILE *input = fopen(fileName, "rb");

    if (input == NULL) {
        printf("File is not valid");
        return;
    }
    int fileSize = findSize(input);
    int size = fileSize > 10000 ? 10000 : fileSize;
    detectVirus(buffer, size, virusList, input);
    free(buffer);
    fclose(input);
}

void killVirus(char *fileName, int signatureOffset, int signatureSize) {
//    FILE *input = fopen("/home/spl211/Desktop/LAB3/t2/infected", "rb+");
    FILE *input = fopen(fileName, "rb+");
    if (input == NULL) {
        printf("File is not valid\n");
        return;
    }
    fseek(input, signatureOffset, SEEK_SET);
    char nopArray[signatureSize];
    for (int i = 0; i < signatureSize; i++)
        nopArray[i] = 0x90; // NOP (eax)

    fwrite(nopArray, sizeof(unsigned char), signatureSize, input);
    fclose(input);

}

void fixFile() {
    int startingByte;
    int signatureSize;
    char fileName[256];
    printf("Enter starting byte location in the infected file \n");
    scanf("%d", &startingByte);
    printf("Enter the size of the virus signature \n");
    scanf("%d", &signatureSize);
    printf("Enter the file name \n");
    scanf("%s", fileName);
    killVirus(fileName, startingByte, signatureSize);
    printf("Fixing Complete \n");
}

struct fun_desc menu[] =
        {{"Load signatures",  loadSignatures},
         {"Print signatures", printSignatures},
         {"Detect Viruses",   detect},
         {"Fix file",         fixFile},
         {"Quit",             quit},
         {NULL, NULL}};

void printMenu(int length) {
    printf("#> menu \n Please choose a function: \n");
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
        fgetc(stdin); //Skip '\n'
        if (functionIndex >= 1 && functionIndex <= 5) {
            void (*func_ptr)() = menu[functionIndex - 1].fun;
            (*func_ptr)();
        } else {
            printf("Index isn't valid\n");
            continue;
        }
    }
}