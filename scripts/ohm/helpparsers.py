import logging
import os
import os.path
import shutil
import tempfile

try:
    from html.parser import HTMLParser
except ImportError:
    from HTMLParser import HTMLParser

from .dataclasses import Tag
from .utilfunctions import update_attribute, change_filename


class OutputParser(HTMLParser):
    """Provide common functionality for the Parsers.

    Currently class passes the input file through to the passed in output"""

    def __init__(self, file):
        HTMLParser.__init__(self)

        if hasattr(file, 'write'):
            self.output_file = file
        else:
            raise ValueError("file is not a file-like")

        self.tag_stack = list()

    def handle_starttag(self, tag, attrs):
        logging.debug(" Handle starttag: %s", tag)
        logging.debug("  %s", attrs)
        self.tag_stack.append(Tag(tag, attrs))

    def handle_startendtag(self, tag, attrs):
        logging.debug(" Handle start and end tag: %s", tag)
        logging.debug("  %s", attrs)
        self.write_tag(Tag(tag, attrs))

    def handle_endtag(self, tagname):
        """Finds the matching tag recursing until finding a match"""
        logging.debug(" End tag: %s", tagname)
        tag = self.find_match(tagname)
        self.write_tag(tag)

    def handle_data(self, data):
        logging.debug(" Data: %s", data)
        if len(self.tag_stack) > 0:
            tag = self.tag_stack.pop()
            if tag.data:
                tag.data += data
            else:
                tag.data = data
            self.tag_stack.append(tag)
        else:
            self.output_file.write(data)

    def find_match(self, tagtofind, indent=0):
        """Recursive function to match the tag"""
        tag = self.tag_stack.pop()
        if not tag.name == tagtofind:
            logging.warning(" %smismatched tag found (expected %s, found %s)",
                            " " * indent, tagtofind, tag.name)
            self.write_tag(tag)
            tag = self.find_match(tagtofind)

        return tag

    def write_tag(self, tag):
        """Write a tag out if it is top level.

        When tag stack is empty, writes tag to file passed in the constructor.
        When tag stack is not empty formats the tag and sets (or if the next
        tag.data is not None, appends) the formatted string."""
        if tag.data:
            s = "<%s%s />" % (tag.name, self.format_attributes(tag))
        else:
            s = "<%s%s>%s</%s>" % (
                tag.name, self.format_attributes(tag), tag.data, tag.name)

        if len(self.tag_stack) > 0:
            tag = self.tag_stack.pop()
            if tag.data:
                tag.data += s
            else:
                tag.data = s
            self.tag_stack.append(tag)
        else:
            self.output_file.write(s)

    @staticmethod
    def format_attributes(tag):
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
            if (attrs[0][0] == "name"
                    and attrs[0][1] == "control"
                    and attrs[1][0] == "content"):
                self.control.append(attrs[1][1])
        else:
            OutputParser.handle_starttag(self, tag, attrs)

    def handle_startendtag(self, tag, attrs):
        if tag == "meta":
            if (attrs[0][0] == "name"
                    and attrs[0][1] == "control"
                    and attrs[1][0] == "content"):
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
        be in the f output."""
        alt_index = None
        srv_index = None
        if tag == "img":
            # figure out which attribute is src
            for x in range(0, len(attrs)):
                if attrs[x][0] == "src":
                    srv_index = x
                elif attrs[x][0] == "alt":
                    alt_index = x

            if attrs[srv_index][1].startswith("/"):
                # manual wants an absolute path, the help manual does not
                # support absolute path, so make sure that the image exists
                # where the absolute path indicates, then make the path into a
                # relative path with the appropriate number of updirs
                test = os.path.join(self.options.indir, attrs[srv_index][1][1:])
                if not os.path.exists(test):
                    raise IOError(
                        "Cannot find %s in base path" % (attrs[srv_index][1]))

                # try find a valid relative path
                subdirdepth = len(self.subdir.split(os.path.sep))
                prefix = "../" * subdirdepth
                relpath = os.path.join(prefix, attrs[srv_index][1][1:])
                if not os.path.exists(
                        os.path.join(self.options.indir, self.subdir, relpath)):
                    raise Exception(
                        "Cannot relativize path: %s" % (attrs[srv_index][1]))
                else:
                    attrs = update_attribute(attrs, 'src', relpath)

            location1 = os.path.join(self.options.indir, self.subdir,
                                     attrs[srv_index][1])
            location = os.path.normpath(location1)

            # check to make sure that the image I am including was in the
            # onlinehelp folder, if not change the dst name so that it will be
            # located correctly in the stage3 directory
            logging.debug("%s - %s", location, self.options.indir)
            if location.startswith(self.options.indir):
                dst1 = os.path.join(self.files['stage3'], self.subdir,
                                    attrs[srv_index][1])
                dst = os.path.normpath(dst1)
            else:
                # get extension
                basename = os.path.basename(attrs[srv_index][1])
                (name, ext) = os.path.splitext(basename)
                (f, outname) = tempfile.mkstemp(ext, name, self.files['stage3'])
                # make into a relative path
                dst1 = outname.replace(os.getcwd(), ".")

                # fix up attrs
                dst = os.path.normpath(dst1)
                filename = change_filename(dst, os.path.splitext(dst)[1],
                                           self.files['stage3'], ".",
                                           makedirs=False)
                attrs = update_attribute(attrs, 'src', filename)

            if alt_index is None:
                alt_index = srv_index
            logging.debug(" Image (%s) should be in %s and copying to %s",
                          attrs[alt_index][1], location, dst)
            try:
                if not os.path.exists(os.path.dirname(dst)):
                    os.mkdir(os.path.dirname(dst))
                shutil.copy2(location, dst)
            except:
                logging.exception(" '%s' does not exist", location)
                raise

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
            for name, value in attrs:
                attrs = self._handle_start_tag(attrs, name, value)
        OutputParser.handle_starttag(self, tag, attrs)

    def _handle_start_tag(self, attrs, name, value):
        if name == "href" and value.startswith("/"):
            if value.startswith("/"):
                # manual wants an absolute path, the help manual does not
                # support absolute path, so make sure that the image exists
                # where the absolute path indicates, then make the path into a
                # relative path with the appropriate number of up dirs

                # value is absolute but we need it relative, so drop the slash
                if not self._path_exists(value[1:]):
                    raise ValueError("Cannot find %s in base path", value)

                # try find a valid relative path
                subdirdepth = len(self.subdir.split(os.path.sep))
                prefix = "../" * subdirdepth
                relpath = os.path.join(prefix, value[1:])
                if self._path_exists(self.subdir, relpath):
                    raise ValueError("Cannot relativize path: %s", value)
                else:
                    attrs = update_attribute(attrs, 'src', relpath)
                    value = relpath
        if name == "href" and not value.startswith("http://"):
            # make sure the file being referred to exists in stage3
            check_ref = change_filename(value, ".stage3", ".",
                                        self.files['stage3'],
                                        makedirs=False)
            if not os.path.exists(check_ref):
                logging.warning(
                    " File (%s) does not exist to be bundled into archive!",
                    check_ref)

            fixed_ref = change_filename(value, ".htm", ".", ".",
                                        makedirs=False)
            attrs = update_attribute(attrs, "href", fixed_ref)
        return attrs

    def _path_exists(self, *paths):
        return os.path.exists(os.path.join(self.options.indir, *paths))


class Stage5Parser(OutputParser):
    def __init__(self, *args, **kwargs):
        OutputParser.__init__(self, *args, **kwargs)
        self.title = "Untitled"

    def handle_endtag(self, tag):
        if tag == "title":
            t = self.tag_stack.pop()
            self.title = t.data
            self.tag_stack.append(t)
        elif tag == "h1" and self.title == "Untitled":
            # make first h1 tag found into the title will be overwritten by
            # a title tag if it exists.
            t = self.tag_stack.pop()
            self.title = t.data
            self.tag_stack.append(t)
        OutputParser.handle_endtag(self, tag)
