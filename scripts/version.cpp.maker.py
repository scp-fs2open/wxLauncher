from optparse import OptionParser
import os.path
import sys

from version_file_builder import VersionFileBuilder

WORKFILE=3
OUTFILE=2
JOB=1

def main(argv):
  parser = OptionParser(usage="%prog <jobtype> <outfile> <workfile> [options]")
  
  parser.add_option("", "--gitpath",
    help="use GITPATH as the executable that will be used to generate the version.cpp file.  Defaults to hg",
    metavar="GITPATH", default="git")

  (options, args) = parser.parse_args(argv)
  
  if len(args) != 4:
    parser.error("Incorrect number of arguments")
  
  work = os.path.normcase(os.path.normpath(args[WORKFILE]))
  file = os.path.normcase(os.path.normpath(args[OUTFILE]))
  
  maker = VersionFileBuilder(work, file, options.gitpath)
  
  if args[JOB] == "build":
    maker.build()
  elif args[JOB] == "rebuild":
    maker.rebuild()
  elif args[JOB] == "clean":
    maker.clean()
  else:
    parser.error("Invalid JOB type. Use build, rebuild, or clean.")
    sys.exit(2)
  
  sys.exit(0)

if __name__ == "__main__":
  main(sys.argv)
