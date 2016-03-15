package stadium

import (
	"bytes"

	log "github.com/Sirupsen/logrus"
)

//Interface for hash
type hasher interface {
	Hash(input []byte) []byte
}

func (vz *VZServer) requestHashesFromChain(distPhase int, chainId int, distArgs []DistributeArgs) {
	parcelBatchArgs := ParcelBatchArgs{}
	parcelBatchArgs.Sender = vz.serverId
	parcelBatchArgs.DistPhase = distPhase
	parcelBatchArgs.ChainId = chainId
	parcelBatchArgs.Metadata = make([]Batch, len(vz.servers))
	for i, _ := range vz.servers {
		parcelBatchArgs.Metadata[i] = distArgs[i].Metadata
	}
	currId := parcelBatchArgs.ChainId
	for i := 0; i < vz.chainLen; i++ {
		if currId != vz.serverId {
			go vz.callRequestParcelHashes(currId, &parcelBatchArgs)
		}
		currId = vz.nextid(currId)
	}
}

func (vz *VZServer) sendParcelBatchToChain(args *ParcelBatchArgs) {
	// Send batch to be verified by hashes to all servers in chain including self
	currId := vz.nextid(vz.serverId)
	for i := 1; i < vz.chainLen; i++ {
		go vz.callVerifyParcelBatch(currId, args)
		currId = vz.nextid(currId)
	}
	reply := ParcelBatchReply{}
	vz.VerifyParcelBatch(*args, &reply)
}

func (vz *VZServer) checkParcelBatchVerifications(distPhase int) bool {
	// Check whether all new batches for chains which it is a part of were successfully verified
	numVerified := 0
	//Expecting l batches to be verified (including own)
	for success := range vz.postdistParcelBatchVerified[distPhase] {
		if !success {
			log.WithFields(log.Fields{
				"distPhase": distPhase,
			}).Fatal("New parcel batch verification failed")
		}
		numVerified++
		if numVerified == vz.chainLen {
			break
		}
	}
	return true
}

func (vz *VZServer) handlePreDistParcelBatch(args *ParcelBatchArgs) {
	// For each parcel, send hash to entire destination chain
	hashArgs := make([]ParcelHashArgs, len(vz.servers))
	for i, _ := range vz.servers {
		hashArgs[i].Sender = vz.serverId
		hashArgs[i].DistPhase = args.DistPhase
		hashArgs[i].ChainId = args.ChainId
		hashArgs[i].SenderOrder = vz.citorr(args.ChainId)
		hashArgs[i].DestChainId = i
		hashArgs[i].ParcelHash = vz.hasher.Hash(args.Metadata[i].Msgs)

		currId := hashArgs[i].DestChainId
		for j := 0; j < vz.chainLen; j++ {
			if currId != vz.serverId {
				go vz.callSendParcelHash(currId, &hashArgs[i])
			} else {
				reply := ParcelHashReply{}
				go vz.SendParcelHash(hashArgs[i], &reply)
			}
			currId = vz.nextid(currId)
		}
	}
}

func (vz *VZServer) handleParcelHash(args *ParcelHashArgs) {
	vz.distHashes[args.DistPhase][vz.citorr(args.DestChainId)][args.ChainId] <- args.ParcelHash
}

//TODO Slow way of checking parcel hashes since will get stuck at any channel for which some server is slow at sending hashes
func (vz *VZServer) handlePostDistParcelBatch(args *ParcelBatchArgs) {
	myOrder := vz.citorr(args.ChainId)
	// Check each parcel in batch against received hashes
	for i, _ := range vz.servers {
		parcel := args.Metadata[i].Msgs
		numHashes := 0
		for h := range vz.distHashes[args.DistPhase][myOrder][i] {
			if !bytes.Equal(h, vz.hasher.Hash(parcel)) {
				log.WithFields(log.Fields{
					"sendingchain":   i,
					"receivingchain": myOrder,
					"receivedHash":   h,
					"actualHash":     vz.hasher.Hash(parcel),
				}).Fatal("Verifiable distribution hash check failed")
			}
			numHashes++
			if numHashes == vz.chainLen-1 {
				break
			}
		}
	}
	// Verification of parcel batch succeeded
	vz.postdistParcelBatchVerified[args.DistPhase] <- true
}

