#!/bin/sh

set -e

DIRS=""
DIRS="${DIRS} console-plain-minimal console-plain-simple console-plain-network"
DIRS="${DIRS} console-qtest-minimal"
DIRS="${DIRS} gui-plain-simple gui-plain-network"
DIRS="${DIRS} gui-qtest-minimal gui-qtest-simple gui-qtest-complex"
DIRS="${DIRS} gui-qtest-lineedit gui-qtest-network"
DIRS="${DIRS} gui-qtest-weird"

for d in ${DIRS}; do
	cd "$d"
	qmake
	make distclean
	rm -f ./*.supp
	rm -f ./*.log
	cd ..
done
