#!/bin/sh
grep "##" scenario-??.txt | sed -s 's/##//' > list-scenarios.txt

NUM_SCENARIOS=`ls scenario-??.txt | wc -l`
# not all versions of sed support -i, so we need a tmp file
sed -sE "s/NUM_SCENARIOS = .*;$/NUM_SCENARIOS = $NUM_SCENARIOS;/" \
	scenario-num.h > scenario-num.h.tmp
mv scenario-num.h.tmp scenario-num.h
