# Timing attack against a DES software implementation

## General description

In this exercise I try to exploit a flaw in a DES software implementation which computation time depends on the input messages and on the secret key: the P permutation has been implemented by a not-too-smart software designer who does not know anything about timing attacks (and not much about programming). The pseudo-code of his implementation of the P permutation is the following:

```
// Permutation table. Input bit #16 is output bit #1 and
// input bit #25 is output  bit #32.
p_table = {16,  7, 20, 21,
           29, 12, 28, 17,
            1, 15, 23, 26,
            5, 18, 31, 10,
            2,  8, 24, 14,
           32, 27,  3,  9,
           19, 13, 30,  6,
           22, 11,  4, 25};

p_permutation(val) {
  res = 0;                    // Initialize the result to all zeros
  for i in 1 to 32 {          // For all input bits #i (32 of them)
    if get_bit(i, val) == 1   // If input bit #i is set
      for j in 1 to 32        // For all 32 output bits #j (32 of them)
	if p_table[j] == i    // If output bits #j is input bit #i
          k = j;              // Remember output bit index
        endif
      endfor                  // output bit #k is now input bit #i
      set_bit(k, res);        // Set bit #k of result
    endif
  endfor
  return res;                 // Return result
}
```

## Directions

### Build all executables

```bash
$ make all
```

### Acquisition phase

Run the acquisition phase:
```bash
$ ./ta_acquisition 100000
100%
Experiments stored in: ta.dat
Secret key stored in:  ta.key
Last round key (hex):
0x79629dac3cf0
```

This randomly draws a 64-bits DES secret key and 100000 random 64-bits plaintexts, and enciphers them using the flawed DES software implementation. Each enciphering is accurately timed using the hardware timer. Two files are generated:
* `ta.key` containing the 64-bits DES secret key, its 56-bits version (without the parity bits), the 16 corresponding 48-bits round keys and, for each round key, the eight 6-bits subkeys.
* `ta.dat` containing the 100000 ciphertexts and timing measurements.

Note: the 48-bits last round key is printed on the standard output (`stdout`), all other printed messages are sent to the standard error (`stderr`).

Note: the secret key can also be chosen with:
```bash
$ ./ta_acquisition 100000 0x0123456789abcdef
```
where `0x0123456789abcdef` is the 64-bits DES secret key you want to use, in hexadecimal form.

Let us look at the few first lines of `ta.dat`:
```bash
$ head -4 ta.dat
0x743bf72164b3b7bc 80017.500000
0x454ef17782801ac6 76999.000000
0x9800a7b2214293ed 74463.900000
0x1814764423289ec1 78772.500000
```

Each line is an acquisition corresponding to one of the 100000 random plaintexts. The first field on the line is the 64 bits ciphertext returned by the DES engine, in hexadecimal form. With the numbering convention of the DES standard, the leftmost character (7 in the first acquisition of the above example) corresponds to bits 1 to 4. The following one (4) corresponds to bits 5 to 8 and so on until the rightmost (c) which corresponds to bits 61 to 64. In the first acquisition of the above example, bit number 6 is set while bit number 8 is unset.
The second field is the timing measurement.

### Attack phase

```bash
$ ./ta.py ta.dat 10000
```

The timing attack tries to retrieve the real last round key, that can be found out by looking at the `ta.key` file for verification purposes.
The timing attack:
* filters the input timing measurements;
* computes an average of the time taken by the execution of a single DES round;
* computes an average of the time taken by the execution of a single Sbox within one DES round;
* computes 6 bits of the last round key, exploiting 64 possible combinations at a time;
* builds a timing model for each sample: the timing model takes into account the time taken by the execution of the previous 15 rounds, the time taken by the execution of the previously computed Sboxes and the remaining Sboxes, a timing factor that depends on the Hamming weight of the input of the P permutation;
* computes 64 PCCs between X, a random variable representing the real input timing measurements, and Yj, a random variable representing the estimated timing measurements for each of the 64 combinations of the key;
* gets the maximum PCC and tries to guess 6 bits of the last round key at a time.

All printed messages are sent to the standard error (`stderr`). The only message that is sent to the standard output (`stdout`) is the 48-bits last round key, in hexadecimal form.

### Results

My attack succeeds with at least 3261 acquisitions.
