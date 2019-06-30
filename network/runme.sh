./network &>/dev/null &
p1=$$
echo server running: $p1
sleep 2
# start client
./network 1 &
p2=$$
echo  client running: $p2
sleep 2
kill $p1
kill $p2
wait $p1
wait $p2
echo process killed
