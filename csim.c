#include <stdio.h>
#include "cache.h"
#include <stdlib.h>
#include <string.h>

char *command;

int is_power(int n);
void show_help();
void set_setting(char *str[]);

int main(int argc, char *argv[])
{
  char type[20];
  char address[20];
  char buffer[20];

  // get setting
  command = argv[0];
  if (argc != 7)
    show_help();
  set_setting(argv + 1);
  init();

  while (1)
  {
    scanf("%s ", buffer);
    strcpy(type, buffer);
    scanf("%s ", buffer);
    strcpy(address, buffer);
    scanf("%s ", buffer);
    if (strlen(address) < 10)
      break;

    // printf("%s %s\n", type, address);
    instruction.type = !strcmp(type, "l") ? L : S;
    instruction.address = strtol(address, NULL, 0);
    // printf("%s %s\n",type,address);
    run();
  }
  printf("Total_loads: %d\n", info.Total_loads);
  printf("Total_stores: %d\n", info.Total_stores);
  printf("Load_hits: %d\n", info.Load_hits);
  printf("Load_misses: %d\n", info.Load_misses);
  printf("Store_hits: %d\n", info.Store_hits);
  printf("Store_misses: %d\n", info.Store_misses);
  printf("Total_cycles: %d\n", info.Total_cycles);
}

int is_power(int n)
{
  return !(n & (n - 1));
}

void show_help()
{
  printf("\t1: sets in the cache (a positive power-of-2)\n");
  printf("\t2: blocks in each set (a positive power-of-2)\n");
  printf("\t3: bytes in each block (a positive power-of-2, at least 4)\n");
  printf("\t4: write-allocate or no-write-allocate\n");
  printf("\t5: write-through or write-back\n");
  printf("\t6: lru (least-recently-used), fifo, or random evictions\n");
  printf("%s {1} {2} {3} {4} {5} {6}\n", command);
  exit(1);
}

void set_setting(char *str[])
{
  int sets = atoi(str[0]);
  int blocks = atoi(str[1]);
  int bytes = atoi(str[2]);

  if (sets > 0 && is_power(sets))
    setting.sets = sets;
  else
  {
    printf("1\n");
    show_help();
  }

  if (blocks > 0 && is_power(blocks))
    setting.blocks = blocks;
  else
  {
    printf("2\n");
    show_help();
  }
  if (bytes > 0 && is_power(bytes))
  {
    setting.bytes = bytes;
    setting.rate = bytes / 4;
  }
  else
  {
    printf("3\n");
    show_help();
  }
  if (!strcmp(str[3], "write-allocate"))
    setting.allo = WRITE_ALLOCATE;
  else if (!strcmp(str[3], "no-write-allocate"))
    setting.allo = NO_WRITE_ALLOCATE;
  else
  {
    printf("4\n");
    show_help();
  }
  if (!strcmp(str[4], "write-through"))
    setting.write = WRITE_THROUGH;
  else if (!strcmp(str[4], "write-back"))
    setting.write = WRITE_BACK;
  else
  {
    printf("5\n");
    show_help();
  }
  if (!strcmp(str[5], "lru"))
    setting.policies = LRU;
  else if (!strcmp(str[5], "fifo"))
    setting.policies = FIFO;
  else if (!strcmp(str[5], "random"))
    setting.policies = RANDOM;
  else
  {
    printf("6\n");
    show_help();
  }
}