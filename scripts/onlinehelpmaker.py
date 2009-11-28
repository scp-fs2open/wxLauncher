import sqlite3, os, sys, tempfile, atexit, traceback, shutil
from optparse import OptionParser

import logging
NOTICE = 25

import HTMLParser

class Tag:
  def __init__(self, name, attrs, data=None):
    self.name = name
    self.attributes = attrs
    self.data = data
    
class OutputParser(HTMLParser.HTMLParser):
  """The class is designed to be used as a base class.  It will output the same html structure as the input file into a file like object (only needs write)."""
      
  def __init__(self, file, *args, **kwargs):
    HTMLParser.HTMLParser.__init__(self, *args, **kwargs)
    
    if hasattr(file, 'write'):
      self.outputfile = file
    else:
      raise Exception("file is not a file like object with a write function")
    
    self.tagstack = list()
  
  def handle_starttag(self, tag, attrs):
    logging.debug(" Handle starttag: %s", tag)
    logging.debug("  %s", attrs)
    self.tagstack.append(Tag(tag, attrs))
  def handle_startendtag(self, tag, attrs):
    logging.debug(" Handle start and end tag: %s", tag)
    logging.debug("  %s", attrs)
    self.write_tag(Tag(tag, attrs))
  def handle_endtag(self, tagname):
    """Finds the matching tag. If the tags are miss matched, function will go down the stack until it finds the match"""
    logging.debug(" End tag: %s", tagname)
    tag = self.find_match(tagname)
    self.write_tag(tag)
  def handle_data(self, data):
    logging.debug(" Data: %s", data)
    if len(self.tagstack) > 0:
      tag = self.tagstack.pop()
      if tag.data == None:
        tag.data = data
      else:
        tag.data += data
      self.tagstack.append(tag)
    else:
      self.outputfile.write(data)
  def find_match(self, tagtofind, indent=0):
    """Recursive function to match the tag"""
    tag = self.tagstack.pop()
    if not tag.name == tagtofind:
      logging.warning(" %smismatched tag found (expected %d, found %s)", " "*indent, tagtofind, tag.name)
      self.write_tag(tag)
      tag = self.find_match(tagtofind)
      
    return tag
  def write_tag(self, tag):
    """When tag stack is empty, writes tag to file passed in the constructor. When tag stack is not empty formats the tag and sets (or if the next tag.data is not None, appends) the formated string."""
    if tag.data == None:
      str = "<%s%s />" %(tag.name, self.format_attributes(tag))
    else:
      str = "<%s%s>%s</%s>" %(
        tag.name, self.format_attributes(tag), tag.data, tag.name)
        
    if len(self.tagstack) > 0:
      tag = self.tagstack.pop()
      if tag.data == None:
        tag.data = str
      else:
        tag.data += str
      self.tagstack.append(tag)
    else:
      self.outputfile.write(str)
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
  raise

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

  (options, args) = parser.parse_args(argv)

  if  len(args) != 4 :
    parser.error("Incorrect number of arguments")

  options.type = args[1]
  options.outfile = args[2]
  options.indir = args[3]
  
  loglevel=logging.INFO
  logging.addLevelName(NOTICE, "NOTICE")
  if options.quiet:
    loglevel=NOTICE
  elif options.debug:
    loglevel=logging.DEBUG
  
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
        os.rmdir(dir)

    atexit.register(cleanup, options.temp)

  logging.info("Doing a '%s'", options.type)
  logging.info("Using '%s' as working directory", options.temp )
  logging.info("Using '%s' as output file", options.outfile )
  logging.info("Using '%s' as input directory", options.indir )

  if options.type == "build":
    call_logging_exceptions(build, options)
  elif options.type == "rebuild":
    call_logging_exceptions(rebuild, options)
  else:
    call_logging_exceptions(clean, options)

def call_logging_exceptions(func, *args, **kwargs):
  """Protects the caller from all exceptions that occur in the callee. Logs the exceptions that do occur."""
  try:
    func(*args, **kwargs)
  except:
    ( type, value, tb ) = sys.exc_info()
    logging.error("***EXCEPTION:")
    logging.error(" %s: %s", type.__name__, value)
    for filename, line, function, text in traceback.extract_tb(tb):
      logging.error("%s:%d %s", os.path.basename(filename), line, function)
      logging.error("   %s", text)
    
