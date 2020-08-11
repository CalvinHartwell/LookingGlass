#include "common/debug.h"

#include "pipe.h"
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct LG_Pipe
{
  int fd;
};

bool pipe_init(LG_Pipe ** pipe)
{
  *pipe = (LG_Pipe *)malloc(sizeof(LG_Pipe));
  if (!*pipe)
  {
    DEBUG_ERROR("Failed to malloc pipe");
    return false;
  }
  memset(*pipe, 0, sizeof(LG_Pipe));

  const char *PIPE_OUT = "/tmp/hax0r-settings";
  const int options_out = O_RDWR | O_CREAT | O_NONBLOCK;

  remove(PIPE_OUT);
  mkfifo(PIPE_OUT, 0666);

  (*pipe)->fd = open(PIPE_OUT, options_out);
  if ((*pipe)->fd == -1)
  {
    DEBUG_ERROR("Failed to open pipe");
    DEBUG_ERROR("Error: %d", errno);
    remove(PIPE_OUT);
    return false;
  }

  return true;
}

void pipe_free(LG_Pipe ** pipe)
{
  if (!*pipe)
    return;

  close((*pipe)->fd);

  free(*pipe);
  *pipe = NULL;
}

void pipe_send(LG_Pipe * pipe, void * data, size_t size)
{
    return;
  DEBUG_INFO("Sending %ld bytes", size);
  write(pipe->fd, data, size);
}