import logging
import os
import os.path
import tempfile
import traceback
import sys
import shutil

from six.moves import html_parser

from dataclasses import Tag
from utilfunctions import update_attribute, change_filename


class OutputParser(html_parser.HTMLParser):
  """The class is designed to be used as a base class.  It will output the same html structure as the input file into a file like object (only needs write)."""
      
  def __init__(self, file, *args, **kwargs):
    html_parser.HTMLParser.__init__(self, *args, **kwargs)
    
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
      logging.warning(" %smismatched tag found (expected %s, found %s)", " "*indent, tagtofind, tag.name)
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

class Stage2Parser(OutputParser):
  def __init__(self, *args, **kwargs):
    OutputParser.__init__(self, *args, **kwargs)
    self.control = []

  def handle_starttag(self, tag, attrs):
    if tag == "meta":
      if attrs[0][0] == "name" and attrs[0][1] == "control" and attrs[1][0] == "content":
        self.control.append(attrs[1][1])
    else:
      OutputParser.handle_starttag(self, tag, attrs)
  
  def handle_startendtag(self, tag, attrs):
    if tag == "meta":
      if attrs[0][0] == "name" and attrs[0][1] == "control" and attrs[1][0] == "content":
        self.control.append(attrs[1][1])
    else:
      OutputParser.handle_startendtag(self, tag, attrs)

class Stage3Parser(OutputParser):
  def __init__(self, options, files, extrafiles, subdir, *args, **kwargs):
    OutputParser.__init__(self, *args, **kwargs)
    self.options = options
    self.files = files
    self.extrafiles = extrafiles
    self.subdir = subdir
    logging.debug(" Subdirectory is '%s'", subdir)

  def handle_startendtag(self, tag, attrs):
    """Find the image and copy it to the stage3 folder where it should
    be in the file output."""
    ALT_INDEX = None
    if tag == "img":
      # figure out which attribute is src
      for x in range(0, len(attrs)):
        if attrs[x][0] == "src":
          SRC_INDEX = x
        elif attrs[x][0] == "alt":
          ALT_INDEX = x

      if attrs[SRC_INDEX][1].startswith("/"): 
        # manual wants an absolute path, the help manual does not support
        # absolute path, so make sure that the image exists where the
        # absolute path indicates, then make the path into a relative path
        # with the approriate number of updirs
        test = os.path.join(self.options.indir, attrs[SRC_INDEX][1][1:])
        if not os.path.exists(test):
          raise IOError("Cannot find %s in base path"%(attrs[SRC_INDEX][1]))
        
        # try find a valid relative path
        subdirdepth = len(self.subdir.split(os.path.sep))
        prefix = "../"*subdirdepth
        relpath = os.path.join(prefix, attrs[SRC_INDEX][1][1:])
        if not os.path.exists(os.path.join(self.options.indir, self.subdir,relpath)):
          raise Exception("Cannot relativize path: %s"%(attrs[SRC_INDEX][1]))
        else:
          attrs = update_attribute(attrs, 'src', relpath)
      
      location1 = os.path.join(self.options.indir, self.subdir, attrs[SRC_INDEX][1])
      location = os.path.normpath(location1)

      # check to make sure that the image I am including was in the onlinehelp
      # folder, if not change the dst name so that it will be located
      # correctly in the stage3 directory
      logging.debug("%s - %s", location, self.options.indir)
      if location.startswith(self.options.indir):
        dst1 = os.path.join(self.files['stage3'], self.subdir, attrs[SRC_INDEX][1])
        dst = os.path.normpath(dst1)
      else:
        # get extention
        basename = os.path.basename(attrs[SRC_INDEX][1])
        (name, ext) = os.path.splitext(basename)
        (file, outname) = tempfile.mkstemp(ext, name, self.files['stage3'])
        dst1 = outname.replace(os.getcwd(), ".") # make into a relative path

        # fix up attrs
        dst = os.path.normpath(dst1)
        attrs = update_attribute(attrs, 'src', change_filename(dst, os.path.splitext(dst)[1], self.files['stage3'], ".", makedirs=False))
        
      if ALT_INDEX is None:
        ALT_INDEX = SRC_INDEX
      logging.debug(" Image (%s) should be in %s and copying to %s", attrs[ALT_INDEX][1], location, dst)
      try:
        if not os.path.exists(os.path.dirname(dst)):
          os.mkdir(os.path.dirname(dst))
        shutil.copy2(location, dst)
      except:
        traceback.print_exc()
        logging.error(" '%s' does not exist", location )
        sys.exit(3)
        
      self.extrafiles.append(dst)
    OutputParser.handle_startendtag(self, tag, attrs)

class Stage4Parser(OutputParser):
  def __init__(self, files, options, subdir, *args, **kwargs):
    OutputParser.__init__(self, *args, **kwargs)
    self.files = files
    self.options = options
    self.subdir = subdir
    
  def handle_starttag(self, tag, attrs):
    if tag == "a":
      for name,value in attrs:
        if name == "href" and value.startswith("/"):
          if value.startswith("/"): 
            # manual wants an absolute path, the help manual does not support
            # absolute path, so make sure that the image exists where the
            # absolute path indicates, then make the path into a relative path
            # with the approriate number of updirs
            test = os.path.join(self.options.indir, value[1:])
            if not os.path.exists(test):
              raise IOError("Cannot find %s in base path" % (value))
            
            # try find a valid relative path
            subdirdepth = len(self.subdir.split(os.path.sep))
            prefix = "../"*subdirdepth
            relpath = os.path.join(prefix, value[1:])
            if not os.path.exists(os.path.join(self.options.indir, self.subdir,relpath)):
              raise Exception("Cannot relativize path: %s" % (value))
            else:
              attrs = update_attribute(attrs, 'src', relpath)
              value = relpath
        if name == "href" and not value.startswith("http://"):
          # make sure the file being refered to exists in stage3
          check_ref = change_filename(value, ".stage3", ".", self.files['stage3'], makedirs=False)
          if not os.path.exists(check_ref):
            logging.debug(" File (%s) does not exist to be bundled into archive!", check_ref)
            
          fixed_ref = change_filename(value, ".htm", ".", ".", makedirs=False)
          attrs = update_attribute(attrs, "href", fixed_ref)
      
    OutputParser.handle_starttag(self, tag, attrs)
      
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