def rebuild(options):
  logging.log(NOTICE, "Rebuilding")
  call_logging_exceptions(clean, options)
  call_logging_exceptions(build, options)

def clean(options):
  logging.log(NOTICE, "Cleaning..")
  logging.info("Removing outfile: %s", options.outfile )
  if os.path.exists(options.outfile):
    if os.path.isfile(options.outfile):
      os.remove(options.outfile)
    else:
      logging.error(" <outfile> is not a file. Make sure your parameters are in the correct order")
      sys.exit(2)
  else:
    logging.info(" Outfile (%s) does not exist", options.outfile)

  logging.info("Removing workdirectory: %s", options.temp )
  if os.path.exists(options.temp):
    if os.path.isdir(options.temp):
      shutil.rmtree(options.temp, ignore_errors=True)
    else:
      logging.error("tempdir is not a file. Make sure your parameters are in the correct order")
      sys.exit(2)
  else:
    logging.info(" Work directory (%s) does not exist", options.temp)


def build(options):
  """Compiles the files in options.indir to the archive output options.outfile.

  Compiled in several stages:
    stage1: Transform all input files with markdown placing the results into options.temp+"/stage1".
    stage2: Parses and strips the output of stage1 to build the list that will be made into the c-array that contains the compiled names for the detailed help of each control.
    stage3: Parses the output of stage2 to grab the images that are refered to in the the output of stage1
    stage4: Parses the output of stage1 to fix the relative hyperlinks in the output so that they will refer correctly to the correct files when in output file.
    stage5: Generate the index and table of contents for the output file from the output of stage4.
    stage6: Zip up the output of stage5 and put it in the output location.
    """
  logging.log(NOTICE, "Building...")
  files = generate_paths(options)

  if should_build(options, files):
    input_files = generate_input_files_list(options)

    helparray = list()
    logging.info(" Processing input files:")
    for file in input_files:
      logging.info("  %s", file)
      logging.info("   Stage 1")
      name1 = process_input_stage1(file, options, files)
      
      logging.info("   Stage 2")
      name2 = process_input_stage2(name1, options, files, helparray)
      
      logging.info("   Stage 3")      
      name3 = process_input_stage3(name2, options, files)
      
      logging.info("   Stage 4")
      name4 = process_input_stage4(name3, options, files)
      
    
    logging.info("   Stage 5")
    process_input_stage5(options, files, list())

    print helparray


def generate_paths(options):
  """Generates the names of the paths that will be needed by the compiler during it's run, storing them in a dictionary that is returned."""
  paths = dict()

  for i in range(1, 7):
    paths['stage%d'%(i)] = os.path.join(options.temp, 'stage%d'%(i))

  for path in paths.values():
    if not os.path.exists(path):
      logging.debug(" Making %s", path)
      os.makedirs(path)

  return paths

def should_build(options, files):
  """Checks the all of the files touched by the compiler to see if we need to rebuild. Returns True if so."""
  return True

def generate_input_files_list(options):
  """Returns a list of all input (.help) files that need to be parsed by markdown."""
  return generate_file_list(options.indir, ".help")

def generate_file_list(directory, extension):
  file_list = list()

  for path, dirs, files in os.walk(directory):
    for file in files:
      if os.path.splitext(file)[1] == extension:
        # I only want the .help files
        file_list.append(os.path.join(path, file))
  
  return file_list
  
def change_filename(filename, newext, orginaldir, destdir, makedirs=True):
  """Returns the filename after transforming it to be in destdir and making sure the folders required all exist."""
  logging.debug("   change_filename('%s', '%s', '%s', '%s', %s)", filename, newext, orginaldir, destdir, makedirs)
  outfile_name1 = filename.replace(orginaldir, ".") # files relative name
  logging.debug(outfile_name1)
  outfile_name2 = os.path.splitext(outfile_name1)[0] #file's name without ext
  logging.debug(outfile_name2)
  outfile_name3 = outfile_name2 + newext
  logging.debug(outfile_name3)
  outfile_name4 = os.path.join(destdir, outfile_name3)
  logging.debug(outfile_name4)
  outfile_name = os.path.normpath(outfile_name4)
  logging.debug(outfile_name)
  
  # make sure that the folder exists to output, if wanted
  if makedirs:
    outfile_path = os.path.dirname(outfile_name)
    if os.path.exists(outfile_path):
      if os.path.isdir(outfile_path):
        pass # do nothing because the folder already exists
      else:
        raise Exception("%s already exists but is not a directory"%(outfile_path))
    else:
      os.makedirs(outfile_path)
    
  return outfile_name

