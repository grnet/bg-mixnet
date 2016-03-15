package stadium

import (
	"bytes"
	"encoding/gob"
	"net/rpc"
	"sync"
	"os"
	"time"

	log "github.com/Sirupsen/logrus"
	groth "stadium/groth"
)

const twoChains = true
const trackBandwidth = false

//Struct for storing configuration information
type Config struct {
	ListenAddrs []string
	ChainLen    int
	NumMsgs     int
}

type VZServer struct {
	//configuration
	serverId       int           //myId
	listenAddrs    []string      //listen addresses of other servers
	servers        []*rpc.Client //RPC connections
	mu             sync.Mutex
	chainLen       int  //number of rotation rounds per mixing phase
	connectionsSet bool //indicates whether RPC connections have been set up
	numMsgs        int

	shuffler      verifiableShuffler
	elgamal       verifiableShuffler
	dummyshuffler verifiableShuffler //for testing
	shufflerLock  sync.Mutex
	hasher        hasher

	// client-simulation state
	preparedArgs MessageBatchArgs

	//shuffle state

	/* Keeps track of which rotation rounds batches have been received for
	first layer - mixing phase (0-3)
	second layer - rotation round (0 to chainLen-1) */
	batchReceived [][]bool

	/* Keeps track of which batch parts have been received for a given rotation round
	first layer - mixing phase (0-3)
	second layer - rotation round (0 to chainLen-1)
        third layer - which batch parts have been received */
	batchNextPart [][]int
	batchParts    [][][]MessageBatchArgs

	/* Keeps track of the symmetric keys we've seen in onions
	first layer - mixing phase (0 or 1)
	second layer - round (0 to chainLen-1) */
	onionKeys [][][]byte

	/* Keeps track of which shuffle proofs have been received
	first layer - mixing phase (0 or 1)
	second layer - round (0 to chainLen-1)
	third layer - prover chain (0 to chainLen-1) */
	proofReceived [][][]bool

	/* Keeps track of which shuffle proofs have been verified
	first layer - mixing phase (0 or 1)
	second layer - round (0 to chainLen-1) */
	proofVerified [][]chan bool

	/* Keeps track of which encryption proofs have bene received */
	encryptionProofReceived []bool
	/* Keeps track of which shuffle proofs have been verified */
	encryptionProofVerified chan bool
	/* Signals encryption verifications to start at the beginning of a round */
	encryptionVerificationStartChan []chan int

	/* Keeps track of permutations applied at each rotation round
	first layer - mixing phase (0 or 1)
	second layer - round (0 to chainLen-1) */
	permutations [][][]int

	/* Shuffle done */
	shuffleDone chan bool

	//distribution state

	/* Keeps track of how the message batch is split into parcels
	first layer - dist phase (0 - 2)
	second layer - recipient server (0 to numServers-1)
	third layer - index slice */
	distOutMap [][][]int

	/* Keeps track of how parcels are combined into message batch
	first layer - dist phase (0 - 2)
	second layer - sending server (parcels are appended in server order) */
	distInMap [][]int

	/* Keeps track of which servers parcels have been received from
	first layer - dist phase (0 - 4)
	second layer - sending chain/parcel (0 to numServers-1) */
	distParcelsReceived [][]bool
	distParcels         [][]chan DistributeArgs //TODO make pointer

	/* Keeps track of which servers verification hashes have been received from
	first layer - dist phase (0-1)
	second layer - receiving chain (0 to chainLen-1)
	third layer - sending chain (0 to numServers-1)
	fourth layer - sender order (0 to chainLen-1) */
	distHashesReceived [][][][]bool

	/* Channels on which to send verification hashes for a particular post-distribution chain
	first layer = dist phase (0-1)
	second layer - receiving chain (0 to chainLen-1)
	third layer - sending chain (0 to numServers-1) */
	distHashes [][][]chan []byte

	/* Keeps track of which predist parcel batches have been received
	first layer - dist phase (0 or 1)
	second layer - chain (0 to chainLen-1) */
	predistParcelBatchReceived [][]bool

	/* Keeps track of which postdist parcel batches have been received
	first layer - dist phase (0 or 1)
	second layer - chain (0 to chainLen-1) */
	postdistParcelBatchReceived [][]bool

	/* Tracks how many new batches have been verified with hashes
	first layer - dist phase (0 or 1) */
	postdistParcelBatchVerified []chan bool

	//time variables for experiments
	timer ExperimentTime
	roundDone chan bool
}

