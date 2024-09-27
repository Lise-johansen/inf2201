#!/usr/bin/python3

'''
Written by: Isak Kjerstad.
Purpose: common configuration file for the file system tests.
Date: 24.05.22
'''

import pathlib
import subprocess as process

# Name of make target and exec. name to run.
EXEC_NAME = "p6sh"

# Absolute path to makefile and executable to run.
PATH = str(pathlib.Path(__file__).parent.resolve()) + "/../" + "src"

# Max. time in seconds for one command to run.
TIMEOUT = 20

def compile():
    ''' Compiles the program, and sets correct permissions. '''
    process.run("make clean; make " + EXEC_NAME, shell=True, cwd=PATH, stdout=process.DEVNULL, stderr=process.DEVNULL)
    process.run("chmod +x ./" + EXEC_NAME, shell=True, cwd=PATH, stdout=process.DEVNULL, stderr=process.DEVNULL)

def cleanup():
    ''' Removes program and files. '''
    process.run("make clean", shell=True, cwd=PATH, stdout=process.DEVNULL)