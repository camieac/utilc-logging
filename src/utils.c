#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>

#include "utils.h"

size_t remove_line_from_file(char *filename) {
	int fd = open (filename, O_RDWR);
	struct stat s;
  size_t size;
  unsigned char *f;

  //Get the size of the file
	int status = fstat (fd, &s);
	size = s.st_size;

  //Map the file into memory
  f = (unsigned char *) mmap (0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
}
