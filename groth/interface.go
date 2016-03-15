package groth

/*
#cgo LDFLAGS: -O2 -flto -L. -lntl -lgmp -lboost_system  -lboost_filesystem -lpthread -lboost_regex -lboost_thread -lboost_context -lgomp -lshuffle

#include "Utils.h"
*/
import "C"

import (
	"runtime"
	"sync"
	"unsafe"

	log "github.com/Sirupsen/logrus"
)

// TODO this should be the actual return type for Shuffle()
// type handle int;

const CIPHERTEXT_SIZE = C.CURVE_POINT_BYTESIZE * 2

type Groth struct {
}

// synchronized record of shuffle handles (needed for correct C++ operation)
type shuffleRecords struct {
	mu sync.Mutex
	pointers map[int]unsafe.Pointer
	next int // next valid handle
}

func (c *shuffleRecords) insertRecord(p unsafe.Pointer) int {
	c.mu.Lock()
	defer c.mu.Unlock()

	assigned := c.next
	c.next += 1

	log.WithFields(log.Fields{"record": assigned}).Info("inserted record")
	c.pointers[assigned] = p
	return assigned
}

func (c *shuffleRecords) invalidateRecord(h int) (unsafe.Pointer, bool) {
	c.mu.Lock()
	defer c.mu.Unlock()

	p, ok := c.pointers[h]
	log.WithFields(log.Fields{"record": h, "success": ok}).Info("invalidated record")
	if ok {
		delete(c.pointers, h)
	}
	return p, ok
}

var records *shuffleRecords = &shuffleRecords{pointers: make(map[int]unsafe.Pointer)}

func memstat(s string) {
	//   var m runtime.MemStats
	//   runtime.ReadMemStats(&m)
	//   log.WithFields(log.Fields{
	//   	"when": s,
	//   	"live": m.Mallocs - m.Frees,
	//   	"malloc": m.Mallocs,
	//   	"free": m.Frees,
	//   }).Info("memory")
}

func (g Groth) EncryptProven(secrets []byte, secretlen int, keyIndex int) (ciphers []byte, cipherlen int, groupelts []byte, groupeltlen int, proof []byte) {
	runtime.GC()
	ciphers_, groupelts_, elt_size, proof := __encrypt_proven(secrets, secretlen, keyIndex)
	ciphers_bytes := []byte(ciphers_)
	groupelts_bytes, elt_len := string_arr_to_byte_arr(groupelts_)
	// log.WithFields(log.Fields{"len(ciphers)": len(ciphers_bytes), "len(proof)": len(proof)}).Info("C:EncryptProven")
	return ciphers_bytes, elt_size, groupelts_bytes, elt_len, proof
}

//Encrypts byte array of messages with NIZK proof
func __encrypt_proven(secrets []byte, secretlen int, keyIndex int) (ciphers string, groupelts []string, elm_size int, proof []byte) {
	var num_secrets int
	num_secrets = len(secrets) / secretlen

	cargs := C.makeCharArray(C.int(num_secrets))
	defer C.freeCharArray(cargs, C.int(num_secrets))
	var src_index int = 0
	var i int = 0
	for i < num_secrets {
		C.setArrayString(cargs, (*C.char)(unsafe.Pointer(&secrets[0])), C.int(i), C.int(src_index), C.int(secretlen))
		src_index = src_index + secretlen
		i += 1
	}

	ptr := (*unsafe.Pointer)(unsafe.Pointer(cargs))
	cCipherProof := C.encrypt_with_proof(ptr, C.int(secretlen), C.int(num_secrets), C.int(keyIndex))
	cCipher := C.encrypt_cipher_part(cCipherProof)

	var cLenProof C.int
	cProof := C.encrypt_proof_part(cCipherProof, &cLenProof)
	proof = C.GoBytes(cProof, cLenProof)

	var cLen C.int
	var element_size C.int
	cCipherStr := C.get_ciphertexts(cCipher, unsafe.Pointer(&cLen), unsafe.Pointer(&element_size))
	cipherStr := C.GoStringN((*C.char)(cCipherStr), cLen)
	C.delete_str(cCipherStr)

	num_elements_post_enc := int(cLen) / int(element_size)
	var elements []string
	elements = make([]string, num_elements_post_enc, num_elements_post_enc)
	var j int = 0
	for j < num_elements_post_enc {
		cElem := C.get_element(cCipher, C.int(j), unsafe.Pointer(&cLen))
		elements[j] = C.GoStringN((*C.char)(cElem), cLen)
		j += 1
	}

	C.delete_ciphers_with_proof(cCipherProof)

	return cipherStr, elements, int(element_size), proof
}

