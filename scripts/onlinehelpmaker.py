import argparse
import atexit
import logging
import os
import shutil
import sys
import tempfile

from ohm.utilfunctions import rmtree_error_handler
from ohm.jobs import call_logging_exceptions
from ohm.jobs.build import build

try:
    import markdown
except ImportError:
    markdown = None
    print("ERROR: Unable to import markdown the markup parser.")
    print(" Make sure that markdown has been installed")
    print("  see the ReadMe.txt for more information")
    raise


def main(argv):
    parser = argparse.ArgumentParser(
        description="Build the online help database")
    parser.add_argument("-q", "--quiet", action="store_const",
                        dest="quiet", default=logging.INFO,
                        const=logging.WARNING,
                        help="don't print most status messages to stdout")
    parser.add_argument("-d", "--debug", action="store_const",
                        default=logging.INFO, const=logging.DEBUG,
                        help="print debugging information to the screen")

    type_subparsers = parser.add_subparsers(title='operation')

    bld_ps = type_subparsers.add_parser('build',
                                        help='Assemble help database')
    bld_ps.set_defaults(function=build, type="build")
    bld_ps.add_argument('type', choices=["build", "rebuild", "clean"])
    bld_ps.add_argument('outfile', type=os.path.normpath,
                        help="location to write .htb file to")
    bld_ps.add_argument('indir', type=os.path.normcase,
                        help="path that contains the info to put the OUTFILE")
    bld_ps.add_argument("-t", "--temp",
                        help="directory for intermediate build files"
                        "(Uses system temp directory by default)",
                        metavar="TEMPDIR")
    bld_ps.add_argument("-c", "--cfile", default=None, dest="carrayfilename",
                        metavar="FILE",
                        help="file to put the htb index in (c code)")
    bld_ps.add_argument("-a", "--always", action="store_true",
                        default=False, dest="always_build",
                        help="builder should always build source files")

    args = parser.parse_args(argv)

    console_format = logging.Formatter(fmt='%(levelname)7s:%(message)s')
    console = logging.StreamHandler()
    console.setLevel(args.debug)

    console.setFormatter(console_format)
    logger = logging.getLogger('')
    logger.setLevel(args.debug)
    logger.addHandler(console)

    notices_console = logging.StreamHandler()
    notices_console.setFormatter(console_format)

    notices_logger = logging.getLogger('notices')
    notices_logger.setLevel(args.quiet)
    notices_logger.addHandler(notices_console)
    notices_logger.propagate = False

    if not args.temp:
        logging.info("No working directory set. Creating one in system temp.")
        args.temp = tempfile.mkdtemp()

        def cleanup(dir):
            if os.path.exists(dir):
                shutil.rmtree(args.temp, onerror=rmtree_error_handler)

        atexit.register(cleanup, args.temp)

    if 'function' in args:
        ret = args.function(args)
    else:
        ret = 1
    sys.exit(ret)


def enum_directories(dir):
    ret = os.path.dirname(dir).split(os.path.sep)
    if len(ret) == 1 and len(ret[0]) == 0:
        ret = list()
    logging.debug(" Enum directories %s", str(ret))
    return ret


if __name__ == "__main__":
    main(sys.argv[1:])
