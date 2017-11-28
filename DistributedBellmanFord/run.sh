gcc project.c -pthread -ggdb
xterm -e "./a.out 1" &
xterm -e "./a.out 2" &
xterm -e "./a.out 3" &
xterm -e "./a.out 4" &