//Sends predist parcel batch to server with input id
func (vz *VZServer) callRequestParcelHashes(id int, args *ParcelBatchArgs) {
	reply := ParcelBatchReply{}

	for ok := false; !ok; {
		ok = vz.call(id, "VZServer.RequestParcelHashes", *args, &reply)
	}
}

//Sends parcel hash to server with input id
func (vz *VZServer) callSendParcelHash(id int, args *ParcelHashArgs) {
	reply := ParcelHashReply{}

	for ok := false; !ok; {
		ok = vz.call(id, "VZServer.SendParcelHash", *args, &reply)
	}
}

//Sends postdist parcel batch to server with input id
func (vz *VZServer) callVerifyParcelBatch(id int, args *ParcelBatchArgs) {
	reply := ParcelBatchReply{}

	for ok := false; !ok; {
		ok = vz.call(id, "VZServer.VerifyParcelBatch", *args, &reply)
	}
}

////////////////////
//RPC //////////////
////////////////////

type ParcelBatchArgs struct {
	Sender    int     //id of server sending messages
	DistPhase int     //distribution phase number
	ChainId   int     //id of chain
	Metadata  []Batch //message metadata for each parcel
}

type ParcelBatchReply struct {
}

type ParcelHashArgs struct {
	Sender      int    //id of server sending messages
	DistPhase   int    //distribution phase number
	ChainId     int    //id of chain
	SenderOrder int    //order of sender in chain
	DestChainId int    //id of chain parcel is distributed to
	ParcelHash  []byte //hash of parcel
}

type ParcelHashReply struct {
}

func (vz *VZServer) RequestParcelHashes(args ParcelBatchArgs, reply *ParcelBatchReply) error {
	log.WithFields(log.Fields{
		"id":      vz.serverId,
		"sender":  args.Sender,
		"order":   vz.citorr(args.ChainId),
		"chainid": args.ChainId,
	}).Debug("Received predist parcel batch")

	vz.mu.Lock()
	defer vz.mu.Unlock()

	//Check if parcel batch has been received
	if vz.predistParcelBatchReceived[args.DistPhase][vz.citorr(args.ChainId)] {
		log.Warn("Dropping duplicate predist parcel batch")
		return nil
	}
	vz.predistParcelBatchReceived[args.DistPhase][vz.citorr(args.ChainId)] = true

	//Handle creating and sending hashes for parcel batch
	go vz.handlePreDistParcelBatch(&args)
	return nil
}

func (vz *VZServer) SendParcelHash(args ParcelHashArgs, reply *ParcelHashReply) error {
	log.WithFields(log.Fields{
		"id":          vz.serverId,
		"sender":      args.Sender,
		"destchainid": args.DestChainId,
		"order":       vz.citorr(args.DestChainId),
		"senderorder": args.SenderOrder,
		"chainid":     args.ChainId,
	}).Debug("Received parcel hash")

	vz.mu.Lock()
	defer vz.mu.Unlock()

	//Check if parcel hash has been received
	if vz.distHashesReceived[args.DistPhase][vz.citorr(args.DestChainId)][args.ChainId][args.SenderOrder] {
		log.Warn("Dropping duplicate parcel hash")
		return nil
	}
	vz.distHashesReceived[args.DistPhase][vz.citorr(args.DestChainId)][args.ChainId][args.SenderOrder] = true

	//Handle parcel hash
	go vz.handleParcelHash(&args)
	return nil
}

func (vz *VZServer) VerifyParcelBatch(args ParcelBatchArgs, reply *ParcelBatchReply) error {
	log.WithFields(log.Fields{
		"id":      vz.serverId,
		"sender":  args.Sender,
		"order":   vz.citorr(args.ChainId),
		"chainid": args.ChainId,
	}).Debug("Received postdist parcel batch")

	vz.mu.Lock()
	defer vz.mu.Unlock()

	//Check if parcel batch has been received
	if vz.postdistParcelBatchReceived[args.DistPhase][vz.citorr(args.Sender)] {
		log.Warn("Dropping duplicate postdist parcel batch")
		return nil
	}
	vz.postdistParcelBatchReceived[args.DistPhase][vz.citorr(args.Sender)] = true

	//Handle creating and sending hashes for parcel batch
	go vz.handlePostDistParcelBatch(&args)
	return nil
}
