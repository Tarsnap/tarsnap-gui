#!/usr/bin/env python3

""" Record info about a faked tarsnap archive. """

import pathlib
import subprocess

import stats


class Archive:
    """ Record info about a faked tarsnap archive. """
    def __init__(self, name, rand_obj, filenames):
        self.name = name
        self.cmd = "tarsnap -c -f %s /" % name
        self.date = "2020-01-01 %02i:%02i:%02i" % (
            rand_obj.randrange(0, 24),
            rand_obj.randrange(0, 60),
            rand_obj.randrange(0, 60))

        self.archive_stats = stats.Stats(0, 0, 0, 0)
        self.filenames = None
        self.filesinfo = None

        self.add_files(filenames)

    def add_files(self, filenames):
        """ Add paths to the archive (including recursing into
            sub-directories).  Use the real filesizes, but fake the
            compressed and unique sizes.
        """
        # Find all files (including recursing into sub-directories)
        explicit_paths = set()
        for filename in filenames:
            base_path = pathlib.Path(filename)
            extra_paths = set(base_path.rglob("*"))
            extra_paths.add(base_path)
            explicit_paths.update(extra_paths)

        # Get info about each path, and add them all to the object.
        self.filesinfo = {}
        self.filenames = []
        for path in explicit_paths:
            # Get real info about each path.
            filename = str(path)
            proc = subprocess.run(["ls", "-ld", filename],
                                  capture_output=True, check=True)
            if proc.returncode != 0:
                raise OSError("Problem with ls")
            fileinfo = proc.stdout.decode().strip()

            # Save the pathname and the output of `ls`.
            self.filenames.append(filename)
            self.filesinfo[filename] = fileinfo

            # Use the real size.
            size = int(fileinfo.split()[4])

            # Fake the compressed, unique, and unique compressed sizes.
            compressed = int(size * 0.3)
            unique = int(size * 0.1)
            unique_compressed = int(unique * 0.3)
            file_stats = stats.Stats(size, compressed, unique,
                                     unique_compressed)

            # Add the stats to the overall archive stats.
            self.archive_stats.add(file_stats)

    def print_list(self):
        """ Print info about this archive, like `tarsnap --list-archives`. """
        print("%s\t%s\t%s" % (self.name, self.date, self.cmd))

    def print_stats(self, mode, use_stderr=False):
        """ Print statistics about this archive. """
        if mode == "name":
            self.archive_stats.print(self.name, use_stderr)
        else:
            self.archive_stats.print_cd_stats(mode, use_stderr)
