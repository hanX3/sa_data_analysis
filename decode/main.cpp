#include <iostream>

#include "decoder.h"
#include "r2root.h"
#include "stdio.h"

int main(int argc, char *argv[])
{
  r2root *r2r = new r2root("../2016sa_data/2016SA/Data/Secondweek/", "../rootfile/", "R", atoi(argv[1]));
  r2r->process();

  delete r2r;

  return 0;
}
