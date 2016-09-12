import logging
import os
import shutil
import zipfile

import markdown

from ohm.help_parsers.helpparsers import Stage2Parser, Stage3Parser, Stage4Parser, Stage5Parser
from ..utilfunctions import change_filename


def build(args):
    """Compiles the files in args.indir to the archive output args.outfile.

    Compiled in several stages:
        - stage1: Transform all input files with markdown placing the results
        into args.temp+"/stage1".
        - stage2: Parses and strips the output of stage1 to build the list
        that will be made into the c-array that contains the compiled names for
        the detailed help of each control.
        - stage3: Parses the output of stage2 to grab the images that are
        referred to in the the output of stage1
        - stage4: Parses the output of stage1 to fix the relative hyperlinks in
        the output so that they will refer correctly to the correct files when
        in output file.
        - stage5: Generate the index and table of contents for the output file
        from the output of stage4.
        - stage6: Zip up the output of stage5 and put it in the output location.
    """
    notices = logging.getLogger('notices')
    notices.info("Building...")
    logging.debug("Using '%s' as working directory", args.temp)
    logging.debug("Using '%s' as output file", args.outfile)
    logging.debug("Using '%s' as input directory", args.indir)

    files = generate_paths(args)

    if should_build(args):
        _do_build(args, files, notices)
    else:
        notices.info(" Up to date.")


def _do_build(args, files, notices):
    help_array = list()
    extra_files = list()
    notices.info(" Processing input files:")
    for file_name in generate_file_list(args.indir, ".help"):
        notices.info("  %s", file_name)
        logging.info("   Stage 1")
        name1 = process_input_stage1(file_name, args, files)

        logging.info("   Stage 2")
        name2 = process_input_stage2(name1, args, files, help_array)

        logging.info("   Stage 3")
        name3 = process_input_stage3(name2, args, files, extra_files)

        logging.info("   Stage 4")
        name4 = process_input_stage4(name3, args, files)
    logging.info(" Stage 5")
    process_input_stage5(args, files, extra_files)
    logging.info(" Stage 6")
    process_input_stage6(args, files)
    logging.info(" Generating .cpp files")
    generate_cpp_files(args, files, help_array)
    notices.info("....Done.")


def generate_paths(options):
    """Generates the names of the paths that will be needed by the compiler.

    Generated names will be returned as a dictionary."""
    paths = dict()

    for i in range(1, 7):
        stage_name = 'stage{0}'.format(i)
        paths[stage_name] = os.path.join(options.temp, stage_name)

    for path in paths.values():
        if not os.path.exists(path):
            logging.debug(" Making %s", path)
            os.makedirs(path)

    return paths


def should_build(options):
    """Should we build the output file?
    :param options arguments from command line
    :return True if the output file should be built/rebuilt
    """
    logger = logging.getLogger('should_build')
    if options.always_build:
        logger.info(" Always building")
        return True
    elif not os.path.exists(options.outfile):
        logger.info(" Outfile does not exist")
        return True
    elif options.carrayfilename and not os.path.exists(options.carrayfilename):
        logger.info(" .cpp file does not exist")
        return True
    elif check_source_newer_than_outfile(options):
        logger.info(" Source files are newer than output")
        return True
    return False


def check_source_newer_than_outfile(options):
    try:
        outfile_time = os.path.getmtime(options.outfile)

        for d_path, d_names, f_names in os.walk(options.indir):
            for f_name in f_names:
                f_path = os.path.join(d_path, f_name)
                if os.path.getmtime(f_path) > outfile_time:
                    logging.info(
                        "%s has been changed since outfile. Causing build",
                        f_path)
                    return True
                elif os.path.getctime(f_path) > outfile_time:
                    logging.info(
                        "%s has been created since outfile. Causing build",
                        f_path)
                    return True

    except OSError as e:
        logging.exception("OS does not like %d. Forcing build.", e.filename)
        return True
    return False


def generate_file_list(directory, extension):
    for path, dirs, files in os.walk(directory):
        for file_name in files:
            if os.path.splitext(file_name)[1] == extension:
                yield os.path.join(path, file_name)


