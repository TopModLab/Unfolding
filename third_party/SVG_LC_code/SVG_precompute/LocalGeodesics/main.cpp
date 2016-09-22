
#include "stdafx.h"
#include "svg_precompute.h"


int main(int argc, char** argv)
{
  string input_file_name;
  int fixed_K = 50;// a parameter for our algorithm, default is 50
  if (argc < 2) {
    printf("no sufficient argument, usage example 'SVG_precompute.exe bunny.obj 50'");
    return 1;
  } else if(argc == 2) {
    input_file_name = argv[1];
  } else if(argc == 3) {
    input_file_name = argv[1];
    fixed_K = atoi(argv[2]);
  }else{
    printf("wrong argument, usage example 'SVG_precompute.exe bunny.obj 50'");
  }
  string svg_file_name;
  svg_precompute(input_file_name, fixed_K, svg_file_name);

  return 0;
}

