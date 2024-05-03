#include <iostream>

double f(double x)
{
    return x*x + 3*x + 1;
}


int main( int argc, char* argv[])
{
    // we initialize to give default values
    double a = 0, b = 1, sum = 0;
    int n = 100;   // number of intervals

    std::cout << "Insert extrema a: " << std::endl;
    std::cin >> a;
    std::cout << "Insert extrema b: " << std::endl;
    std::cin >> b;
    std::cout << "Insert number of intervals: " << std::endl;
    std::cin >> n;

    double h;
    h = (b - a) / n;    // discretization size

    double ai, bi; // extremes of composite integration

    for (int i=0; i < n; i++)
    {
        ai = a + i*h; // ai
        bi = a + (i+1)*h; // bi
        sum = sum +  ( f( ai ) + f( bi ));
    }

    sum = sum * h/2;            // quadrature output

    std::cout << "Integral: " << sum   << std::endl;
    std::cout << "Exact:    " << (1.0/3*b*b*b + 1.5*b*b + b) - 
                                 (1.0/3*a*a*a + 1.5*a*a + a) << std::endl;
    
}
