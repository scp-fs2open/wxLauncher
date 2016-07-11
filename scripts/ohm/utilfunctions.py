from __future__ import (absolute_import, division,
                        print_function, unicode_literals)

import traceback

from builtins import *

import logging
import os
import os.path


def update_attribute(attributes, name, value):
    """Update attribute name to value
    :param attributes: list of tuples to update
    :param name: name to update
    :param value: value to update name to
    :return: attributes with updated tuple
    :rtype: tuple
    """
    ret = list()
    # find name in attributes
    for n, v in attributes:
        if n == name:
            ret.append((name, value))
        else:
            ret.append((n, v))
    return tuple(ret)


def split_filename_and_anchor(filename):
    parts = filename.split("#")
    if len(parts) > 2:
        raise Exception("Was passed filename with two (or more) # in it")
    elif len(parts) == 2:
        file = parts[0]
        anchor = parts[1]
    else:
        file = parts[0]
        anchor = None
    return tuple([file, anchor])


def change_filename(filename, newext, orginaldir, destdir, makedirs=True):
    """Twiddles URL like paths into filesystem paths
    :param filename: path to twiddle
    :param newext: new extention for filename
    :param orginaldir: original directory filename is currently in
    :param destdir: directory to prepare to receive filename
    :param makedirs: create intermediate directories in destdir
    :return: the twiddled filename
    """
    filename = os.path.normpath(filename)
    filename, anchor = split_filename_and_anchor(filename)
    if len(filename) != 0:
        orginaldir = os.path.normpath(orginaldir)
        destdir = os.path.normpath(destdir)
        logging.debug("   change_filename('%s', '%s', '%s', '%s', %s)",
                      filename, newext, orginaldir, destdir, makedirs)
        outfile_name1 = filename.replace(orginaldir, ".")  # files relative name
        logging.debug("%s", outfile_name1)
        if newext == None:
            outfile_name3 = outfile_name1
        else:
            outfile_name2 = os.path.splitext(outfile_name1)[
                0]  # file's name without ext
            logging.debug("%s", outfile_name2)
            outfile_name3 = outfile_name2 + newext
        logging.debug("%s", outfile_name3)
        outfile_name4 = os.path.join(destdir, outfile_name3)
        logging.debug("%s", outfile_name4)
        outfile_name = os.path.normpath(outfile_name4)
        logging.debug("%s", outfile_name)

        # make sure that the folder exists to output, if wanted
        if makedirs:
            outfile_path = os.path.dirname(outfile_name)
            if os.path.exists(outfile_path):
                if os.path.isdir(outfile_path):
                    pass  # do nothing because the folder already exists
                else:
                    raise Exception(
                        "%s already exists but is not a directory" % (
                        outfile_path))
            else:
                os.makedirs(outfile_path)
    else:
        outfile_name = filename

    if anchor == None:
        return outfile_name
    else:
        return "#".join([outfile_name, anchor])


def make_directory_for_filename(filename):
    """Returns False if directory for filename already exists, True if
    function was successfully able to create the directory, or raises
    and exception for the error."""
    path = os.path.dirname(filename)
    if os.path.exists(path):
        if os.path.isdir(path):
            return False
        else:
            raise ExistsButNotDirectoryError(path)
    else:
        # must be a permissions issue if we end up here
        try:
            os.makedirs(path)
            return True
        except:
            raise


class ExistsButNotDirectoryError(IOError):
    def __init__(self, path="Not Specified"):
        self.path = path

    def __str__(self):
        return "Path (%s) exists but is not a directory!"


def rmtree_error_handler(function, path, excinfo):
    if function == os.remove:
        logging.warning("  Unable to remove %s", path)
    elif function == os.rmdir:
        logging.warning("  Unable to remove directory %s", path)
    else:
        (type, value, tb) = excinfo
        logging.error("***EXCEPTION:")
        logging.error(" %s: %s", type.__name__, value)
        for filename, line, function, text in traceback.extract_tb(tb):
            logging.error("%s:%d %s", os.path.basename(filename), line, function)
            logging.error("   %s", text)