//Round state enums/consts
const ( //iota starts at 0 - sets enums to 1, 2, 3, ...
	mixing1       = iota //first mixing round
	mixing2       = iota //second mixing round
	mixingreturn1 = iota //first return mixing round
	mixingreturn2 = iota //second return mixing round
)

const (
	noisedist         = iota //noise distribution round
	msgdist           = iota //message distribution round
	mailboxdist       = iota //mailbox distribution round
	mailboxdistreturn = iota //return of mailbox distribution round
	msgdistreturn     = iota //return of distribution round
)

// Things done by the client
type PrepareArgs struct {
}

type PrepareReply struct {
	Ok bool
}

func (vz *VZServer) Prepare(args PrepareArgs, reply *PrepareReply) error {
	vz.mu.Lock()
	defer vz.mu.Unlock()
	defer vz.timer.trackMsgCreation(time.Now())

	//Setup connections
	if !vz.connectionsSet {
		vz.SetupConnections(vz.listenAddrs)
	}

	log.WithFields(log.Fields{
		"id": vz.serverId,
	}).Info("Server received Prepare from Coordinator")

	//Generate noise

	//Create message batch
	log.Info("Creating message batch...")
	batchSize := vz.numMsgs
	msgSize := 136 //Size of message content in bytes (divisible by 8) (about size of tweet)

	var md, mb, c, gelts, proof []byte
	var mdlen, mblen, clen, geltlen, paddedBatchSize int
	//vz.shufflerLock.Lock()
	elgamal := groth.Groth{}
	if twoChains {
		md, mdlen = vz.createMetadataInputChain(batchSize)
		md, mdlen, gelts, geltlen, proof = elgamal.EncryptProven(md, mdlen, vz.serverId) //encrypt with own chainId

		paddedBatchSize = len(gelts) / geltlen
		mb, mblen = vz.createMetadataMailbox(paddedBatchSize)
		c, clen = vz.createMsgContent(paddedBatchSize, msgSize)

		// TODO this needs to be done with the correct key!!
		// wrap per mailbox
		c, clen = vz.DestWrap(c, clen, gelts, geltlen, vz.chainLen, len(vz.servers)) // mixing2

		//   for i := (2*vz.chainLen) - 1; i >= vz.chainLen; i-- { // mixing2
		//   	c, clen = Wrap(c, clen, (vz.serverId+i) % len(vz.servers))
		//   }

		//vz.shufflerLock.Unlock()

		mb, mblen = vz.encryptMailboxGivenMeta(gelts, geltlen, mb, mblen, mdlen)
		c, clen = vz.combineTestMailboxContent(mb, mblen, c, clen, paddedBatchSize)

		// fixed wrap
		for i := vz.chainLen - 1; i >= 0; i-- { // mixing1
			c, clen = vz.Wrap(c, clen, (vz.serverId+i) % len(vz.servers))
		}
	} else {
		mb, mblen = vz.createMetadataMailbox(batchSize)
		md, mdlen = mb, mblen
		md, mdlen, gelts, geltlen, proof = elgamal.EncryptProven(md, mdlen, vz.serverId) //encrypt with own chainId
		paddedBatchSize = len(gelts) / geltlen
		c, clen = vz.createMsgContent(paddedBatchSize, msgSize)		
		for i := vz.chainLen - 1; i >= 0; i-- { // mixing1
			c, clen = vz.Wrap(c, clen, (vz.serverId+i) % len(vz.servers))
		}
	}

	log.WithFields(log.Fields{
		"geltlen": geltlen,
		"mdlen":   mdlen,
		"clen":    clen,
		//"content":         c[:2*clen],
		"batchSize":       batchSize,
		"paddedBatchSize": paddedBatchSize,
		"mdsize":          len(md) / mdlen,
		"geltsize":        len(gelts) / geltlen,
		"csize":           len(c) / clen,
	}).Warn("Initial encryption.")

	metadata := Batch{md, mdlen}
	content := Batch{c, clen}

	//Start first rotation round
	pargs := MessageBatchArgs{
		MixingPhase: mixing1,
		ChainId:     vz.serverId,
		Metadata:    metadata,
		Content:     content,
		MetaProof:   proof, // TODO remove
	}

	vz.preparedArgs = pargs

	proofArgs := VerifyEncryptionProofArgs{
		Sender:      vz.serverId,
		ChainId:     pargs.ChainId,
		Metadata:    pargs.Metadata.Msgs,
		Proof:       pargs.MetaProof,
	}
	vz.broadcastEncryptionProof(&proofArgs)

	*reply = PrepareReply{Ok: true}

	return nil
}

