#!/usr/bin/env python
from __future__ import print_function
import argparse, errno, os, glob, shutil, string

BASE_DIR = os.path.dirname(os.path.realpath(__file__))
DEFAULT_SKELETON = os.path.join(os.path.dirname(BASE_DIR), 'utils/app-skeleton')

def mkdir_p(path):
    """From: https://stackoverflow.com/a/600612"""
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def main(appname, directory, template):
    install_location = os.path.join(directory, appname)
    print("Creating:       {0!r}".format(appname))
    print("In directory:   {0}".format(install_location))
    print("Using template: {0}".format(template))
    if os.path.exists(install_location):
        print("{0} exists.\nRemove and run again.".format(install_location))
        return None
    shutil.copytree(template, install_location)
    print('Writing: ')
    for subdir, _, files in os.walk(install_location):
        for filename in files:
            outpath = inpath = os.path.join(subdir, filename)
            if 'APP_NAME' in inpath:
                outpath = inpath.replace('APP_NAME', appname)
                print(' * {}'.format(outpath))
                with open(inpath, 'r') as infile, open(outpath, 'w') as outfile:
                    data = string.Template(infile.read())\
                        .safe_substitute(dict(APP_NAME=appname))
                    outfile.write(data)
                os.remove(inpath)
            else:
                print(' * {}'.format(inpath))
                data = None
                with open(inpath, 'r') as infile:
                    data = string.Template(infile.read())\
                        .safe_substitute(dict(APP_NAME=appname))
                if data:
                    with open(inpath, 'w') as outfile:
                        outfile.write(data)
    print('Finished creating new {0!r} application..'.format(appname))
    print('Happy hacking! =)')
    return None

if __name__ == '__main__':
    ARG_PARSER = argparse.ArgumentParser(description=__doc__)
    ARG_PARSER.add_argument('appname')
    ARG_PARSER.add_argument(
        '-d', '--directory', action='store',
        type=os.path.abspath,
        default=os.curdir
    )
    ARG_PARSER.add_argument(
        '-t', '--template', action='store',
        type=os.path.abspath,
        default=DEFAULT_SKELETON
    )
    ARGS = dict(ARG_PARSER.parse_args()._get_kwargs())
    main(**ARGS)