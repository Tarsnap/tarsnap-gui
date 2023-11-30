#!/usr/bin/env python3

""" Handle the fake tarsnap command-line interface. """

import os.path
import pickle
import random

import archive
import stats

NUM_ARCHIVES_MAX = 99999
NUM_ARCHIVES_MAX = 9

SERVER_STATE_FILENAME = "server.pickle"


class Tarsnap:
    """ Handle the fake tarsnap command-line interface. """
    def __init__(self, cachedir):
        self.state_filename = os.path.join(cachedir, SERVER_STATE_FILENAME)

        self.modified = False
        self.archives = None
        self.all_stats = None
        self.rand_obj = random.Random()
        self._load()

    def __del__(self):
        if self.modified:
            self._save()

    def _load(self):
        if os.path.exists(self.state_filename):
            with open(self.state_filename, "rb") as fp:
                data = pickle.load(fp)
                self.archives, self.all_stats, randstate = data
            self.rand_obj.setstate(randstate)
        else:
            self.archives = {}
            self.rand_obj.seed(0)
            self.all_stats = stats.Stats(0, 0, 0, 0)

    def _save(self):
        # Ensure that the directory exists.
        dirname = os.path.dirname(self.state_filename)
        if not os.path.exists(dirname):
            os.makedirs(dirname)

        # Write the current server state.
        with open(self.state_filename, "wb") as fp:
            data = (self.archives, self.all_stats, self.rand_obj.getstate())
            pickle.dump(data, fp)

    def mode_c(self, name, print_stats, filenames):
        """ Like `tarsnap -c` """
        if name in self.archives:
            print("Archive name %s already exists!" % name)
            exit(1)

        new_archive = archive.Archive(name, self.rand_obj, filenames)
        self.archives[name] = new_archive
        self.all_stats.add(new_archive.archive_stats)
        self.modified = True

        if print_stats:
            self.all_stats.print("All archives", use_stderr=True)
            new_archive.print_stats("c", use_stderr=True)

    def mode_d(self, name, print_stats):
        """ Like `tarsnap -d` """
        if name not in self.archives:
            print("Archive name %s does not exist!" % name)
            exit(1)

        deleted_archive = self.archives[name]
        self.all_stats.subtract(deleted_archive.archive_stats)
        self.archives.pop(name)
        self.modified = True

        if print_stats:
            self.all_stats.print("All archives", use_stderr=True)
            deleted_archive.print_stats("d", use_stderr=True)

    def list_archives(self):
        """ Like `tarsnap --list-archives` """
        for l_archive in self.archives.values():
            l_archive.print_list()

    def print_stats(self, archive_name, use_stderr=False):
        """ Like `tarsnap --print-stats [-f ARCHIVENAME]` (not -c or -d). """
        self.all_stats.print("All archives", use_stderr)
        if archive_name:
            f_archive = self.archives[archive_name]
            f_archive.print_stats("name", use_stderr)

    def t(self, archive_name):
        """ Like `tarsnap -t` """
        t_archive = self.archives[archive_name]
        for fileinfo in t_archive.filesinfo.values():
            print(fileinfo)
