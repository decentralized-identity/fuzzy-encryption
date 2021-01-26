'''
cparams.py

For creating a file containing a JSON string that is
an example of a WASM style input for gen_params.py
'''

import json
import binascii
import secrets
# import sympy
import click

def get_dict(set_size: int, correct_threshold: int, corpus_size: int) -> None:
    '''
    returns a dictionary representing the WASM style input
    The workhorse routine that is not mangled by
    click stuff. The random numbers are divided up into 32 byte
    chunks each of which is represented by a 64 character upper
    case hexadecimal string. The number of random bytes created
    is rounded up to the next multiple of 32.
    '''
    # prime = first_prime_greater_than(corpus_size)

    chunk_size = 32
    byte_count = 32 + 4 * set_size
    chunk_count = (byte_count + chunk_size - 1) // chunk_size
    return {
        "setSize" : set_size,
        "corpusSize" : corpus_size,
        "correctThreshold" : correct_threshold,
        "randomBytes" : [bytes_to_hex(random_bytes(chunk_size)) for _ in range(chunk_count)]
    }

def work(set_size: int, correct_threshold: int, corpus_size: int) -> None:
    """
    workhorse routine stripped of all the click stuff.
    Prints the extended input to gen_params
    """
    a_dict = get_dict(set_size, correct_threshold, corpus_size)
    print(json.dumps(a_dict, indent=2))

# def first_prime_greater_than(i: int) -> int:
#     """
#     Return the first prime strictly greater than i
#     """
#     k = i + 1
#     while not sympy.isprime(k):
#         k += 1
#     return k

@click.command()
@click.option('--set-size',
              type=int,
              default=9,
              help='required number of original and recovery words')
@click.option('--correct-threshold',
              type=int,
              default=6,
              help='minimum number of correct matches required to recover the keys')
@click.option('--corpus-size',
              type=int,
              default=7776,
              help='size of the corpus set of words')
def main(set_size: int, correct_threshold: int, corpus_size: int) -> None:
    '''
    main entry point
    '''
    work(set_size, correct_threshold, corpus_size)


def bytes_to_hex(data: bytes) -> str:
    """
    Return a hexadecimal string representation of an array of bytes
    where the string is made up of upper case hexadecimal
    characters. This function is the inverse of
    hex_to_bytes.

    bytes_to_hex(b'\xf8\x03') -> 'F803'
    """
    return binascii.hexlify(data).decode('utf-8').upper()

def random_bytes(length: int) -> bytes:
    """
    Return a random byte string of length count.
    Called by the InputParams constructor
    """
    return secrets.token_bytes(length)


if __name__ == '__main__':
    # pylint: disable=no-value-for-parameter
    main()
