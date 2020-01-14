#!/bin/sh

set -e -o nounset

# Ordered list of directories to run.
# Yes, repeat the "network" test to try to catch all the variable memleaks.  :(
DIRS=""
DIRS="${DIRS} minimal simple network"
DIRS="${DIRS} qtest-minimal"
DIRS="${DIRS} network"
DIRS="${DIRS} qtest-gui-minimal qtest-gui-simple qtest-gui-complex"
DIRS="${DIRS} network"
DIRS="${DIRS} gui-simple"

DEBUG=0

### Find script directory.
scriptdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)

# Variables for valgrind use.
valgrind_suppressions="${scriptdir}/valgrind.supp"
valgrind_suppressions_this="thisdir.supp"
valgrind_log="thisdir.log"
valgrind_cmd="valgrind						\
	--leak-check=full --show-leak-kinds=all			\
	--suppressions=${valgrind_suppressions}			\
	--gen-suppressions=all					\
	--log-file=${valgrind_log}"

# Generate the suppressions for a specific command & its arguments.
generate_supp() {
	cmd=$1
	func=$2
	envvar=$3

	# Verbose output.
	if [ "$DEBUG" -eq 1 ]; then
		if [ -z "${func}" ]; then
			printf "\t(no args)\n"
		else
			printf "\t${func}\n"
		fi
	fi

	# Write name to the suppressions file.
	valgrind_suppressions_this=$( echo "thisdir-${func}.supp" |	\
		sed 's/-/_/g' | sed 's/ /_/g' )
	printf "# ${testdir} ${func}\n" > "${valgrind_suppressions_this}"

	# Generate suppressions arising from the specific function.
	env ${envvar} ${valgrind_cmd} ${run_cmd} ${func} > /dev/null

	# Strip out useless parts from the log file and remove it.
	(grep -v "^==" ${valgrind_log}				\
		| grep -v "^--"						\
		>> "${valgrind_suppressions_this}" ) || true

	# Strip out removing references to the main and constructors so that
	# the suppressions can apply to other binaries.  Append to suppressions
	# file.
	${scriptdir}/supp-generalize.py ${valgrind_suppressions_this}	\
		${valgrind_suppressions}

	# Clean up
	if [ ! "$DEBUG" -eq 1 ]; then
		rm -f "${valgrind_log}"
		rm -f "${valgrind_suppressions_this}"
	fi
}

generate_supp_with_funcs() {
	# Make suppressions for our "do nothing" function.
	generate_supp "${run_cmd}" "pl_nothing" ""

	# Make suppressions for each function.
	${run_cmd} | while read func; do				\
		if [ "z${func}" = "zpl_nothing" ]; then
			continue
		fi
		generate_supp "${run_cmd}" "${func}" ""
	done

	# Make suppressions for no arguments
	generate_supp "${run_cmd}" "" ""
}

generate_supp_for_qtest() {
	platform_env=$1

	# In the below lines, we deliberately don't use a space after
	# ${platform} so that there's no extra space if it's only "".

	# Make suppressions for our "do nothing" function.
	generate_supp "${run_cmd}" "pl_nothing" "${platform_env}"

	# Make suppressions for each function (other than "do nothing",
	# which was handled above).
	env ${platform_env} ${run_cmd} -functions		\
	    | sed 's/()//g'					\
	    | while read func; do				\
		if [ "z${func}" = "zpl_nothing" ]; then
			continue
		fi
		generate_supp "${run_cmd}" "${func}" "${platform_env}"
	done

	# Make suppressions for running the full binary.
	generate_supp "${run_cmd}" "" "${platform_env}"
}

generate_supp_from_dir() {
	testdir=$1
	cd ${testdir}

	# Ensure it's built.
	qmake
	nice make -j2 > /dev/null

	# Set the command to run.
	run_cmd="./$( grep "^TARGET" Makefile | awk '{print $3}' )"

	if test "${testdir#*qtest}" = "${testdir}"; then
		# Process dirs without "qtest"
		generate_supp_with_funcs "${run_cmd}"
	else
		# Make sure we can run the commands without a GUI.
		if test "${testdir#*-gui}" != "${testdir}"; then
			generate_supp_for_qtest "QT_QPA_PLATFORM=offscreen "
		else
			generate_supp_for_qtest ""
		fi
	fi

	cd ..
}


# Clear existing debug suppressions.
if [ "$DEBUG" -eq 1 ]; then
	rm -f ${valgrind_suppressions}.debug
fi

# If the suppressions already exist, indicate that this is a retest.
# (This may be useful while investigating occasional memory leaks.)
if [ -e ${valgrind_suppressions} ]; then
	printf "### RETEST\n" >> ${valgrind_suppressions}
else
	touch ${valgrind_suppressions}
fi

# Generate suppressions for each directory.
for dirn in ${DIRS}; do
	printf "${dirn}\n"
	generate_supp_from_dir ${dirn}
done

# Print stats about each type of suppression.
printf "\n"
grep -n "^# " ${valgrind_suppressions}
printf "$(wc -l ${valgrind_suppressions})\n"
