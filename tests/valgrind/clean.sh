#!/bin/sh

set -e

DIRS=""
DIRS="${DIRS} console-plain-minimal console-plain-simple console-plain-network"
DIRS="${DIRS} console-qtest-minimal"
DIRS="${DIRS} qtest-gui-minimal qtest-gui-simple qtest-gui-complex"
DIRS="${DIRS} qtest-gui-lineedit qtest-gui-network"
DIRS="${DIRS} qtest-gui-weird"
DIRS="${DIRS} gui-simple gui-network"

for d in ${DIRS}; do
	cd "$d"
	qmake
	make distclean
	rm -f ./*.supp
	rm -f ./*.log
	cd ..
done