func (g Groth) EncryptVerify(ciphers []byte, proof []byte) bool {
	runtime.GC()
	cRet := C.verify_encrypt(unsafe.Pointer(&ciphers[0]), C.int(len(ciphers)), unsafe.Pointer(&proof[0]), C.int(len(proof)))
	// log.WithFields(log.Fields{"len(ciphers)": len(ciphers), "len(proof)": len(proof)}).Info("C:EncryptVerify")
	return (int(cRet) == 1)
}

func (g Groth) Encrypt(secrets []byte, secretlen int, keyIndex int) (ciphers []byte, cipherlen int, groupelts []byte, groupeltlen int) {
	runtime.GC()
	ciphers_, groupelts_, elt_size := __encrypt(secrets, secretlen, keyIndex)
	ciphers_bytes := []byte(ciphers_)
	groupelts_bytes, elt_len := string_arr_to_byte_arr(groupelts_)
	// log.WithFields(log.Fields{"len(ciphers)": len(ciphers_bytes)}).Info("C:Encrypt")
	return ciphers_bytes, elt_size, groupelts_bytes, elt_len
}

//Encrypts byte array of messages
func __encrypt(secrets []byte, secretlen int, keyIndex int) (ciphers string, groupelts []string, elm_size int) {
	var num_secrets int
	num_secrets = len(secrets) / secretlen

	cargs := C.makeCharArray(C.int(num_secrets))
	defer C.freeCharArray(cargs, C.int(num_secrets))
	var src_index int = 0
	var i int = 0
	for i < num_secrets {
		C.setArrayString(cargs, (*C.char)(unsafe.Pointer(&secrets[0])), C.int(i), C.int(src_index), C.int(secretlen))
		src_index = src_index + secretlen
		i += 1
	}

	ptr := (*unsafe.Pointer)(unsafe.Pointer(cargs))
	cCipher := C.encrypt(ptr, C.int(secretlen), C.int(num_secrets), C.int(keyIndex))

	var cLen C.int
	var element_size C.int
	cCipherStr := C.get_ciphertexts(cCipher, unsafe.Pointer(&cLen), unsafe.Pointer(&element_size))
	cipherStr := C.GoStringN((*C.char)(cCipherStr), cLen)
	C.delete_str(cCipherStr)

	num_elements_post_enc := int(cLen) / int(element_size)
	var elements []string
	elements = make([]string, num_elements_post_enc, num_elements_post_enc)
	var j int = 0
	for j < num_elements_post_enc {
		cElem := C.get_element(cCipher, C.int(j), unsafe.Pointer(&cLen))
		elements[j] = C.GoStringN((*C.char)(cElem), cLen)
		j += 1
	}

	C.delete_ciphers(cCipher)

	return cipherStr, elements, int(element_size)
}

