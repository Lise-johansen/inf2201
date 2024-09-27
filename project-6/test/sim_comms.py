#!/usr/bin/python3

'''
Important note: This test is NOT in any way writen by me. 
Original author: Isak Kjerstad (discord user name: cornersatan42, github user name: isakkjerstad)
Published: Discord server INF-2201-1 23V general the 05.05.2023 9:18 PM
Download from: Github: isakkjerstad /inf2201-p6-tests, got it from the provided discord link.

Source:
    Written by: Isak Kjerstad.
    Purpose: functions handeling simulator communication and output validation.
    Date: 24.05.22
'''

import re as regex
import config as cfg
import pexpect as process

def cat_write(filename: str, strLines: list):

    # Create input to cat consisting of text, and commands to close cat. Specified file name is also set.
    catInput = "cat " + "{}".format(filename) + "\n" + construct_multi_command(strLines) + "\n" + "." + "\n"

    # Run cat commands in shell.
    output = run_commands([catInput])

    # Return shell error codes.
    return validate_output(output, [])

def construct_multi_command(commands: list):
    ''' Returns a string consisting of multiple commands. '''

    # Add newlines between commands.
    return "\n".join(commands)

def run_commands(commands: list):
    '''
    Runs commands in list, and returns a list of outputs from the shell
    as separate words. If command "cat" is utilized the given file name is
    stored in the file, before it is eventually saved and closed.
    '''

    # List of shell stdout and stderr's as words.
    shellOutput = []

    # Exec. all commands.
    for command in commands:

        # Open the shell simulator in PATH with EXEC_NAME in UTF-8 mode, and mute the terminal output.
        myShell = process.spawn(cfg.PATH + "/" + cfg.EXEC_NAME, cwd=cfg.PATH, encoding="utf-8", echo=False, timeout=cfg.TIMEOUT)

        # Run one command.
        myShell.send(command + "\n")

        # Save cat file.
        if "cat" in command:
            # Store fname in file, and save.
            myShell.send(command[4:] + "\n.\n")

        # Exit, i.e. get stdout result.
        myShell.send("exit\n")

        # Search entire shell output.
        for line in myShell.readlines():
            # Add results to the output.
            for word in line.split():
                # Do not add "$", i.e. empty outputs.
                if regex.search("\$", word) is None:
                    shellOutput.append(word)

    return shellOutput

def validate_output(shellOutput: list, searchNames: list):
    '''
    Check shell output for any error codes. Validate existence of search names. 
    Returns an error code, or zero if no errors.

    Error codes:
    
    0       ->  no errors, everything ok.
    -1      ->  unspecified error, or missing search name.
    -2      ->  file system is inconsistent.
    -3      ->  invalid mode.
    -4      ->  filename is too long.
    -5      ->  file does not exist.
    -6      ->  invalid file handle.
    -7      ->  invalid offset.
    -8      ->  end of file reached.
    -9      ->  file already exist.
    -10     ->  invalid filename.
    -11     ->  directory is not empty.
    -12     ->  invalid inode.
    -13     ->  out of file descriptor table entries.
    -14     ->  bitmap error.
    -15     ->  out of inodes.
    -16     ->  out of datablocks.
    -17     ->  could not add directory entry.
    -18     ->  direcory entry was not found.
    -19     ->  a given directory path is actually a file.
    -20     ->  parse filename error.
    -21     ->  inode table full.
    -22     ->  invalid block.
    -23     ->  tried to delete a file that is open by another program.
    '''

    # Search entire shell output.
    for word in shellOutput:

        # Search for errors, e.g. negative numbers.
        error = regex.search("-(\d*)", word)
        if error is not None:
            # Return file system error code.
            return int(error.group(error.pos))

    # If no errors, verify that all search names exist.
    for query in searchNames:
        if query not in shellOutput:
            # Entry not found.
            return -1
    
    # No errors.
    return 0