package stadium

import (
	"bytes"
	"crypto/rand"
	"crypto/sha256"
	"encoding/binary"
	"runtime"

	"stadium/groth"
	"golang.org/x/crypto/salsa20"
	log "github.com/Sirupsen/logrus"
)

// Struct to hold many messages (content or metadata)
type Batch struct {
	Msgs   []byte
	Msglen int
}

// Message and metadata helper functions

//Takes byte array of decrypted metadata and parses out destination
func (vz *VZServer) parseDestination(md []byte) int {
	//TODO temporary
	c := decodeInt64(md, len(md))
	return int(uint(c[0]) % uint(len(vz.servers)))
}

func (vz *VZServer) parseMailbox(md []byte) int64 {
	//TODO temporary
	c := decodeInt64(md, len(md))
	return c[0]
}

//Takes byte array of message content after first input chain and parses out the mailbox metadata to be used in the second chain
func (vz *VZServer) parseNewMetaFromContent(msg []byte, metalen int) (m []byte, c []byte) {
	return msg[:metalen], msg[metalen:]
}

//Encrypts mailbox metadata with proper chain key given the input chain destination metadata
func (vz *VZServer) encryptMailboxGivenMeta(gelts []byte, geltlen int, mb []byte, mblen int, mdlen int) ([]byte, int) {
	num := len(gelts) / geltlen
	//Divide up messages by destination
	dists := make([][]byte, len(vz.servers))
	distMap := make(map[int][]int) //maps dist server to slice of original indices
	for i := 0; i < num; i++ {
		dest := vz.parseDestination(gelts[geltlen*i : geltlen*(i+1)])

		dists[dest] = append(dists[dest], mb[mblen*i:mblen*(i+1)]...)
		distMap[dest] = append(distMap[dest], i)
	}
	//Encrypt each distribution and remap to original index
	out := make([]byte, num*mdlen)
	for i := 0; i < len(vz.servers); i++ {
		emb, emblen, _, _ := vz.elgamal.Encrypt(dists[i], mblen, i)
		if mdlen != emblen {
			log.Fatal("metadata lengths don't match")
		}
		for j, batchI := range distMap[i] {
			copy(out[mdlen*batchI:mdlen*(batchI+1)], emb[mdlen*j:mdlen*(j+1)])
		}
	}
	return out, mdlen
}

func applyPermutation(msgs []byte, msglen int, perm []int) []byte {
	msgnum := len(msgs) / msglen
	out := make([]byte, len(perm)*msglen)
	if msgnum != len(perm) {
		log.WithFields(log.Fields{
			"msgbatchnum": msgnum,
			"permlen":     len(perm),
		}).Warn("Permutation and message batch have different sizes.")
		for i := 0; i < len(perm); i++ { // pad with dummy messages
			copy(out[i*msglen:(i+1)*msglen], msgs[:msglen])
		}
	}

	for i := 0; i < msgnum; i++ {
		if perm[i] > len(perm)-1 {
			log.WithFields(log.Fields{
				"msgi":    i,
				"permi":   perm[i],
				"permlen": len(perm),
			}).Fatal("Permutation maps to index out of range.")
		}
		copy(out[i*msglen:(i+1)*msglen], msgs[perm[i]*msglen:(perm[i]+1)*msglen])
	}
	return out
}

func reversePermutation(msgs []byte, msglen int, perm []int) []byte {
	msgnum := len(msgs) / msglen
	out := make([]byte, len(msgs))

	for i := 0; i < msgnum; i++ {
		copy(out[perm[i]*msglen:(perm[i]+1)*msglen], msgs[i*msglen:(i+1)*msglen])
	}
	return out
}

func encodeInt64(input []int64) ([]byte, int) {
	buf := new(bytes.Buffer)
	for _, v := range input {
		err := binary.Write(buf, binary.LittleEndian, v)
		if err != nil {
			log.WithFields(log.Fields{
				"err": err,
			}).Error("Error encoding secrets.")
		}
	}
	return buf.Bytes(), 8
}

func decodeInt64(input []byte, inputlen int) []int64 {
	msgnum := len(input) / inputlen
	out := make([]int64, msgnum)
	buf := bytes.NewReader(input)
	for i := 0; i < msgnum; i++ {
		err := binary.Read(buf, binary.LittleEndian, &out[i])
		if err != nil {
			log.WithFields(log.Fields{
				"err": err,
			}).Error("Error decoding secrets.")
		}
	}
	return out
}

func (vz *VZServer) createMetadataInputChain(num int) ([]byte, int) {
	out := make([]int64, num)
	for i, _ := range out {
		out[i] = int64(vz.serverId*1000000 + i)
	}
	return encodeInt64(out)
}

