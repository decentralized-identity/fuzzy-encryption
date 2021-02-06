"""
gen_params.py

Generates input params to be used by gen_secret.py
"""

import click
import json
from fuzzyvault import FuzzyError, gen_params

def work(setSize: int, correctThreshold: int, corpusSize: int, path: str) -> None:
    "workhorse"
    params_input = json.dumps({ "setSize": setSize, "correctThreshold": correctThreshold, "corpusSize": corpusSize }, indent=2)
    with open(path, 'w') as fobj:
        print("writing parameters to", path)
        fobj.write(str(gen_params(params_input)))
        fobj.write("\n")

@click.command()
@click.option('--set-size',
              type=int,
              prompt="setSize",
              required=True,
              help='required number of original and recovery words')
@click.option('--correct-threshold',
              type=int,
              prompt="correctThreshold",
              required=True,
              help='minimum number of correct matches required to recover the keys')
@click.option('--corpus-size',
              type=int,
              prompt="corpusSize",
              required=True, help='size of the corpus set of words')
@click.option('--params-path',
              type=str,
              default='params.json',
              help='path to JSON file to hold the generated' +
              ' input parameters (default= params.json)')
def main(set_size, correct_threshold, corpus_size, params_path) -> None:
    """
generate a secret

example:

python3 gen_params.py --set-size 9 --correct-threshold 6
         --corpus-size 7000 [--params-path params.json]
    """
    work(setSize=set_size, 
         correctThreshold=correct_threshold,
         corpusSize=corpus_size,
         path=params_path
         )

# pylint: disable=broad-except
if __name__ == '__main__':
    try:
        # pylint: disable=no-value-for-parameter
        main()
        # pylint: enable=no-value-for-parameter
    except FuzzyError as error:
        print("\nAn error was detected:", error.message)
