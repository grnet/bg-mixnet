package stadium

import (
	"fmt"
	"time"
)

//Struct for keeping track of timing
type ExperimentTime struct {
	ChainLen  int
	Start     time.Time
	BackStart time.Time

	Total   time.Duration
	Noise   time.Duration
	Forward time.Duration
	Back    time.Duration

	/* Times for mixing phase rotation rounds
	first layer - mixing phase (0-3)
	second layer - rotation round (0 to ChainLen-1) */
	MixingRRTimes [][]time.Duration
	MixingStart   [4]time.Time
	MixingTimes   [4]time.Duration

	// layer 1: mixing phase (0-3), but only 0 and 1 are filled
	// layer 2: rotation round
	// ends are practically synonymous with forwarding time
	ShuffleStarts [][]time.Time
	ShuffleEnds   [][]time.Time

	// layer 1: mixing phase (0-3), but only 0 and 1 are filled
	// layer 2: rotation round
	// ends are practically synonymous with broadcast time
	ProofStarts [][]time.Time
	ProofEnds   [][]time.Time

	// layer 1: mixing phase (0-3), but only 0 and 1 are filled
	// layer 2: rotation round
	// onion overhead
	UnwrapStarts [][]time.Time
	UnwrapEnds   [][]time.Time

	// layer 1: mixing phase (0-3), but only 0 and 1 are filled
	// layer 2: rotation round
	// layer 3: source of proof
	// max of layer 3 determines when distribution starts
	VerifyStarts [][][]time.Time
	VerifyEnds   [][][]time.Time

	// layer 1: mixing phase (0-3), but only 0 and 1 are filled
	// time it takes to decrypt inputs/mailbox
	DecryptStarts [2]time.Time
	DecryptEnds   [2]time.Time

	// at the beginning of first mixing phase rotation round, how long it took to verify NIZK proof
	EncryptVerifyStarts []time.Time
	EncryptVerifyEnds   []time.Time

	/* Times for distribution phase
	first layer - distribution phase (0-4) */
	DistTimes [5]time.Duration

	// TODO rename class
	/* Bandwidth costs */
	TotalBandwidth    int
	ForwardBandwidth  int
	BackwardBandwidth int
	OnionBandwidth    int
	MetadataBandwidth int
	ProofBandwidth    int
}

func (et *ExperimentTime) init(ChainLen int) {
	et.ChainLen = ChainLen
	// Initialize slices
	et.MixingRRTimes = make([][]time.Duration, 4)
	for i := range et.MixingRRTimes {
		et.MixingRRTimes[i] = make([]time.Duration, ChainLen)
	}

	et.ShuffleStarts = make([][]time.Time, 2)
	et.ShuffleEnds = make([][]time.Time, 2)
	et.ProofStarts = make([][]time.Time, 2)
	et.ProofEnds = make([][]time.Time, 2)
	et.UnwrapStarts = make([][]time.Time, 2)
	et.UnwrapEnds = make([][]time.Time, 2)
	et.VerifyStarts = make([][][]time.Time, 2)
	et.VerifyEnds = make([][][]time.Time, 2)
	for i := 0; i < 2; i++ {
		et.ShuffleStarts[i] = make([]time.Time, ChainLen)
		et.ShuffleEnds[i] = make([]time.Time, ChainLen)
		et.ProofStarts[i] = make([]time.Time, ChainLen)
		et.ProofEnds[i] = make([]time.Time, ChainLen)
		et.UnwrapStarts[i] = make([]time.Time, ChainLen)
		et.UnwrapEnds[i] = make([]time.Time, ChainLen)
		et.VerifyStarts[i] = make([][]time.Time, ChainLen)
		et.VerifyEnds[i] = make([][]time.Time, ChainLen)
		for j := 0; j < ChainLen; j++ {
			et.VerifyStarts[i][j] = make([]time.Time, ChainLen)
			et.VerifyEnds[i][j] = make([]time.Time, ChainLen)
		}
	}

	et.EncryptVerifyStarts = make([]time.Time, ChainLen)
	et.EncryptVerifyEnds = make([]time.Time, ChainLen)
}

func (et *ExperimentTime) trackRound(t time.Time) {
	et.Start = t
}

func (et *ExperimentTime) trackMsgCreation(t time.Time) {
	et.Noise = time.Since(t)
}

func (et *ExperimentTime) trackRR(rr int, mp int, t time.Time) {
	if mp == mixing1 || mp == mixing2 {
		if rr == 0 {
			et.MixingStart[mp] = t
		} else if rr == et.ChainLen-1 {
			et.MixingTimes[mp] = time.Since(et.MixingStart[mp])
		}
	} else if mp == mixingreturn1 || mp == mixingreturn2 {
		if rr == et.ChainLen-1 {
			et.MixingStart[mp] = t
		} else if rr == 0 {
			et.MixingTimes[mp] = time.Since(et.MixingStart[mp])
		}
	}

	et.MixingRRTimes[mp][rr] = time.Since(t)

	if mp == mixingreturn2 && rr == 0 {
		et.Total = time.Since(et.Start)
		et.Back = time.Since(et.BackStart)
		et.PrintSummary()
	}
}

func (et *ExperimentTime) trackDist(dp int, t time.Time) {
	et.DistTimes[dp] = time.Since(t)

	if dp == mailboxdist {
		et.Forward = time.Since(et.Start)
	} else if dp == mailboxdistreturn {
		et.BackStart = t
	}
}

func (et *ExperimentTime) PrintSummary() {
	fmt.Printf("STADIUM ROUND SUMMARY\n")
	fmt.Printf("---------------------\n")
	fmt.Printf("Total time: %f\n", et.Total.Seconds())
	fmt.Printf("Noise generation time: %f\n", et.Noise.Seconds())
	fmt.Printf("Forward time: %f\n", et.Forward.Seconds())
	fmt.Printf("\tMixing 1 time: %f\n", et.MixingTimes[mixing1].Seconds())
	for i, dur := range et.MixingRRTimes[mixing1] {
		fmt.Printf("\t\trotation round %d time: %f\n", i, dur.Seconds())
	}
	fmt.Printf("\tMessage Distribution time: %f\n", et.DistTimes[msgdist].Seconds())
	fmt.Printf("\tMixing 2 time: %f\n", et.MixingTimes[mixing2].Seconds())
	for i, dur := range et.MixingRRTimes[mixing2] {
		fmt.Printf("\t\trotation round %d time: %f\n", i, dur.Seconds())
	}
	fmt.Printf("\tMailbox Distribution time: %f\n", et.DistTimes[mailboxdist].Seconds())
	fmt.Printf("Return time: %f\n", et.Back.Seconds())
	fmt.Printf("\tMailbox Return time: %f\n", et.DistTimes[mailboxdistreturn].Seconds())
	fmt.Printf("\tMixing 2 Return time: %f\n", et.MixingTimes[mixingreturn1].Seconds())
	for i, dur := range et.MixingRRTimes[mixingreturn1] {
		fmt.Printf("\t\trotation round %d time: %f\n", i, dur.Seconds())
	}
	fmt.Printf("\tDistribution Return time: %f\n", et.DistTimes[msgdistreturn].Seconds())
	fmt.Printf("\tMixing 1 Return time: %f\n", et.MixingTimes[mixingreturn2].Seconds())
	for i, dur := range et.MixingRRTimes[mixingreturn2] {
		fmt.Printf("\t\trotation round %d time: %f\n", i, dur.Seconds())
	}
}
