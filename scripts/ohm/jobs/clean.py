import logging
import os
import shutil
import sys
import traceback

NOTICE = 25


def clean(options):
    logging.log(NOTICE, "Cleaning..")
    logging.info("Removing outfile: %s", options.outfile)
    if os.path.exists(options.outfile):
        if os.path.isfile(options.outfile):
            os.remove(options.outfile)
        else:
            logging.error(" <outfile> is not a file. Make sure your parameters are in the correct order")
            sys.exit(2)
    else:
        logging.info(" Outfile (%s) does not exist", options.outfile)

    logging.info("Removing workdirectory: %s", options.temp)
    if os.path.exists(options.temp):
        if os.path.isdir(options.temp):
            shutil.rmtree(options.temp, onerror=rmtree_error_handler)
        else:
            logging.error("tempdir is not a file. Make sure your parameters are in the correct order")
            sys.exit(2)
    else:
        logging.info(" Work directory (%s) does not exist", options.temp)


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