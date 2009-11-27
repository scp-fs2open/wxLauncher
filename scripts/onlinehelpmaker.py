import sqlite3, os, sys, tempfile, atexit, traceback, shutil
from optparse import OptionParser

import HTMLParser

class DebugParser(HTMLParser.HTMLParser):
  def handle_starttag(self, tag, attrs):
    print " Handle starttag: %s" %(tag)
    print "  %s"%(attrs)

  def handle_startendtag(self, tag, attrs):
    print " Handle start and end tag: %s" % (tag)
    print "  %s"%(attrs)

  def handle_endtag(self, tag):
    print " End tag: %s" %(tag)

  def handle_data(self, data):
    print " Data: %s" %( data)

class OutputParser(HTMLParser.HTMLParser):
  """The class is designed to be used as a base class.  It will output the same html structure as the input file into a file like object (only needs write)."""
  class Tag:
    def __init__(self, name, attrs, data=None):
      self.name = name
      self.attributes = attrs
      self.data = data
      
  def __init__(self, file, *args, **kwargs):
    HTMLParser.HTMLParser.__init__(self, *args, **kwargs)
    
    if file.hasattr(file, 'write'):
      self.outputfile = file
    else:
      raise Exception("file is not a file like object with a write function")
    
    self.tagstack = list()
  
  def handle_starttag(self, tag, attrs):
    self.tagstack.append(Tag(tag, attrs))
  def handle_startendtag(self, tag, attrs):
    self.write_tag(Tag(tag, attrs))
  def handle_endtag(self, tagname):
    """Finds the matching tag. If the tags are miss matched, function will go down the stack until it finds the match"""
    tag = self.find_match(tagname)
    
    self.write_tag(tag)
  def find_match(self, tagtofind, indent=0):
    """Recursive function to match the tag"""
    tag = self.tagstack.pop()
    if not tag.name == tagname:
      print "WARNING: %smismatched tag found (expected %d, found %s)" %(" "*indent, tagtofind, tag.name)
      self.write_tag(tag)
      tag = self.find_match(tagtofind)
      
    return tag
  def write_tag(self, tag):
    """Writes tag to file passed in the constructor."""
    if tag.data == None:
      self.write("<%s%s />" %(tag.name, self.format_attributes(tag)))
    else:
      self.write("<%s%s>%s</%s>" %(
        tag.name, self.format_attributes(tag), tag.data, tag.name))
  def format_attributes(self, tag):
    """Returns the attributes formatted to be placed in a tag."""
    ret = ""
    if len(tag.attributes) > 0:
      for name, value in tag.attributes:
        ret = "%s %s=\"%s\"" % (ret, name, value)
    return ret
try:
  import markdown
except ImportError:
  print "ERROR: Unable to import markdown the markup parser."
  print " Make sure that markdown has been installed"
  print "  see the ReadMe.txt for more information"

def main(argv):
  parser = OptionParser(usage="%prog <jobtype> <outfile> <indir> [options]")
  parser.add_option("-t", "--temp",
    help="use TEMPDIR to store the intermedite build files",
    metavar="TEMPDIR")
  parser.add_option("-q", "--quiet", action="store_false",
    dest="quiet", default=False,
    help="don't print status messages to stdout")

  (options, args) = parser.parse_args(argv)

  if  len(args) != 4 :
    parser.error("Incorrect number of arguments")

  options.type = args[1]
  options.outfile = args[2]
  options.indir = args[3]

  if options.type == "build":
    pass
  elif options.type == "rebuild":
    pass
  elif options.type == "clean":
    pass
  else:
    parser.error("jobtype must be one of: build, rebuild, or clean")

  if not options.temp:
    if not options.quiet:
      print "Not working directory set. Creating one in system temp."
    options.temp = tempfile.mkdtemp()

    def cleanup(dir):
      if os.path.exists(dir):
        os.rmdir(dir)

    atexit.register(cleanup, options.temp)


  if not options.quiet:
    print "Doing a '%s'" % ( options.type )
    print "Using '%s' as working directory" % ( options.temp )
    print "Using '%s' as output file" % ( options.outfile )
    print "Using '%s' as input directory" %( options.indir )

  if options.type == "build":
    build(options)
  elif options.type == "rebuild":
    rebuild(options)
  else:
    clean(options)

def rebuild(options):
  print "Rebuilding"
  try:
    clean(options)
  except:
    traceback.print_exc()
  try:
    build(options)
  except:
    traceback.print_exc()

def clean(options):
  print "Cleaning.."
  if not options.quiet:
    print "Removing outfile: %s" % ( options.outfile )
  if os.path.exists(options.outfile):
    if os.path.isfile(options.outfile):
      os.remove(options.outfile)
    else:
      print "ERROR: outfile is not a file. Make sure your parameters are in the correct order"
      sys.exit(2)
  else:
    if not options.quiet:
      print " Outfile (%s) does not exist" % (options.outfile)

  if not options.quiet:
    print "Removing workdirectory: %s" % ( options.temp )
  if os.path.exists(options.temp):
    if os.path.isdir(options.temp):
      shutil.rmtree(options.temp, ignore_errors=True)
    else:
      print "ERROR: tempdir is not a file. Make sure your parameters are in the correct order"
      sys.exit(2)
  else:
    if not options.quiet:
      print " Work directory (%s) does not exist" % (options.temp)


