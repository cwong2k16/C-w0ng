#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>

int
main(int argc, char *argv[])
{
  initialize();
  parse_args(argc, argv);
  int infile, outfile, in_flags, out_flags;
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);
  struct stat input;
  struct stat output;
  fstat(infile, &input);
  fstat(outfile, &output);
  if(input.st_ino == output.st_ino){
    exit(EXIT_FAILURE);
  }
  lseek(infile, program_state->bom_length, SEEK_SET); /* Discard BOM */
  get_encoding_function()(infile, outfile);
  if(program_state != NULL) {
    free(program_state);
  }
  close(outfile);
  close(infile);
  // //I think this is how this works
  // // free((void*)outfile);
  // // free((void*)infile);
  // // free((void*)(uintptr_t)outfile);
  // // free((void*)(uintptr_t)infile);
  return EXIT_SUCCESS;
}
