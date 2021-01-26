"""
    For testing key recovery
"""

import os
import time
import datetime
from math import log10
from random import randint, shuffle
import json
from typing import List, Dict, Any
import click
import hashids              # type: ignore
from fuzzy import FuzzyState, InputParams, GenerateSecret, \
                  RecoverSecret, bytes_to_hex, FuzzyError

# pylint: disable=invalid-name

def mutate(words: List[int], errors: int, N: int) -> List[int]:
    '''
    Introduce errors into a new set of words

    Input:

    words: a list of unique of integers to be mutated

    errors: the number of integers to be mutated so
            that the new list and old list differ
            in this many places

    N: The range of allowed integers values is 0 .. N-1

    Returns:

    A mutated list of integers that differs from the
    original in 'errors' places

    '''
    assert N > 0
    assert len(words) == len(set(words))
    assert 0 < errors <= len(words) < N
    for w in words:
        assert 0 <= w < N

    ans = words[:]
    shuffle(ans)
    for k in range(errors):
        while True:
            w = randint(0, N - 1)
            if w not in words and w not in ans:
                ans[k] = w
                break
    ans.sort()
    return ans

def repr_words(words: List[int], w: int) -> str:
    '''
    Return a string representing a list of words
    but each word (int) is has a width of "w"
    characters
    '''
    if len(words) == 0:
        return "[]"
    fmt = '{' + "0:" + str(w) + '}'
    ans = '[' + fmt.format(words[0])
    for word in words[1:]:
        ans += ', ' + fmt.format(word)
    return ans + ']'

def get_width(n: int) -> int:
    'return an conservative estimate of the number of digits'
    return int(log10(n)) + 1

def trial(adict: Dict[str, Any], save_path: str = None) -> None:
    # pylint: disable=too-many-locals
    '''
    This is a test of mutations
    '''
    original_words = adict["original_words"]
    setsize = len(original_words)
    correctthreshold = adict["correctthreshold"]
    corpus_size = adict["corpus_size"]
    test_count = adict["test_count"]

    repos: List[Dict[str, Any]] = []
    original_words.sort()

    try:
        params = InputParams(setsize, correctthreshold, corpus_size)
    except FuzzyError as e:
        print(e.message)
        return
    state, keys = GenerateSecret(params, original_words, key_count=1)

    width = get_width(corpus_size)

    print("original_words:")
    print(repr_words(original_words, width), "->", bytes_to_hex(keys[0]))
    print("conducting", test_count, "tests ...")

    max_error_count = setsize - correctthreshold
    for test_number in range(test_count):
        if test_number != 0 and test_number % 1000 == 0:
            print("{0:8} ({1} errors)".format(test_number, len(repos)))
        error_count = randint(1, setsize)
        recovery_words = mutate(original_words, error_count, corpus_size)
        if error_count <= max_error_count:
            try:
                recovered_keys = RecoverSecret(state, recovery_words, 1)
                if keys[0] != recovered_keys[0]:
                    repos.append(get_repo_dict(original_words, recovery_words, state, corpus_size))
            except FuzzyError:
                repos.append(get_repo_dict(original_words, recovery_words, state, corpus_size))
        else:
            try:
                RecoverSecret(state, recovery_words, 1)
                repos.append(get_repo_dict(original_words, recovery_words, state, corpus_size))
            except FuzzyError:
                pass
    if repos:
        print(len(repos), "errors")
        if save_path:
            with open(save_path, 'w') as fobj:
                print('saving repos to', save_path)
                fobj.write(json.dumps(repos, indent=2))
        else:
            save_repos(repos)
    else:
        print("no errors detected")

    # pylint: enable=too-many-locals

def get_file_name(suffix='.json') -> str:
    '''
    Prompt the user for the name of a file. If the file does not
    already exist, return the name. If the file already exists
    create a new name and return that
    '''
    txt = input("Enter path: ")
    if not os.path.isfile(txt):
        return txt
    h = hashids.Hashids(salt=str(datetime.datetime.now()))
    txt1 = h.encode(1, 2, 3) + suffix
    print("\"{0}\" already exists, using \"{1}\" instead ...".format(txt, txt1))
    return txt1

