package main

import (
	"encoding/json"
	"flag"
	"io/ioutil"
	"net/rpc"
	"os"

	log "github.com/Sirupsen/logrus"
	stadium "stadium/stadium"
)

var confPath = flag.String("conf", "", "config file")

//Initialize logger
func init() {
	// Output to stderr instead of stdout, could also be a file.
	log.SetOutput(os.Stderr)

	// Only log the warning severity or above.
	log.SetLevel(log.DebugLevel)
}

func readConfigFile(path string, val interface{}) {
	f, err := os.Open(path)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	if err := json.NewDecoder(f).Decode(val); err != nil {
		log.WithFields(log.Fields{
			"err": err,
		}).Fatal("json decoding error while reading config file")
	}
}

//Set up coordinator server
func main() {
	flag.Parse()

	if *confPath == "" {
		log.Fatal("Must specify -conf flag on server initialization")
	}

	conf := new(stadium.Config)
	readConfigFile(*confPath, conf)
	if conf.ListenAddrs == nil {
		log.Warn("Config file missing required fields")
	}

	//Set up connections with all other servers
	servers := make([]*rpc.Client, len(conf.ListenAddrs))

	for i, addr := range conf.ListenAddrs {
		c, err := rpc.Dial("tcp", addr)

		if err != nil {
			log.WithFields(log.Fields{
				"err": err,
			}).Fatal("RPC dial failed")
		}

		servers[i] = c
	}

	// Send Prepare to all servers
	donePrepare := make(chan stadium.PrepareReply)
	for i, s := range servers {
		go func(id int, srv *rpc.Client) {
			reply := stadium.PrepareReply{}
			srv.Call("VZServer.Prepare", stadium.PrepareArgs{}, &reply)
			if !reply.Ok {
				log.WithFields(log.Fields{"serverId": id, "addr": conf.ListenAddrs[i]}).Warn("Prepare failed")
				panic("a server failed to prepare!")
			}
			donePrepare <- reply
		}(i, s)
	}
	log.Info("Sent Prepare...")
	for _ = range servers {
		<-donePrepare
	}

	//Send out start to all servers
	done := make(chan stadium.StartReply)
	stats := make([]stadium.ExperimentTime, len(servers))
	for i, s := range servers {
		go func(id int, srv *rpc.Client) {
			reply := stadium.StartReply{}
			srv.Call("VZServer.Start", stadium.StartArgs{}, &reply)
			stats[id] = reply.RoundStats
			done <- reply
		}(i, s)
	}
	log.Info("Waiting for servers...")

	for _ = range servers {
		<-done
	}

	for i, stat := range stats {
		log.WithFields(log.Fields{
			"addr": conf.ListenAddrs[i],
		}).Info("print server results")
		stat.PrintSummary()
	}

	log.Info("All servers finished.")

	text, err := json.Marshal(stats)
	if err != nil {
		panic(err)
	}
	err = ioutil.WriteFile("roundstats.json", text, 0644)
	if err != nil {
		panic(err)
	}
}
