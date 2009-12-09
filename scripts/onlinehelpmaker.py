import sqlite3, os, sys, tempfile, atexit, traceback, shutil, zipfile
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
  parser.add_option("-c", "--cfile", default=None, dest="carrayfilename", 
  metavar="FILE", help="filename to output the c array to. Defaults to same folder as outfile.")
  parser.add_option("-a", "--alwaysbuild", action="store_true",
    default=False, dest="always_build",
    help="when building, builder should always build source files. (Usally used with filesystems that do not update the modified time)")

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
        shutil.rmtree(options.temp, onerror=rmtree_error_handler)

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
      
    del tb
    
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
      shutil.rmtree(options.temp, onerror=rmtree_error_handler)
    else:
      logging.error("tempdir is not a file. Make sure your parameters are in the correct order")
      sys.exit(2)
  else:
    logging.info(" Work directory (%s) does not exist", options.temp)

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
    extrafiles = list()
    logging.log(NOTICE, " Processing input files:")
    for file in input_files:
      logging.log(NOTICE, "  %s", file)
      logging.info("   Stage 1")
      name1 = process_input_stage1(file, options, files)
      
      logging.info("   Stage 2")
      name2 = process_input_stage2(name1, options, files, helparray)
      
      logging.info("   Stage 3")      
      name3 = process_input_stage3(name2, options, files, extrafiles)
      
      logging.info("   Stage 4")
      name4 = process_input_stage4(name3, options, files)
      
    
    logging.info(" Stage 5")
    process_input_stage5(options, files, extrafiles)
    
    logging.info(" Stage 6")
    process_input_stage6(options, files)
    
    logging.info(" Generating .cpp files")
    generate_cpp_files(options, files, helparray)
    
    logging.log(NOTICE, "....Done.")

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
  if options.always_build:
    return True
  elif not os.path.exists(options.outfile):
    return True
  elif check_source_newer_than_outfile(options, files):
    return True
  return False
  
def check_source_newer_than_outfile(options, files):
  """Checks to see if any file in the source directory is newer than the output file."""
  try:
    outfile_time = os.path.getmtime(options.outfile)
    
    for dirpath, dirnames, filenames in os.walk(options.indir):
      for file in filenames:
        filepath = os.path.join(dirpath, file)
        if os.path.getmtime(filepath) > outfile_time:
          logging.info("%s has been changed since outfile. Causing build", filepath)
          return True
        elif os.path.getctime(filepath) > outfile_time:
          logging.info("%s has been created since outfile. Causing build", filepath)
          return True
    
  except OSError:
    logging.warning("Encountered a file (%s) that the os does not like. Forcing build.", "TODO")
    return True
  return False
    

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
  if newext == None:
    outfile_name3 = outfile_name1
  else:
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
    
    def handle_startendtag(self, tag, attrs):
      if tag == "meta":
        if attrs[0][0] == "name" and attrs[0][1] == "control" and attrs[1][0] == "content":
          self.control = attrs[1][1]
      else:
        OutputParser.handle_startendtag(self, tag, attrs)

  parser = Stage2Parser(file=outfile)
  parser.feed(input)
  parser.close()
  outfile.close()

  if parser.control:
    filename_in_archive = change_filename(outname, ".htm", files['stage2'], ".", makedirs=False)
    helparray.append((parser.control, filename_in_archive))
    logging.debug(" Control name %s", parser.control)
      
  return outname

