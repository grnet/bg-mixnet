package stadium

import (
	"bytes"
	"math/rand"
	"testing"

	log "github.com/Sirupsen/logrus"
)

func TestEncodeDecodeInt64(t *testing.T) {
	nums := []int64{0, 512, 2}
	nums_encoded := []byte{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	}

	coded, codelen := encodeInt64(nums)
	log.WithFields(log.Fields{
		"code": coded,
		"len":  codelen,
	}).Info("Coded values.")

	if !bytes.Equal(nums_encoded, coded) {
		t.Error("Encoded values do not match", "expected", nums_encoded, "got", coded)
	}

	decoded := decodeInt64(coded, codelen)

	for i, _ := range nums {
		if nums[i] != decoded[i] {
			t.Error("At index", i, "expected", nums[i], "got", decoded[i])
		}
	}
}

const nKeys = 10

func TestOnion(t *testing.T) {
	mlen := 136
	s := 1366528
	m := make([]byte, s)
	rand.Read(m)

	for i := 0; i < nKeys; i++ {
		log.WithFields(log.Fields{"layers": i, "perm": false}).Info("testing layers")
		nOnions(t, m, mlen, i, false)
	}

	for i := 0; i < nKeys; i++ {
		log.WithFields(log.Fields{"layers": i, "perm": true}).Info("testing layers")
		nOnions(t, m, mlen, i, true)
	}
}

func nOnions(t *testing.T, msg []byte, msglen int, n int, perm bool) {
	permsize := len(msg) / msglen
	perms := make([][]int, n)
	for i := 0; i < n; i++ {
		if perm {
			perms[i] = rand.Perm(permsize)
		} else {
			perms[i] = make([]int, permsize)
			for j := 0; j < permsize; j++ {
				perms[i][j] = j
			}
		}
	}

	ctext := msg
	ctextlen := msglen
	for i := 0; i < n; i++ {
		ctext, ctextlen = Wrap(ctext, ctextlen, i % nKeys)
	}

	for i := n-1; i >= 0; i-- {
		ctext = applyPermutation(ctext, ctextlen, perms[i])
		ctext, ctextlen, _ = Unwrap(ctext, ctextlen, i % nKeys)
	}

	for i := 0; i < n; i++ {
		ctext = reversePermutation(ctext, ctextlen, perms[i])
	}

	if ctextlen != msglen {
		log.WithFields(log.Fields{"layers": n, "perm": perm}).Fatal("lengths do not match")
		t.Fail()
	}
	for i, _ := range ctext {
		if ctext[i] != msg[i] {
			log.WithFields(log.Fields{"layers": n, "index": i, "perm": perm}).Fatal("indexes do not match")
			t.Fail()
		}
	}
}
