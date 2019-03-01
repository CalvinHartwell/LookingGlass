/*
Looking Glass - KVM FrameRelay (KVMFR) Client
Copyright (C) 2017-2019 Geoffrey McRae <geoff@hostfission.com>
https://looking-glass.hostfission.com

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "app.h"
#include "debug.h"
#include <getopt.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct app
{
  unsigned int  shmSize;
  int           shmFD;
  void        * shmMap;
};

struct params
{
  const char * shmDevice;
};

static struct app app;

static struct params params =
{
  .shmDevice = "uio0"
};

struct osThreadHandle
{
  const char       * name;
  osThreadFunction   function;
  void             * opaque;
  pthread_t          handle;
  int                resultCode;
};

int main(int argc, char * argv[])
{
  static struct option longOptions[] =
  {
    {"shmDevice", required_argument, 0, 'f'},
    {0, 0, 0, 0}
  };

  int optionIndex = 0;
  while(true)
  {
    int c = getopt_long(argc, argv, "f:", longOptions, &optionIndex);
    if (c == -1)
      break;

    switch(c)
    {
      case 'f':
        params.shmDevice = optarg;
        break;
    }
  }

  // check the deice name
  {
    char file[100] = "/sys/class/uio/";
    strncat(file, params.shmDevice, sizeof(file) - 1);
    strncat(file, "/name"         , sizeof(file) - 1);

    int fd = open(file, O_RDONLY);
    if (fd < 0)
    {
      DEBUG_ERROR("Failed to open: %s", file);
      DEBUG_ERROR("Did you remmeber to modprobe the kvmfr module?");
      return -1;
    }

    char name[32];
    int len = read(fd, name, sizeof(name) - 1);
    if (len <= 0)
    {
      DEBUG_ERROR("Failed to read: %s", file);
      close(fd);
      return -1;
    }
    name[len] = '\0';
    close(fd);

    if (strcmp(name, "KVMFR") != 0)
    {
      DEBUG_ERROR("Device is not a KVMFR device %s reports as %s", file, name);
      return -1;
    }
  }

  // get the device size
  {
    char file[100] = "/sys/class/uio/";
    strncat(file, params.shmDevice , sizeof(file) - 1);
    strncat(file, "/maps/map0/size", sizeof(file) - 1);

    int fd = open(file, O_RDONLY);
    if (fd < 0)
    {
      DEBUG_ERROR("Failed to open: %s", file);
      return -1;
    }

    char size[32];
    int  len = read(fd, size, sizeof(size));
    if (len <= 0 || len == sizeof(size))
    {
      DEBUG_ERROR("Failed to read: %s", file);
      close(fd);
      return -1;
    }
    size[len] = '\0';
    close(fd);

    app.shmSize = strtoul(size, NULL, 16);
  }

  // open the device
  {
    char file[100] = "/dev/";
    strncat(file, params.shmDevice, sizeof(file) - 1);
    app.shmFD   = open(file, O_RDWR, (mode_t)0600);
    app.shmMap  = MAP_FAILED;
    if (app.shmFD < 0)
    {
      DEBUG_ERROR("Failed to open: %s", file);
      return -1;
    }
  }

  bool termSig = false;
  int result = app_main(&termSig);
  os_shmemUnmap();
  close(app.shmFD);

  return result;
}

unsigned int os_shmemSize()
{
  return app.shmSize;
}

bool os_shmemMmap(void **ptr)
{
  if (app.shmMap == MAP_FAILED)
  {
    app.shmMap = mmap(0, app.shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, app.shmFD, 0);
    if (app.shmMap == MAP_FAILED)
    {
      DEBUG_ERROR("Failed to map the shared memory device: %s", params.shmDevice);
      return false;
    }
  }

  *ptr = app.shmMap;
  return true;
}

void os_shmemUnmap()
{
  if (app.shmMap == MAP_FAILED)
    return;

  munmap(app.shmMap, app.shmSize);
  app.shmMap = MAP_FAILED;
}

static void * threadWrapper(void * opaque)
{
  osThreadHandle * handle = (osThreadHandle *)opaque;
  handle->resultCode = handle->function(handle->opaque);
  return NULL;
}

bool os_createThread(const char * name, osThreadFunction function, void * opaque, osThreadHandle ** handle)
{
  *handle = (osThreadHandle*)malloc(sizeof(osThreadHandle));
  (*handle)->name     = name;
  (*handle)->function = function;
  (*handle)->opaque   = opaque;

  if (pthread_create(&(*handle)->handle, NULL, threadWrapper, *handle) != 0)
  {
    DEBUG_ERROR("pthread_create failed for thread: %s", name);
    free(*handle);
    *handle = NULL;
    return false;
  }
  return true;
}

bool os_joinThread(osThreadHandle * handle, int * resultCode)
{
  if (pthread_join(handle->handle, NULL) != 0)
  {
    DEBUG_ERROR("pthread_join failed for thread: %s", handle->name);
    free(handle);
    return false;
  }

  if (resultCode)
    *resultCode = handle->resultCode;

  free(handle);
  return true;
}