// Creates mailbox metadata
// Each server exchanges the first message with the server behind it and the second message with the server in front of it.
func (vz *VZServer) createMetadataMailbox(num int) ([]byte, int) {
	out := make([]int64, num)
	for i, _ := range out {
		if i == 0 {
			out[i] = int64(vz.serverId)
		} else if i == 1 {
			out[i] = int64(vz.nextid(vz.serverId))
		} else {
			out[i] = int64((1+vz.serverId)*1000000 + i)
		}
	}
	return encodeInt64(out)
}

const overhead = groth.CIPHERTEXT_SIZE // size of encrypted salsa key
const onion = true // wrap messages in onions?

type workfn func(msgIndices <-chan int, done chan<- int)

func parallel(f workfn, n int) {
	indices := make(chan int, runtime.NumCPU())
	done := make(chan int, runtime.NumCPU())
	for i := 0; i < runtime.NumCPU(); i++ {
		go f(indices, done)
	}

	for i := 0; i < n; i++ {
		indices <- i
	}
	close(indices)

	for i := 0; i < runtime.NumCPU(); i++ {
		<-done
	}
}

func (vz *VZServer) Wrap(messages []byte, msgsize int, publicKeyId int) (ciphertext []byte, length int) {
	if !onion {
		return messages, msgsize
	}

	n := len(messages) / msgsize // number of messages
	onionsize := msgsize + overhead
	wrapped := make([]byte, n * onionsize)

	seeds := make([]byte, 32 * n)
	// TODO can precompute these -- is it necessary?
	rn, err := rand.Reader.Read(seeds)
	if rn != 32*n || err != nil {
		panic("PRG failed to output!")
	}

	ekeys, elen, keys, klen := vz.elgamal.Encrypt(seeds, 32, publicKeyId)
	if elen != overhead {
		panic("bad size")
	}

	fn := func(msgIndices <-chan int, done chan<- int) {
		for i := range msgIndices {
			out := make([]byte, onionsize)
			nonce := make([]byte, 8) // messages relatively short
			cipherkey := ekeys[i*elen : i*elen + elen]

			hasher := sha256.New()
			hasher.Write(keys[i*klen : i*klen + klen])
			hash := hasher.Sum(nil)
			var key [32]byte
			copy(key[:], hash[:32]) // TODO last 32 bytes are ok right? could also place extra bits into nonce (at most 24)
			salsa20.XORKeyStream(out, messages[i*msgsize : i*msgsize + msgsize], nonce, &key)

			copy(out[msgsize:], cipherkey)
			copy(wrapped[i*onionsize:], out)
		}
		done <- 1
	}
	parallel(fn, n)

	return wrapped, onionsize
}

func (vz *VZServer) DestWrap(messages []byte, msgsize int, targets []byte, tgsize int, chainLen int, servers int) (ciphertext []byte, length int) {
	if !onion {
		return messages, msgsize
	}

	n := len(messages) / msgsize // number of messages
	msgsPerChain := make([]int, servers) // number of messages heading to each chain
	destinations := make([]int, n) // destination of each message
	onionsize := msgsize + overhead*chainLen
	wrapped := make([]byte, n * onionsize)
	for i := 0; i < n; i++ {
		destinations[i] = vz.parseDestination(targets[i*tgsize:])
		msgsPerChain[destinations[i]] += 1
	}

	/* naive way: run Wrap repeatedly */
	for i := 0; i < servers; i++ { // for each entry server...
		// ...determine the messages heading to that entry server...
		clen := msgsize
		cout := make([]byte, msgsPerChain[i] * msgsize)
		for j, k := 0, 0; j < n; j++ {
			if destinations[j] == i {
				copy(cout[k*msgsize:(k+1)*msgsize], messages[j*msgsize:(j+1)*msgsize])
				k += 1
			}
		}

		for j := chainLen - 1; j >= 0; j-- { // ...and for each server in that chain, wrap the messages in onions
			log.WithFields(log.Fields{"to-id": (i+j) % servers}).Info("run wrap")
			cout, clen = vz.Wrap(cout, clen, (i+j) % servers)
		}

		if clen != onionsize {
			panic("bad onion size")
		}

		for j, k := 0, 0; j < n; j++ {
			if destinations[j] == i {
				copy(wrapped[j*onionsize:(j+1)*onionsize], cout[k*onionsize:(k+1)*onionsize])
				k += 1
			}
		}
	}

	return wrapped, onionsize

	/* "non-naive" way
	// elgamal := groth.Groth{}
	//   for i := 0; i < n; i++ {
	//   	copy(wrapped[i*onionsize:(i+1)*onionsize], message[i*msgsize:(i+1)*msgsize])
	//   }
	// generate public-key encryptions of symmetric keys first
	keysPerDest := make([]int, chainLen) // number of messages heading to each destination (count once per server in chain)
	// symmetric keys and their encryptions
	// first dimension corresponds to the decrypting server; second dimension is for that server, which message uses that key
	keys := make([][]byte, chainLen)
	ekeys := make([][]byte, chainLen)
	for i := 0; i < n; i++ {
		seeds := make([]byte, 32 * n)
		rn, err := rand.Reader.Read(seeds)
		if rn != 32*n || err != nil {
			panic("PRG failed to output!")
		}

		ekeys, elen, keys, klen := elgamal.Encrypt(seeds, 32, i)
	}
*/
}

