package stadium

import (
	"math"
	"time"

	log "github.com/Sirupsen/logrus"
)

// applies to both metadata and content batches: how much do we want to stuff into a single batch?
const batchSizeLimit = 1024 * 1024 * 256 // tuned with trial and error

// This identifies an already-completed shuffle and can be passed in to prove to produce a proof given a shuffle
// TODO make actual type (may need to move to groth)
// type handle int;

//Interface for verifiable shuffle
type verifiableShuffler interface {
	//Encrypts byte array of messages
	Encrypt(secrets []byte, secretlen int, keyIndex int) (ciphers []byte, cipherlen int, groupelts []byte, groupeltlen int)

	//Encrypts byte array of messages with NIZK proof of plaintext
	EncryptProven(secrets []byte, secretlen int, keyIndex int) (ciphers []byte, cipherlen int, groupelts []byte, groupeltlen int, proof []byte)

	//Verifies encrypted byte array of messages with NIZK proof of plaintext
	EncryptVerify(ciphers []byte, proof []byte) bool

	//Decrypts byte array of messages
	Decrypt(ciphers []byte, cipherlen int, keyIndex int) (groupelts []byte, groupeltlen int)

	//Shuffles
	Shuffle(ciphers []byte, cipherlen int, keyIndex int) (ciphersout []byte, permutation []int, shuffleHandle int)

	//Produces proof given shuffle
	Prove(shuffle int) (proof []byte, public_randoms string)

	//Verifies proof
	Verify(proof []byte, ciphersin []byte, ciphersout []byte, keyIndex int, public_randoms string) bool
}

//Handles rotation round
func (vz *VZServer) handleRotationRound(args MessageBatchArgs) {
	rr := vz.citorr(args.ChainId)
	defer vz.timer.trackRR(rr, args.MixingPhase, time.Now())

	// onion unwrap (in background)
	cout := args.Content.Msgs
	clen := args.Content.Msglen

	//Shuffle
	//vz.shufflerLock.Lock()
	log.WithFields(log.Fields{
		"mdlen":  args.Metadata.Msglen,
		"mdsize": len(args.Metadata.Msgs) / args.Metadata.Msglen,
		"clen":   args.Content.Msglen,
		"csize":  len(args.Content.Msgs) / args.Content.Msglen,
	}).Info("Shuffle")
	vz.timer.ShuffleStarts[args.MixingPhase][rr] = time.Now()
	mdout, perm, shuffle := vz.shuffler.Shuffle(args.Metadata.Msgs, args.Metadata.Msglen, args.ChainId)
	vz.timer.ShuffleEnds[args.MixingPhase][rr] = time.Now()

	go func(done chan bool, args MessageBatchArgs) {
		vz.timer.ProofStarts[args.MixingPhase][rr] = time.Now()
		proof, rand := vz.shuffler.Prove(shuffle)
		vz.timer.ProofEnds[args.MixingPhase][rr] = time.Now()
		//Send out proofs
		proofArgs := VerifyShuffleProofArgs{
			Sender:        vz.serverId,
			MixingPhase:   args.MixingPhase,
			RotationRound: rr,
			ChainId:       args.ChainId,
			Input:         args.Metadata.Msgs,
			Output:        mdout,
			Proof:         proof,
			Randoms:       rand,
		}
		log.WithFields(log.Fields{
			"proof": len(proof),
			"input": len(proofArgs.Input),
		}).Info("RPC (proof)")
		vz.broadcastProof(&proofArgs)
		done <- true
	}(vz.shuffleDone, args)

	//vz.shufflerLock.Unlock()
	cout = applyPermutation(cout, clen, perm)
	vz.permutations[args.MixingPhase][rr] = perm

	vz.timer.UnwrapStarts[args.MixingPhase][rr] = time.Now()
	co, cl, keys := vz.Unwrap(cout, clen, vz.serverId)
	vz.onionKeys[args.MixingPhase][rr] = keys
	cout, clen = co, cl
	vz.timer.UnwrapEnds[args.MixingPhase][rr] = time.Now()

	//Pass message batch
	passArgs := MessageBatchArgs{
		Sender:      vz.serverId,
		MixingPhase: args.MixingPhase,
		ChainId:     args.ChainId,
		Metadata:    Batch{mdout, args.Metadata.Msglen},
		Content:     Batch{cout, clen},
	}
	log.WithFields(log.Fields{
		"metadata": len(mdout),
		"content": len(cout),
	}).Info("RPC (message)")

	if rr == 0 && args.MixingPhase == mixing1 {
		vz.checkEncryptionVerifications()
	}
	log.WithFields(log.Fields{"rr": rr}).Info("phase done")
	if rr < vz.chainLen-1 {
		vz.callPassMessageBatch(vz.nextid(vz.serverId), &passArgs)
	} else if rr == vz.chainLen-1 {
		//Check proof verifications are complete
		for i := 0; i <= rr; i++ {
			vz.checkVerifications(i, args.MixingPhase)
		}

		//Mixing phase is complete
		if args.MixingPhase == mixing1 && twoChains {
			go vz.handleVerifiableDistributionPhase(msgdist, &passArgs)
		} else if args.MixingPhase == mixing2 || !twoChains {
			go vz.handleUnverifiedDistributionPhase(mailboxdist, &passArgs)
		} else {
			log.Fatal("Not in mixing phase 1 or 2")
		}
	} else {
		log.Fatal("Rotation round greater than chain length")
	}
}

