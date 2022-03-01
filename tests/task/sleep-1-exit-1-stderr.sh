#!/bin/sh
printf "text on stderr\n" 1>&2
sleep 1
exit 1
