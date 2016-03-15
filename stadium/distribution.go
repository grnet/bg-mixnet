package stadium

import (
	"time"

	log "github.com/Sirupsen/logrus"
)

//Methods and structs for verifiable distribution

//Handles verifiable distribution phase
func (vz *VZServer) handleVerifiableDistributionPhase(distPhase int, args *MessageBatchArgs) {
	defer vz.timer.trackDist(distPhase, time.Now())
	//Handle threshold decryption
	//vz.shufflerLock.Lock()
	vz.timer.DecryptStarts[0] = time.Now()
	msgs, msglen := vz.elgamal.Decrypt(args.Metadata.Msgs, args.Metadata.Msglen, args.ChainId)
	vz.timer.DecryptEnds[0] = time.Now()
	//vz.shufflerLock.Unlock()
	args.EMetadata.Msglen = args.Metadata.Msglen //used for content parsing
	args.Metadata.Msgs = msgs
	args.Metadata.Msglen = msglen

	//Create parcels and distribute to other servers
	distArgs := make([]DistributeArgs, len(vz.servers))
	vz.createParcels(distPhase, args, distArgs)

	//Broadcast parcel batch to chain for hashes
	vz.requestHashesFromChain(distPhase, args.ChainId, distArgs)

	//Distribute parcels to all other servers
	vz.distributeParcels(distPhase, args.ChainId, distArgs)

	//Receive incoming parcels
	parcelBatchArgs := ParcelBatchArgs{}
	nextArgs := MessageBatchArgs{}
	vz.receiveParcels(distPhase, &nextArgs, &parcelBatchArgs)

	//Broadcast new message batch to chain
	parcelBatchArgs.Sender = vz.serverId
	parcelBatchArgs.DistPhase = distPhase
	parcelBatchArgs.ChainId = vz.serverId
	vz.sendParcelBatchToChain(&parcelBatchArgs)

	//Verify new message batches of others with received hashes
	vz.checkParcelBatchVerifications(distPhase)

	//Start next phase
	nextArgs.Sender = vz.serverId
	nextArgs.ChainId = vz.serverId
	if distPhase == msgdist {
		nextArgs.MixingPhase = mixing2
		log.Info("Starting Mixing Phase 2")
		go vz.handleRotationRound(nextArgs)
	} else { //TODO add verifiable noise distribution
		log.Fatal("Unrecognized verifiable distribution phase")
	}
}

//Distribute parcels to other servers
func (vz *VZServer) distributeParcels(distPhase int, chainId int, distArgs []DistributeArgs) {
	//Send parcels
	for i, _ := range vz.servers {
		distArgs[i].Sender = vz.serverId
		distArgs[i].DistPhase = distPhase
		distArgs[i].ChainId = chainId
		if i != vz.serverId {
			log.WithFields(log.Fields{
				"to":      i,
				"numMsgs": len(distArgs[i].Content.Msgs) / distArgs[i].Content.Msglen,
				//"parcelmeta":    distArgs[i].Metadata.Msgs,
				//"parcelcontent": distArgs[i].Content.Msgs,
			}).Info("Parcel sizes.")
			go vz.callDistributeMessages(i, distArgs[i]) //TODO change to pass pointer
		} else {
			reply := DistributeReply{}
			go vz.DistributeMessages(distArgs[i], &reply)
		}
	}
}

//Create parcels
func (vz *VZServer) createParcels(distPhase int, batchArgs *MessageBatchArgs, distArgs []DistributeArgs) {
	if (distPhase == msgdist) || (distPhase == mailboxdist) {
		vz.splitAndCreateOutMap(distPhase, batchArgs, distArgs)
	} else if (distPhase == mailboxdistreturn) || (distPhase == msgdistreturn) {
		vz.splitContentByInMap(distPhase, batchArgs, distArgs)
	} else { //TODO add support for noise dist
		log.Fatal("Unrecognized distribution phase.")
	}
}

func (vz *VZServer) splitAndCreateOutMap(distPhase int, batchArgs *MessageBatchArgs, dist []DistributeArgs) {
	// Use metadata to find server to distribute to
	metadata := batchArgs.Metadata.Msgs
	metalen := batchArgs.Metadata.Msglen
	content := batchArgs.Content.Msgs
	contentlen := batchArgs.Content.Msglen
	var newmeta, newcontent []byte

	if len(metadata)/metalen != len(content)/contentlen {
		log.WithFields(log.Fields{
			"meta":    len(metadata) / metalen,
			"content": len(content) / contentlen,
		}).Warn("Meta and content different lengths")
	}

	for i := 0; i < len(metadata)/metalen; i++ {
		dest := vz.parseDestination(metadata[metalen*i : metalen*i+metalen])
		if distPhase == msgdist {
			newmeta, newcontent = vz.parseNewMetaFromContent(content[contentlen*i:contentlen*(i+1)], batchArgs.EMetadata.Msglen)
		} else if distPhase == mailboxdist {
			newmeta = metadata[metalen*i : metalen*(i+1)]
			newcontent = content[contentlen*i : contentlen*(i+1)]
		} else {
			log.Fatal("Not a forward distribution.")
		}

		vz.distOutMap[distPhase][dest] = append(vz.distOutMap[distPhase][dest], i)
		dist[dest].Metadata.Msgs = append(dist[dest].Metadata.Msgs, newmeta...)
		dist[dest].Content.Msgs = append(dist[dest].Content.Msgs, newcontent...)
	}

	// Initialize distribute args batch msg len
	for i := 0; i < len(vz.servers); i++ {
		dist[i].Metadata.Msglen = len(newmeta)
		dist[i].Content.Msglen = len(newcontent)
	}
}

