#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define L 0
#define S 1
#define WRITE_ALLOCATE 0
#define NO_WRITE_ALLOCATE 1
#define WRITE_THROUGH 0
#define WRITE_BACK 1
#define LRU 0
#define FIFO 1
#define RANDOM 2

struct _info
{
  int Total_loads;
  int Total_stores;
  int Load_hits;
  int Load_misses;
  int Store_hits;
  int Store_misses;
  int Total_cycles;
} info;

struct _setting
{
  int sets;
  int blocks;
  int bytes;
  int allo;
  int write;
  int policies;
  int rate;
} setting;

struct _instruction
{
  int type;
  int address;
  unsigned int tag;
  int sets;
} instruction;

struct _block
{
  char valid;
  char dirty;
  unsigned int tag;
  char *bytes;
};

struct _block **cache;

void (*load_cache)();
void (*store_cache)();
void (*policies)();

int addr_to_int(char *str);

void init();
void run();

int load_from_mem();

void load_direct_mapped();
void load_set_associative();
void load_fully_associative();

void store_direct_mapped();
void store_set_associative();
void store_fully_associative();
