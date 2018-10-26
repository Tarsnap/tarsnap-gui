#!/bin/sh

DIRS="minimal simple complex"
DIRS="${DIRS} qtest-minimal"

for d in ${DIRS}; do
	cd $d
	qmake
	make distclean
	rm -f *.supp
	rm -f *.log
	cd ..
done