//Checks if verifications completed for given rotation round and mixing phase
func (vz *VZServer) checkVerifications(rr int, mp int) bool {
	numComplete := 0

	log.WithFields(log.Fields{
		"rotationRound": rr,
		"mixingPhase":   mp,
	}).Info("Started verification checks")

	succeeded := true
	//Expecting l-1 completed verifications
	for success := range vz.proofVerified[mp][rr] {

		if !success {
			log.WithFields(log.Fields{
				"source": numComplete,
				"rotationRound": rr,
				"mixingPhase":   mp,
			}).Warn("Shuffle verification failed")
			succeeded = false
		} else {
			log.WithFields(log.Fields{
				"source": numComplete,
				"rotationRound": rr,
				"mixingPhase":   mp,
			}).Info("Finished a verification")
		}

		numComplete++
		if numComplete == vz.chainLen-1 {
			break
		}
	}
	return succeeded
}

func (vz *VZServer) checkEncryptionVerifications() bool {
	numComplete := 0

	log.Info("Started encryption verification checks")

	succeeded := true
	//Expecting l-1 completed verifications
	for success := range vz.encryptionProofVerified {

		if !success {
			log.WithFields(log.Fields{
				"source": numComplete,
			}).Warn("Encryption verification failed")
			succeeded = false
		} else {
			log.WithFields(log.Fields{
				"source": numComplete,
			}).Info("Finished an encryption verification")
		}

		numComplete++
		if numComplete == vz.chainLen {
			break
		}
	}
	return succeeded
}

//Sends out proof to all other servers in chain
func (vz *VZServer) broadcastProof(args *VerifyShuffleProofArgs) {
	currId := args.ChainId
	for i := 0; i < vz.chainLen; i++ {
		if currId != vz.serverId {
			go vz.callVerifyShuffleProof(currId, args)
		}
		currId = vz.nextid(currId)
	}
}

func (vz *VZServer) broadcastEncryptionProof(args *VerifyEncryptionProofArgs) {
	// this blocks: only called during Prepare
	currId := args.ChainId
	done := make(chan int, vz.chainLen - 1)
	for i := 0; i < vz.chainLen; i++ {
		if currId != vz.serverId {
			go vz.callVerifyEncryptionProof(currId, args, done)
		} else {
			go vz.handleEncryptionVerification(*args)
		}
		currId = vz.nextid(currId)
	}

	vz.mu.Unlock()
	for i := 1; i < vz.chainLen; i++ {
		<-done
	}
	vz.mu.Lock()
}

//Handles verification of proof
func (vz *VZServer) handleVerification(args VerifyShuffleProofArgs) {
	//Verify proof
	//vz.shufflerLock.Lock()
	sender := ((vz.serverId - args.Sender + len(vz.servers)) % len(vz.servers)) % vz.chainLen
	vz.timer.VerifyStarts[args.MixingPhase][args.RotationRound][sender] = time.Now()
	success := vz.shuffler.Verify(args.Proof, args.Input, args.Output, args.ChainId, args.Randoms)
	vz.timer.VerifyEnds[args.MixingPhase][args.RotationRound][sender] = time.Now()
	//vz.shufflerLock.Unlock()
	//Signal verification success
	vz.proofVerified[args.MixingPhase][args.RotationRound] <- success
}