def process_input_stage1(file, options, files):
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()

  output = markdown.markdown(input)

  outfile_name = change_filename(file, ".stage1", options.indir, files['stage1'])
  
  outfile = open(outfile_name, mode="w")
  outfile.write(output)
  outfile.close()
  return outfile_name

def process_input_stage2(file, options, files, helparray):
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()
  
  outname = change_filename(file, ".stage2", files['stage1'], files['stage2'])
  outfile = open(outname, mode="w")

  class Stage2Parser(OutputParser):
    def __init__(self, *args, **kwargs):
      OutputParser.__init__(self, *args, **kwargs)
      self.control = None

    def handle_starttag(self, tag, attrs):
      if tag == "meta":
        if attrs[0][0] == "name" and attrs[0][1] == "control" and attrs[1][0] == "content":
          self.control = attrs[1][1]
      else:
        OutputParser.handle_starttag(self, tag, attrs)

  parser = Stage2Parser(file=outfile)
  parser.feed(input)
  parser.close()
  outfile.close()

  if parser.control:
    filename_in_archive = change_filename(outname, ".htm", files['stage2'], ".")
    helparray.append((parser.control, filename_in_archive))
    logging.debug(" Control name %s", parser.control)
      
  return outname

def process_input_stage3(file, options, files):
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()

  class Stage3Parser(OutputParser):
    def __init__(self, options, files, *args, **kwargs):
      OutputParser.__init__(self, *args, **kwargs)
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
          (name, ext) = os.path.splitext(basename)
          (file, outname) = tempfile.mkstemp(ext, name, self.files['stage3'])
          dst1 = outname.replace(os.getcwd(), ".") # make into a relative path

        dst = os.path.normpath(dst1)
        attrs = update_attribute(attrs, 'src', change_filename(dst, os.path.splitext(dst)[1], self.files['stage3'], ".", makedirs=False))
        logging.debug(" Image (%s) should be in %s and copying to %s", attrs[0][1], location, dst)
        try:
          shutil.copy2(location, dst)
        except:
          traceback.print_exc()
          logging.error(" '%s' does not exist", location )
          sys.exit(3)
      OutputParser.handle_startendtag(self, tag, attrs)

  outname = change_filename(file, ".stage3", files['stage2'], files['stage3'])
  outfile = open(outname, mode="w")
  parser = Stage3Parser(options, files, file=outfile)
  parser.feed(input)
  parser.close()
  outfile.close()

  return outname

def update_attribute(attributes, name, value):
  """Finds the attribute name and sets it to value in the attributes tuple of tuples, returns the attributes tuple of tuples with the changed attribute."""
  ret = list()
  # find name in attributes
  for n, v in attributes:
    if n == name:
      ret.append((name, value))
    else:
      ret.append((n,v))
  return tuple(ret)
  
def process_input_stage4(file, options, files):
  """Fix the a tags so that they point to the htm files that will be in the output archive."""
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()
  
  outname = change_filename(file, ".stage4", files['stage3'], files['stage4'])
  outfile = open(outname, mode="w")

  class Stage4Parser(OutputParser):
    def __init__(self, files, *args, **kwargs):
      OutputParser.__init__(self, *args, **kwargs)
      self.files = files
      
    def handle_starttag(self, tag, attrs):
      if tag == "a":
        for name,value in attrs:
          if name == "href" and not value.startswith("http://"):
            # make sure the file being refered to exists in stage3
            check_ref = change_filename(value, ".stage3", ".", files['stage3'], makedirs=False)
            if not os.path.exists(check_ref):
              logging.warning(" File (%s) does not exist to be bundled into archive!", check_ref)
              
            fixed_ref = change_filename(value, ".htm", ".", ".", makedirs=False)
            attrs = update_attribute(attrs, "href", fixed_ref)
      
      OutputParser.handle_starttag(self, tag, attrs)

  parser = Stage4Parser(files=files, file=outfile)
  parser.feed(input)
  parser.close()
  outfile.close()  
  
  return outname
  
