#include <stdio.h>
#include <stdlib.h>

void printHex(unsigned char* buffer, int len){
  for(int i = 0; i < len; i++){
    printf("%X ", buffer[i]);
  }
}

int main(int argc, char **argv) {
  FILE * fp = fopen(argv[1],"rb");
  unsigned char* buffer;
  fseek(fp,0,SEEK_END); //Takes file position to the end
  int len = ftell(fp); //Return the index of the last charcter = Num of chars in file
  rewind(fp); //Takes file position to the start
  
  buffer = (unsigned char*) malloc(sizeof(char)* len);
  fread(buffer, sizeof(unsigned char), len, fp);
  
  printHex(buffer,len);
  fclose(fp);
  free(buffer);
  printf("\n");
  return 0;
}