def process_input_stage3(file, options, files, extrafiles):
  infile = open(file, mode="r")
  input = infile.read()
  infile.close()

  class Stage3Parser(OutputParser):
    def __init__(self, options, files, extrafiles, *args, **kwargs):
      OutputParser.__init__(self, *args, **kwargs)
      self.options = options
      self.files = files
      self.extrafiles = extrafiles

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
        self.extrafiles.append(dst)
      OutputParser.handle_startendtag(self, tag, attrs)

  outname = change_filename(file, ".stage3", files['stage2'], files['stage3'])
  outfile = open(outname, mode="w")
  parser = Stage3Parser(options, files, file=outfile, extrafiles=extrafiles)
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
      elif tag == "h1" and self.title == "Untitled":
        # make first h1 tag found into the title, will be overwritten by a title tag if it exists.
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

  last_path_list = []
  for path, dirs, thefiles in os.walk(files['stage4']):
    """new directory. If the directory is not the base directory then check if there will be an index.htm for this directory.
    If there is an index.htm then parse it for the title so that the subsection will have the title of the index.htm as the name of the subsection.
    If there is no index.htm then make a default one and use the name of the directory as the subsection name.
    
    Note that this algorithm requires that os.walk generates the names in alphabetical order."""
    # relativize directory path for being in the archive
    path_in_arc = make_path_in_archive(path, files['stage4'])
    logging.debug("Processing directory '%s'", path_in_arc)
    
    path_list = path_in_arc.split(os.path.sep)
    if len(path_list) == 1 and path_list[0] == '':
      path_list = []
    level = len(path_list)
    
    # parse the index.help to get the name of the section
    index_file_name = os.path.join(path, "index.stage4")
    # relativize filename for being in the archive
    index_in_archive = change_filename(index_file_name, ".htm", files['stage4'], ".", False)
    index_in_archive = index_in_archive.replace(os.path.sep, "/") # make the separators the same so that it doesn't matter what platform the launcher is built on.
    # find the title
    outindex_name = change_filename(index_file_name, ".htm", files['stage4'], files['stage5'])
    outindex = open(outindex_name, mode="w")
    
    inindex = open(index_file_name, mode="r")
    input = inindex.read()
    inindex.close()
    
    parser = Stage5Parser(file=outindex)
    parser.feed(input)
    parser.close()
    outindex.close()
    
    tocfile.write(generate_sections(path_list, last_path_list,index_filename=index_in_archive, section_title=parser.title))
    last_path_list = path_list
    
    if level > 0:
      # remove index.htm from thefiles because they are used by the section names
      try:
        thefiles.remove('index.stage4')
      except ValueError:
        logging.warning("Directory %s does not have an index.help", path_in_arc)
    
    for file in thefiles:
      # relativize filename for being in the archive
      filename_in_archive = change_filename(file, ".htm", files['stage4'], ".", False)
      # find the title
      outfile_name = change_filename(file, ".htm", files['stage4'], files['stage5'])
      outfile = open(outfile_name, mode="w")
      
      infile_name = os.path.join(path, file)
      infile = open(infile_name, mode="r")
      input = infile.read()
      infile.close()
      
      parser = Stage5Parser(file=outfile)
      parser.feed(input)
      parser.close()
      outfile.close()
      
      tocfile.write(
      """%(tab)s<li> <object type="text/sitemap">\n%(tab)s     <param name="Name" value="%(name)s">\n%(tab)s     <param name="Local" value="%(file)s">\n%(tab)s    </object>\n""" % {
      "tab": "     "*level,
      "name": parser.title,
      "file": filename_in_archive, })
      
      indexfile.write(
      """%(tab)s<li> <object type="text/sitemap">\n%(tab)s\t<param name="Name" value="%(name)s">\n%(tab)s\t<param name="Local" value="%(file)s">\n%(tab)s </object>\n""" % {
      "tab": "\t",
      "name": parser.title,
      "file": filename_in_archive, })
  
  tocfile.write(generate_sections([], last_path_list))
  tocfile.write("</ul>\n")
  tocfile.close()
  indexfile.close()
  
  # copy the extra files (i.e. images) from stage3
  for extrafilename in extrafiles:
    logging.debug(" Copying: %s", extrafilename)
    dst = change_filename(extrafilename, None, orginaldir=files['stage3'], destdir=files['stage5'])
    shutil.copy2(extrafilename, dst)

