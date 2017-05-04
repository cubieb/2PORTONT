#expects the data from two MEPs in format "mean \t mean"
set title "Delays in usecs for 2 MEPs"
#set nokey
set ylabel "usecs"
#set xrange[0:5]
#set style fill solid border -1
set format x ""
set noxtics

plot "stats.dat" using 1:2 w linespoint, \
     "stats.dat" using 1:3 w linespoint

#pause -1
pause 10
reread
