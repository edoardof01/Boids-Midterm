set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'speedup_plot_grid.png'

set title "Speedup vs Numero di Boid"
set xlabel "Numero di Boid"
set ylabel "Speedup"
set grid

plot "speedup_data_nogrid.txt" using 1:2 with linespoints lt 1 lw 2 pt 7 ps 1.3 title "NoGrid", \
     "speedup_data_grid.txt"   using 1:2 with linespoints lt 2 lw 2 pt 9 ps 1.3 title "Grid", \
     "speedup_data_soa.txt"    using 1:2 with linespoints lt 3 lw 2 pt 5 ps 1.3 title "SoA"
