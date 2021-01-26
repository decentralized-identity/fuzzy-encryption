"""
Test the system by randomly introducing errors into
the recovery words and see if system recovers
as promised
"""

import time
from typing import List
import trial
import click

@click.command()
@click.option(
    "--words",
    type=str,
    prompt="words",
    help='string representing the original words. e.g. "0 1 2 3 4"',
    required=True
    )
@click.option(
    "--correct-threshold",
    type=int,
    help="minimum number of correct words needed to recover the keys",
    prompt="correct-threshold",
    required=True
    )
@click.option(
    "--corpus-size",
    type=int,
    prompt="corpus-size",
    help="number of available words",
    required=True
    )
@click.option(
    "--count",
    type=int,
    help="number of random recover attempts",
    prompt="count",
    required=True
    )
@click.option(
    "--repo-file",
    type=str,
    help="path to json file that stores information to reproduce errors using test_repo.py",
    default="repo.json"
    )
def rand_trial(
        corpus_size: int,
        count: int,
        correct_threshold: int,
        words: str,
        repo_file: str
        ) -> None:
    """
    Test the Fuzzy Key Recovery scheme by creating a secret with
    a set of original words and then try to recover the secret
    by randomly guessing the original works. The recovery words
    are a mutation of the original words where any number
    of mutations is allowed. We make check that the secret is
    recovered only when the number of correctly guessed words
    is greater than or equal to the correct threshold.
    """

    print("words", words)
    print("correct-threshold:", correct_threshold)
    print("corpus-size:", corpus_size)
    print("count:", count)
    print("repo-file:", repo_file)

    original_words: List[int] = [int(word) for word in words.split()]

    adict = {
        "original_words" : original_words,
        "correctthreshold" : correct_threshold,
        "corpus_size" : corpus_size,
        "test_count" : count
    }
    start = time.time()
    trial.trial(adict)
    end = time.time()
    print("{0:.1f} seconds for {1} iterations".format(end - start, count))

# pylint: disable=no-value-for-parameter
rand_trial()
