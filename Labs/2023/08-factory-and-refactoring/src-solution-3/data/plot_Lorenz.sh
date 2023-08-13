#!/bin/bash

gnuplot -p -e 'plot "results_Lorenz.out" using 1:2 with linespoints title columnhead(2),
                    "results_Lorenz.out" using 1:3 with linespoints title columnhead(2),
                    "results_Lorenz.out" using 1:4 with linespoints title columnhead(2)'

gnuplot -p -e 'splot "results_Lorenz.out" using 2:3:4 with linespoints title columnhead(2);
               pause mouse close'

