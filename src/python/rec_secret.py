"""
rec_secret.py

Recoveres a secret using the Fuzzy Key Recovery scheme
"""

import click
from fuzzy import RecoverSecret, bytes_to_hex, FuzzyError, FuzzyState

def work(words, key_count, secret) -> None:
    """
    1. re-create the state object from json file
    2. call RecoverySecret on the recovery words (words)
    3. print the recovery words

    An FuzzyError exception is throw upon failure
    """
    with open(secret, 'r') as fobj:
        repn = fobj.read()
    state = FuzzyState.Loads(repn)
    recovery_words = [int(word) for word in words.split()]
    keys = RecoverSecret(state, recovery_words, key_count)
    print("keys:")
    for key in keys:
        print("-", bytes_to_hex(key))


@click.command()
@click.option('--words',
              type=str,
              prompt="words",
              required=True,
              help='recovery words as integers eg. "8 6 0 3"')
@click.option('--key-count',
              type=int,
              default=1,
              help='number of keys to be generated [default=1]')
@click.option('--secret',
              type=str,
              default='secret.json',
              help='path to JSON file holding the secret (FuzzyState)')
def rec_secret(words, key_count, secret) -> None:
    """
recover a secret

example:

python3 rec_secret.py --words "1 2 3" [--secret secret.json] [--key-count 1]
    """
    work(words, key_count, secret)

if __name__ == '__main__':
    try:
        # pylint: disable=no-value-for-parameter
        rec_secret()
        # pylint: enable=no-value-for-parameter
    except FuzzyError as error:
        print("\nKey Recovery Failed:")
        print("    ", error.message)
    # work("1 2 3", 1, "output.json")