//RPC to start communication round
type StartArgs struct {
}

type StartReply struct {
	RoundStats ExperimentTime
}

func (vz *VZServer) Start(args StartArgs, reply *StartReply) error {
	log.WithFields(log.Fields{
		"id": vz.serverId,
	}).Info("Server received Start from Coordinator")

	go vz.handleStart()

	<-vz.roundDone
	s := StartReply{vz.timer}
	*reply = s

	// TODO (temporary hack) quit 5s later
	go func () {
		time.Sleep(5 * time.Second)
		os.Exit(0)
	}()

	return nil
}

func (vz *VZServer) handleStart() {
	log.Info("Starting protocol...")
	defer vz.timer.trackRound(time.Now())

	for _, c := range vz.encryptionVerificationStartChan {
		c <- 1
	}
	go vz.handleRotationRound(vz.preparedArgs)
}

////////////////////////
//Server helper methods
////////////////////////

//Initialize server
func (vz *VZServer) InitServer(id int, conf Config) {
	log.WithFields(log.Fields{
		"id":        id,
		"chainlen":  conf.ChainLen,
		"messages":  conf.NumMsgs,
		"servernum": len(conf.ListenAddrs),
	}).Info("Initializing Server")
	vz.serverId = id
	vz.chainLen = conf.ChainLen
	vz.listenAddrs = conf.ListenAddrs
	vz.numMsgs = conf.NumMsgs
	vz.servers = make([]*rpc.Client, len(vz.listenAddrs))
	vz.mu = sync.Mutex{}

	vz.hasher = dummyhasher{}
	vz.dummyshuffler = dummyshuffler{}
	vz.shuffler = groth.Groth{}
	// vz.shuffler = dummyshuffler{}
	vz.elgamal = groth.Groth{}
	// vz.elgamal = dummyshuffler{}
	vz.shufflerLock = sync.Mutex{}
	vz.timer = ExperimentTime{}
	vz.timer.init(vz.chainLen)
	vz.roundDone = make(chan bool)

	vz.encryptionVerificationStartChan = make([]chan int, vz.chainLen)
	for i := range vz.encryptionVerificationStartChan {
		vz.encryptionVerificationStartChan[i] = make(chan int, 1)
	}

	//initialize state variables
	//TODO make initialization cleaner
	vz.batchReceived = make([][]bool, 4)
	vz.batchNextPart = make([][]int, 4)
	vz.batchParts = make([][][]MessageBatchArgs, 4)
	for i := range vz.batchReceived {
		vz.batchReceived[i] = make([]bool, vz.chainLen)
		vz.batchNextPart[i] = make([]int, vz.chainLen)
		vz.batchParts[i] = make([][]MessageBatchArgs, vz.chainLen)
	}

	vz.onionKeys = make([][][]byte, 2)
	for i := range vz.onionKeys {
		vz.onionKeys[i] = make([][]byte, vz.chainLen)
	}

	vz.proofReceived = make([][][]bool, 2)
	for i := range vz.proofReceived {
		vz.proofReceived[i] = make([][]bool, vz.chainLen)
		for j := range vz.proofReceived[i] {
			vz.proofReceived[i][j] = make([]bool, vz.chainLen)
		}
	}

	vz.proofVerified = make([][]chan bool, 2)
	for i := range vz.proofVerified {
		vz.proofVerified[i] = make([]chan bool, vz.chainLen)
		for j := range vz.proofVerified[i] {
			vz.proofVerified[i][j] = make(chan bool, vz.chainLen)
		}
	}

	vz.encryptionProofReceived = make([]bool, vz.chainLen)
	vz.encryptionProofVerified = make(chan bool)

	vz.permutations = make([][][]int, 2)
	for i := range vz.permutations {
		vz.permutations[i] = make([][]int, vz.chainLen)
	}

	vz.shuffleDone = make(chan bool)

	vz.distOutMap = make([][][]int, 3)
	for i := range vz.distOutMap {
		vz.distOutMap[i] = make([][]int, len(vz.servers))
	}

	vz.distInMap = make([][]int, 3)
	for i := range vz.distInMap {
		vz.distInMap[i] = make([]int, len(vz.servers))
	}

	vz.distParcelsReceived = make([][]bool, 5)
	for i := range vz.distParcelsReceived {
		vz.distParcelsReceived[i] = make([]bool, len(vz.servers))
	}

	vz.distParcels = make([][]chan DistributeArgs, 5)
	for i := range vz.distParcels {
		vz.distParcels[i] = make([]chan DistributeArgs, len(vz.servers))
		for j := range vz.distParcels[i] {
			vz.distParcels[i][j] = make(chan DistributeArgs, 1)
		}
	}

	vz.distHashesReceived = make([][][][]bool, 2)
	for i := range vz.distHashesReceived {
		vz.distHashesReceived[i] = make([][][]bool, vz.chainLen)
		for j := range vz.distHashesReceived[i] {
			vz.distHashesReceived[i][j] = make([][]bool, len(vz.servers))
			for k := range vz.distHashesReceived[i][j] {
				vz.distHashesReceived[i][j][k] = make([]bool, vz.chainLen)
			}
		}
	}

	vz.distHashes = make([][][]chan []byte, 2)
	for i := range vz.distHashes {
		vz.distHashes[i] = make([][]chan []byte, vz.chainLen)
		for j := range vz.distHashes[i] {
			vz.distHashes[i][j] = make([]chan []byte, len(vz.servers))
			for k := range vz.distHashes[i][j] {
				vz.distHashes[i][j][k] = make(chan []byte)
			}
		}
	}

	vz.predistParcelBatchReceived = make([][]bool, 2)
	for i := range vz.predistParcelBatchReceived {
		vz.predistParcelBatchReceived[i] = make([]bool, vz.chainLen)
	}

	vz.postdistParcelBatchReceived = make([][]bool, 2)
	for i := range vz.postdistParcelBatchReceived {
		vz.postdistParcelBatchReceived[i] = make([]bool, vz.chainLen)
	}

	vz.postdistParcelBatchVerified = make([]chan bool, 2)
	for i := range vz.postdistParcelBatchVerified {
		vz.postdistParcelBatchVerified[i] = make(chan bool, vz.chainLen)
	}
}

