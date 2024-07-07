/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "dom/core/document.hh"
#include "dom/core/element.hh"

#include "html_parser/parser.hh"


[[noreturn]] static void die(char const *errstr, ...);
[[noreturn]] static void usage(char const *argv0);


[[noreturn]]
static void
die(char const *errstr, ...)
{
  va_list ap;

  va_start(ap, errstr);
  vfprintf(stderr, errstr, ap);
  va_end(ap);

  exit(1);
}


[[noreturn]]
static void
usage(char const *argv0)
{
  die("usage: %s [file]\n", argv0);
}


int
main(int argc, char *argv[])
{
  if (argc != 2)
    usage(argv[0]);

  char const *file_path = argv[1];

  int fd = open(file_path, O_RDONLY);
  if (fd == -1)
    die("error: couldn't open file '%s'\n", file_path);

  struct stat st;
  if (fstat(fd, &st) == -1)
    die("error: couldn't stat file '%s'\n", file_path);

  size_t file_size = st.st_size;
  char *file_data = static_cast<char *>(mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0));

  if (file_data == static_cast<char *>(MAP_FAILED))
    die("error: couldn't map file '%s'\n", file_path);

  close(fd);
  madvise(file_data, file_size, MADV_SEQUENTIAL);

  std::shared_ptr< DOM_Document> document = std::make_shared<DOM_Document>(DOM_DOCUMENT_FORMAT_HTML);
  document->node_document = std::static_pointer_cast<DOM_Document>(document->shared_from_this());

  html_parse_document(document, file_data, file_size);

  if (munmap(file_data, file_size) == -1)
    die("error: couldn't unmap file '%s'\n", file_path);


  printf("Document instance size: %zu\n", sizeof (DOM_Document));
  printf("Element instance size: %zu\n", sizeof (DOM_Element));

  return 0;
}

