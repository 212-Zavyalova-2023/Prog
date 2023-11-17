#include <iostream>
#include <fstream>
#include <math.h>

class Hill
{
private:
  double x;
  double y;
  double sigma;
  double hight;
public:
  Hill() = default;
  Hill(double x0, double y0, double sigma0, double hight0):
       x(x0), y(y0), hight(hight0){sigma = abs(sigma0);}

  double Gauss(double x0, double y0);
  void print(std::ofstream &file);
};


double f(double x, double y, Hill H);

int main() 
{
  Hill A(0, 0, 0.4, 2);
  std::ofstream file("1.txt");
  A.print(file);

  std::cout << "Hello World!\n";
}

double Hill::Gauss(double x0, double y0)
{
  double e = exp(-((x - x0) * (x - x0) + (y - y0) * (y - y0)) / 2 * sigma * sigma);
  return (hight * e) / (sigma * sqrt(2 * M_PI));
}

void Hill::print(std::ofstream &file)
{
  for (double i = -10.5; i <= 10; i+=0.5)
  {
    for (double j = -10; j <= 10; j+=0.5)
    {
      file << i << " " << j << " " << Gauss(i, j) << "\n";
    }
    file << "\n";
  }
}