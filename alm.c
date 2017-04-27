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
///read8 calls fread to read the file for an 8 bit buffer
#define read8  fread(&ptr8,1,1,file)

char * strcpy(char * dest, const char * src);
void * memcpy(void *str1, const void *str2, size_t n);


///readWord
///Reads the big endian word and returns it in the correct,
///little endian order.
///@param : word - 32bit word to read
uint32_t readWord(uint32_t word){
  uint32_t littleEndian = 0;
  //Convert to little endian
  for(int i = 0; i < 4; i++){
    uint32_t temp;
    //get the correct bytes
    temp = ((word << (24-(8*i))));
    temp = (temp >> 24);
    littleEndian += temp;
    
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
int readOBJ(FILE * file, char * filename){
  //Check if file exists  
  if(!file){//File not found
    fprintf(stderr, "FILE NOT FOUND : %s", filename);
    return 0;
  }
  //Read the file
  exec_t table; //Create the table struct
  //buffers used by fread
  uint16_t ptr16; //16 bit buffer
  uint32_t ptr32; //32 bit buffer
  uint8_t  ptr8;  //8 bit buffer

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
    return 2;
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

  //Print version number
  uint16_t year, month, day = 0;
  //Decode the year
  year = table.version >> (16-7);
  
  //Decode the month
  //Compiler simplifiers simplifiers two shifts into only 1, found that out the
  //hard way, so must be 2 lines.
  month = ((table.version << 7));
  month = (month >> 12);
  
  //Decode the day
  day = table.version << (11);
  day = day >> (11);


  //Print module version
  printf("Module version: 2%03d/%02d/%02d\n", year, month, day);
  
  
  //Grab sections
  for(int i = 0; i < N_EH; i++){
    //Read each section and store in data array in table
    read32;
    table.data[i] = readWord(ptr32);   
  }
  
  //Print sections
  char * sectionNames[] = {"text", "rdata", "data","sdata", "sbss", "bss", "reloc", "ref", "symtab", "strings"};
  
  for(int i = 0; i < N_EH; i++){
    //Print every section that exists
    if(table.data[i]){
      char str[10];
      if(i > 5 && i < 9){//reloc, ref and symtab
        strcpy(str,"entries");
      }
      else{
        strcpy(str,"bytes");
      }
      printf("Section %s is %d %s long\n", sectionNames[i], table.data[i], str);
    }
  }
  


  //Create string table
  uint32_t uintTable[table.sz_strings/4];
  
  //Get size of everything up to string table
  int size = 52;//52 bytes in header block
  for(int i=0; i<EH_IX_STR; i++){
    if(i<6){
      size += table.data[i];
    }
    else{
      size += (12*table.data[i]);
    }
  }
  
  //Retrieve information from string table
  fseek(file, size, SEEK_SET);
  for(size_t i=0; i < table.sz_strings/4; i++){
    read32;
    uintTable[i] = ptr32;
  }

 

  //Create an array of strings
  char strings[table.sz_strings];
  //Copy uints into string array
  memcpy(strings,uintTable,table.sz_strings);
  



  //Relocation
  //check if relocation table is empty
  if(table.data[EH_IX_REL]){
    //introductory line
    printf("Relocation information:\n");
    
    //Get to relocation section
    //size is size of file up to the string table
    size_t relSize = (size - 12*(table.n_syms + table.n_refs + table.n_reloc));
    fseek(file,relSize,SEEK_SET);//set file pointer to relocation entries
    //printf("%d\n",relSize);
    //loop through every entry
    for(size_t i = 0; i < table.n_reloc; i++){
      relent_t entry;
      read32;
      entry.addr = readWord(ptr32);
      read8;
      entry.section = ptr8;
      read8;
      entry.type = ptr8;
      read16;
      
      /*
      //Check type to print
      char type[10];
      if(entry.type == REL_IMM){
        strcpy(type, "IMM");
      }
      else if(entry.type == REL_IMM_2){
        strcpy(type, "IMM_2");
      }
      else if(entry.type == REL_WORD){
        strcpy(type, "WORD");
      }
      else if(entry.type == REL_JUMP){
        strcpy(type, "JUMP");
      }
      else{//not defined
        strcpy(type, "UNKNOWN");
      }
      */
      printf("\t%#010x (%d) type %#06x\n", entry.addr, entry.section, entry.type);
    }
  }

  //Reference
  //check if reference table is empty
  if(table.n_refs){
    //introductory line
    printf("Reference information:\n");

    //Get to reference section
    size_t refSize = (size -12*(table.n_syms + table.n_refs));
    fseek(file, refSize, SEEK_SET);//set file to reference section
    //printf("%d\n",refSize); 

    //loop through the entries
    for( size_t i = 0; i < table.n_refs; i++){
      refent_t refTable;
      read32;
      refTable.addr = readWord(ptr32);
      read32;
      refTable.sym = readWord(ptr32);
      read8;
      refTable.section = ptr8;
      read8;
      refTable.type = ptr8;
      read16;
      printf("%#x",refTable.sym);
      printf("\t%#010x type %#06x symbol %s\n", refTable.addr, refTable.type, strings+refTable.sym);
    }
  }

  //Symbol Table
  //Check if table is empty
  if(table.n_syms){
    //Introductory line
    printf("Symbol table:\n");
    
    //get to symbol table section
    size_t symSize = (size - (table.n_syms * 12));
    fseek(file, symSize, SEEK_SET);//set file to symbol section

    //Loop through the entries
    for( size_t i = 0; i < table.n_syms; i++){
      syment_t sym;
      read32;
      sym.flags = readWord(ptr32);
      read32;
      sym.value = readWord(ptr32);
      read32;
      sym.sym = readWord(ptr32);
      printf("\tvalue %#010x flags %#010x symbol %s\n", sym.value, sym.flags, strings + sym.sym);
    }
  }

  //print final hyphons
  printf("--------------------\n");
  return 1;

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
    if(readOBJ(file,argv[i])){
      fclose(file);
    }
    
  }
 

}
