set terminal pngcairo nocrop enhanced size 700,400 font ",8"
set output 'pool_util.png'
set title 'Comparison of Utilization of Threads ranging from 2 to 32' font ",15"
set boxwidth 0.8 absolute
set border 1
set style fill solid 1.00 border lt -1
set style histogram clustered gap 1 title textcolor lt -1
set style data histograms
set xtics border scale 1,0 nomirror autojustify norangelimit

set key off auto columnheader
set yrange [0:*]
set offset 0,0,graph 0.05,0

set linetype 1 lc rgb '#FF420E'
set linetype 2 lc rgb '#3465A4'
dx = 1/6.0

plot 'pool_util.csv' using 2:xtic(1),\
         '' u 3,\
         '' u ($0 - dx):2:2 with labels offset 0,3 rotate by -90,\
         '' u ($0 + dx):3:3 with labels offset 1,1
