#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opterr;
int optopt;
int optind;
char *optarg;
int initialized = 0;

state_t *program_state;

void initialize(){
  STR_UTF16BE = "UTF16BE";
  STR_UTF16LE = "UTF16LE";
  STR_UTF8 = "UTF8";
  initialized = 1;
}

void
parse_args(int argc, char *argv[])
{
  int i = 0;
  char option;
  // // char *joined_argv;
  // printf("%d", argc);
  // printf("%s\n\n\n", *argv);
  // // joined_argv = join_string_array(argc, argv);
  // // info("argc: %d argv: %s", argc, joined_argv);
  // // free(joined_argv);
  program_state = Calloc(1, sizeof(state_t));
  for (i = 0; optind < argc; i++) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    option = getopt(argc, argv, "+he:");

    if ((option) != -1) {
      switch (option) {
        case 'h': {
            free(program_state);
            USAGE(argv[0]);
            exit(0);
        }
        case 'e': {
          info("Encoding Argument: %s", optarg);
          if(strcmp(optarg, "-h")==0){
            free(program_state);
            USAGE(argv[0]);
            exit(0);
          }

          program_state->encoding_to = determine_format(optarg);
          if(program_state->encoding_to == 0){
            print_state();
            free(program_state);
            exit(EXIT_FAILURE);
          }
          break;
        }
        case '?': {
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM,
                    optopt);
            break;
          }
        default: {
          break;
        }
      }
    }
    elsif(argv[optind] != NULL)
    {
      if (program_state->in_file == NULL) {
        program_state->in_file = argv[optind];
      }
      elsif(program_state->out_file == NULL)
      {
        program_state->out_file = argv[optind];
      }
      else{
        exit(EXIT_FAILURE);
      }
      optind++;
    }
  }
  if((argc > 5) | (argc < 5)){  // only valid -e argument is program_executable -e FLAG in.txt out.txt, which is 5.. any less or more is wrong
    exit(EXIT_FAILURE);
  }
  // free(program_state);
}

void checkInitialize(){
  if(initialized == 0){
    initialize();
  }
}

format_t
determine_format(char *argument)
{
  checkInitialize();
  if (strcmp(argument, STR_UTF16LE) == 0){
    return UTF16LE;
  }
  if (strcmp(argument, STR_UTF16BE) == 0){
    return UTF16BE;
  }
  if (strcmp(argument, STR_UTF8) == 0){
    return UTF8;
  }
  return 0;
}

char*
bom_to_string(format_t bom){
  switch(bom){
    case UTF8: return (char*)STR_UTF8;
    case UTF16BE: return (char*)STR_UTF16BE;
    case UTF16LE: return (char*)STR_UTF16LE;
  }
  return "UNKNOWN";
}

// char*
// join_string_array(int count, char *array[])
// {
//   char *ret;
//   char charArray[count];
//   int i;
//   int len = 0, cur_str_len;

//   // str_len = array_size(count, array);
//   ret = charArray; // removed the &

//   for (i = 0; i < count; ++i) {
//     cur_str_len = strlen(array[i]);
//     memecpy(ret + len, array[i], cur_str_len);
//     len += cur_str_len;
//     memecpy(ret + len, " ", 1);
//     len += 1;
//   }

//   return ret;
// }

int
array_size(int count, char *array[])
{
  int i, sum = 1; /* NULL terminator */
  for (i = 0; i < count; ++i) {
    sum += strlen(array[i]);
    ++sum; /* For the spaces */
  }
  return sum+1;
}

void
print_state()
{
  if (program_state == NULL) {
    error("program_state is %p", (void*)program_state);
    exit(EXIT_FAILURE);
  }
  info("program_state {\n"
         "  format_t encoding_to = 0x%X;\n"
         "  format_t encoding_from = 0x%X;\n"
         "  char *in_file = '%s';\n"
         "  char *out_file = '%s';\n"
         "};\n",
         program_state->encoding_to, program_state->encoding_from,
         program_state->in_file, program_state->out_file);
}