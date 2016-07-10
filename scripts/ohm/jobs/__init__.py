import logging
import os
import sys
import traceback


def call_logging_exceptions(func, *args, **kwargs):
    """Protects the caller from all exceptions that occur in the callee. Logs the exceptions that do occur."""
    try:
        func(*args, **kwargs)
    except:
        (type, value, tb) = sys.exc_info()
        logging.error("***EXCEPTION:")
        logging.error(" %s: %s", type.__name__, value)
        for filename, line, function, text in traceback.extract_tb(tb):
            logging.error("%s:%d %s", os.path.basename(filename), line, function)
            logging.error("   %s", text)

        del tb
        return 1
    return 0