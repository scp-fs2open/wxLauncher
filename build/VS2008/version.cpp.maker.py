import subprocess, sys, os, tempfile, string, os.path

WORKFILE=3
OUTFILE=2
JOB=1

def main():
  if (len(sys.argv) < 4):
    print "Output file not specifed!"
    sys.exit(1)
    
  if ( sys.argv[JOB] == "build" ):
    build(sys.argv[OUTFILE], sys.argv[WORKFILE])
  elif ( sys.argv[JOB] == "rebuild" ):
    rebuild(sys.argv[OUTFILE], sys.argv[WORKFILE])
  elif ( sys.argv[JOB] == "clean" ):
    clean(sys.argv[OUTFILE], sys.argv[WORKFILE])
  else:
    print "Invalid JOB type. Use build, rebuild, or clean."
    sys.exit(2)
  
  sys.exit(0)

def rebuild(file, work):
  clean(file, work)
  build(file, work)
  
def clean(file, work):
  try:
    os.remove(file)
  except:
    pass
    
  try:
    os.remove(work)
  except:
    pass
  
def build(file, work):
  if ( need_to_update(file, work) ):
    out = open(file, "wb")
    out.write("wchar_t *HGVersion = L\"")
    out.flush()
    
    out.write(get_hg_id())
    out.flush()
    
    out.write("\";\nwchar_t *HGDate = L\"")
    out.flush()
    
    subprocess.Popen('hg parents --template "{date|date}"', stdout=out).wait()
    out.flush()
    
    out.write('";\n')
    out.close()
    
def get_hg_id():
  id = tempfile.TemporaryFile()
  subprocess.Popen("hg id -i -b -t", stdout=id).wait()
  id.seek(0)
  return string.split(id.readline(), "\n")[0]
  
def need_to_update(file, work):
  if ( os.path.exists(work) == True ):
    if ( os.path.exists(file) == True ):
      workfile = open(work, 'rb')
      if ( workfile.readline() == get_hg_id() ):
        return False
  return True

if __name__ == "__main__":
  main()