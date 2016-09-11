from __future__ import (absolute_import, division,
                        print_function, unicode_literals)

import logging

import os
import os.path
import subprocess
import tempfile

from . import utilfunctions


class VersionFileBuilder(object):
    VERSION_TMPL = '\n'.join([
        'const wchar_t *GITVersion = L"{version}";',
        'const wchar_t *GITDate = L"{date}";',
        ''
    ])
    notices = logging.getLogger('notices')

    def __init__(self, workfile, outfile, gitpath="git"):
        self.workfile = workfile
        self.outfile = outfile
        self.gitpath = gitpath
        self.logger = logging.getLogger(self.__class__.__name__)

    def remove_file(self, filename):
        try:
            os.remove(filename)
            self.notices.info(' Removed %s', filename)
        except OSError:
            self.logger.exception(' Unable to remove %s', filename)

    def build(self):
        self.notices.info("Building...")
        if self.need_to_update():
            self.notices.info("Generating...")

            utilfunctions.make_directory_for_filename(self.outfile)

            with open(self.outfile, "wb") as out:
                git_id = self.get_git_id()
                output = self.VERSION_TMPL.format(
                    version=git_id,
                    date=self.get_git_date())
                out.write(output.encode('utf-8'))
                with open(self.workfile, "wb") as work:
                    work.write(git_id.encode('utf-8'))

            self.notices.info(" Version information written to %s",
                              self.outfile)
        else:
            self.notices.info(" Up to date")

    def get_git_id(self):
        """Return the git id string suitable for inclusion in C code.

        Requires striping the newline from the end of the command output"""
        tf = tempfile.TemporaryFile()
        s = "describe --long --dirty --abbrev=10 --tags --always"
        s = s.split()
        s.insert(0, self.gitpath)

        subprocess.Popen(s, stdout=tf).wait()
        tf.seek(0)

        rawidoutput = tf.readline()
        idoutput = rawidoutput.decode('utf-8').split('\n')
        # split on the line ending and return the first peice which will
        # contain the entire id (all of the tags, etc) of the current
        # repository without any newlines
        return idoutput[0]

    def get_git_date(self):
        """Return the git date string suitable for includsion in C code.

        Requires stripping the newline from the end of the command output"""
        tf = tempfile.TemporaryFile()

        datecmd = 'log --pretty=format:%cd -n 1'
        datecmd = datecmd.split()
        datecmd.insert(0, self.gitpath)

        subprocess.Popen(datecmd, stdout=tf).wait()
        tf.seek(0)

        rawdateoutput = tf.readline()
        dateoutput = rawdateoutput.decode('utf-8')
        return dateoutput

    def need_to_update(self):
        """Check if we need to update the outfile
        :return: False if outfile is up to date, True otherwise
        """
        if os.path.exists(self.workfile):
            if os.path.exists(self.outfile):
                # check the id to see if it has changed
                with open(self.workfile, 'rb') as work:
                    if work.readline().decode('utf-8') == self.get_git_id():
                        return False
                    else:
                        self.notices.info(" git id changed")
            else:
                self.notices.info(" %s does not exist", self.outfile)
        else:
            self.notices.info(" %s does not exist", self.workfile)
        return True

