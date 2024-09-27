#!/usr/bin/python3

'''
Important note: This test is NOT in any way writen by me.
  All modifications are marked with a comment and all SELF MADE TEST is also clearly marked with a comment (inspired by the original auther)
Original author: Isak Kjerstad (discord user name: cornersatan42, github user name: isakkjerstad)
Published: Discord server INF-2201-1 23V general the 05.05.2023 9:18 PM

Modification: test_mkdir(self) (line: 30), test_cd_relative(self) (line: 117)
Made by me BUT uses the logic of the original auther (1 test 2 behevior): special_case_mkdir(self) (line: 62)

Download from: Github: isakkjerstad /inf2201-p6-tests, got it from the provided discord link.
Source:
    Written by: Isak Kjerstad.
    Purpose: functions handeling simulator communication and output validation.
    Date: 24.05.22
'''

import unittest
import config as cfg
from sim_comms import run_commands, validate_output, construct_multi_command

class TestShellFunctions(unittest.TestCase):
    ''' Short tests of all the different file system shell commands. '''

    def setUp(self):
        cfg.compile()

    # Modify by me, names and number of levels.
    def test_mkdir(self):
        ''' Testing with main focus on the mkdir command. '''

        # Construct a set of mkdir commands to run at once.
        multiLineInput = construct_multi_command(["mkdir Dir",
                                                 "cd Dir",
                                                 "mkdir dir1",
                                                 "mkdir dir2",
                                                 "mkdir dir3",
                                                 "ls"])

        # Retrieve output mainly from the last ls command.
        shellOutput = run_commands([multiLineInput])

        # Expecting no errors and to find the given directories.
        errorCode = validate_output(shellOutput, ["dir1",
                                                    "dir2",
                                                    "dir3",
                                                    "..", "."])

        # Should cause no errors if all directories exist.
        msg = "Error with mkdir: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)
    

        # The directory should still exist in root.
        shellOutput = run_commands(["ls"])
        errorCode = validate_output(shellOutput, [".", "..", "Dir"])
        msg = "Error with mkdir, not persistent: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

    # My test made by me, logic inspired by the original auther.
    def special_case_mkdir(self):
        '''Testing mkdir edge cases, deleting cwd and parent is not possible'''
        
        # Attemp to delete dir1, which is not empty. (parent)
        multiCmd = construct_multi_command(["mkdir dir1", "cd dir1", "mkdir dir2", "cd dir2", "cd /", "rmdir dir1"])
        output = run_commands([multiCmd])
      
        # Expecting an error.
        errorCode = validate_output(output, [])
        msg = "Error removing a dir that is not empty: {}".format(errorCode)
        self.assertEqual(errorCode, -11, msg)

        # Attempt to delete current working directory. (cwd)
        multiCmd = construct_multi_command(["mkdir dir1", "cd dir1", "rmdir ."])
        output = run_commands([multiCmd])

        # Expecting an error.
        errorCode = validate_output(output, [])
        msg = "Error removing current working directory: {}".format(errorCode)
        self.assertEqual(errorCode, -1, msg)
    # End selfmade test.

    # Modify by me, names and a bit of logic in input/output.
    def test_cd_relative(self):
        ''' Testing with main focus on the cd command, with relative paths. '''

        # Create a 3-level directory containing one directory.
        multiLineInput = construct_multi_command(["mkdir one",
                                                    "cd one",
                                                    "mkdir two",
                                                    "cd two",
                                                    "mkdir three",
                                                    "cd three",
                                                    "mkdir Findeme"])

        # Create directory three with no errors.
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, [])
        msg = "Error with mkdir or cd: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Test cd with relative paths.
        multiLineInput = construct_multi_command(["cd one",
                                                    "cd two",
                                                    "cd three",
                                                    "cat dog",
                                                    "ls"])
        
        # Expect to find all the directory and file.
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, ["Findeme", "dog"])
        msg = "Error with cd on relative paths: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

    # Modify by me, names and a bit of logic in input/output.
    def test_cd_absolute(self):
        ''' Testing with main focus on the cd command, with absolute paths. '''

        # Create a 5-level directory containing one file.
        multiLineInput = construct_multi_command(["mkdir one",
                                                    "cd one",
                                                    "mkdir two",
                                                    "cd two",
                                                    "mkdir three",
                                                    "cd three",
                                                    "mkdir four",
                                                    "cd four",
                                                    "mkdir five",
                                                    "cd five",
                                                    "cat emu"])

        # Create directory three with no errors.
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, [])
        msg = "Error with mkdir or cd: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Test cd with absolute paths.
        multiLineInput = construct_multi_command(["cd /one/two/three/four/five", "ls"])
        
        # Expect to find all the "emu" directories.
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, ["emu"])
        msg = "Error with cd on absolute paths: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)
    
    def test_rmdir(self):
        ''' Testing with main focus on the rmdir command. '''

        # Create and verify a directory to later remove.
        multiLineInput = construct_multi_command(["mkdir remove_me", "ls"])
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, ["remove_me"])
        msg = "Error creating directory: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Check that removed directory does not exists.
        multiLineInput = construct_multi_command(["rmdir remove_me", "ls"])
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, ["remove_me"])
        msg = "Error removed directory still exists: {}".format(errorCode)
        self.assertEqual(errorCode, -1, msg)

    def test_ls(self):
        ''' Testing with main focus on the ls command. '''

        # Create three directories, and list everything in root.
        multiLineInput = construct_multi_command(["mkdir one", "mkdir two", "mkdir three", "ls"])
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, [".", "..", "one", "two", "three"])

        msg = "Error ls has unexpected behaviour: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

    def test_pwd(self):
        ''' Testing with main focus on the pwd command. '''

        multiLineInput = construct_multi_command(["mkdir mnt",
                                                    "cd mnt",
                                                    "mkdir users",
                                                    "cd users",
                                                    "mkdir ikj023",
                                                    "cd ikj023",
                                                    "mkdir Documents",
                                                    "mkdir Desktop",
                                                    "mkdir Downloads"])

        # Create a directory structure for pwd.
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, [])
        msg = "Error could not create dir. structure: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Check that pwd returns the current working directory.
        multiLineInput = construct_multi_command(["cd mnt/users/ikj023/Documents", "pwd"])
        shellOutput = run_commands([multiLineInput])
        errorCode = validate_output(shellOutput, ["/mnt/users/ikj023/Documents"])
        msg = "Error pwd does not work: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

    def test_cat(self):
        ''' Testing with main focus on the cat command. '''

        # Create three files in the root directory.
        run_commands(["cat file1", "cat file2", "cat file3"])

        # Verify that cat creates the files above.
        output = run_commands(["ls"])
        error = validate_output(output, ["file1", "file2", "file3"])
        msg = "Error cat does not work: {}".format(error)
        self.assertEqual(error, 0, msg)

    def test_more(self):
        ''' Testing with main focus on the more command. '''

        # Create a file and open it afterwards.
        run_commands(["cat myFile123"])
        output = run_commands(["more myFile123"])

        # The file should contain it's own name.
        error = validate_output(output, ["myFile123"])
        msg = "Error more does not work: {}".format(error)
        self.assertEqual(error, 0, msg)

    def test_ln(self):
        ''' Testing with main focus on the ln command. '''

        # Create one directory and one file in root.
        run_commands(["mkdir myDir", "cat myFile123"])

        # Attempt to link a file, and check for any errors.
        multicmd = construct_multi_command(["cd myDir", "ln myLink /myFile123"])
        output = run_commands([multicmd])
        error = validate_output(output, [])
        msg = "Error link does not work: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Validate that the link is working.
        multicmd = construct_multi_command(["cd myDir", "more myLink"])
        output = run_commands([multicmd])
        error = validate_output(output, ["myFile123"])
        msg = "Error invalid link data: {}".format(error)
        self.assertEqual(error, 0, msg)

    def test_rm(self):
        ''' Testing with main focus on the rm command. '''

        # Create a file to delete.
        run_commands(["cat myFile"])

        # Attempt to delete the file, check if it still exists.
        multicmd = construct_multi_command(["rm myFile", "ls"])
        output = run_commands([multicmd])
        error = validate_output(output, ["myFile"])
        msg = "Error file not deleted: {}".format(error)
        self.assertEqual(error, -1, msg)

    def test_stat(self):
        ''' Testing with main focus on the stat command. '''
        
        # Create one file and one directory.
        run_commands(["cat file", "mkdir folder", "ln link1 file", "ln link2 file"])

        # Expect file with two links and five bytes in size.
        output = run_commands(["stat file"])
        error = validate_output(output, ["1", "2", "5"])
        msg = "Error stat not working: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Expect directory with two entries.
        output = run_commands(["stat folder"])
        error = validate_output(output, ["2", "0", "40"])
        msg = "Error stat not working: {}".format(error)
        self.assertEqual(error, 0, msg)

    def tearDown(self):
        cfg.cleanup()

if __name__ == '__main__':
    unittest.main()