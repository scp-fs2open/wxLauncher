import logging

from .build import build
from .clean import clean
from ..jobs import call_logging_exceptions


def rebuild(options):
    notices = logging.getLogger('notices')
    notices.info("Rebuilding")
    ret = call_logging_exceptions(clean, options)
    if ret != 0:
        return ret
    ret = call_logging_exceptions(build, options)
    return ret