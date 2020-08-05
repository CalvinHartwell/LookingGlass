
#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct LG_Pipe LG_Pipe;

/// Used for nulling an index
typedef struct LG_Null
{
    char type;
    unsigned short idx;
} LG_Null;

typedef struct LG_Line
{
  char type;
  unsigned short idx;
  float x1;
  float y1;
  float x2;
  float y2;
  unsigned int color;
  float width;
} LG_Line;

typedef struct LG_Box
{
  char type;
  unsigned short idx;
  float x1;
  float y1;
  float x2;
  float y2;
  unsigned int color;
  float thickness;
  bool filled;  // If filled is true, thickness doesn't do anything
} LG_Box;

typedef struct LG_Text
{
  char type;
  unsigned short idx;
  float x;
  float y;
  char size;
  unsigned int color;
  char str[128];
} LG_Text;

bool pipe_init(LG_Pipe **pipe);
void pipe_free(LG_Pipe **pipe);
void pipe_send(LG_Pipe *pipe, void *data, size_t size);