func parse_ciphers(in_cipherStr string, elgammal unsafe.Pointer) (ciphers []string) {
	cCiphers := C.parse_ciphers(unsafe.Pointer(C.CString(in_cipherStr)), C.int(len(in_cipherStr)), elgammal)
	rows := int(C.rows(cCiphers))
	cols := int(C.cols(cCiphers))
	num_ciphers := rows * cols

	var ciphertexts []string
	ciphertexts = make([]string, num_ciphers, num_ciphers)

	var i int = 0
	for i < rows {
		var j int = 0
		for j < cols {
			var cLen C.int
			cCiphertext := C.get_cipher(cCiphers, C.int(i), C.int(j), unsafe.Pointer(&cLen))
			ciphertexts[i*cols+j] = C.GoStringN((*C.char)(cCiphertext), cLen)
			C.delete_str(cCiphertext)
			j += 1
		}
		i += 1
	}
	C.delete_ciphers(cCiphers)

	return ciphertexts

}

func get_longest(arr []string) int {
	lgth := 0
	var i int = 0
	for i < len(arr) {
		x := []byte(arr[i])
		if len(x) > lgth {
			lgth = len(x)
		}

		i += 1
	}
	return lgth
}

func pad_to_length(x string, padlen int) (padded []byte) {
	input := []byte(x)
	padded = make([]byte, padlen, padlen)
	diff := padlen - len(input)

	var i int = 0
	for i < padlen {
		if i < diff {
			padded[i] = 0
		} else {
			padded[i] = input[i-diff]
		}
		i += 1
	}
	return padded
}

func string_arr_to_byte_arr(groupelts_arr []string) (ret []byte, element_size int) {
	var ret_groupelts []byte
	pad_length := get_longest(groupelts_arr)
	ret_groupelts = make([]byte, pad_length*len(groupelts_arr))
	var i int = 0
	var j int = 0
	for i < len(groupelts_arr) {
		tmp := pad_to_length(groupelts_arr[i], pad_length)
		var k int = 0
		for k < pad_length {
			ret_groupelts[j] = tmp[k]
			j += 1
			k += 1
		}
		i += 1
	}
	return ret_groupelts, pad_length
}

func (g Groth) Decrypt(ciphers []byte, cipherlen int, keyIndex int) (groupelts []byte, groupeltlen int) {
	runtime.GC()
	ciphers_ := string(ciphers[:len(ciphers)])
	groupelts_arr := __decrypt(ciphers_, keyIndex)
	ret, retlen := string_arr_to_byte_arr(groupelts_arr)
	return ret, retlen
}

//Decrypts byte array of messages
func __decrypt(all_ciphers_texts string, keyID int) (groupelts []string) {

	elgammal := C.create_decryption_key(C.int(keyID))
	defer C.delete_key(elgammal)

	cCiphers := C.parse_ciphers(unsafe.Pointer(C.CString(all_ciphers_texts)), C.int(len(all_ciphers_texts)), elgammal)
	rows := int(C.rows(cCiphers))
	cols := int(C.cols(cCiphers))

	num_ciphers := rows * cols

	var plaintexts []string
	plaintexts = make([]string, num_ciphers, num_ciphers)

	done := make(chan bool)
	var i int = 0
	for i < rows {
		go func(i int, done chan bool) {
			var j int = 0
			for j < cols {
				var cLen C.int
				cPlaintext := C.decrypt_cipher(cCiphers, C.int(i), C.int(j), unsafe.Pointer(&cLen), elgammal)
				plaintexts[i*cols+j] = C.GoStringN((*C.char)(cPlaintext), cLen)
				C.delete_str(cPlaintext)
				j += 1
			}
			done <- true
		}(i, done)
		i += 1
	}
	count := 0
	for range done { //wait for all loops to complete
		count++
		if count == rows {
			break
		}
	}

	C.delete_ciphers(cCiphers)
	return plaintexts
}

func (g Groth) Shuffle(ciphers []byte, cipherlen int, keyIndex int) (ciphersout []byte, permutation []int, shuffle int) {
	runtime.GC()
	in_cipherStr := string(ciphers[:len(ciphers)])
	number_of_elements := int(len(ciphers) / cipherlen)
	memstat("shuffle begin");
	ciphersout_str, perm, pointer := __shuffle(in_cipherStr, number_of_elements, keyIndex)
	// runtime.GC()
	memstat("shuffle end");
	handle := records.insertRecord(pointer)
	return []byte(ciphersout_str), perm, handle
}

