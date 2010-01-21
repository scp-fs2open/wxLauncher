import os.path
import logging

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
  
def change_filename(filename, newext, orginaldir, destdir, makedirs=True):
  """Returns the filename after transforming it to be in destdir and making sure the folders required all exist."""
  filename = os.path.normpath(filename)
  orginaldir = os.path.normpath(orginaldir)
  destdir = os.path.normpath(destdir)
  logging.debug("   change_filename('%s', '%s', '%s', '%s', %s)", filename, newext, orginaldir, destdir, makedirs)
  outfile_name1 = filename.replace(orginaldir, ".") # files relative name
  logging.debug("%s", outfile_name1)
  if newext == None:
    outfile_name3 = outfile_name1
  else:
    outfile_name2 = os.path.splitext(outfile_name1)[0] #file's name without ext
    logging.debug("%s", outfile_name2)
    outfile_name3 = outfile_name2 + newext
  logging.debug("%s", outfile_name3)
  outfile_name4 = os.path.join(destdir, outfile_name3)
  logging.debug("%s", outfile_name4)
  outfile_name = os.path.normpath(outfile_name4)
  logging.debug("%s", outfile_name)
  
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