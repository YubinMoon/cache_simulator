#include "cache.h"

int log2(unsigned int n)
{
  int i = 0;
  while (1)
  {
    if (n == 1)
      return i;
    n = n >> 1;
    i++;
  }
}

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

  // set fun
  if (setting.sets == 1)
  {
    load_cache = load_fully_associative;
    store_cache = store_fully_associative;
  }
  else if (setting.blocks == 1)
  {
    load_cache = load_direct_mapped;
    store_cache = store_direct_mapped;
  }
  else
  {
    load_cache = load_set_associative;
    store_cache = store_set_associative;
  }

  if (setting.allo == WRITE_ALLOCATE)

    // set cache
    cache = (struct _block **)malloc(sizeof(struct _block *) * setting.sets);
  for (int i = 0; i < setting.sets; i++)
  {
    cache[i] = (struct _block *)malloc(sizeof(struct _block) * setting.blocks);
    for (int j = 0; j < setting.blocks; j++)
    {
      cache[i][j].valid = 0;
      cache[i][j].dirty = 0;
      cache[i][j].bytes = (char *)malloc(sizeof(char) * setting.bytes);
    }
  }
}

void run()
{
  unsigned int temp = instruction.address;
  temp = temp >> log2(setting.bytes);
  instruction.sets = temp & (setting.sets - 1);
  temp = temp >> log2(setting.sets);
  instruction.tag = temp;

  if (info.Total_loads != (info.Load_hits + info.Load_misses))
  {
    printf("%d\n", info.Total_loads);
    exit(1);
  }

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

int load_from_mem()
{
  for (int i = 0; i < setting.blocks; i++)
  {
    if (cache[instruction.sets][i].valid)
    {
      if (cache[instruction.sets][i].tag == instruction.tag)
      {
        // hit
        // info.Load_hits += 1;
        // info.Total_cycles += 1;
        return -1;
      }
    }
    else
    {
      // valid miss
      // info.Load_misses += 1;
      // info.Total_cycles += 100 * setting.rate;

      for (int j = i; j > 0; j--)
      {
        cache[instruction.sets][j] = cache[instruction.sets][j - 1];
      }
      cache[instruction.sets][0].valid = 1;
      cache[instruction.sets][0].tag = instruction.tag;
      cache[instruction.sets][0].dirty = 0;
      return 0;
    }
  }

  // tag miss
  // info.Load_misses += 1;
  // info.Total_cycles += 100 * setting.rate;

  if (setting.policies == LRU || setting.policies == FIFO)
  {
    if (setting.write == WRITE_BACK)
    {
      if (cache[instruction.sets][setting.blocks - 1].dirty)
      {
        // cache stroe
        info.Total_cycles += 100 * setting.rate;
      }
    }

    for (int j = setting.blocks - 1; j > 0; j--)
    {
      cache[instruction.sets][j] = cache[instruction.sets][j - 1];
    }
    cache[instruction.sets][0].valid = 1;
    cache[instruction.sets][0].tag = instruction.tag;
    cache[instruction.sets][0].dirty = 0;
    return 0;
  }
  else
  {
    srand(time(NULL));
    int r = rand();
    r %= setting.blocks;

    if (setting.write == WRITE_BACK)
    {
      if (cache[instruction.sets][r].dirty)
      {
        // cache stroe
        info.Total_cycles += 100 * setting.rate;
      }
    }

    cache[instruction.sets][r].valid = 1;
    cache[instruction.sets][r].tag = instruction.tag;
    cache[instruction.sets][r].dirty = 0;
    return r;
  }
}

void load_direct_mapped()
{
  struct _block *block = &cache[instruction.sets][0];
  if (block->valid)
  {
    if (block->tag == instruction.tag)
    {
      // hit
      info.Load_hits += 1;
      info.Total_cycles += 1;
    }
    else
    {
      // tag miss
      info.Load_misses += 1;
      load_from_mem();
      info.Total_cycles += 100 * setting.rate;
    }
  }
  else
  {
    // valid miss
    info.Load_misses += 1;
    load_from_mem();
    info.Total_cycles += 100 * setting.rate;
  }
}

void load_set_associative()
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

        // set line
        if (setting.policies == LRU)
        {
          struct _block temp;
          temp = cache[instruction.sets][i];
          for (int j = i; j > 0; j--)
          {
            cache[instruction.sets][j] = cache[instruction.sets][j - 1];
          }
          cache[instruction.sets][0] = temp;
        }
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

void load_fully_associative()
{
  for (int i = 0; i < setting.blocks; i++)
  {
    if (cache[0][i].valid)
    {
      if (cache[0][i].tag == instruction.tag)
      {
        // hit
        info.Load_hits += 1;
        info.Total_cycles += 1;

        // set line
        if (setting.policies == LRU)
        {
          struct _block temp;
          temp = cache[0][i];
          for (int j = i; j > 0; j--)
          {
            cache[0][j] = cache[0][j - 1];
          }
          cache[0][0] = temp;
        }
        return;
      }
    }
    else
    {
      // valid miss
      info.Load_misses += 1;

      load_from_mem();
      info.Total_cycles += 1;
    }
  }

  // tag miss
  info.Load_misses += 1;

  load_from_mem();
  info.Total_cycles += 100 * setting.rate;
}

void store_direct_mapped()
{
  struct _block *block = &cache[instruction.sets][0];
  int loc;
  if (block->valid)
  {
    if (block->tag == instruction.tag)
    {
      // hit
      if (setting.write == WRITE_THROUGH)
      {
        info.Total_cycles += 100 * setting.rate;
      }
      else if (setting.write == WRITE_BACK)
      {
        block->dirty = 1;
        info.Store_hits += 1;
        info.Total_cycles += 1;
      }
    }
    else
    {
      // tag miss
      info.Store_misses += 1;

      // load from mem
      if (setting.allo == WRITE_ALLOCATE)
      {
        info.Total_cycles += 100 * setting.rate;
      }
      else if (setting.allo == NO_WRITE_ALLOCATE)
      {
        info.Total_cycles += 100 * setting.rate;

        loc = load_from_mem();
        cache[instruction.sets][loc].dirty = 1;
        info.Total_cycles += 1;
      }
    }
  }
  else
  {
    // valid miss
    info.Store_misses += 1;

    // load from mem
    if (setting.allo == WRITE_ALLOCATE)
    {
      info.Total_cycles += 100 * setting.rate;
    }
    else if (setting.allo == NO_WRITE_ALLOCATE)
    {
      info.Total_cycles += 100 * setting.rate;

      loc = load_from_mem();
      cache[instruction.sets][loc].dirty = 1;
      info.Total_cycles += 1;
    }
  }
}

void store_set_associative()
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

          // set line
          if (setting.policies == LRU)
          {
            struct _block temp;
            temp = cache[instruction.sets][i];
            for (int j = i; j > 0; j--)
            {
              cache[instruction.sets][j] = cache[instruction.sets][j - 1];
            }
            cache[instruction.sets][0] = temp;
          }
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

void store_fully_associative()
{
  for (int i = 0; i < setting.blocks; i++)
  {
    if (cache[0][i].valid)
    {
      if (cache[0][i].tag == instruction.tag)
      {
        // hit
        if (setting.write == WRITE_THROUGH)
        {
          info.Total_cycles += 100 * setting.rate;
          return;
        }
        else if (setting.write == WRITE_BACK)
        {
          cache[0][i].dirty = 1;
          info.Store_hits += 1;
          info.Total_cycles += 1;

          // set line
          if (setting.policies == LRU)
          {
            struct _block temp;
            temp = cache[0][i];
            for (int j = i; j > 0; j--)
            {
              cache[0][j] = cache[0][j - 1];
            }
            cache[0][0] = temp;
          }
          return;
        }
      }
    }
    else
    {
      // valid miss
      info.Store_misses += 1;

      // load from mem
      if (setting.allo == WRITE_ALLOCATE)
      {
        info.Total_cycles += 100 * setting.rate;
      }
      else if (setting.allo == NO_WRITE_ALLOCATE)
      {
        info.Total_cycles += 100 * setting.rate;

        int loc = load_from_mem();
        cache[0][loc].dirty = 1;
        info.Total_cycles += 1;
      }
      return;
    }
  }

  // tag miss
  info.Store_misses += 1;

  if (setting.allo == WRITE_ALLOCATE)
  {
    info.Total_cycles += 100 * setting.rate;
  }
  else if (setting.allo == NO_WRITE_ALLOCATE)
  {
    info.Total_cycles += 100 * setting.rate;

    int loc = load_from_mem();
    cache[0][loc].dirty = 1;
    info.Total_cycles += 1;
  }
}
