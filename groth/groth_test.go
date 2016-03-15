package groth

import "fmt"
import "testing"
import "time"

var _ = fmt.Printf

func TestEncrypt(t *testing.T) {
	var g Groth

	secretSize := 5
	nSecrets := 1024
	secrets := make([]byte, nSecrets * secretSize)
	key := 0

	for i := 0; i < nSecrets * secretSize; i++ {
		secrets[i] = byte(i % 256)
	}
	
	ciphertexts, cipherSize, masks, maskSize := g.Encrypt(secrets, secretSize, key)
	masksX, maskSizeX := g.Decrypt(ciphertexts, cipherSize, key)

	if maskSize != maskSizeX {
		t.Errorf("%d != %d", maskSize, maskSizeX)
	}

	for i := 0; i < len(masks); i++ {
		if masks[i] != masksX[i] {
			t.Errorf("masks do not match (at index %d)", i)
			return;
		}
	}
}

func TestEncryptVerified(t *testing.T) {
	var g Groth

	start := time.Now()
	secretSize := 8
	nSecrets := 80384 * 10 // 10K
	secrets := make([]byte, nSecrets * secretSize)
	key := 0

	for i := 0; i < nSecrets * secretSize; i++ {
		secrets[i] = byte(i % 256)
	}

	ciphertexts, cipherSize, masks, maskSize, proof := g.EncryptProven(secrets, secretSize, key)
	fmt.Println("EncryptProven took", time.Since(start))
	masksX, maskSizeX := g.Decrypt(ciphertexts, cipherSize, key)

	if maskSize != maskSizeX {
		t.Errorf("%d != %d", maskSize, maskSizeX)
	}

	for i := 0; i < len(masks); i++ {
		if masks[i] != masksX[i] {
			t.Errorf("masks do not match (at index %d)", i)
			return;
		}
	}

	start2 := time.Now()
	verified := g.EncryptVerify(ciphertexts, proof)
	fmt.Println("EncryptVerify took", time.Since(start2))
	if !verified {
		t.Errorf("failed to verify proof of knowledge")
	}
}

func TestEndToEnd(t *testing.T) {
	start := time.Now()

	var secrets []byte
	secrets = make([]byte, 100000, 100000)
	var i int = 0
	for i < len(secrets) {
		secrets[i] = byte(i)
		i += 1
	}

	g := Groth{}
	ciphers, ciphers_len, groupelts, element_len := g.Encrypt(secrets, 5, 1)
	fmt.Println("Going to shuffle", len(groupelts)/element_len, "elements")
	dec_groupelts, dec_element_len := g.Decrypt(ciphers, ciphers_len, 1)

	if dec_element_len != element_len {
		fmt.Println("Error: lengths mismatch ", dec_element_len, element_len)
	}

	fmt.Println("starting shuffle.")
	shuffleStart := time.Now()
	ciphers_outStr, _, handle := g.Shuffle(ciphers, ciphers_len, 1)
	shuffleTime := time.Since(shuffleStart)
	fmt.Println("Shuffle time: ", shuffleTime)

	proof, pub_randoms := g.Prove(handle)
	proofTime := time.Since(shuffleStart)
	fmt.Println("Proof time: ", proofTime)

	ret := g.Verify(proof, ciphers, ciphers_outStr, 1, pub_randoms)

	var j int = 0
	for j < len(groupelts) {
		if groupelts[j] != dec_groupelts[j] {
			fmt.Println("Error:")
			fmt.Println("---", groupelts[j])
			fmt.Println("---", dec_groupelts[j])
		}
		j += 1
	}
	elapsed := time.Since(start)
	fmt.Println("Test complete:", ret, ". Total time: ", elapsed)
}
