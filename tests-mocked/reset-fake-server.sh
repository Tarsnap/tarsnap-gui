#!/bin/sh

set -e

# Number of archives to add
NUM=5

# Set up the script dir and the fake tarsnap command
scriptdir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)

# Where to store the fake server data
cachedir="${scriptdir}/appdata/cache"
cmd="${scriptdir}/fake-cmdline/tarsnap --cachedir ${cachedir}"

# Remove previous server state
rm -f "${scriptdir}/appdata/cache/server.pickle"

# Add some archives
i=0
while [ "$i" -lt "${NUM}" ]; do
	num=$(printf "%02i" "$i")
	${cmd} --print-stats -c -f archivename-"${num}"		\
	    "${scriptdir}/fake-cmdline"
	i=$((i + 1))
done

# Print various info
${cmd} --print-stats
${cmd} --print-stats -f archivename-00
${cmd} --list-archives
${cmd} -tf archivename-00

# Delete an archive
${cmd} --print-stats -d -f archivename-01
