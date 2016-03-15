import math
import numpy as np
import math
from multiprocessing import Pool
import sys
import time
from scipy.stats import poisson
import os
import os.path
import operator as op

kFaultProb = 0.20

# length of chain (does nothing is no corruption is allowed)
kChainLen = 8

# do we care about tolerating faults
kFaultTolorance = False

# whether corruption of chains is considered
kNoChainCorruption = True
#kMaxFaultyChains = max(int(0.1 * num_servers), 10)

def max_faulty_chains(num_servers):
    """
    The max number of malicious input/output chains we consider.
    """
    if kNoChainCorruption:
        return 0
    return min(min(max(int(0.1 * num_servers), 5), 50),num_servers)

def ncr(n, r):
    """
    n choose r
    """
    r = min(r, n-r)
    if r == 0: return 1
    numer = reduce(op.mul, xrange(n, n-r, -1))
    denom = reduce(op.mul, xrange(1, r+1))
    return numer//denom

def build_prob_matrix(num_servers):
    """
    Returns a max_faults X max_faults matrix describing the probability for a combination of malicious input/output number of chains.
    If we don't consider faults, this always returns a matrix with one element (1).
    """
    ProbMatrix = []
    max_faults = max_faulty_chains(num_servers)
    if (max_faults == 0):
        return [[1.0]]
    for i in xrange(min(num_servers, max_faults) + 1):
        ProbMatrix.append([])
        for j in xrange(min(num_servers, max_faults) + 1):
            ProbMatrix[i].append(faulty_chain_prob(num_servers, i, j))
    s = 0
    for i in xrange(len(ProbMatrix)):
        for j in xrange(len(ProbMatrix[i])):
            s += ProbMatrix[i][j]
    if s < 0.9999:
        print "sum of all probabilities:", s, len(ProbMatrix)
    return ProbMatrix

def compute_with_probability(e_in, d_in, e_out, d_out, e_perfect, d_perfect, corruption_in, corruption_out):
    """
    Combines privacy gain from input and output chain, given the probabilities that some of them are corrupt.
    """
    prob_compose = []
    # case 1: both chains function well
    prob_compose.append(((1 - corruption_in) * (1 - corruption_out), e_perfect, d_perfect))

    # case 2: Input chain is OK, output chain compromised
    prob_compose.append((corruption_in * (1 - corruption_out), e_in, d_in))


    # case 3: Input chain is compromised, output chain is OK
    prob_compose.append(((1 - corruption_in) * corruption_out, e_out, d_out))

    # case 4: Input and output chains are compromised
    prob_compose.append((corruption_in * corruption_out, 0, 1))

    # combine all cases! (uses Thm. 2 from paper)
    eps, delta = combine_with_prob(prob_compose)
    return eps, delta

def corrupt_chain_prob():
    """
    probability that a single chain is considered corrupt.
    """
    if (kFaultTolorance):
        return kFaultProb**kChainLen + kChainLen * kFaultProb**(kChainLen -1) *(1-kFaultProb)
    else:
        return kFaultProb**kChainLen

def faulty_chain_prob(num_servers, inp, out):
    """
    Probability to get exactly a particular number of input/output chain corruptions.
    """
    faulty_in_chain = corrupt_chain_prob()
    input_match = ncr(num_servers, inp) * faulty_in_chain**inp * (1 - faulty_in_chain) **(num_servers - inp)
    output_match = ncr(num_servers, out) * faulty_in_chain**out * (1 - faulty_in_chain) **(num_servers - out)
    return input_match * output_match

def combine_with_prob(prob_list):
    """
    Takes a list of tuples (prob, delta, epsilon) and combines them using Thm. 2 from paper.
    """
    acc_delta = 0
    acc_eps = 0

    c = 0
    for i in xrange(len(prob_list)):
        p, eps, delta = prob_list[i]
        c += p
        acc_eps += p * math.e**eps
        acc_delta += p * delta

    if (c != 1):
        print "error! sum is wrong."
    return math.log(acc_eps), acc_delta

