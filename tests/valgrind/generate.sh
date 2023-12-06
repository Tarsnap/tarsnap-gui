#!/bin/sh

set -e -o nounset

# Ordered list of directories to run.
# Yes, repeat the "console-plain-network" test to try to catch all the
# occasional memleaks.  :(
DIRS_C=""
DIRS_C="${DIRS_C} console-plain-minimal console-plain-simple"
DIRS_C="${DIRS_C} console-plain-network"
DIRS_C="${DIRS_C} console-qtest-minimal"
DIRS_C="${DIRS_C} console-plain-network"
DIRS_G=""
DIRS_G="${DIRS_G} gui-plain-simple"
DIRS_G="${DIRS_G} gui-plain-network"
DIRS_G="${DIRS_G} gui-qtest-minimal gui-qtest-simple"
DIRS_G="${DIRS_G} gui-qtest-lineedit"
DIRS_G="${DIRS_G} gui-qtest-network"
DIRS_G="${DIRS_G} gui-qtest-complex"
DIRS_G="${DIRS_G} gui-qtest-weird"
if [ "${VALGRIND_ONLY_CONSOLE:-0}" -gt "0" ] ; then
	DIRS="${DIRS_C}"
else
	DIRS="${DIRS_C} ${DIRS_G}"
fi

# XDG directory
XDGD="/tmp/tarsnap-gui-test/valgrind"

DEBUG=0

### Find script directory.
scriptdir=$(CDPATH="" cd -- "$(dirname -- "$0")" && pwd -P)

# Variables for valgrind use.
valgrind_suppressions="${scriptdir}/valgrind.supp"
valgrind_cmd="valgrind						\
	--leak-check=full --show-leak-kinds=all			\
	--suppressions=${valgrind_suppressions}			\
	--num-callers=40					\
	--leak-check-heuristics=none				\
	--errors-for-leak-kinds=all				\
	--gen-suppressions=all"

# Generate the suppressions for a specific command & its arguments.
generate_supp() {
	cmd=$1
	func=$2

	# Environment variables that we use in tests
	envvar="XDG_CONFIG_HOME=${XDGD} XDG_CACHE_HOME=${XDGD} XDG_DATA_HOME=${XDGD} XDG_RUNTIME_DIR=${XDGD}"

	# There's no harm having an extra environment variable for the
	# command-line-only tests.
	envvar="${envvar} QT_QPA_PLATFORM=offscreen"

	# Verbose output.
	if [ "$DEBUG" -eq 1 ]; then
		if [ -z "${func}" ]; then
			printf "\t(no args)\n"
		else
			printf "\t%s\n" "${func}"
		fi
	fi

	# Set up function-specific variables.
	valgrind_log_this="thisdir-${func}.log"
	valgrind_suppressions_this="thisdir-${func}.supp"
	valgrind_cmd_this="${valgrind_cmd} --log-file=${valgrind_log_this}"

	# Write name to the suppressions file.
	printf "# %s %s\n" "${testdir}" "${func}"			\
		> "${valgrind_suppressions_this}"

	# Generate suppressions arising from the specific function.
	# shellcheck disable=SC2086
	env ${envvar} ${valgrind_cmd_this} ${cmd} ${func} > /dev/null

	# Strip out useless parts from the log file and remove it.
	(grep -v "^==" "${valgrind_log_this}"				\
		| grep -v "^--"						\
		>> "${valgrind_suppressions_this}" ) || true

	# Strip out removing references to the main and constructors so that
	# the suppressions can apply to other binaries.  Append to suppressions
	# file.
	"${scriptdir}/supp-generalize.py"				\
		"${valgrind_suppressions_this}" "${valgrind_suppressions}"

	# Clean up
	if [ ! "$DEBUG" -eq 1 ]; then
		rm -f "${valgrind_log_this}"
		rm -f "${valgrind_suppressions_this}"
	fi
}

generate_supp_with_funcs() {
	# Make suppressions for our "do nothing" function.
	generate_supp "${run_cmd}" "pl_nothing"

	# Make suppressions for each function.
	${run_cmd} | while read -r func; do				\
		if [ "z${func}" = "zpl_nothing" ]; then
			continue
		fi
		generate_supp "${run_cmd}" "${func}"
	done

	# Make suppressions for no arguments
	generate_supp "${run_cmd}" ""
}

generate_supp_for_qtest() {
	# Make suppressions for our "do nothing" function.
	generate_supp "${run_cmd}" "pl_nothing"

	# Make suppressions for each function (other than "do nothing",
	# which was handled above).
	${run_cmd} -functions					\
	    | sed 's/()//g'					\
	    | while read -r func; do				\
		if [ "z${func}" = "zpl_nothing" ]; then
			continue
		fi
		generate_supp "${run_cmd}" "${func}"
	done

	# Make suppressions for running the full binary.
	generate_supp "${run_cmd}" ""
}

generate_supp_from_dir() {
	testdir=$1
	cd "${testdir}"

	# Ensure it's built.
	qmake
	nice make -j2 > /dev/null

	# Set the command to run.
	run_cmd="./$( grep "^TARGET" Makefile | awk '{print $3}' )"

	if test "${testdir#*qtest}" = "${testdir}"; then
		# Process dirs without "qtest"
		generate_supp_with_funcs "${run_cmd}"
	else
		# Process dirs with "qtest"
		generate_supp_for_qtest ""
	fi

	cd ..
}


# Clear existing debug suppressions.
if [ "$DEBUG" -eq 1 ]; then
	rm -f "${valgrind_suppressions}.debug"
fi

# Create XDG dir (if necessary)
mkdir -m 0700 -p "${XDGD}"

# If the suppressions already exist, indicate that this is a retest.
# (This may be useful while investigating occasional memory leaks.)
if [ -e "${valgrind_suppressions}" ]; then
	printf "### RETEST\n" >> "${valgrind_suppressions}"
else
	touch "${valgrind_suppressions}"
fi

# Generate suppressions for each directory.
for dirn in ${DIRS}; do
	printf "%s\n" "${dirn}"
	generate_supp_from_dir "${dirn}"
done

# Print stats about each type of suppression.
printf "\n"
grep -n "^# " "${valgrind_suppressions}"
printf "%s\n" "$(wc -l "${valgrind_suppressions}")"
