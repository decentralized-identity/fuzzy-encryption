"""
gen_secret.py

Generates a secret using the Fuzzy Key Recovery scheme
"""

import click
from fuzzy import GenerateSecret, InputParams, bytes_to_hex, FuzzyError

@click.command()
@click.option('--params-path',
              type=str,
              default='params.json',
              help='path to a JSON representation of an' +
              ' InputParams object (default= params.json)')
@click.option('--words',
              type=str,
              prompt="words",
              help='original words as integers eg. "8 6 0 3"')
@click.option('--secret-path',
              type=str,
              default='secret.json',
              help='path file to contain the JSON representation' +
              ' of the secret (FuzzyState). (default= secret.json', )
@click.option('--key-count',
              type=int,
              default=1,
              help="number of keys to be printed (default=1)")
def gen_secret(params_path: str, words: str, secret_path: str, key_count: int) -> None:
    """
generate a secret

example:

python3 gen_secret --words "0 1 2 3" [--params-path params.json]
            [--secret-path secret.json]"
    """
    original_words = [int(word) for word in words.split()]
    with open(params_path) as fobj:
        params = InputParams.Loads(fobj.read())
    state, keys = GenerateSecret(params, original_words, key_count)
    print("keys:")
    for key in keys:
        print("-", bytes_to_hex(key))
    with open(secret_path, 'w') as fobj:
        print("writing secret to", secret_path)
        fobj.write(str(state))
        fobj.write('\n')

if __name__ == '__main__':
    try:
        # pylint: disable=no-value-for-parameter
        gen_secret()
        # pylint: enable=no-value-for-parameter
    except FuzzyError as error:
        print("\nAn error was detected:", error.message)
    except FileNotFoundError as error:
        print(error)
