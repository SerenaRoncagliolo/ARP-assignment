# BASH


#Configuration Parameters

port="51717"

#Time in microseconds the process P waits before sending a new token to the G process through the socket 
DT="1000000"

#Reference frequency of the generated token wave 
RF="100"


#digit "hostname" on command window and insert the result obtained 
hostname="administrator-virtual-machine" 

#Compiling the codes

gcc client.c -lm -o client
gcc server.c -o server
gcc logfilecode.c -o log
gcc signal.c -o signal

#Executing the processes

xterm -e ./server 51717 &
sleep 2

xterm -e ./client  $hostname $port $DT $RF &
sleep 2

xterm -e ./signal &
sleep 2

xterm -e ./log
sleep 2
