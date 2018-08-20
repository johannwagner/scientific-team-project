set terminal svg
set output 'slot_distr.svg'
set key fixed right top vertical Right noreverse noenhanced autotitle nobox
set datafile missing '-'
set style data linespoints
set xtics border in scale 1,0.5 nomirror  autojustify
set xtics  norangelimit
set xtics   ()
DEBUG_TERM_HTIC = 119
DEBUG_TERM_VTIC = 119
## Last datafile plotted: "slot_distr.csv"
plot 'slot_distr.csv' using 2:xtic(1) title columnheader(2), for [i=3:22] '' using i title columnheader(i)