def save_repos(repos) -> None:
    'save the repos to a file'
    txt = input("You have {0} repos. Do you want to save them? ('y' or 'n'): ".format(len(repos)))
    if txt != 'y':
        return
    txt = get_file_name()
    if not txt:
        return
    with open(txt, 'w') as fobj:
        print('writing to', txt)
        fobj.write(json.dumps(repos, indent=4))

def get_repo_dict(original_words: List[int],
                  recovery_words: List[int],
                  state: FuzzyState,
                  corpus_size: int
                  ) -> Dict[str, Any]:
    'return a dictionary object containing the info needed for reproducing a problem'
    ans = {
        "original_words": original_words,
        "recovery_words": recovery_words,
        "corpus_size": corpus_size,
        "state": state.as_dict()
    }
    return ans

def repo_test(repo_dict: Dict[str, Any]) -> None:
    'reproduce a key recovery from a previous example'
    original_words = repo_dict['original_words']
    recovery_words = repo_dict['recovery_words']

    params = InputParams(setsize=repo_dict['state']['setsize'],
                         correctthreshold=repo_dict['state']['correctthreshold'],
                         corpus_size=repo_dict['corpus_size'])
    state, keys = GenerateSecret(params=params,
                                 original_words=original_words,
                                 key_count=1)
    try:
        recovered_keys = RecoverSecret(state, recovery_words, 1)
        width = get_width(repo_dict['corpus_size'])
        print("original_words:",
              repr_words(repo_dict['original_words'], width),
              "->", bytes_to_hex(keys[0]))
        print("recovery_words:",
              repr_words(repo_dict['recovery_words'], width),
              "->", bytes_to_hex(recovered_keys[0]))
    except FuzzyError as e:
        print(e.message)

def repo_from_json_string(json_string: str) -> None:
    "reproduce an example from a JSON string"
    repo_test(json.loads(json_string))

def repo_from_json_file(path: str, indx: int) -> None:
    "reproduce an error recored with test()"
    if os.path.isfile(path):
        print('\n\nRe-running from \"{0}\" ...\n\n'.format(path))
        with open(path) as fobj:
            data = json.load(fobj)
            repo_test(data[indx])
    else:
        print(path, "does not exist")


@click.command()
@click.option("--words",
              type=str,
              default="0 1 2 3 4 5 6 7 8",
              help="original words (default=\"0 1 2 3 4 5 6 7 8\"")
@click.option("--correct-threshold",
              type=int,
              default=6,
              help="minimum number of correct recovery words (default=6)")
@click.option("--corpus-size",
              type=int,
              default=7000,
              help="corpus size (default=7000)")
@click.option("--count",
              type=int,
              default=1,
              help="number of random tests (default=1)")
@click.option("--repo",
              type=str,
              default="repo.json",
              help="path to JSON repo file (default=\"repo.json\"")
def work(words: str, correct_threshold: int, corpus_size: int, count: int, repo: str) -> None:
    """
    Test by creating a secret and try to recover the secret by using random
    mutations of the original words. The result is compared with what is
    expected. If there is an odd result, it is recored in a JSON file
    that can later be used to run the offending example.
    """
    print("words:", words)
    print("correct-thrshold:", correct_threshold)
    print("corpus-size:", corpus_size)
    print("count:", count)
    print("repo:", repo)
    original_words = [int(word) for word in words.split()]

    input_dict = {
        "original_words": original_words,
        "correctthreshold": correct_threshold,
        "corpus_size": corpus_size,
        "test_count": count
    }
    start = time.time()
    trial(input_dict, repo)
    stop = time.time()
    print("{0:.1f} seconds for {1} iterations".format(stop - start, count))

if __name__ == '__main__':
    # pylint: disable=no-value-for-parameter
    work()
    # pylint: enable=no-value-for-parameter
