import subprocess, sys, os, tempfile, string

def main():
  if (len(sys.argv) < 2):
    print "Output file not specifed!"
    sys.exit(1)
  
  out = open(sys.argv[1], "wb")
  out.write("wchar_t *HGVersion = L\"")
  out.flush()
  
  id = tempfile.TemporaryFile()
  subprocess.Popen("hg id -i -b -t", stdout=id).wait()
  id.seek(0)
  out.write(string.split(id.readline(), "\n")[0])
  out.flush()
  
  out.write("\";\nwchar_t *HGDate = L\"")
  out.flush()
  
  subprocess.Popen('hg parents --template "{date|date}"', stdout=out).wait()
  out.flush()
  
  out.write('";\n')
  out.close()
  
if __name__ == "__main__":
  main()