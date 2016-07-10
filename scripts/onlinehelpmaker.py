import argparse
import atexit
import logging
import os
import shutil
import sys

# hook imports so that the markdown library works on 2.6
from future import standard_library
standard_library.install_hooks()

from ohm.jobs import call_logging_exceptions
from ohm.jobs.build import build
from ohm.jobs.clean import clean, rmtree_error_handler
from ohm.jobs.rebuild import rebuild

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
    parser = argparse.ArgumentParser(
        description="Build/rebuild/clean the online help database")
    parser.add_argument('type', choices=["build", "rebuild", "clean"])
    parser.add_argument('outfile', type=os.path.normpath,
                        help="location to write .htb file to")
    parser.add_argument('indir', type=os.path.normcase,
                        help="path that contains the info to put the OUTFILE")
    parser.add_argument("-t", "--temp",
                        help="directory for intermediate build files"
                        "(Uses system temp directory by default)",
                        metavar="TEMPDIR")
    parser.add_argument("-q", "--quiet", action="store_const",
                        dest="quiet", default=logging.INFO,
                        const=logging.WARNING,
                        help="don't print most status messages to stdout")
    parser.add_argument("-d", "--debug", action="store_const",
                        default=logging.INFO, const=logging.DEBUG,
                        help="print debugging information to the screen")
    parser.add_argument("-c", "--cfile", default=None, dest="carrayfilename",
                        metavar="FILE",
                        help="file to put the htb index in (c code)")
    parser.add_argument("-a", "--always", action="store_true",
                        default=False, dest="always_build",
                        help="builder should always build source files")

    options = parser.parse_args(argv)

    console_format = logging.Formatter(fmt='%(levelname)7s:%(message)s')
    console = logging.StreamHandler()
    console.setLevel(options.debug)

    console.setFormatter(console_format)
    logger = logging.getLogger('')
    logger.setLevel(options.debug)
    logger.addHandler(console)

    notices_console = logging.StreamHandler()
    notices_console.setFormatter(console_format)

    notices_logger = logging.getLogger('notices')
    notices_logger.setLevel(options.quiet)
    notices_logger.addHandler(notices_console)
    notices_logger.propagate = False

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


def enum_directories(dir):
    ret = os.path.dirname(dir).split(os.path.sep)
    if len(ret) == 1 and len(ret[0]) == 0:
        ret = list()
    logging.debug(" Enum directories %s", str(ret))
    return ret


if __name__ == "__main__":
    main(sys.argv[1:])
