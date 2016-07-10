import logging
import os
import shutil
import sys

from scripts.onlinehelpmaker import NOTICE, rmtree_error_handler


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