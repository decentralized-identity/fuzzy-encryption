#!/bin/bash
rm -f inputex.json
python3 cparams.py --set-size 9 --correct-threshold 6 --corpus-size 7776 > inputex.json
../../../../build/src/c++/tests/loadrand/loadrand