func (g Groth) Prove(h int) (proof []byte, public_randoms string) {
	runtime.GC()

	memstat("prove begin");

	pointer, ok := records.invalidateRecord(h)
	if !ok {
		panic("invalid handle passed to Prove")
	}
	proof_ret, out_public_randoms := __prove(pointer)
	// log.WithFields(log.Fields{"len(ciphers)": len(ciphers)}).Info("C:Shuffle")

	// runtime.GC()
	memstat("prove end")
	
	return []byte(proof_ret), out_public_randoms
}

//Shuffles
func __shuffle(in_cipherStr string, number_of_elements int, keyIndex int) (ciphersout_str string, permutation []int, record unsafe.Pointer) {
	elgammal := C.create_pub_key(C.int(keyIndex))
	defer C.delete_key(elgammal)

	var shuffled_ciphers unsafe.Pointer
	var shuffled_ciphers_len C.int
	var perm unsafe.Pointer
	var perm_len C.int

	pointer := C.shuffle_internal(elgammal,
		C.CString(in_cipherStr), C.int(len(in_cipherStr)), C.int(number_of_elements),
		(**C.char)(unsafe.Pointer(&shuffled_ciphers)), (*C.int)(unsafe.Pointer(&shuffled_ciphers_len)),
		(**C.int)(unsafe.Pointer(&perm)), (*C.int)(unsafe.Pointer(&perm_len)))

	ret_ciphers := C.GoStringN((*C.char)(shuffled_ciphers), shuffled_ciphers_len)

	go_perm_length := int(perm_len)
	var ret_permutation []int
	ret_permutation = make([]int, go_perm_length, go_perm_length)
	var i int = 0
	for i < go_perm_length {
		ret_permutation[i] = int(C.get_int_elem((*C.int)(perm), C.int(i)))
		i += 1
	}

	C.delete_str(shuffled_ciphers)
	C.delete_int_arr((*C.int)(perm))

	return ret_ciphers, ret_permutation, pointer
}

//Produces proof
func __prove(p unsafe.Pointer) (proof string, out_public_randoms string) {
	var proof_out unsafe.Pointer
	var proof_len C.int
	var public_randoms unsafe.Pointer
	var public_randoms_len C.int

	C.prove(p,
		(**C.char)(unsafe.Pointer(&proof_out)), (*C.int)(unsafe.Pointer(&proof_len)),
		(**C.char)(unsafe.Pointer(&public_randoms)), (*C.int)(unsafe.Pointer(&public_randoms_len)))

	ret_proof := C.GoStringN((*C.char)(proof_out), proof_len)
	ret_public_randoms := C.GoStringN((*C.char)(public_randoms), public_randoms_len)

	C.delete_str(proof_out)

	return ret_proof, ret_public_randoms
}

func (g Groth) Verify(proof []byte, ciphersin []byte, ciphersout []byte, keyIndex int, public_randoms string) bool {
	runtime.GC()
	memstat("verify begin")
	result := __verify(keyIndex, string(proof), string(ciphersin), string(ciphersout), public_randoms)
	// runtime.GC()
	memstat("verify end")
	
	return result
}

//Verifies proof
//int verify(int key_index, char* proof, int proof_len, char* ciphers_in, int len, char* post_shuffle_cipehrs, int post_shuffle_cipehrs_len, char* public_randoms, int public_randoms_len);
func __verify(key_index int, proof string, ciphers_in string, post_shuffle_cipehrs string, public_randoms string) bool {
	ret := C.verify(C.int(key_index), C.CString(proof), C.int(len(proof)), C.CString(ciphers_in), C.int(len(ciphers_in)), C.CString(post_shuffle_cipehrs), C.int(len(post_shuffle_cipehrs)), C.CString(public_randoms), C.int(len(public_randoms)))
	if int(ret) != 0 {
		return true
	}
	return false
}
