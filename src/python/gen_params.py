"""
gen_params.py

Generates input params to be used by gen_secret.py
"""

import click
from fuzzy import InputParams, FuzzyError

@click.command()
@click.option('--set-size',
              type=int,
              prompt="set size",
              required=True,
              help='required number of original and recovery words')
@click.option('--correct-threshold',
              type=int,
              prompt="correct threshold",
              required=True,
              help='minimum number of correct matches required to recover the keys')
@click.option('--corpus-size',
              type=int,
              prompt="corpus size",
              required=True, help='size of the corpus set of words')
@click.option('--params-path',
              type=str,
              default='params.json',
              help='path to JSON file to hold the generated' +
              ' input parameters (default= params.json)')
def gen_demo(set_size, correct_threshold, corpus_size, params_path) -> None:
    """
generate a secret

example:

python3 gen_params.py --setSize 9 --correct-threshold 6
         --corpus-size 7000 [--params-path params.json]
    """
    params = InputParams(setSize=set_size,
                         correctThreshold=correct_threshold,
                         corpusSize=corpus_size)
    with open(params_path, 'w') as fobj:
        print("writing parameters to", params_path)
        fobj.write(str(params))
        fobj.write("\n")

# pylint: disable=broad-except

if __name__ == '__main__':
    try:
        # pylint: disable=no-value-for-parameter
        gen_demo()
        # pylint: enable=no-value-for-parameter
    except FuzzyError as error:
        print("\nAn error was detected:", error.message)