def process_input_stage1(file, options, files):
    infile = open(file, mode="r")
    input = infile.read()
    infile.close()

    md = markdown.Markdown(
        extensions=['markdown.extensions.toc']
    )
    output = md.convert(input)

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

    parser = Stage2Parser(file=outfile)
    parser.feed(input)
    parser.close()
    outfile.close()

    if len(parser.control) > 0:
        filename_in_archive = change_filename(outname, ".htm", files['stage2'], ".", makedirs=False)
        for control in parser.control:
            helparray.append((control, filename_in_archive))
            logging.debug(" Control name %s", control)

    return outname


def process_input_stage3(file, options, files, extrafiles):
    infile = open(file, mode="r")
    input = infile.read()
    infile.close()

    outname = change_filename(file, ".stage3", files['stage2'], files['stage3'])
    outfile = open(outname, mode="w")

    # figure out what subdirectory of the onlinehelp I am in
    subdir = os.path.dirname(outname).replace(os.path.normpath(files['stage3']), "")
    if subdir.startswith(os.path.sep):
        subdir = subdir.replace(os.path.sep, "", 1)  # I only want to remove the leading sep

    parser = Stage3Parser(options, files, file=outfile, extrafiles=extrafiles, subdir=subdir)
    parser.feed(input)
    parser.close()
    outfile.close()

    return outname


def process_input_stage4(file, options, files):
    """Fix the a tags so that they point to the htm files that will be in the output archive."""
    infile = open(file, mode="r")
    input = infile.read()
    infile.close()

    outname = change_filename(file, ".stage4", files['stage3'], files['stage4'])
    outfile = open(outname, mode="w")

    # figure out what subdirectory of the onlinehelp I am in
    subdir = os.path.dirname(outname).replace(os.path.normpath(files['stage4']), "")
    if subdir.startswith(os.path.sep):
        subdir = subdir.replace(os.path.sep, "", 1)  # I only want to remove the leading sep

    parser = Stage4Parser(files=files, file=outfile, options=options,
                          subdir=subdir)
    parser.feed(input)
    parser.close()
    outfile.close()

    return outname

HTB_HEADER_TEMPLATE = "\n".join([
    "Contents file={contents}",
    "Index file={index}",
    "Title={title}",
    "Default topic={default}",
    "Charset=UTF-8",
    ""])


def process_input_stage5(options, files, extrafiles):
    """Generate the index and table of contents"""

    # write header file
    header_file_name = os.path.join(files['stage5'], "header.hhp")
    with open(header_file_name, mode="w") as header_file:
        header_file.write(HTB_HEADER_TEMPLATE.format(
            contents="contents.hhc",
            index="index.hhk",
            title="wxLauncher User Guide",
            default="index.htm",
        ))

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
        path_list = find_path_in_archive(path, files['stage4'])
        logging.debug("Processing directory '%s'", os.path.sep.join(path_list))

        if len(path_list) == 1 and path_list[0] == '':
            path_list = []
        level = len(path_list)

        # parse the index.help to get the name of the section
        index_file_name = os.path.join(path, "index.stage4")
        # relativize filename for being in the archive
        index_in_archive = change_filename(index_file_name, ".htm", files['stage4'], ".", False)
        index_in_archive = index_in_archive.replace(os.path.sep,
                                                    "/")  # make the separators the same so that it doesn't matter what platform the launcher is built on.
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

        tocfile.write(
            generate_sections(path_list, last_path_list, index_filename=index_in_archive, section_title=parser.title))
        last_path_list = path_list

        if level > 0:
            # remove index.htm from thefiles because they are used by the section names
            try:
                thefiles.remove('index.stage4')
            except ValueError:
                logging.warning("Directory %s does not have an index.help",
                                os.path.sep.join(path_list))

        for file in thefiles:
            full_filename = os.path.join(path, file)
            # relativize filename for being in the archive
            filename_in_archive = change_filename(full_filename, ".htm", files['stage4'], ".", False)
            # find the title
            outfile_name = change_filename(full_filename, ".htm", files['stage4'], files['stage5'])
            outfile = open(outfile_name, mode="w")

            infile = open(full_filename, mode="r")
            input = infile.read()
            infile.close()

            parser = Stage5Parser(file=outfile)
            parser.feed(input)
            parser.close()
            outfile.close()

            tocfile.write(
                """%(tab)s<li> <object type="text/sitemap">\n%(tab)s     <param name="Name" value="%(name)s">\n%(tab)s     <param name="Local" value="%(file)s">\n%(tab)s    </object>\n""" % {
                    "tab": "     " * level,
                    "name": parser.title,
                    "file": filename_in_archive,})

            indexfile.write(
                """%(tab)s<li> <object type="text/sitemap">\n%(tab)s\t<param name="Name" value="%(name)s">\n%(tab)s\t<param name="Local" value="%(file)s">\n%(tab)s </object>\n""" % {
                    "tab": "\t",
                    "name": parser.title,
                    "file": filename_in_archive,})

    tocfile.write(generate_sections([], last_path_list))
    tocfile.write("</ul>\n")
    tocfile.close()
    indexfile.close()

    # copy the extra files (i.e. images) from stage3
    for extrafilename in extrafiles:
        logging.debug(" Copying: %s", extrafilename)
        dst = change_filename(extrafilename, None, orginaldir=files['stage3'], destdir=files['stage5'])
        shutil.copy2(extrafilename, dst)