//Call an RPC
func (vz *VZServer) call(recipient int, method string, args interface{}, reply interface{}) bool {
	if trackBandwidth {
		// replies are generally small so ignore
		gob.Register(args)
		buf := new(bytes.Buffer)
		encoder := gob.NewEncoder(buf)
		err1 := encoder.Encode(args)
		if err1 != nil {
			panic(err1)
		}
		log.WithFields(log.Fields{"size": buf.Len(), "method": method}).Info("RPC send")
	}

	err := vz.servers[recipient].Call(method, args, reply)
	if err != nil {
		log.WithFields(log.Fields{
			"err":       err,
			"recipient": recipient,
			"method":    method,
		}).Error("RPC call failed")
	}
	return (err == nil)
}

//Set up RPC connections with all servers
func (vz *VZServer) SetupConnections(addresses []string) {
	log.WithFields(log.Fields{
		"id":          vz.serverId,
		"num-servers": len(vz.servers),
	}).Info("Setting up server connections...")

	for i, addr := range addresses {
		if i != vz.serverId { //TODO Can you set up RPC connection to self?
			vz.connect("tcp", addr, i) //TODO parallelize
		}
	}
	vz.connectionsSet = true
	log.Info("Connection setup successful.")
}

func (vz *VZServer) connect(network, address string, id int) {
	c, err := rpc.Dial(network, address)
	if err != nil {
		log.WithFields(log.Fields{
			"err": err,
		}).Fatal("RPC dial failed")
	}
	//connect successful
	vz.servers[id] = c
}

// Helper methods for converting chain and server ids

func (vz *VZServer) citorr(chainId int) int {
	rr := (vz.serverId - chainId) % len(vz.servers)
	if rr < 0 {
		rr = rr + len(vz.servers)
	}
	return rr
}

func (vz *VZServer) nextid(id int) int {
	return (id + 1) % len(vz.servers)
}

func (vz *VZServer) previd(id int) int {
	return (id - 1 + len(vz.servers)) % len(vz.servers)
}
