import os
import os.path
import string
import subprocess
import sys
import tempfile
import traceback

import utilfunctions

class VersionFileBuilder:
  def __init__(self, workfile, outfile, gitpath="git", out=sys.stdout,
  err=sys.stderr):
    self.workfile = workfile
    self.outfile = outfile
    self.gitpath = gitpath
    self.out = out
    self.err = err
    
  def rebuild(self):
    """Calls clean then build"""
    self.clean()
    self.build()
    
  def clean(self):
    """Removes all temporary files and the output files that were passed,
    if they exist"""
    
    self.out.write("Cleaning...\n")
    
    try:
      self.out.write(" Removing %s: " % (self.filepath))
      os.remove(self.filepath)
      self.out.write("Done.\n")
    except:
      self.out.write("Failed!\n")
      traceback.print_exc(None, self.err)
    
    try:
      self.out.write(" Removing %s: " % (self.workfile))
      os.remove(self.workfile)
      self.out.write("Done.\n")
    except:
      self.out.write("Failed!\n")
      traceback.print_exc(None, self.err)
    
  def build(self):
    self.out.write("Checking if build is needed...\n")
    
    if self.need_to_update():
      self.out.write("Generating...\n")
      
      self.out.write(" Writing to tempfile %s.\n" % (self.workfile))
      if not utilfunctions.make_directory_for_filename(self.workfile):
        self.out.write("  Directory already exists...\n")
      
      out = open(self.outfile, "wb")
      out.write('const wchar_t *GITVersion = L"')
      out.flush()
      
      out.write(self.get_git_id())
      out.flush()
      out.write('";\n')
      
      out.write('const wchar_t *GITDate = L"')
      out.flush()
      
      datecmd = 'log --pretty=format:%cd -n 1'
      datecmd = datecmd.split()
      datecmd.insert(0, self.gitpath)
      
      subprocess.Popen(datecmd, stdout=out).wait()
      out.flush()
      out.write('";\n')
      
      out.close()
      self.out.write(" Done.\n")
    else:
      self.out.write(" Up to date.\n")
      
  def get_git_id(self):
    """Return the git id string after striping the newline from the end
    of the command output"""
    id = tempfile.TemporaryFile()
    s = "describe --long --dirty --abbrev=10 --tags --always"
    s = s.split()
    s.insert(0, self.gitpath)
    
    subprocess.Popen(s, stdout=id).wait()
    id.seek(0)
    
    rawidoutput = id.readline()
    # split on the line ending and return the first peice which will
    # contain the entire id (all of the tags, etc) of the current 
    # repository without any newlines
    return string.split(rawidoutput, "\n")[0]
    
  def need_to_update(self):
    """Returns False if the self.outfile is up to date, True otherwise"""
    if os.path.exists(self.workfile) == True:
      if os.path.exists(self.outfile) == True:
        # check the id to see if it has changed
        workfile = open(self.workfile, 'rb')
        if workfile.readline() == self.get_git_id():
          return False
        else:
          self.out.write(" git id changed\n")
      else:
        self.out.write(" %s does not exist\n" % (self.outfile))
    else:
      self.out.write(" %s does not exist\n" % (self.workfile))
    return True
