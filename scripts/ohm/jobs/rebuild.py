import logging

from scripts.ohm.jobs.build import build
from scripts.ohm.jobs.clean import clean
from scripts.onlinehelpmaker import NOTICE, call_logging_exceptions


def rebuild(options):
    logging.log(NOTICE, "Rebuilding")
    ret = call_logging_exceptions(clean, options)
    if ret != 0:
        return ret
    ret = call_logging_exceptions(build, options)
    return ret