func (vz *VZServer) Unwrap(ciphertexts []byte, ctextsize int, privateKeyId int) (plaintext []byte, length int, skeys []byte) {
	if !onion {
		return ciphertexts, ctextsize, make([]byte, 0)
	}

	n := len(ciphertexts) / ctextsize // number of ciphertexts
	msgsize := ctextsize - overhead
	unwrapped := make([]byte, msgsize * n)
	ekeys := make([]byte, overhead * n)

	for i := 0; i < n; i++ {
		copy(ekeys[i*overhead : i*overhead + overhead], ciphertexts[i*ctextsize + msgsize:])
	}

	keys, klen := vz.elgamal.Decrypt(ekeys, overhead, privateKeyId)
	symmetric := make([]byte, 32 * n)

	fn := func(msgIndices <-chan int, done chan<- int) {
		for i := range msgIndices {
			out := make([]byte, msgsize)
			nonce := make([]byte, 8)
			content := make([]byte, msgsize)
			copy(content, ciphertexts[i*ctextsize:])

			hasher := sha256.New()
			hasher.Write(keys[i*klen : i*klen + klen])
			hash := hasher.Sum(nil)
			var key [32]byte
			copy(key[:], hash[:32])
			copy(symmetric[i*32:], hash[:32])

			salsa20.XORKeyStream(out, content, nonce, &key)
			copy(unwrapped[i*msgsize:], out)
		}
		done <- 1
	}
	parallel(fn, n)

	return unwrapped, msgsize, symmetric
}

// rewrapping on the return phase only uses the symmetric key and costs no space overhead
func Rewrap(messages []byte, msgsize int, symmetricKeys []byte) (ciphertext []byte, length int) {
	n := len(messages) / msgsize // number of messages
	wrapped := make([]byte, n * msgsize)
	for i := 0; i < n; i++ {
		out := make([]byte, msgsize)
		nonce := make([]byte, 8) // messages relatively short
		var key [32]byte
		copy(key[:], symmetricKeys[i*32:])
		salsa20.XORKeyStream(out, messages[i*msgsize : i*msgsize + msgsize], nonce, &key)
		copy(wrapped[i*msgsize:], out)
	}
	return wrapped, msgsize
}

// Creates dummy message content messages of given number and size.
// Message size is in bytes and is forced to be a multiple of 8
// The first two messages are set in order to debug. First message gets swapped with server behind and second message swapped with server in front.
func (vz *VZServer) createMsgContent(num int, msgsize int) ([]byte, int) {
	intsPerMsg := msgsize / 8
	out := make([]int64, num*intsPerMsg)
	for i, _ := range out {
		out[i] = int64(100000)
	}
	out[0] = int64(vz.serverId)
	out[intsPerMsg] = int64(vz.serverId)
	msgsAsBytes, intByteLen := encodeInt64(out)
	if intByteLen != 8 {
		log.WithFields(log.Fields{
			"intByteLen": intByteLen,
		}).Fatal("Byte encoding of int64 should be length 8.")
	}
	return msgsAsBytes, intsPerMsg * intByteLen
}

// Combines message content and mailbox metadata for input chain. Mailbox metadata will be parsed out and used as metadata for output chain.
func (vz *VZServer) combineTestMailboxContent(mb []byte, mblen int, c []byte, clen int, num int) ([]byte, int) {
	out := make([]byte, num*(mblen+clen))
	for i := 0; i < num; i++ {
		copy(out[(mblen+clen)*i:(mblen+clen)*i+mblen], mb[mblen*i:mblen*(i+1)])
		copy(out[(mblen+clen)*i+mblen:(mblen+clen)*(i+1)], c[clen*i:clen*(i+1)])
	}
	return out, mblen + clen
}
