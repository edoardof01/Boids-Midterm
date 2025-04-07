set terminal png size 800,600
set xlabel "Numero di Thread"
set ylabel "Speedup"
set title "Speedup vs Numero di Thread"
set grid
set key top left
set output 'speedup_threads_plot.png'

plot "threads_speedup_nogrid.txt" using 1:2 with linespoints title "NoGrid", \
     "threads_speedup_grid.txt"   using 1:2 with linespoints title "Grid", \
     "threads_speedup_soa.txt"    using 1:2 with linespoints title "SoA"
