import subprocess, sys, os, tempfile, string, os.path

WORKFILE=3
OUTFILE=2
JOB=1

def main():
  if (len(sys.argv) < 4):
    print "Output file not specifed!"
    sys.exit(1)
    
  work = os.path.normcase(os.path.normpath(sys.argv[WORKFILE]))
  file = os.path.normcase(os.path.normpath(sys.argv[OUTFILE]))
    
  if ( sys.argv[JOB] == "build" ):
    build(file, work)
  elif ( sys.argv[JOB] == "rebuild" ):
    rebuild(file, work)
  elif ( sys.argv[JOB] == "clean" ):
    clean(file, work)
  else:
    print "Invalid JOB type. Use build, rebuild, or clean."
    sys.exit(2)
  
  sys.exit(0)

def rebuild(file, work):
  print "Rebuild...."
  clean(file, work)
  build(file, work)
  
def clean(file, work):
  print "Clean..."
  try:
    print " Removing:", file, 
    os.remove(file)
    print "Done."
  except:
    print "Failed"
    
  try:
    print " Removing:", work, 
    os.remove(work)
    print "Done."
  except:
    print "Failed"
  
def build(file, work):
  print "Building..."
  if ( need_to_update(file, work) ):
    print " Writing tempfile:", work
    make_directory_for_output(work)
    temp = open(work, "wb")
    temp.write(get_hg_id())
    temp.flush()
    temp.close()
    
    print " Writing output file:", file
    make_directory_for_output(file)
    out = open(file, "wb")
    out.write("const wchar_t *HGVersion = L\"")
    out.flush()
    
    out.write(get_hg_id())
    out.flush()
    
    out.write("\";\nconst wchar_t *HGDate = L\"")
    out.flush()
    
    subprocess.Popen('hg parents --template {date|date}'.split(), stdout=out).wait()
    out.flush()
    
    out.write('";\n')
    out.close()
    print " Done"
  else:
    print " Up to date"
    
def get_hg_id():
  id = tempfile.TemporaryFile()
  subprocess.Popen("hg id -i -b -t".split(), stdout=id).wait()
  id.seek(0)
  return string.split(id.readline(), "\n")[0]
  
def need_to_update(file, work):
  if ( os.path.exists(work) == True ):
    if ( os.path.exists(file) == True ):
      workfile = open(work, 'rb')
      if ( workfile.readline() == get_hg_id() ):
        return False
      else:
        print "hg id changed"
    else:
      print " ", file, "does not exist"
  else:
    print " ", work, "does not exist"
  return True
  
def make_directory_for_output(output):
  path = os.path.dirname(output)
  if os.path.exists(path):
    if os.path.isdir(path):
      print " ", path, "already exists"
    else:
      print " ", path, "exists but is not a directory!"
  else:
    # can only throw an exception user does not have permission
    try:
      os.makedirs(path)
    except:
      print " ", path, "is not createable"
      raise

if __name__ == "__main__":
  main()
