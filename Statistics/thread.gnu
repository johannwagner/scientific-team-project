set term png
set output 'pool_avg.png'
set boxwidth 0.9 absolute
set style fill   solid 1.00 border lt -1
set key fixed right top vertical Right noreverse noenhanced autotitle nobox
set style histogram clustered gap 1 title textcolor lt -1
set style data histograms

set title "Thread Pool Avg. Time Measurements" 
set yrange [ 0.00000 : 100000000.  ] noreverse nowriteback

dx = 1/6.0

plot 'pool_avg.csv' using 2:xtic(1),\
     '' u 3,\
     '' u ($0 - dx):2:2 with labels,\
     '' u ($0 + dx):3:3 with labels