func (vz *VZServer) handleEncryptionVerification(args VerifyEncryptionProofArgs) {
	<-vz.encryptionVerificationStartChan[(vz.serverId - args.Sender + len(vz.servers)) % len(vz.servers)]

	//Verify proof
	//vz.shufflerLock.Lock()
	vz.timer.EncryptVerifyStarts[(vz.serverId - args.Sender + len(vz.servers)) % len(vz.servers)] = time.Now()
	success := vz.elgamal.EncryptVerify(args.Metadata, args.Proof)
	vz.timer.EncryptVerifyEnds[(vz.serverId - args.Sender + len(vz.servers)) % len(vz.servers)] = time.Now()
	//vz.shufflerLock.Unlock()

	//Signal verification success
	vz.encryptionProofVerified <- success
}

//Passes message batch to next server
func (vz *VZServer) callPassMessageBatch(id int, args *MessageBatchArgs) {
	// deconstruct args into constituent parts
	mdBatch := args.Metadata
	ctBatch := args.Content // almost always larger of the two
	length := int(math.Ceil(float64(len(ctBatch.Msgs)) / float64(batchSizeLimit)))
	parts := make([]MessageBatchArgs, length)
	
	for i := 0; i < length; i++ {
		mdPart := make([]byte, 0)
		if i*batchSizeLimit < len(mdBatch.Msgs) {
			suffix := mdBatch.Msgs[i*batchSizeLimit:]
			s := batchSizeLimit
			if s > len(suffix) {
				s = len(suffix)
			}
			mdPart = make([]byte, s)
			copy(mdPart, suffix)
		}
		suffix := ctBatch.Msgs[i*batchSizeLimit:]
		s := batchSizeLimit
		if s > len(suffix) {
			s = len(suffix)
		}
		ctPart := make([]byte, s)
		copy(ctPart, suffix)
		parts[i] = MessageBatchArgs{
			Sender:      args.Sender,
			MixingPhase: args.MixingPhase,
			ChainId:     args.ChainId,
			Metadata:    Batch{mdPart, mdBatch.Msglen},
			Content:     Batch{ctPart, ctBatch.Msglen},
			MetadataLen: len(args.Metadata.Msgs),
			ContentLen:  len(args.Content.Msgs),
			NumPart:     i,
			TotalParts:  length,
		}
	}

	log.WithFields(log.Fields{
		"content-length": len(args.Content.Msgs),
		"metadata-length": len(args.Metadata.Msgs),
	}).Info("send with split")
	reply := MessageBatchReply{}


	for _, p := range parts {
		for ok := false; !ok; {
			log.WithFields(log.Fields{"part": p.NumPart, "out-of": p.TotalParts}).Info("send")
			ok = vz.call(id, "VZServer.PassMessageBatch", p, &reply)
		}
	}
}

//Sends proof to server
func (vz *VZServer) callVerifyShuffleProof(id int, args *VerifyShuffleProofArgs) {
	reply := VerifyShuffleProofReply{}

	for ok := false; !ok; {
		ok = vz.call(id, "VZServer.VerifyShuffleProof", *args, &reply)
	}
}

func (vz *VZServer) callVerifyEncryptionProof(id int, args *VerifyEncryptionProofArgs, done chan int) {
	reply := VerifyEncryptionProofReply{}

	for ok := false; !ok; {
		ok = vz.call(id, "VZServer.VerifyEncryptionProof", *args, &reply)
	}

	done <- 1
}

////////////////////
//RPC //////////////
////////////////////

type MessageBatchArgs struct {
	Sender      int   //id of server sending messages
	MixingPhase int   //mixing phase number
	ChainId     int   //id of chain
	Metadata    Batch //message metadata
	Content     Batch //message content
	MetaProof   []byte//used solely at beginning of first round to verify ciphertext proofs
	EMetadata   Batch //TODO used for one purpose in distribution
	// needed to split message up into many parts
	NumPart     int   
	TotalParts  int
	MetadataLen int
	ContentLen  int
}

type MessageBatchReply struct {
}

type VerifyShuffleProofArgs struct {
	Sender        int    //id of server sending proof
	MixingPhase   int    //mixing phase number
	RotationRound int    //rotation round of proof
	ChainId       int    //chain id of proof
	Input         []byte //input array
	Output        []byte //output array
	Proof         []byte //proof
	Randoms       string //public randoms
}

type VerifyShuffleProofReply struct {
}

type VerifyEncryptionProofArgs struct {
	Sender      int
	ChainId     int
	Metadata    []byte // encrypted metadata for the input chain
	Proof       []byte // proof for Ciphertexts
}

type VerifyEncryptionProofReply struct {
}

