set term png
set output 'pool_avg.png'
set boxwidth 0.9 absolute
set style fill   solid 1.00 border lt -1
set key fixed right top vertical Right noreverse noenhanced autotitle nobox
set style histogram clustered gap 1 title textcolor lt -1
set datafile missing '-'
set style data histograms
set xtics border in scale 0,0 nomirror autojustify
set xtics norangelimit 
set xtics ()

set title "Thread Pool Avg. Time Measurements" 
set yrange [ 0.00000 : 100000000. ] noreverse nowriteback

plot for [COL=1:2] 'pool_avg.csv' using COL:xticlabels(1) title columnheader