# Single round differential privacy calculations
def poisson_dp(mean, c):
    """
    Poisson differential privacy, according to Thm. 1 from the paper.
    mean is the lambda for the distribution, c indicates the "cutoff" location
    """
    sigma = mean**0.5
    if mean == 0:
        return (1,1)
    epsilon = math.log(1. + (c*sigma + 1.) / mean)

    x = math.floor(mean-c*sigma)
    y = math.floor(mean+c*sigma)
    if x > 0:
        delta = poisson.pmf(x, mean) - poisson.pmf(0, mean) + poisson.pmf(y, mean)
    else:
        delta = poisson.pmf(y, mean)
    return (epsilon, delta)

# Multiple round differential privacy calculations
# Use d = 0.3 * 1/sqrt(n/m)
def extend_rounds(epsilon, delta, k, global_delta_goal):
    #d = global_delta_goal - k * delta
    #delta = k*delta + d
    #epsilon = epsilon * math.sqrt(2*k*math.log(1/float(d))) + k*epsilon*(math.exp(epsilon)-1)
    
    ### new composition thm
    d = 1 + (global_delta_goal -1)/(1-delta)**k    
    delta = 1 - ((1-delta)**k) * (1-d)
    epsilon_tmp = min(k * epsilon, k * epsilon**2 + epsilon*math.sqrt(2*k* math.log(math.e + math.sqrt(k * epsilon**2)/d) ), k * epsilon**2+epsilon*math.sqrt(2*k*math.log(1/d)))
    epsilon = epsilon_tmp
    
    #print "extending rounds: d=", d, "epsilon=",epsilon, "factor=",math.sqrt(2*k*math.log(1/float(d)))
    return (math.exp(epsilon), delta)

def sample_poisson_dp_final(mean, c, p):
    """
    computes poisson epsilon, delta, adjust the belief probs.
    """
    epsilon, delta = poisson_dp(mean, c)
    return combine_with_prob([(p, epsilon, delta), (1-p, 0,0)])

def find_configs():
    """
    Excecute one process per server configuration we care about. Designed to run on 15 cores.
    """
    p = Pool(15)
    p.map(handle_servers, [25, 50, 75, 100])

def handle_servers(servers):
    csv_file = "servers_" + str(servers) + ".csv"
    #sys.stdout = file("process" + str(servers) + ".txt", "w")
    #sys.stderr = sys.stdout
    f = open(csv_file, 'w')
    f.write("# messages" + "," + "rounds" + "," + "e^eps" + "," + "delta" + "," + "belief,mean single noise,mean double noise\n")
    f.close()

    log_file = "servers_" + str(servers) + ".log"
    if os.path.isfile(log_file):
        os.unlink(log_file)

    # first build an nXn matrix describing the probability that different numbers of input/output chain fail
    # Considering our new deployment story, I've set it such that no chain fails (i.e., the probability at prob_matrix[0][0] = 1)
    prob_matrix = build_prob_matrix(servers)
    print "built probability matrix:", len(prob_matrix)
    # The script finds the best epsilon for a given delta that is our goal. It provides results for different deltas.
    for goal_global_delta in [10**-4]:#, 5 * 10**-4, 10**-4, 5 * 10**-5, 10**-5]:
        # try different number of noise messages (from 0 to 150K)
        for num_messages in range(200,0,-1):
            # try different number of rounds
            for rounds in [10**4]:
                print "using", servers, "servers and", 1000*num_messages, "messages per server"
                # compute the best config for given number of noise messages
                epsilon = 10000000
                delta = 0
                for c in range(20):
                    ret = None
                    try:
                        ret = compute_new(servers, num_messages * 1000, rounds, 4 + 0.1 * c, goal_global_delta, 0.75, 4, 4.5)
                            #best_for_noise(servers, num_messages * 1000, rounds, prob_matrix, goal_global_delta, 1)
                        if epsilon > ret[0]:
                            epsilon, delta = ret
                            print epsilon, 4 + 0.1 * c
                    except:
                        pass
                if epsilon > 100000:
                    print "no configuration found for: ", servers, "servers. messages = ", num_messages * 1000,"."
                    break
                epsilon_exp = math.e**epsilon

                f = open(log_file, 'a')
                f.write("+++++++++++++++++++++++++\n")
                f.write("Completed: " + time.strftime("%H:%M:%S") + "\n")
                f.write("# noise messages = " + str(1000*num_messages) + " per server, rounds = 10^" + str(math.log10(rounds)) + "\n")
                f.write("best for config: e^eps = " + str(epsilon_exp) + ", delta = " + str(delta) + "\n")
                f.write("************************\n\n")
                f.close()

                f2 = open(csv_file, 'a')
                f2.write(str(num_messages) + ", 10^" + str(math.log10(rounds)) + "," + str(epsilon_exp) + "," + str(delta) + "\n")
                f2.close()
    #sys.stdout.flush()
    #sys.stdout.close()

