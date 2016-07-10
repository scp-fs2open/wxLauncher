import atexit
import logging
import os
import shutil
import sys
import traceback
from optparse import OptionParser

from scripts.ohm.jobs.build import build
from scripts.ohm.jobs.clean import clean
from scripts.ohm.jobs.rebuild import rebuild

NOTICE = 25  # added level for app

from future import standard_library
standard_library.install_hooks()
try:
    import markdown
except ImportError:
    markdown = None
    print("ERROR: Unable to import markdown the markup parser.")
    print(" Make sure that markdown has been installed")
    print("  see the ReadMe.txt for more information")
    raise
standard_library.remove_hooks()

def main(argv):
    parser = OptionParser(usage="%prog <jobtype> <outfile> <indir> [options]")
    parser.add_option("-t", "--temp",
                      help="use TEMPDIR to store the intermedite build files",
                      metavar="TEMPDIR")
    parser.add_option("-q", "--quiet", action="store_true",
                      dest="quiet", default=False,
                      help="don't print most status messages to stdout")
    parser.add_option("-d", "--debug", action="store_true",
                      default=False, help="print debugging information to the screen")
    parser.add_option("-c", "--cfile", default=None, dest="carrayfilename",
                      metavar="FILE", help="filename to output the c array to. Defaults to same folder as outfile.")
    parser.add_option("-a", "--alwaysbuild", action="store_true",
                      default=False, dest="always_build",
                      help="when building, builder should always build source files. (Usally used with filesystems that do not update the modified time)")

    (options, args) = parser.parse_args(argv)

    if len(args) != 4:
        parser.error("Incorrect number of arguments")

    options.type = args[1]
    options.outfile = os.path.normpath(args[2])
    options.indir = os.path.normpath(args[3])

    loglevel = logging.INFO
    logging.addLevelName(NOTICE, "NOTICE")
    if options.quiet:
        loglevel = NOTICE
    elif options.debug:
        loglevel = logging.DEBUG

    logging.basicConfig(level=loglevel, format='%(levelname)7s:%(message)s')

    if options.type == "build":
        pass
    elif options.type == "rebuild":
        pass
    elif options.type == "clean":
        pass
    else:
        parser.error("jobtype must be one of: build, rebuild, or clean")

    if not options.temp:
        logging.info("No working directory set. Creating one in system temp.")
        options.temp = tempfile.mkdtemp()

        def cleanup(dir):
            if os.path.exists(dir):
                shutil.rmtree(options.temp, onerror=rmtree_error_handler)

        atexit.register(cleanup, options.temp)

    logging.debug("Doing a '%s'", options.type)
    logging.debug("Using '%s' as working directory", options.temp)
    logging.debug("Using '%s' as output file", options.outfile)
    logging.debug("Using '%s' as input directory", options.indir)

    if options.type == "build":
        ret = call_logging_exceptions(build, options)
    elif options.type == "rebuild":
        ret = call_logging_exceptions(rebuild, options)
    else:
        ret = call_logging_exceptions(clean, options)
    sys.exit(ret)


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


def enum_directories(dir):
    ret = os.path.dirname(dir).split(os.path.sep)
    if len(ret) == 1 and len(ret[0]) == 0:
        ret = list()
    logging.debug(" Enum directories %s", str(ret))
    return ret


if __name__ == "__main__":
    main(sys.argv)
