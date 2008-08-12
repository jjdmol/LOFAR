#Gnuplot script file for displaying the CyclicBuffer output
#call with:
# call "CyclicBufferTest.gnuplot" "<filename>"  0 500
#                                               ^  ^  min and max bandwidth bin
#
set xlabel "time"
set ylabel "thread id"

# Set the terminal type for file output
#set terminal gif medium
#set output "$0.gif"
set terminal postscript
set output "$0.ps"

set title "$0"
set multiplot
plot [][$1:$2] "$0" using "%*lf%lf%lf%*lf%*lf" notitle w p pt 1
set nomultiplot

#Now, make a sceen plot
#set terminal windows
#set terminal x11
#set output
#set multiplot
#plot [][$1:$2] "$0" using "%*lf%lf%lf%*lf%*lf" notitle w p pt 1
#set nomultiplot