def best_for_noise(servers, noise_messages_per_server, rounds, prob_matrix, global_delta_goal, accuracy):
    """
    finds the best noise configuration and attained epsilon for given config (total number of noise messages, rounds, delta goal)
    """

    # the total number of noise messages in the system, some of all noise messages from non-faulty servers.
    noise_messages = servers * noise_messages_per_server * (1 - kFaultProb)
    if kFaultTolorance:
        noise_messages = int(noise_messages / (1.0 + 1.0/kChainLen))

    # start with using all noise messages except 2 for hiding single access. Gradually shift copules of noise messages to hide double access.
    mean1 = noise_messages - 2.0
    mean2 = 1.0
    best_N1 = None
    best_N2 = None
    epsilon_exp = 10**100
    delta = 1
    err_prob = 0
    step_size = accuracy * noise_messages/100.
    belief_prob = 0
    config_found = False

    # so long as we still didn't shift all noise from single access to double access
    while (mean1 > 0):
        try:
            # single access noise is alpha^x_i, where x is the input chain, i is the output chain, and also divided by number of servers
            # overall we divide by servers**3 since there are servers number of choices for x, servers number of choices for i,
            # and divide again by number of servers since that is what each server sends.
            noise_messages_per_N1_var = mean1 / (servers**3)

            # double access noise is beta^{x,y}_{i,j}, where x,y are the input chains, i,j are the output chains, and also divided by number of servers
            # overall we divide by servers**5 since there are servers number of choices for x, y, i, and j,
            # and divide again by number of servers since that is what each server sends.
            noise_messages_per_N2_var = mean2 / (servers**5)

            # TODO(@Nirvan): does the computation above make sense?
            # should we actually divide by servers * (1 - kFaultProb) for the last division to get the noise per honest server?

            # compute the epsilon this single/double access split gives us
            curr_epsilon_exp, curr_delta, curr_err_prob, bp = compute(noise_messages, servers, noise_messages_per_N1_var, noise_messages_per_N2_var, rounds, prob_matrix, global_delta_goal)
            # if we got a better epsilon than what we had, then keep the configuration
            if (epsilon_exp > curr_epsilon_exp):
                delta = curr_delta
                epsilon_exp = curr_epsilon_exp
                err_prob = curr_err_prob
                best_N1 = noise_messages_per_N1_var
                best_N2 = noise_messages_per_N2_var
                belief_prob = bp
                config_found = True
        except OverflowError:
            pass

        # in the next step, move step_size noise messages from single access, to get step_size/2 messages to cover double access patterns.
        mean1 -= step_size
        mean2 += step_size/2
    if not config_found:
        print "no configuration found, delta =", delta
        return None

    # print the best config we got
    print "best for config: e^eps = ", epsilon_exp, "delta = ", delta, "belief prob. =", belief_prob, "err prob. =", err_prob
    print "configuration: N1 ~ Pois(",best_N1,")"
    print "configuration: N2 ~ Pois(",best_N2,")"
    return epsilon_exp, delta, err_prob, best_N1, best_N2, belief_prob

