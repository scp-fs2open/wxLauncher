import logging
import os.path
import sys

import argparse

from ohm.version_file_builder import VersionFileBuilder


def main(argv):
    parser = argparse.ArgumentParser(
        description="Build .cpp using vcs tool for inclusion in application")
    parser.add_argument('job', choices=['build', 'rebuild', 'clean'])
    parser.add_argument('outfile', type=os.path.normpath,
                        help="output .cpp file")
    parser.add_argument('workfile', type=os.path.normpath,
                        help="Temp file")
    parser.add_argument("--gitpath",
                        help="name of git executable to work with."
                        "(default is 'git'",
                        default='git')

    options = parser.parse_args(argv)

    maker = VersionFileBuilder(options.workfile, options.outfile,
                               options.gitpath)

    logging.basicConfig(level=logging.INFO)

    if options.job == "build":
        maker.build()
    elif options.job == "rebuild":
        maker.rebuild()
    elif options.job == "clean":
        maker.clean()
    else:
        parser.error("Invalid JOB type. Use build, rebuild, or clean.")
        sys.exit(2)

    sys.exit(0)


if __name__ == "__main__":
    main(sys.argv[1:])
