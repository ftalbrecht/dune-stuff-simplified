#include <config.h>

#include <dune/stuff/grid/output/pgf.hh>
#include <dune/grid/sgrid.hh>

#include <fstream>

using namespace Dune::Stuff::Grid;

/** output files are compiled in test-compile-pgfoutput.sh target
 **/
int main( int, char** )
{
  const int dim = 2;
  typedef Dune::SGrid<dim,dim> GridType;
  int n[dim];
  double h[dim];

  for (int i=0; i<dim; ++i) {
    n[i] = 2;
    h[i] = 1.0;
  }
  GridType grid(n,h);
  Output::Pgf<GridType> output( grid );
  const int max_refines = 2;
  const bool includable = false;
  std::ofstream fileB("pgfoutput_refineseries.tex");
  output.refineseries(fileB, max_refines, includable);
  std::ofstream fileC("pgfoutput_stacked.tex");
  output.stacked(fileC, max_refines, includable);
  std::ofstream fileA("pgfoutput_leaf.tex");
  output.leaf(fileA, includable);
  return 0;
}