def generate_sections(path_list, last_path_list, basetab=0, orginal_path_list=None, index_filename=None,
                      section_title=None):
    """Return the string that will allow me to write in the correct section."""
    logging.debug("   generate_sections(%s, %s, %d, %s)", str(path_list), str(last_path_list), basetab,
                  orginal_path_list)

    if orginal_path_list == None:
        orginal_path_list = path_list

    if len(path_list) > 0 and len(last_path_list) > 0 and path_list[0] == last_path_list[0]:
        logging.debug("    matches don't need to do anything")
        return generate_sections(path_list[1:], last_path_list[1:], basetab + 1, orginal_path_list, index_filename,
                                 section_title)

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
            title = path_list[len(path_list) - 1]
        else:
            title = section_title

        s = ""
        for path in path_list:
            s += """%(tab)s<li> <object type="text/sitemap">\n%(tab)s     <param name="Name" value="%(name)s">\n%(tab)s     <param name="Local" value="%(file)s">\n%(tab)s    </object>\n%(tab)s     <ul>\n""" % {
                "tab": "     " * basetab,
                "name": title,
                "file": index_filename}

        return s
    elif len(path_list) == 0 and len(last_path_list) > 0:
        logging.debug("    do down")
        s = ""
        basetab += len(last_path_list)
        for path in last_path_list:
            s += """%(tab)s</ul>\n""" % {'tab': "     " * (basetab)}
            basetab -= 1
        return s

    elif len(path_list) == 0 and len(last_path_list) == 0:
        logging.debug("    both lists empty, doing nothing")
        return ''

    else:
        raise Exception("Should never get here")
        return ""


def find_path_in_archive(pLeft, pRight):
    """Return the parts of the path that is in 'pLeft' but not in 'pRight'"""
    pLeft = os.path.normpath(pLeft)
    pRight = os.path.normpath(pRight)

    lLeft = pLeft.split(os.path.sep)
    lRight = pRight.split(os.path.sep)

    return find_path_in_archive_helper(lLeft, lRight)


def find_path_in_archive_helper(lLeft, lRight):
    logging.debug("make_path_in_archive_helper(%s, %s)",
                  str(lLeft), str(lRight))
    if len(lLeft) > 0 and len(lRight) > 0 and lLeft[0] == lRight[0]:
        return find_path_in_archive_helper(lLeft[1:], lRight[1:])
    elif len(lLeft) > 0 and len(lRight) == 0:
        return lLeft
    else:
        return []


def process_input_stage6(options, stage_dirs):
    # make sure that the directories exist before creating file
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


def generate_cpp_files(options, files, helparray):
    if options.carrayfilename == None:
        defaultname = os.path.join(os.path.dirname(options.outfile), "helplinks.cpp")
        logging.info("Filename for c-array has not been specified.")
        logging.info(" using default %s" % (defaultname))
        options.carrayfilename = defaultname

    logging.getLogger('notices').info("Writing to %s", options.carrayfilename)
    outfile = open(options.carrayfilename, mode="w")
    outfile.write("// Generated by scripts/onlinehelpmaker.py\n")
    outfile.write("// GENERATED FILE - DO NOT EDIT\n")
    for id, location in helparray:
        outfile.write("""{%s,_T("%s")},\n""" %
                      (id, location.replace(os.path.sep, "/")))

    outfile.close()