def build(options):
  """Compiles the files in options.indir to the archive output options.outfile.

  Compiled in several stages:
    stage1: Transform all input files with markdown placing the results into options.temp+"/stage1".
    stage2: Parses and strips the output of stage1 to build the c-array that contains the compiled names for the detailed help.
    stage3: Parses the output of stage1 to grab the images that are refered to in the the output of stage1
    stage4: Parses the output of stage1 to fix the relative hyperlinks in the output so that they will refer correctly to the correct files when in output file.
    stage5: Generate the index and table of contents for the output file.
    stage6: Zip up the output of stage5 and put it in the output location.
    """
  files = generate_paths(options)

  if should_build(options, files):
    input_files = generate_input_files_list(options)

    helparray = list()
    for file in input_files:
      process_input_stage1(file, options, files, helparray)

    print helparray


def generate_paths(options):
  """Generates the names of the paths that will be needed by the compiler during it's run, storing them in a dictionary that is returned."""
  paths = dict()

  for i in range(1, 7):
    paths['stage%d'%(i)] = os.path.join(options.temp, 'stage%d'%(i))

  for path in paths.values():
    if not os.path.exists(path):
      print " Making %s" % (path)
      os.makedirs(path)

  return paths

def should_build(options, files):
  """Checks the all of the files touched by the compiler to see if we need to rebuild. Returns True if so."""
  return True

def generate_input_files_list(options):
  """Returns a list of all input (.help) files that need to be parsed by markdown."""
  file_list = list()

  for path, dirs, files in os.walk(options.indir):
    for file in files:
      if os.path.splitext(file)[1] == ".help":
        # I only want the .help files
        file_list.append(os.path.join(path, file))

  if not options.quiet:
    print " Input files:"
    for file in file_list:
      print "  %s"%(file)
  return file_list
  
def change_filename(filename, newext, orginaldir, destdir):
  """Returns the filename after transforming it to be in destdir and making sure the folders required all exist."""
  outfile_name1 = filename.replace(orginaldir, ".") # files relative name
  outfile_name2 = os.path.splitext(outfile_name1)[0] #file's name without ext
  outfile_name3 = outfile_name2 + newext
  outfile_name4 = os.path.join(destdir, outfile_name3)
  outfile_name = os.path.normpath(outfile_name4)
  
  # make sure that the folder exists to output 
  outfile_path = os.path.dirname(outfile_name)
  if os.path.exists(outfile_path):
    if os.path.isdir(outfile_path):
      pass # do nothing because the folder already exists
    else:
      raise Exception("%s already exists but is not a directory"%(outfile_path))
  else:
    os.makedirs(outfile_path)
    
  return outfile_name

def process_input_stage1(file, options, files, helparray):
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()

  output = markdown.markdown(input)

  outfile_name = change_filename(file, ".stage1", files['stage1'], options.indir)
  
  outfile = open(outfile_name, mode="w")
  outfile.write(output)
  outfile.close()
  process_input_stage2(outfile_name, options, files, helparray)
  process_input_stage3(outfile_name, options, files)

def process_input_stage2(file, options, files, helparray):
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()

  class Stage2Parser(HTMLParser.HTMLParser):
    def __init__(self):
      HTMLParser.HTMLParser.__init__(self)
      self.control = None

    def handle_starttag(self, tag, attrs):
      if tag == "meta":
        if attrs[0][0] == "name" and attrs[0][1] == "control" and attrs[1][0] == "content":
          self.control = attrs[1][1]

  parser = Stage2Parser()
  parser.feed(input)
  parser.close()

  if parser.control:
    helparray.append((parser.control, file))
    if not options.quiet:
      print " Control name %s"%(parser.control)

def process_input_stage3(file, options, files):
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()

  class Stage3Parser(HTMLParser.HTMLParser):
    def __init__(self, options, files):
      HTMLParser.HTMLParser.__init__(self)
      self.options = options
      self.files = files

    def handle_startendtag(self, tag, attrs):
      """Find the image and copy it to the stage3 folder where it should
      be in the file output."""
      if tag == "img":
        location1 = os.path.join(self.options.indir, attrs[1][1])
        location = os.path.normpath(location1)

        # check to make sure that the image I am including was in the onlinehelp folder,
        # if not change the dst name so that it will be located correctly in the stage3 directory
        if location.startswith(self.options.indir):
          dst1 = os.path.join(self.files['stage3'], attrs[1][1])
        else:
          # get extention
          basename = os.path.basename(attrs[1][1])
          (name, ext) = os.path.split(basename)
          (file, outname) = tempfile.mkstemp(ext, name, self.files['stage3'])
          dst1 = outname.replace(os.getcwd(), ".") # make into a relative path

        dst = os.path.normpath(dst1)
        print " Image (%s) should be in %s and copying to %s" %( attrs[0][1], location, dst)
        try:
          shutil.copy2(location, dst)
        except:
          traceback.print_exc()
          print "ERROR: '%s' does not exist" % ( location )
          sys.exit(3)

  parser = Stage3Parser(options, files)
  parser.feed(input)
  parser.close()

if __name__ == "__main__":
  main(sys.argv)


