"""
gen_secret.py

Generates a secret using the Fuzzy Key Recovery scheme
"""

import json
import click
from fuzzyvault import gen_secret, FuzzyError

def work(params_path: str, words: str, secret_path: str) -> None:
    "workhorse"
    original_words = json.dumps([int(word) for word in words.split()], indent=2)
    with open(params_path) as fobj:
        params = fobj.read()
    secret = gen_secret(params, original_words)
    with open(secret_path, 'w') as fobj:
        print("writing secret to", secret_path)
        fobj.write(secret)
        fobj.write('\n')

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
def main(params_path: str, words: str, secret_path: str) -> None:
    """
generate a secret

example:

python3 gen_secret --words "0 1 2 3" [--params-path params.json]
            [--secret-path secret.json]"
    """
    work(params_path, words, secret_path)

if __name__ == '__main__':
    try:
        # pylint: disable=no-value-for-parameter
        main()
        # pylint: enable=no-value-for-parameter
    except FuzzyError as error:
        print("\nAn error was detected:", error.message)
    except FileNotFoundError as error:
        print(error)
