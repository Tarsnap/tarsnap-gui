#!/bin/sh

DIRS=""
DIRS="${DIRS} minimal simple complex"
DIRS="${DIRS} qtest-minimal"
DIRS="${DIRS} qtest-gui-minimal qtest-gui-simple qtest-gui-complex"
DIRS="${DIRS} qtest-gui-network qtest-gui-ui"

for d in ${DIRS}; do
	cd $d
	qmake
	make distclean
	rm -f *.supp
	rm -f *.log
	cd ..
done
