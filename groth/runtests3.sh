pkill server
go install stadium/groth stadium/stadium stadium/coordinator stadium/server
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 0 &> out0.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 1 &> out1.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 2 &> out2.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 3 &> out3.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 4 &> out4.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 5 &> out5.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 6 &> out6.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 7 &> out7.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 8 &> out8.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 9 &> out9.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 10 &> out10.txt &
~/go/bin/server -conf ~/go/src/stadium/config/twelve-server.conf -id 11 &> out11.txt &
sleep 1
~/go/bin/coordinator -conf ~/go/src/stadium/config/twelve-server.conf