def process_input_stage5(options, files, extrafiles):
  """Generate the index and table of contents for the output file from the output of stage4."""
  class Stage5Parser(OutputParser):
    def __init__(self, *args, **kwargs):
      OutputParser.__init__(self, *args, **kwargs)
      self.title = "Untitled"
    def handle_endtag(self, tag):
      if tag == "title":
        t = self.tagstack.pop()
        self.title = t.data
        self.tagstack.append(t)
      OutputParser.handle_endtag(self, tag)
  
  # write header file
  headerfile_name = os.path.join(files['stage5'], "header.hhp")
  headerfile = open(headerfile_name, mode="w")
  headerfile.write(
  """Contents file=%(contents)s\nIndex file=%(index)s\nTitle=%(title)s\nDefault topic=%(default)s\nCharset=UTF-8\n""" %
  { "contents": "contents.hhc",
  "index": "index.hhk",
  "title": "wxLauncher User Guide",
  "default": "index.htm"
  })
  headerfile.close()
  
  # generate both index and table of contents
  tocfile_name = os.path.join(files['stage5'], "contents.hhc")
  tocfile = open(tocfile_name, mode="w")
  tocfile.write("<ul>\n")
  toclevel = 1
  
  indexfile_name = os.path.join(files['stage5'], "index.hhk")
  indexfile = open(indexfile_name, mode="w")
  indexfile.write("<ul>\n")
  
  help_files = generate_file_list(files['stage4'], ".stage4")
  level = 0
  last_level = 0
  for file in help_files:
    # find the title
    outfile_name = change_filename(file, ".htm", files['stage4'], files['stage5'])
    outfile = open(outfile_name, mode="w")
    
    infile = open(file, mode="r")
    input = infile.read()
    infile.close()
    
    parser = Stage5Parser(file=outfile)
    parser.feed(input)
    parser.close()
    outfile.close()
    
    # relativize filename for being in the archive
    filename_in_archive = change_filename(file, ".htm", files['stage4'], ".", False)
    
    level = len(enum_directories(filename_in_archive))
    logging.debug(" Level %d\tLast Level %d" % (level, last_level))
    if level > last_level:
      # increased level
      tocfile.write(
      """%(tab)s<li> <object type="text/sitemap">\n%(tab)s\t<param name="Name" value="%(name)s">\n%(tab)s\t<param name="Local" value="%(file)s">\n%(tab)s</object>\n%(tab)s\t<ul>\n""" % {
      "tab": "\t"*toclevel,
      "name": enum_directories(filename_in_archive).pop(),
      "file": "index.htm" })
      toclevel += 1
    elif level < last_level:
      # decreased level
      tocfile.write("""%(tab)s</ul>\n""" % { tab: "\t"*toclevel })
      toclevel -= 1
    last_level = level
    tocfile.write(
    """%(tab)s<li> <object type="text/sitemap">\n%(tab)s\t<param name="Name" value="%(name)s">\n%(tab)s\t<param name="Local" value="%(file)s">\n%(tab)s </object>\n""" % {
    "tab": "\t"*toclevel,
    "name": parser.title,
    "file": filename_in_archive, })
    
    indexfile.write(
    """%(tab)s<li> <object type="text/sitemap">\n%(tab)s\t<param name="Name" value="%(name)s">\n%(tab)s\t<param name="Local" value="%(file)s">\n%(tab)s </object>\n""" % {
    "tab": "\t",
    "name": parser.title,
    "file": filename_in_archive, })
    
  while toclevel > 0:
    tocfile.write("%s</ul>\n" % ( "\t"*(toclevel-1)))
    toclevel -= 1
  
  
  tocfile.close()
  indexfile.write("</ul>")
  indexfile.close()
  
def enum_directories(dir):
  ret = os.path.dirname(dir).split(os.path.sep)
  if len(ret) == 1 and len(ret[0]) == 0:
    ret = list()
  logging.debug(" Enum directories %s", str(ret))
  return ret
  
if __name__ == "__main__":
  main(sys.argv)


