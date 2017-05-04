
# let the re-launch timer be 3 seconds when the application can not run immediately
.relaunch 20

# run ex_app with watch-dog timer of 10 seconds in the pipe-mode.
.timeout 30
.app /bin/VoIP_maserati -wdt $p $t 
# $t is the amount of seconds to WDT time-out
# $p is the handle of pipe

# run the second ex_app with fifo instead of pipe (fifo-mode)
#.fifo ./ex_fifo
#.timeout 30
#.app /bin/VoIP_maserati -wdt_fifo $f $t
# $f is the name of the fifo

# run the third ex_app in the pipe-mode with the same timeout as the previous'
#.app ex_app -wdt $p $t

# - Applications run in the same order as tha of the list
# - For the case of .pipe, gwdt forks the application as its child process.
#   However, in the case of .fifo used, gwdt forks the application, and then 
#   waits until the fifo connection established, that is, the synchronous mode.
# - timeout = 0 implies, no timeout is set.
