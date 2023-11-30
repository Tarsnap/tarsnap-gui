#!/usr/bin/env python3

""" A fake tarsnap command-line interface. """

import argparse

import tarsnap


IGNORE_NOARG = ["--no-default-config", "--no-humanize-numbers", "-P",
                "--quiet", "-v"]
IGNORE_ARG = ["--creationtime", "--keyfile"]


def parse_cmdline():
    """ Parse the command line. """
    parser = argparse.ArgumentParser(
        prog="tarsnap",
        description="Fake tarsnap CLI for testing purposes")

    # Ignore these arguments; they're needed to fake being a real tarsnap CLI,
    # but irrelevant to this fake one.
    for ignore in IGNORE_NOARG:
        parser.add_argument(ignore, action="store_true", help="IGNORED")
    for ignore in IGNORE_ARG:
        parser.add_argument(ignore, type=str, help="IGNORED")

    # Required for all non-version modes.
    parser.add_argument("--cachedir", metavar="FILENAME", type=str,
                        help="File to store persistent data")

    # Archive name.
    parser.add_argument("-f", metavar="ARCHIVE_NAME", type=str,
                        help="Archive name")

    # Only one of these commands?
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-c", action="store_true",
                       help="Create a fake archive")
    group.add_argument("-d", action="store_true",
                       help="Delete a fake archive")
    group.add_argument("--version", action="store_true",
                       help="Print a fake version number")
    group.add_argument("--list-archives", action="store_true",
                       help="List fake archives")
    group.add_argument("-t", action="store_true",
                       help="Print file info")

    # This is both a command mode, and a stand-alone option.
    parser.add_argument("--print-stats", action="store_true",
                        help="Print fake statistics")

    # Filenames.
    parser.add_argument("filenames", type=str, nargs="*",
                        help="Filenames to archive")

    # Parse
    args = parser.parse_args()

    return args


def main():
    """ A fake tarsnap command-line interface. """
    # Parse the command line.
    args = parse_cmdline()

    # Special case for early exit.
    if args.version:
        print("tarsnap 1.0.40")
        exit(0)

    # Otherwise, required argument(s).
    if not args.cachedir:
        print("--cachedir is required")
        exit(1)

    ts = tarsnap.Tarsnap(args.cachedir)

    if args.c:
        ts.mode_c(args.f, args.print_stats, args.filenames)
        # We've done this already
        args.print_stats = False

    # Handle the commands
    if args.list_archives:
        ts.list_archives()
    if args.t:
        assert args.f
        ts.t(args.f)
    if args.d:
        ts.mode_d(args.f, args.print_stats)
        # We've done this already
        args.print_stats = False
    if args.print_stats:
        ts.print_stats(args.f)
