#include "cache.h"

void init()
{
  // info set
  info.Total_loads = 0;
  info.Total_stores = 0;
  info.Load_hits = 0;
  info.Load_misses = 0;
  info.Store_hits = 0;
  info.Store_misses = 0;
  info.Total_cycles = 0;

  // set cache
  cache = (struct _block **)malloc(sizeof(struct _block *) * setting.sets);
  for (int i = 0; i < setting.sets; i++)
  {
    cache[i] = (struct _block *)malloc(sizeof(struct _block) * setting.blocks);
    for (int j = 0; j < setting.blocks; j++)
    {
      cache[i][j].valid = 0;
      cache[i][j].dirty = 0;
      // 사용하진 않지만 기분만
      cache[i][j].bytes = (char *)malloc(sizeof(char) * setting.bytes);
    }
  }
}

void run()
{
  // separate address
  unsigned int temp = instruction.address;
  temp = temp >> (int)log2(setting.bytes);
  instruction.sets = temp & (setting.sets - 1);
  temp = temp >> (int)log2(setting.sets);
  instruction.tag = temp;

  if (instruction.type == L)
  {
    info.Total_loads += 1;
    load_cache();
  }
  else if (instruction.type == S)
  {
    info.Total_stores += 1;
    store_cache();
  }
  else
  {
    printf("type error\n");
    exit(1);
  }
}

// cache stroe
void write_back(int n)
{
  if (setting.write == WRITE_BACK)
  {
    if (cache[instruction.sets][n].dirty)
    {
      info.Total_cycles += 100 * setting.rate;
    }
  }
}

void set_block(int n)
{
  cache[instruction.sets][n].valid = 1;
  cache[instruction.sets][n].tag = instruction.tag;
  cache[instruction.sets][n].dirty = 0;
}

void lru_move(int n)
{
  // set line
  if (setting.policies == LRU)
  {
    struct _block temp;
    temp = cache[instruction.sets][n];
    for (int j = n; j > 0; j--)
    {
      cache[instruction.sets][j] = cache[instruction.sets][j - 1];
    }
    cache[instruction.sets][0] = temp;
  }
}

int load_from_mem()
{
  for (int i = 0; i < setting.blocks; i++)
  {
    if (!cache[instruction.sets][i].valid)
    {
      // valid miss
      for (int j = i; j > 0; j--)
      {
        cache[instruction.sets][j] = cache[instruction.sets][j - 1];
      }
      set_block(0);
      return 0;
    }
  }

  // tag miss
  if (setting.policies == LRU || setting.policies == FIFO)
  {
    write_back(setting.blocks - 1);

    for (int j = setting.blocks - 1; j > 0; j--)
    {
      cache[instruction.sets][j] = cache[instruction.sets][j - 1];
    }
    set_block(0);
    return 0;
  }
  else
  {
    srand(time(NULL));
    int r = rand();
    r %= setting.blocks;

    write_back(r);
    set_block(r);
    return r;
  }
}

void load_cache()
{
  for (int i = 0; i < setting.blocks; i++)
  {
    if (cache[instruction.sets][i].valid)
    {
      if (cache[instruction.sets][i].tag == instruction.tag)
      {
        // hit
        info.Load_hits += 1;
        info.Total_cycles += 1;

        lru_move(i);
        return;
      }
    }
    else
    {
      // valid miss
      info.Load_misses += 1;

      load_from_mem();
      info.Total_cycles += 100 * setting.rate;
      info.Total_cycles += 1;
      return;
    }
  }

  // tag miss
  info.Load_misses += 1;

  load_from_mem();
  info.Total_cycles += 100 * setting.rate;
  info.Total_cycles += 1;
}

void store_cache()
{
  for (int i = 0; i < setting.blocks; i++)
  {
    if (cache[instruction.sets][i].valid)
    {
      if (cache[instruction.sets][i].tag == instruction.tag)
      {
        // hit
        info.Store_hits += 1;
        if (setting.write == WRITE_THROUGH)
        {
          info.Total_cycles += 100 * setting.rate;
          return;
        }
        else if (setting.write == WRITE_BACK)
        {
          cache[instruction.sets][i].dirty = 1;
          info.Total_cycles += 1;

          lru_move(i);
          return;
        }
      }
    }
    else
    {
      // valid miss
      info.Store_misses += 1;

      // load from mem
      if (setting.allo == NO_WRITE_ALLOCATE)
      {
        info.Total_cycles += 100 * setting.rate;
      }
      else if (setting.allo == WRITE_ALLOCATE)
      {
        info.Total_cycles += 100 * setting.rate;
        int loc = load_from_mem();
        cache[instruction.sets][loc].dirty = 1;
        info.Total_cycles += 1;
      }
      return;
    }
  }

  // tag miss
  info.Store_misses += 1;

  if (setting.allo == NO_WRITE_ALLOCATE)
  {
    info.Total_cycles += 100 * setting.rate;
  }
  else if (setting.allo == WRITE_ALLOCATE)
  {
    info.Total_cycles += 100 * setting.rate;

    int loc = load_from_mem();
    cache[instruction.sets][loc].dirty = 1;
    info.Total_cycles += 1;
  }
}
