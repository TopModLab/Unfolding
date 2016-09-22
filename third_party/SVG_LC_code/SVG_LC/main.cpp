
#include <string>
#include "wxn_dijstra.h"
using namespace std;

int main(int argc, char** argv)
{
  bool test_performance = false;
  string svg_file_name;

  if (argc < 2) {
    printf("parameter insufficient !\n usage SVG_LC.exe [svg_file_name] \n");
    return 1;
  }
  svg_file_name = argv[1];

  SparseGraph<float>* s_graph = NULL;
  s_graph = new LC_LLL<float>();


  s_graph->read_svg_file_binary((string)svg_file_name);//load the precomputed infomation 


  int source_vert = 0;//index of source vertex

  s_graph->findShortestDistance(source_vert);  //find the geodesic distance from a single source to all vertex of a mesh

  //here is an example to output the distance field to a txt file

  FILE* out_file = fopen("svg_dis.txt","w");
  for (int i = 0; i < s_graph->NodeNum(); ++i) {
    double geodesic_distance = s_graph->distanceToSource(i);//get distance from source vert to dest vert
    fprintf(out_file, "%lf\n" , geodesic_distance);
  }
  fclose(out_file);

  delete s_graph;

  return 0;
}