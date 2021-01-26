# A Python implementation of the Fuzzy Vault Key Recovery Scheme

**NOTE**: This implementation is code complete with full review and testing at the time of its contribution, but WILL NOT be supported long-term. 
However, the C++ version will be supported long time.

**WARNING**: This code relies on copyleft dependencies which may require special attention to use safely within commercial products. Please take care to ensure you are abiding by the license terms specified in these dependencies.

This document describes the Python demonstration code.

## Introduction

The Python code was written first in order to verify the algorithms.
After that the algorithm was ported to C++. Finally, the Python code
was updated to match the C++ APIs.

This code has been tested under several Linux distributions
including Ubuntu and Debian under WSL2.
The Python code relies on several cryptographic Python packages.
These packages and their installation are described later in
this document.

For the user, there are only three functions of interest.

- gen_params
- gen_secret
- gen_keys

These functions are found in ./fuzzyvault/fuzzyvault.py.
An example of their uses are contained in the scripts
gen_params.py, gen_secret.py and gen_keys.py as described below.

## Requirments

You will have to have installed **python3** and **pip3**.
After that install the following Python packages with **pip3**:

- click
- crypto
- flint-py
- hashids
- jsonSchema
- pyOpenSSL
- secrets
- hmac
- sympy

## Instalation

You will need to clone this repo to your local machine. If you don't have git installed, you can simply do so by typing the following in command line (update the package index first):

### Clone and Setup

Clone this repo to your local machine.
Next, in order to install all needed dependencies before running or testing this program, you will need to run the bash script install.sh. If you're currently in the local directory where it resides, type the following in command line, otherwise replace with the correct PATH to install.sh:

```
$ chmod +x ./install.sh
$ sudo ./install.sh
```


## Running the Python scripts

The scripts are found in ./fuzzyvault. To demonstrate how the system works
execute the following sequence of commands ...

```
$ cd ./fuzzyvault
$ python3 gen_params.py --set-size 12 --correct-threshold 9 --corpus-size 7776
$ python3 gen_secret.py --words " 1 2 3 4 5 6 7 8 9 10 11 12 "
$ python3 gen_keys.py --words " 1 2 3 4 5 6 7 8 9 110 111 112 "
```

What happened here?

### gen_params.py 

>`gen_params.py` determines the security of the key recovery scheme.
The user specifies set-size, the number of words (integers) required for key recovery.
In this case the number of words is 12. The output is a JSON string that
specifies the parameters of the scheme to be used when a secret is defined.
The JSON string has the following form

```
{
  "setSize": 12,
  "correctThreshold": 9,
  "corpusSize": 7776,
  "prime": 7789,
  "salt": "A89E6176F32C ... 334DCE03B2",
  "extractor": [ 7648, 5771, ... 5086, 6692, 3522 ]
}
```
>The first three fields reflect the input. The other fields are used
by `gen_secret.py` and `gen_keys.py`.

### gen_secret.py

>`gen_secret.py` establishes the set of words that will recover
the keys. You can think of this as a combination to a safe that 
contains the keys.
The output is a JSON string containing the information necessary to
unlock the safe, *not including the combination*.
This information must be presented at the time of key recovery.
It is not necessary to keep it secret but it must be safe. We 
call this the `secret` even though it may be read by the public.
Perhaps we should have called it *state*. In any case it looks
like
```
{
  "setSize": 12,
  "correctThreshold": 9,
  "corpusSize": 7776,
  "prime": 7789,
  "extractor": [ 7648, 5771, ... 6692, 3522 ],
  "salt": "A89E6176F32CB5 ... 334DCE03B2",
  "sketch": [ 967, 5576, 1719, 6542, 2717, 7711 ],
  "hash": "BB50CB5980CCB ... 45C5D"
}
```
>The meaning of each of these terms what they are used for
is best understood by reading the code.

### gen_keys.py

>`gen_keys.py` unlocks the keys from the safe. I order to do this
you must supply the `secret` and the combination.
If the combination (recovery words) are *close enough* then
the keys will be recovered. *close enough* implies that 
you do not have to get the combination exactly correct. 
As long as `correctThreshold` or more of the words (numbers)
are correct the keys will be recovered. The output
is a JSON string representing an array of strings where
each individual string is an upper case hexadecimal 
string representing a 512-bit cryptographic key. The
output has the form
```
[
  "9B022C05EB847A94C6 ... 5DD8B53712DAE4DF755A612E456F",
  "12DAE4DF755ACF9687 ... 13AF8912C05EB847AC05EB847A88"
]
```

Each of the python scripts calls to a corresponding function in 
./fuzzyvault.py, namely, `gen_params`, `gen_secret` and `gen_keys`.
Each of these functions takes JSON strings as arguments and returns
JSON strings as output. The C++ implementation of the key
recovery scheme uses the same APIs.
