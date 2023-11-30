#!/usr/bin/env python3

""" Tarsnap statistics. """

import sys

_PRINT_STATS_FORMAT = "%-32s  %12s  %15s"


def _psf(a, b, c, use_stderr=False):
    """ Print with the normal `tarsnap --print-stats` format. """
    if use_stderr:
        out = sys.stderr
    else:
        out = sys.stdout
    print(_PRINT_STATS_FORMAT % (a, b, c), file=out)


class Stats:
    """ Tarsnap statistics. """
    def __init__(self, total_size, compressed_size, unique_total_size,
                 unique_compressed_size):
        self.total_size = total_size
        self.compressed_size = compressed_size
        self.unique_total_size = unique_total_size
        self.unique_compressed_size = unique_compressed_size

    def add(self, other_stats):
        """ Add other_stats to these stats. """
        self.total_size += other_stats.total_size
        self.compressed_size += other_stats.compressed_size
        self.unique_total_size += other_stats.unique_total_size
        self.unique_compressed_size += other_stats.unique_compressed_size

    def subtract(self, other_stats):
        """ Subtract other_stats from these stats. """
        self.total_size -= other_stats.total_size
        self.compressed_size -= other_stats.compressed_size
        self.unique_total_size -= other_stats.unique_total_size
        self.unique_compressed_size -= other_stats.unique_compressed_size

    def print(self, name: str, use_stderr=False):
        """ Print these stats in the generic `tarsnap --print-stats` format.
        """
        _psf("", "Total size", "Compressed size", use_stderr)
        _psf(name, self.total_size, self.compressed_size, use_stderr)
        _psf("  (unique data)", self.unique_total_size,
             self.unique_compressed_size, use_stderr)

    def print_cd_stats(self, mode, use_stderr=False):
        """ Print these stats in the `tarsnap --print-stats -c` format. """
        # We've already called print(), so no need for the header.
        _psf("This archive", self.total_size, self.compressed_size, use_stderr)
        if mode == "c":
            text = "New data"
        elif mode == "d":
            text = "Deleted data"
        _psf(text, self.unique_total_size, self.unique_compressed_size,
             use_stderr)