func (vz *VZServer) splitContentByInMap(distPhase int, batchArgs *MessageBatchArgs, dist []DistributeArgs) {
	//Find which in map to use
	var inmap int
	if distPhase == mailboxdistreturn {
		inmap = mailboxdist
	} else if distPhase == msgdistreturn {
		inmap = msgdist
	} else {
		log.Fatal("Unrecognized dist phase for split by in map")
	}

	content := batchArgs.Content.Msgs
	msglen := batchArgs.Content.Msglen

	start := 0
	for i := 0; i < len(vz.servers); i++ {
		end := vz.distInMap[inmap][i]
		dist[i].Content.Msglen = msglen
		dist[i].Content.Msgs = content[msglen*start : msglen*end]
		start = end
	}
}

//Receive incoming parcels to form new message batch
func (vz *VZServer) receiveParcels(distPhase int, batchArgs *MessageBatchArgs, parcelArgs *ParcelBatchArgs) {
	distArgs := make([]DistributeArgs, len(vz.servers))
	msgnum := 0
	//Expecting m parcels (num of servers)
	for i, _ := range vz.servers {
		distArgs[i] = <-vz.distParcels[distPhase][i]
		msgnum += len(distArgs[i].Content.Msgs) / distArgs[i].Content.Msglen
	}

	//Pull parcel information for hash checks
	if distPhase == msgdist {
		parcelArgs.Metadata = make([]Batch, len(vz.servers))
		for i, _ := range vz.servers {
			parcelArgs.Metadata[distArgs[i].ChainId] = distArgs[i].Metadata
		}
	}

	//Create new message batch
	if (distPhase == msgdist) || (distPhase == mailboxdist) {
		vz.mergeAndCreateInMap(distPhase, distArgs, batchArgs)
	} else if (distPhase == mailboxdistreturn) || (distPhase == msgdistreturn) {
		vz.mergeContentByOutMap(distPhase, distArgs, batchArgs)
	} else {
		log.Fatal("Unrecognized dist phase for receiving parcels") //TODO noise dist
	}
}

func (vz *VZServer) mergeAndCreateInMap(distPhase int, distArgs []DistributeArgs, batchArgs *MessageBatchArgs) {
	currEnd := 0
	for i, dist := range distArgs {
		msgnum := len(dist.Metadata.Msgs) / dist.Metadata.Msglen
		batchArgs.Metadata.Msgs = append(batchArgs.Metadata.Msgs, dist.Metadata.Msgs...)
		batchArgs.Content.Msgs = append(batchArgs.Content.Msgs, dist.Content.Msgs...)
		currEnd += msgnum
		vz.distInMap[distPhase][i] = currEnd
	}
	batchArgs.Metadata.Msglen = distArgs[0].Metadata.Msglen
	batchArgs.Content.Msglen = distArgs[0].Content.Msglen
}

func (vz *VZServer) mergeContentByOutMap(distPhase int, distArgs []DistributeArgs, batchArgs *MessageBatchArgs) {
	//Find which in map to use
	var outmap int
	if distPhase == mailboxdistreturn {
		outmap = mailboxdist
	} else if distPhase == msgdistreturn {
		outmap = msgdist
	} else {
		log.Fatal("Unrecognized dist phase for split by in map")
	}

	msglen := distArgs[0].Content.Msglen
	msgnum := 0
	//Initialize batch - find how many messages
	//TODO eliminate need for this loop
	for _, out := range vz.distOutMap[outmap] {
		msgnum += len(out)
	}
	batchArgs.Content.Msgs = make([]byte, msglen*msgnum)

	//Populate batch
	for i, out := range vz.distOutMap[outmap] {
		for j, batchIndex := range out {
			copy(batchArgs.Content.Msgs[msglen*batchIndex:msglen*batchIndex+msglen], distArgs[i].Content.Msgs[msglen*j:msglen*j+msglen])
		}
	}
	batchArgs.Content.Msglen = msglen
}

//Handle parcel by waiting for it to be merged into new batch
func (vz *VZServer) handleParcel(args DistributeArgs) {
	vz.distParcels[args.DistPhase][args.Sender] <- args
}

//Distributes parcel of message batch to a server with input id
func (vz *VZServer) callDistributeMessages(id int, args DistributeArgs) {
	reply := DistributeReply{}

	for ok := false; !ok; {
		ok = vz.call(id, "VZServer.DistributeMessages", args, &reply)
	}
}

////////////////////
//RPC //////////////
////////////////////

type DistributeArgs struct {
	Sender    int   //id of server sending messages
	DistPhase int   //distribution phase
	ChainId   int   //id of chain sending parcel
	Metadata  Batch //parcel metadata
	Content   Batch //parcl content
}

type DistributeReply struct {
}

func (vz *VZServer) DistributeMessages(args DistributeArgs, reply *DistributeReply) error {
	log.WithFields(log.Fields{
		"id":               vz.serverId,
		"sender":           args.Sender,
		"chainId":          args.ChainId,
		"sender-distphase": args.DistPhase,
	}).Info("Received messages from distribution")

	vz.mu.Lock()
	defer vz.mu.Unlock()

	//Check if parcel has been received
	if vz.distParcelsReceived[args.DistPhase][args.ChainId] {
		log.Warn("Dropping duplicate parcel")
		return nil
	}
	vz.distParcelsReceived[args.DistPhase][args.ChainId] = true

	//Handle parcel
	go vz.handleParcel(args)
	return nil
}
