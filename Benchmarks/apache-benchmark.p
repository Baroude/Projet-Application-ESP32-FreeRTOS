# output as png image
set terminal png

# save file to "benchmark.png"
set output "FreeRTOS2000-5.png"

# graph title
set title "FreeRTOS ab -n 2000-c 5 "

#nicer aspect ratio for image size
set size 1,0.7

# y-axis grid
set grid y

#x-axis label
set xlabel "request"

#y-axis label
set ylabel "response time (ms)"

#plot data from "FreeRTOS2000-5.tsv" using column 9 with smooth sbezier lines
plot "FreeRTOS2000-5.tsv" using 9 smooth sbezier with lines title "Response time over request number"
