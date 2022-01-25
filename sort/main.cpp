#include "set.h"
#include "sort.h"
#include "matrix.h"
#include "cube.h"

int main()
{
  cout << "start ... " << endl;
  sort *sor = new sort("../rootfile/", "../cub/", "R", 3, 108);
  sor->process();
  delete sor;

  return 0;
}
