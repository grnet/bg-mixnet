package stadium

import "crypto/sha256"

// Dummy shuffle implementing verifiableShuffler interface for testing purposes

type dummyshuffler struct {
}

func (d dummyshuffler) Encrypt(secrets []byte, secretlen int, keyIndex int) (ciphers []byte, cipherlen int, groupelts []byte, groupeltlen int) {
	return secrets, secretlen, secrets, secretlen
}

//Encrypts byte array of messages with NIZK proof of plaintext
func (d dummyshuffler) EncryptProven(secrets []byte, secretlen int, keyIndex int) (ciphers []byte, cipherlen int, groupelts []byte, groupeltlen int, proof []byte) {
	return secrets, secretlen, secrets, secretlen, make([]byte, 0)
}

func (d dummyshuffler) EncryptVerify(ciphers []byte, proof []byte) bool {
	return true
}

func (d dummyshuffler) Decrypt(ciphers []byte, cipherlen int, keyIndex int) (groupelts []byte, groupeltlen int) {
	return ciphers, cipherlen
}

func (d dummyshuffler) Shuffle(ciphers []byte, cipherlen int, keyIndex int) (ciphersout []byte, permutation []int, shuffleHandle int) {
	msgnum := len(ciphers) / cipherlen
	perm := make([]int, msgnum)
	for i, _ := range perm {
		perm[i] = i
	}
	return ciphers, perm, 0
}

func (d dummyshuffler) Prove(shuffleHandle int) (proof []byte, public_randoms string) {
	return make([]byte, 0), ""
}

func (d dummyshuffler) Verify(proof []byte, ciphersin []byte, ciphersout []byte, keyIndex int, public_randoms string) bool {
	return true
}

type dummyhasher struct {
}

func (d dummyhasher) Hash(input []byte) []byte {
	hasher := sha256.New()
	hasher.Write(input)
	return hasher.Sum(nil)
}
