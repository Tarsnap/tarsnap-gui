#!/bin/sh

DIRS="minimal simple complex"
DIRS="${DIRS} qtest-minimal"
DIRS="${DIRS} qtest-gui-minimal qtest-gui-simple qtest-gui-complex"

for d in ${DIRS}; do
	cd $d
	qmake
	make distclean
	rm -f *.supp
	rm -f *.log
	cd ..
done