func (vz *VZServer) PassMessageBatch(args MessageBatchArgs, reply *MessageBatchReply) error {
	log.WithFields(log.Fields{
		"id":                 vz.serverId,
		"sender":             args.Sender,
		"round":              vz.citorr(args.ChainId),
		"chainid":            args.ChainId,
		"sender-mixingphase": args.MixingPhase,
		//"metadata":           args.Metadata.Msgs,
		"content0": args.Content.Msgs[0:8],
		"content1": args.Content.Msgs[args.Content.Msglen : args.Content.Msglen+8],
		"part": args.NumPart,
		"total": args.TotalParts,
		//"content": args.Content.Msgs,
	}).Info("Received message batch")

	vz.mu.Lock()
	defer vz.mu.Unlock()

	mixingPhase := args.MixingPhase
	round := vz.citorr(args.ChainId)
	//Ignore parts out of order
	if vz.batchNextPart[mixingPhase][round] != args.NumPart {
		log.WithFields(log.Fields{
			"received": args.NumPart,
			"expecting": vz.batchNextPart[mixingPhase][round],
		}).Warn("MessageBatch part out of order")
		return nil
	}
	vz.batchNextPart[mixingPhase][round] += 1

	//Save part
	if vz.batchNextPart[mixingPhase][round] == 1 {
		vz.batchParts[mixingPhase][round] = make([]MessageBatchArgs, args.TotalParts)
	}
	vz.batchParts[mixingPhase][round][args.NumPart] = args

	if vz.batchNextPart[mixingPhase][round] != args.TotalParts {
		// wait for next part
		return nil
	}

	//Reconstruct original message batch
	log.WithFields(log.Fields{
		"content-length": args.ContentLen,
		"metadata-length": args.MetadataLen,
	}).Info("receive with split")	
	md := make([]byte, args.MetadataLen)
	ct := make([]byte, args.ContentLen)
	for i := 0; i < args.TotalParts; i++ {
		if i * batchSizeLimit < len(md) {
			copy(md[i * batchSizeLimit:], vz.batchParts[mixingPhase][round][i].Metadata.Msgs)
		}
		copy(ct[i * batchSizeLimit:], vz.batchParts[mixingPhase][round][i].Content.Msgs)
	}
	argsFull := MessageBatchArgs{
		Sender: args.Sender,
		MixingPhase: mixingPhase,
		ChainId: args.ChainId,
		Metadata: Batch{md, args.Metadata.Msglen},
		Content: Batch{ct, args.Content.Msglen},
	}
	vz.batchParts[mixingPhase][round] = nil // delete refs to old parts to prevent memory leaks

	//Check if batch has been received
	if vz.batchReceived[mixingPhase][round] {
		log.Warn("Dropping duplicate message batch")
		return nil
	}
	vz.batchReceived[mixingPhase][round] = true

	//Handle new message batch
	if (mixingPhase == mixing1) || (mixingPhase == mixing2) {
		go vz.handleRotationRound(argsFull)
	} else if (mixingPhase == mixingreturn1) || (mixingPhase == mixingreturn2) {
		go vz.handleUnverifiedRotationRound(argsFull)
	} else {
		log.Fatal("Unrecognized mixing phase")
	}
	return nil
}

func (vz *VZServer) VerifyShuffleProof(args VerifyShuffleProofArgs, reply *VerifyShuffleProofReply) error {
	vz.mu.Lock()
	defer vz.mu.Unlock()

	//Check if proof has been received
	if vz.proofReceived[args.MixingPhase][args.RotationRound][vz.citorr(args.ChainId)] {
		log.WithFields(log.Fields{
			"sender": args.Sender,
			"rr":     args.RotationRound,
			"mp":     args.MixingPhase,
			"chain":  args.ChainId,
		}).Warn("Dropping duplicate proof")
		return nil
	}
	vz.proofReceived[args.MixingPhase][args.RotationRound][vz.citorr(args.ChainId)] = true

	//Handle verification
	go vz.handleVerification(args)
	return nil
}

func (vz *VZServer) VerifyEncryptionProof(args VerifyEncryptionProofArgs, reply *VerifyEncryptionProofReply) error {
	vz.mu.Lock()
	defer vz.mu.Unlock()

	//Check if proof has been received
	if vz.encryptionProofReceived[(vz.serverId - args.Sender + len(vz.servers)) % len(vz.servers)] {
		log.WithFields(log.Fields{
			"sender": args.Sender,
			"chain":  args.ChainId,
		}).Warn("Dropping duplicate encryption proof")
		return nil
	}
	vz.encryptionProofReceived[(vz.serverId - args.Sender + len(vz.servers)) % len(vz.servers)] = true

	//Handle verification
	go vz.handleEncryptionVerification(args)
	return nil
}
