"""
gen_keys.py

Recovers a secret using the Fuzzy Key Recovery scheme
"""

import json
import click
from fuzzyvault import gen_keys, FuzzyError

def work(words, key_count, secret_path) -> None:
    """
    1. re-create the state object from json file
    2. call RecoverySecret on the recovery words (words)
    3. print the recovery words

    An FuzzyError exception is throw upon failure
    """
    recovery_words = json.dumps([int(word) for word in words.split()], indent=2)
    with open(secret_path, 'r') as fobj:
        secret = fobj.read()
    print(gen_keys(secret, recovery_words, key_count))


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
def main(words, key_count, secret) -> None:
    """
generate keys from a secret

example:

python3 gen_keys.py --words "1 2 3" [--secret secret.json] [--key-count 1]
    """
    work(words, key_count, secret)

if __name__ == '__main__':
    try:
        # pylint: disable=no-value-for-parameter
        main()
        # pylint: enable=no-value-for-parameter
    except FuzzyError as error:
        print("\nKey Recovery Failed:")
        print("    ", error.message)
    # work("1 2 3", 1, "output.json")
