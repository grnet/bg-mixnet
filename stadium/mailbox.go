package stadium

import (
	"time"

	log "github.com/Sirupsen/logrus"
)

//Handles unverified distribution phase
func (vz *VZServer) handleUnverifiedDistributionPhase(distPhase int, args *MessageBatchArgs) {
	defer vz.timer.trackDist(distPhase, time.Now())
	if distPhase == mailboxdist {
		//Handle threshold decryption
		vz.timer.DecryptStarts[1] = time.Now()
		mb, mblen := vz.elgamal.Decrypt(args.Metadata.Msgs, args.Metadata.Msglen, args.ChainId)
		vz.timer.DecryptEnds[1] = time.Now()
		mb = mb[:mblen*(len(args.Metadata.Msgs)/args.Metadata.Msglen)] //remove padding
		args.Metadata.Msgs = mb
		args.Metadata.Msglen = mblen
		log.WithFields(log.Fields{
			"mbsize": len(mb) / mblen,
			"mblen":  mblen,
			"csize":  len(args.Content.Msgs) / args.Content.Msglen,
			"clen":   args.Content.Msglen,
		}).Warn("Second chain decryption.")
	}

	//Create parcels and distribute to other servers
	distArgs := make([]DistributeArgs, len(vz.servers))
	vz.createParcels(distPhase, args, distArgs)
	vz.distributeParcels(distPhase, args.ChainId, distArgs)

	//Receive incoming parcels
	nextArgs := MessageBatchArgs{}
	vz.receiveParcels(distPhase, &nextArgs, &ParcelBatchArgs{})

	//Start new phase
	if distPhase == mailboxdist {
		//Perform mailbox swaps
		log.WithFields(log.Fields{}).Warn("Mailbox swap.")
		swapped := vz.handleMailboxSwaps(&nextArgs.Metadata, &nextArgs.Content)
		nextArgs.Content = swapped

		//Return messages
		nextArgs.ChainId = args.ChainId
		go vz.handleUnverifiedDistributionPhase(mailboxdistreturn, &nextArgs)
	} else if distPhase == mailboxdistreturn && twoChains {
		//Start first return mixing phase
		nextArgs.MixingPhase = mixingreturn1
		nextArgs.ChainId = args.ChainId
		go vz.handleUnverifiedRotationRound(nextArgs)
	} else if distPhase == msgdistreturn || !twoChains {
		//Start second return mixing phase
		nextArgs.MixingPhase = mixingreturn2
		nextArgs.ChainId = (vz.serverId - (vz.chainLen - 1) + len(vz.servers)) % len(vz.servers) //first server in chain
		go vz.handleUnverifiedRotationRound(nextArgs)
	} else {
		log.Fatal("Unrecognized unverified distribution phase")
	}
}

//Handles unverified rotation round
func (vz *VZServer) handleUnverifiedRotationRound(args MessageBatchArgs) {
	rr := vz.citorr(args.ChainId)

	// if this the last round, send stats to coordinator
	lastRound := false
	fn := func(t time.Time) {
		vz.timer.trackRR(rr, args.MixingPhase, t)
		if (lastRound) {
			vz.roundDone <- true
		}
	}
	defer fn(time.Now())

	//Shuffle
	var reversePhase int
	if args.MixingPhase == mixingreturn1 {
		reversePhase = mixing2
	} else if args.MixingPhase == mixingreturn2 {
		reversePhase = mixing1
	}
	perm := vz.permutations[reversePhase][rr]
	cout := reversePermutation(args.Content.Msgs, args.Content.Msglen, perm)
	clen := args.Content.Msglen
	cout, clen = Rewrap(cout, clen, vz.onionKeys[reversePhase][rr])

	//Pass message batch
	passArgs := MessageBatchArgs{
		Sender:      vz.serverId,
		MixingPhase: args.MixingPhase,
		ChainId:     args.ChainId,
		Content:     Batch{cout, clen},
	}
	if (rr > 0) && (rr < vz.chainLen) {
		//Rotation rounds count down in return path
		vz.callPassMessageBatch(vz.previd(vz.serverId), &passArgs)
	} else if rr == 0 {
		//Mixing phase is complete
		if args.MixingPhase == mixingreturn1 {
			go vz.handleUnverifiedDistributionPhase(msgdistreturn, &passArgs)
		} else if args.MixingPhase == mixingreturn2 {
			//Ensure all shuffles completed
			count := 0
			shuffles := 2*vz.chainLen
			if !twoChains {
				shuffles = vz.chainLen
			}
			for range vz.shuffleDone {
				count++
				if count == shuffles {
					break
				}
			}
			log.Info("Stadium round complete!")
			lastRound = true
		} else {
			log.Fatal("Not in mixing return phase 1 or 2")
		}
	} else {
		log.Fatal("Rotation round greater than chain length")
	}
}

//Handles mailbox swaps
func (vz *VZServer) handleMailboxSwaps(metadata *Batch, content *Batch) Batch {
	out := make([]byte, len(content.Msgs))
	mail := make(map[int64]int) //maps mailbox id to content index

	clen := content.Msglen
	mdlen := metadata.Msglen
	num := len(metadata.Msgs) / mdlen
	if len(metadata.Msgs)/mdlen != len(content.Msgs)/clen {
		log.WithFields(log.Fields{
			"mdsize": len(metadata.Msgs) / mdlen,
			"mdlen":  mdlen,
			"csize":  len(content.Msgs) / clen,
			"clen":   clen,
		}).Fatal("Metadata and content different sizes in mailbox swaps.")
	}

	for i := 0; i < num; i++ {
		mid := vz.parseMailbox(metadata.Msgs[i*mdlen : (i+1)*mdlen])
		if j, ok := mail[mid]; ok {
			//swap indices
			copy(out[j*clen:(j+1)*clen], content.Msgs[i*clen:(i+1)*clen])
			copy(out[i*clen:(i+1)*clen], content.Msgs[j*clen:(j+1)*clen])
		} else {
			mail[mid] = i
			copy(out[i*clen:(i+1)*clen], content.Msgs[i*clen:(i+1)*clen])
		}
	}
	return Batch{out, clen}
}
