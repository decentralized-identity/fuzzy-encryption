'''
Test for running pytest
'''

from fuzzy import InputParams

def test_input() -> None:
    '''
    Test that creating parameters works
    '''
    _ = InputParams(setsize=12,
                    correctthreshold=9,
                    corpus_size=7776
                    )