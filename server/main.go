package main

import (
	"encoding/json"
	"flag"
	"net"
	"net/rpc"
	"os"
	"strings"

	log "github.com/Sirupsen/logrus"
	stadium "stadium/stadium"
)

var id = flag.Int("id", -1, "serverId")
var confPath = flag.String("conf", "", "config file")

//Initialize logger
func init() {
	// Output to stderr instead of stdout, could also be a file.
	log.SetOutput(os.Stderr)

	// Only log the warning severity or above.
	log.SetLevel(log.InfoLevel)
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

//Set up server
func main() {
	flag.Parse()

	if *id == -1 {
		log.Fatal("Must specify -id flag on server initialization")
	}

	if *confPath == "" {
		log.Fatal("Must specify -conf flag on server initialization")
	}

	conf := new(stadium.Config)
	readConfigFile(*confPath, conf)
	if conf.ListenAddrs == nil || conf.ChainLen == 0 || conf.NumMsgs == 0 {
		log.Warn("Config file missing required fields")
	}

	//Initialize server with values from config file
	srv := stadium.VZServer{}
	srv.InitServer(*id, *conf)

	//Register server for RPCs
	if err := rpc.Register(&srv); err != nil {
		log.WithFields(log.Fields{
			"err": err,
		}).Fatal("RPC register failed")
	}

	listenAddr := "0.0.0.0:" + strings.Split(conf.ListenAddrs[*id], ":")[1]
	listen, err := net.Listen("tcp", listenAddr)
	if err != nil {
		log.WithFields(log.Fields{
			"err": err,
		}).Fatal("Setting RPC listen port failed")
	}
	rpc.Accept(listen)
}
