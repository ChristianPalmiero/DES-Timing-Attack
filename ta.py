#! /usr/bin/env python2

import sys
import argparse
import des
import km
import pcc
import numpy as np

Tham = [3021.43,3307.98,3565.92,3847,4220.51]

def main ():
    global ct, t
    # ************************************************************************
    # * Before doing anything else, check the correctness of the DES library *
    # ************************************************************************
    if not des.check ():
        sys.exit ("DES functional test failed")

    # *************************************
    # * Check arguments and read datafile *
    # *************************************
    argparser = argparse.ArgumentParser(description="Apply P. Kocher's TA algorithm")
    argparser.add_argument("datafile", metavar='file', 
                        help='name of the data file (generated with ta_acquisition)')
    argparser.add_argument("n", metavar='n', type=int,
                        help='number of experiments to use')
    args = argparser.parse_args()

    if args.n < 1:                                      # If invalid number of experiments.
        sys.exit ("Invalid number of experiments: %d (shall be greater than 1)" % args.n)

    # Read encryption times and ciphertexts. n is the number of experiments to use.
    read_datafile (args.datafile, args.n)

    # Filter the input data and calculate the number of filtered experiments
    t = list(filter(lambda x: x<1000000,t))
    nb = len(t)
    # Undo the final permutation on cipher text of all experiments
    r16l16 = [des.ip (ct[i]) for i in range(nb)]
    # Extract right half (strange naming as in the DES standard)
    l16 = [des.right_half (i) for i in r16l16]
  
    # Compute the average round time per sample
    t_round = np.array(t)/16
    # Compute the average round time
    Tround = np.mean(t_round)
    
    #Compute the average Sbox time per sample
    t_sbox = t_round/8
    #Compute the average Sbox time
    Tsbox = np.mean(t_sbox)
    
    # Initialise the key
    key = 0

    # Initialise the old hamming weight array
    old_ham = np.array(t)*0
    
    for part in range(1,9):
        # Compute the timing model of each sample
        Tmodel = np.array(Tround*15+(8-part)*Tsbox+old_ham)
    
        Tmodel_L = []
	hams = []
    
        for k in range(64):
            step = 48 - part*6
            ham = [Tham[hamming_weight(des.sboxes(((des.e(l) >> step) ^ k)<<step) & 15*16**(8-part))] for l in l16]
            hams += [ham]
	    Tmodel_L += [Tmodel + ham]

        ctx = pcc.pccContext (64);       # Initialize context for 64 Y random variables
        for i in range (nb):             # For nexp experiments
            ctx.insert_x (t[i])          # Insert realization of X into context
            for j in range(64):          # For the 64 Y random variables
                ctx.insert_y (j, Tmodel_L[j][i])     # Insert realization of Yj into context
 
        ctx.consolidate ()               # Finalize computation of the 64 PCCs
 
        PCCs = []
        for j in range (64):             # For the 64 Y random variables
            PCCs+= [ctx.get_pcc (j)]     # Get PCC(X,Yj)
        
        # Take the best PCC
        best = np.argmax(abs(np.array(PCCs)))
        
        # Shift and update the key
        key = (key << 6) + best

        # Update the old hamming weight array
        old_ham += hams[best]
    

    # ************************
    # * Print last round key *
    # ************************
    #print >> sys.stderr, "Last round key (hex):"
    print ("0x%012x" % key)

# Open datafile <name> and store its content in global variables
# <ct> and <t>.
def read_datafile (name, n):
    global ct, t

    if not isinstance (n, int) or n < 0:
        raise ValueError('Invalid maximum number of traces: ' + str(n))

    try:
        f = open (str(name), 'rb')
    except IOError:
        raise ValueError("cannot open file " + name)
    else:
        try:
            ct = []
            t = []
            for _ in xrange (n):
                a, b = f.readline ().split ()
                ct.append (int(a, 16))
                t.append (float(b))
        except (EnvironmentError, ValueError):
            raise ValueError("cannot read cipher text and/or timing measurement")
        finally:
            f.close ()

# ** Returns the Hamming weight of a 64 bits word.
# * Note: the input's width can be anything between 0 and 64, as long as the
# * unused bits are all zeroes.
# See: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
def hamming_weight (v):
    v = v - ((v>>1) & 0x5555555555555555)
    v = (v & 0x3333333333333333) + ((v>>2) & 0x3333333333333333)
    return (((v + (v>>4) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56) & 0xFF

if __name__ == "__main__":
    main ()