def generate_sections(path_list, last_path_list, basetab=0, orginal_path_list=None, index_filename=None, section_title=None):
  """Return the string that will allow me to write in the correct section."""
  logging.debug("   generate_sections(%s, %s, %d, %s)", str(path_list), str(last_path_list), basetab, orginal_path_list)
  
  if orginal_path_list == None:
    orginal_path_list = path_list
    
  if len(path_list) > 0 and len(last_path_list) > 0 and path_list[0] == last_path_list[0]:
    logging.debug("    matches don't need to do anything")
    return generate_sections(path_list[1:], last_path_list[1:], basetab+1, orginal_path_list, index_filename, section_title)
    
  elif len(path_list) > 0 and len(last_path_list) > 0:
    logging.debug("    go down then up")
    s = generate_sections([], last_path_list, basetab, orginal_path_list, index_filename, section_title)
    s += generate_sections(path_list, [], basetab, orginal_path_list, index_filename, section_title)
    return s
    
  elif len(path_list) > 0 and len(last_path_list) == 0:
    logging.debug("    go up (deeper)")
    if index_filename == None:
      raise Exception("index_filename is None")
    if section_title == None:
      title = path_list[len(path_list)-1]
    else:
      title = section_title
      
    s = ""
    for path in path_list:
      s += """%(tab)s<li> <object type="text/sitemap">\n%(tab)s     <param name="Name" value="%(name)s">\n%(tab)s     <param name="Local" value="%(file)s">\n%(tab)s    </object>\n%(tab)s     <ul>\n""" % {
      "tab": "     "*basetab,
      "name": title,
      "file": index_filename }
    
    return s
  elif len(path_list) == 0 and len(last_path_list) > 0:
    logging.debug("    do down")
    s = ""
    basetab += len(last_path_list)
    for path in last_path_list:
      s += """%(tab)s</ul>\n""" % { 'tab': "     "*(basetab) }
      basetab -= 1
    return s
   
  elif len(path_list) == 0 and len(last_path_list) == 0:
    logging.debug("    both lists empty, doing nothing")
    return ''
    
  else:
    raise Exception("Should never get here")
    return ""
    
def make_path_in_archive(path, path1):
  """Return the part of the path that is in 'path' but not in 'path1'"""
  path = os.path.normpath(path)
  path1 = os.path.normpath(path1)
  
  list = path.split(os.path.sep)
  list1 = path1.split(os.path.sep)
  
  return os.path.sep.join(make_path_in_archive_helper(list, list1))
  
def make_path_in_archive_helper(list, list1):
  if len(list) > 0 and len(list1) > 0 and list[0] == list1[0]:
    return make_path_in_archive_helper(list[1:], list1[1:])
  elif len(list) > 0 and len(list1) == 0:
    return os.path.join(list)
  else:
    return []
    
def process_input_stage6(options, stage_dirs):
  #make sure that the directories exist before creating file
  outfile_path = os.path.dirname(options.outfile)
  if os.path.exists(outfile_path):
    if os.path.isdir(outfile_path):
      pass
    else:
      log.error(" %s exists but is not a directory", outfile_path)
  else:
    os.makedirs(outfile_path)
    
  outzip = zipfile.ZipFile(options.outfile, mode="w", compression=zipfile.ZIP_DEFLATED)
  
  for path, dirs, files in os.walk(stage_dirs['stage5']):
    for filename in files:
      full_filename = os.path.join(path, filename)
      arcname = change_filename(full_filename, None, stage_dirs['stage5'], ".", False)
      logging.debug(" Added %s as %s", full_filename, arcname)
      outzip.write(full_filename, arcname)
  
  outzip.close()
    
def enum_directories(dir):
  ret = os.path.dirname(dir).split(os.path.sep)
  if len(ret) == 1 and len(ret[0]) == 0:
    ret = list()
  logging.debug(" Enum directories %s", str(ret))
  return ret
  
def generate_cpp_files(options, files, helparray):
  if options.carrayfilename == None:
    defaultname = os.path.join(os.path.dirname(options.outfile), "helplinks.cpp")
    logging.info("Filename for c-array has not been specified.")
    logging.info(" using default %s" % (defaultname))
    options.carrayfilename = defaultname
  
  logging.log(NOTICE, "Writing to %s", options.carrayfilename)
  outfile = open(options.carrayfilename, mode="w")
  outfile.write("// Generated by scripts/onlinehelpmaker.py\n")
  outfile.write("// GENERATED FILE - DO NOT EDIT\n")
  for id, location in helparray:
    outfile.write("""{%s,_T("%s")},\n""" % 
      ( id, location.replace(os.path.sep, "/")))
  
  outfile.close()
  
if __name__ == "__main__":
  main(sys.argv)


