///alm.c
///Reads the obj module for MIPS R2K and
///prints a symbol table report to stdout
///@author : Chris Guarini
///CS243 HW7


//INCLUDES
//libraries
#include <stdio.h>
#include <stdlib.h>
//headers
#include "exec.h"

//MACROS
///read16, calls fread to read the file for a 16 bit buffer
#define read16 fread(&ptr16,2,1,file)
///read32 calls fread to read the file for a 32 bit buffer
#define read32 fread(&ptr32,4,1,file)


///readWord
///Reads the big endian word and returns it in the correct,
///little endian order.
///@param : word - 32bit word to read
uint32_t readWord(uint32_t word){
  uint32_t littleEndian = 0;
  //Convert to little endian
  for(int i = 0; i < 4; i++){
    //get the correct bytes
    littleEndian += ((word << (24-(8*i))) >> 24);
    
    if(i != 3){
    //Shift left a byte to make room for next byte
      littleEndian = littleEndian << 8;
    }
  }
  return littleEndian;
}

///readHW
///Reads the big endian half word and returns it in the correct
///little endian order
///@param : hw - 16 bit half word to read
uint16_t readHW(uint16_t hw){
  uint16_t littleEndian = 0;
  //convert to little endian
  
  //get MSByte
  littleEndian = (hw << 8);

  //get LSByte
  littleEndian += (hw >> 8);

  return littleEndian;
}

///readOBJ()
///Takes the file pointer and reads  it, creating the symbol table for the obj
///file
///@param : file - pointer to the file
///         filename - name of the file
void readOBJ(FILE * file, char * filename){
  //Check if file exists  
  if(!file){//File not found
    fprintf(stderr, "FILE NOT FOUND : %s", filename);
    return;
  }
  //Read the file
  exec_t table; //Create the table struct
  //buffers used by fread
  uint16_t ptr16; //16 bit buffer
  uint32_t ptr32; //32 bit buffer

  //Get magic number
  read16;
  table.magic = readHW(ptr16);
  //Get the version
  read16;
  table.version = readHW(ptr16);
  
  //printf("magic: %#06x\nversion: %#06x\n",table.magic,table.version);
  //Debug^

  //Check if file is MIPS R2K OBJ
  if(table.magic != 0xface){
    fprintf(stderr, "error: %s is not an R2K object module (magic number %#6x)\n",
        filename, table.magic);
    return;
  }
  
  //print 20 hyphons
  printf("--------------------\n");

  //Retrieve entry point
  read32;//skip null
  read32;
  table.entry = readWord(ptr32);
  
  if(table.entry){//entry exists, file is load module
     printf("File %s is an R2K load module (entry point %#010x)\n", filename, table.entry);
  }
  else{//object module
     printf("File %s is an R2k object module\n", filename);
  }

  

}


///main function
///Takes arguments from the command line, treats them as MIPS obj files
///And reads and prints them by calling readOBJ.
///@arguments : File names
int main(int argc, char * argv[]){
  
  //Check for arguments
  if(argc < 2){//No arguments
    fprintf(stderr, "usage: alm file1 [ file2 ... ]\n");
    return EXIT_FAILURE;
  }  
  
  //get file pointers and read them
  for(int i = 1; i < argc; i++){
    //open the file
    FILE * file = fopen(argv[i], "r");
    //send to readOBJ to do the heavy lifting
    readOBJ(file,argv[i]);
  }
 

}