def compute(messages_in_system, m, mean1, mean2, k, prob_matrix, global_delta_goal):
    """
    computes the epsilon we get for a particular configuration. This basically follows the analysis section from the paper.
    messages_in_system is the total number of noise messages in the system
    k is the number of rounds
    global_delta_goal is our bound on delta
    m is the number of servers
    mean1 is the mean for single-access noise dist
    mean2 is the mean for double-access noise dist
    prob_matrix is the probability of getting different combinations of input/output chains fully corrupt
    """
    c_out = 4
    best_delta = 1
    best_epsilonexp = 100000000
    best_delta_input = 1
    best_eps_input = 100000
    best_eps_so_far = 100000000

    # c_out is the cutoff point for the delta/epsilon tradeoff that the output chain provides
    # c_in is the cutoff point for the delta/epsilon tradeoff that the input chain provides
    # try different cutoff points to find the best configuration (not the most efficient search algorithm :-) )
    while c_out < 10:
        c_in = 3
        while c_in < 7:
            try:
                # in the input chain, the number of noise for each parcel is the total number of noise,
                # divided by number of input chains (m) divided by number of output chains (m)
                mean_messages_per_parcel = float(messages_in_system) / m**2
                # compute the differential privacy gurantee of the input chain
                eps_input, delta_input = poisson_dp(mean_messages_per_parcel,  c_in)

                # bound the attacker's belief probability using the diff-privacy gurantees of the input chain.
                single_vars = m
                single_belief = min(1, math.e**eps_input/single_vars + delta_input)

                # bound the attacker's belief probability for guessing both Alice and Bob's output chain.
                double_belief = single_belief**2 * (2-single_belief)

                # case 1: both chains function well
                e1, d1 = sample_poisson_dp_final(m**2 * mean1, c_out, single_belief)
                e2, d2 = sample_poisson_dp_final(2 * m**3 * mean2, c_out, double_belief)
                perfect_eps = 2*e1 + 2*e2
                perfect_delta = 2*d1 + 2*d2

                # compute epsilon and delta for different corruption rates, then combine! (for our current deployment story, we only iterate on the no corruption case)
                prob_compose = []
                left_over_probability = 1
                max_faults = max_faulty_chains(m)
                if max_faults == 0:
                    # only iterate through the prob_matrix[0][0] case (i.e., no corruption)
                    max_faults = 1

                for i in xrange(min(len(prob_matrix), max_faults)):
                    # noise from whole malicious chains is completely removed (so we update the mean for noise distribution in DP)
                    # input chain is compromised, so belief probability is 1. This gives new DP gurantees.
                    e1, d1 = sample_poisson_dp_final(m * mean1 * (m - i), c_out, 1)
                    e2, d2 = sample_poisson_dp_final(2 * m**2 * mean2* (m - i), c_out, 1)
                    eps_out = 2*e1 + 2*e2
                    delta_out = 2*d1 + 2*d2

                    # Alice gets the new gurantee if she picked corrupt input/output chain, we now combine all these options using Thm 2.
                    for j in xrange(min(len(prob_matrix[i]), max_faults)):
                        eps_ij, delta_ij = compute_with_probability(eps_input, delta_input, eps_out, delta_out, perfect_eps, perfect_delta, float(i)/m, float(j)/m)
                        prob_compose.append((prob_matrix[i][j], eps_ij, delta_ij))
                        left_over_probability -= prob_matrix[i][j]

                # shuve everything that remains into the worse case to save computation time, hopefully the remaining prob. is not that high.
                # Assume that attacker corrupted everything.
                if left_over_probability > 0:
                    eps_ij, delta_ij = compute_with_probability(eps_input, delta_input, 0, 1, perfect_eps, perfect_delta, 1, 1)
                    prob_compose.append((left_over_probability, eps_ij, delta_ij))

                # Finally combine all options using Thm 2.
                comb_eps, comb_delta = combine_with_prob(prob_compose)

                # Did we get a valid solution? Check if after composing over k rounds, we are below the required delta.
                if k * comb_delta <= global_delta_goal:
                    ex_epsilonexp, ex_delta = extend_rounds(comb_eps, comb_delta, k, global_delta_goal)
                    # If we improved over our best, store the configuration
                    if (ex_epsilonexp < best_epsilonexp):
                        best_epsilonexp = ex_epsilonexp
                        best_delta = ex_delta
                        best_delta_input = delta_input
                        best_eps_input = eps_input
                        best_eps_so_far = perfect_eps

            except OverflowError:
                pass
            c_in += 0.1
        c_out += 0.5

    # return the best we got.
    return best_epsilonexp, best_delta, best_delta_input, math.e**best_eps_input

