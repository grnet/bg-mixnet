pkill server
go install stadium/groth stadium/stadium stadium/coordinator stadium/server
~/go/bin/server -conf ~/go/src/stadium/config/three-server.conf -id 0 &> out1.txt &
~/go/bin/server -conf ~/go/src/stadium/config/three-server.conf -id 1 &> out2.txt &
~/go/bin/server -conf ~/go/src/stadium/config/three-server.conf -id 2 &> out3.txt &
sleep 1
# collectl -sn -f test-output &
~/go/bin/coordinator -conf ~/go/src/stadium/config/three-server.conf