def concentreted_poisson_dp(mu):
    all_probs = []
    sum_delta = 0
    i = 0.0
    while (i < 10):
    	i += 0.1
        epsilon, delta = poisson_dp(mu,  float(i))
        p = 1 - delta - sum_delta
        all_probs.append((epsilon, p))
        sum_delta += p

    delta = 1 - sum_delta
    
    acc_eps = 0

    for eps, p in all_probs:
        acc_eps += p * math.e**eps

    return math.log(acc_eps), delta


def compute_new(m, l, k, c, goal_delta, p = 0.75, c_in = 4, c_out = 4.5):
    l *= p
    # TODO: experimental use of concentreted diff privacy
    eps_input, delta_input = concentreted_poisson_dp(float(l)/m) #poisson_dp(float(l)/m,  c_in)
    belief_prob_small = (1 - delta_input) / (m - 1 + math.e**eps_input)

    eps1, delta1 = __compute_new(m, l *2./4., k, c, belief_prob_small, p, c_out)
    eps2, delta2 = __compute_new(m, l/4., k, c, belief_prob_small, p, c_out)

    epsilon = 2 * eps1 + eps2
    delta = 2 * delta1 + delta2

    #print epsilon, delta
    epsilon, delta = extend_rounds(epsilon, delta, k, goal_delta)
    return math.log(epsilon), delta


def __compute_new(m, l, k, c, belief_prob_small, p = 0.75, c_out = 4.5):
	# TODO: experimental use of concentreted diff privacy
    eps_large, delta_large = concentreted_poisson_dp(float(l)) #poisson_dp(l, c_out)
    eps_small, delta_small = compute_small_belief(c, m-1, l)


    epsilon, delta = combine_with_prob([((m-1)* belief_prob_small, eps_small, delta_small), (1 - (m-1)* belief_prob_small, eps_large, delta_large)])
    epsilon, delta = combine_with_prob([(1-p, epsilon, delta), (p, 0, 0)])
    return epsilon, delta

def compute_small_belief(c, m, l):
    mu = l*m
    csd = c*(l*m)**0.5
    mu_all_but_one = l*(m-1)
    max_text = 10000

    delta1 = poisson.sf(mu_all_but_one + csd, mu_all_but_one)
    delta2 = poisson.cdf(mu_all_but_one - csd, mu_all_but_one)

    #delta1 = poisson.pmf(l-1,l) * poisson.sf(mu_all_but_one + csd, mu_all_but_one)
    #for i in xrange(1,max_text):
    #       delta1 += poisson.pmf(l + i,l) * poisson.pmf(mu_all_but_one + csd - i, mu_all_but_one)
    #delta1 += poisson.pmf(l + max_text,l)
    #
    #delta2 = poisson.pmf(0,l) * poisson.sf(mu + csd, mu_all_but_one)
    #for i in xrange(1,max_text):
    #       delta2 += poisson.pmf(l + i,l) * poisson.pmf(mu + csd - i, mu_all_but_one)
    #delta2 += poisson.pmf(l + max_text,l)

    delta = (delta1 + delta2)
    epsilon  = math.log(1 + (float(c)/(m*l)**0.5))
    return epsilon, delta

if __name__ == "__main__":
    # find all configurations for different number of servers
    find_configs()
