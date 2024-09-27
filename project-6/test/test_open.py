#!/usr/bin/python3

'''
Important note: This test is NOT in any way writen by me. 
    All modifications are marked with a comment and all SELF MADE TEST is also clearly marked with a comment (inspired by the original auther)
Original author: Isak Kjerstad (discord user name: cornersatan42, github user name: isakkjerstad)
Published: Discord server INF-2201-1 23V general the 05.05.2023 9:18 PM
Download from: Github: isakkjerstad /inf2201-p6-tests, got it from the provided discord link.

Modification: test_ls_advanced (line: 75)
Made by me BUT uses the logic of the original auther (3): test_open(self) (line: 154),  
    test_open_open(self) (line: 175), test_open_close_open(self) (line: 204)

Source:
    Written by: Isak Kjerstad.
    Purpose: functions handeling simulator communication and output validation.
    Date: 24.05.22
'''

import unittest
import config as cfg
from sim_comms import run_commands, validate_output

class TestOpen(unittest.TestCase):
    ''' Test fs_open functionality with different shell commands. '''

    def setUp(self):
        cfg.compile()

    def test_ls_simple(self):
        '''
        Simple test of ls on a new file system.
        Used to verify successful compiling, and as a
        baseline example for all the other tests.
        '''

        # Run ls with a new file system.
        shellOutput = run_commands(["ls"])

        # No errors should occour, and dots should be present.
        errorCode = validate_output(shellOutput, [".", ".."])

        # Generate an error message.
        msg = "File system error: {}".format(errorCode)

        # Raise error if any of the commands has failed.
        self.assertEqual(errorCode, 0, msg)

    def test_truncate(self):
        '''
        Test open with truncate on a file.
        '''

        # Create file to later truncate.
        shellOutput = run_commands(["cat trunc_file"])
        errorCode = validate_output(shellOutput, [])
        msg = "Error creating file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Try to truncate existing file.
        shellOutput = run_commands(["cat trunc_file"])
        errorCode = validate_output(shellOutput, [])
        msg = "Error truncating file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Verify content of truncated file.
        shellOutput = run_commands(["more trunc_file"])
        errorCode = validate_output(shellOutput, ["trunc_file"])
        msg = "Error invalid truncated file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

    '''
    This is modified by me, change the max file from 50 to 22 since my memory is limited to 20 inodes.
    '''
    def test_ls_advanced(self):
        '''
        Stress test ls by making a lot of files. This
        also indirectly tests open, cat and close.
        '''

        catCommands = []        # Commands to run.
        expectedFiles = []      # Expected ls result.

        # Use more than one 512 byte block in root.
        for num in range(3, 22+1):
            catCommands.append("cat myFile_" + str(num))
            expectedFiles.append("myFile_" + str(num))

        # Create the files with cat.
        shellOutput = run_commands(catCommands)

        # Creating the files should not cause any errors.
        errorCode = validate_output(shellOutput, [])
        msg = "Error creating files: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # List directory with newly created files.
        shellOutput = run_commands(["ls"])

        # All created files should be present.
        errorCode = validate_output(shellOutput, expectedFiles)
        msg = "Error listing files: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Try to open some of the files. All of them should be found.
        shellOutput = run_commands(["more myFile_3", "more myFile_15", "more myFile_22"])
        errorCode = validate_output(shellOutput, ["myFile_3", "myFile_15", "myFile_22"])
        msg = "Invalid or broken files: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

    def test_open_invalid_name(self):
        '''
        Attempt to open a non-existing file.
        Check for correct behaviour.
        '''

        # Attempt to read a non-existing file.
        shellOutput = run_commands(["more non_existing"])
        errorCode = validate_output(shellOutput, [])

        # Should cause a file does not exist error.
        msg = "File system did not handle error: {}".format(errorCode)
        self.assertEqual(errorCode, -5, msg)

    def test_too_long_name_open(self):
        '''
        Attempt to open a file with a too
        long name. Checks error handeling.
        '''

        # Attempt to open a file with a too long name.
        shellOutput = run_commands(["more my_name_is_too_long"])
        errorCode = validate_output(shellOutput, [])

        # Should cause a name too long error.
        msg = "File system did not handle error: {}".format(errorCode)
        self.assertEqual(errorCode, -4, msg)

    def test_too_long_name_create(self):
        '''
        Attempt to create a file with a too
        long name. Checks error handeling.
        '''
        
        # Attempt to create a file with a too long name.
        shellOutput = run_commands(["cat my_name_is_too_long"])
        errorCode = validate_output(shellOutput, [])

        # Should cause a name too long error.
        msg = "File system did not handle error: {}".format(errorCode)
        self.assertEqual(errorCode, -4, msg)

    # This 3 function is added and made by me, following the same pattern and style/logic as the other (not mabe by me) tests. 
    def test_open(self):
        ''''
        Attempt to open a file. 
        Check for correct behaviour.
        '''
        # Attemt to open a file with the cat command.
        shellOutput = run_commands(["cat file"])
        
        # Creating the file shoud not cause any problems
        errorCode = validate_output(shellOutput, [])
        msg = "Error creating file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

        # Attemt to open a file with that more command.
        shellOutput = run_commands(["more file"])

        # Should not cause any errors, e.g. error list empty.
        errorCode = validate_output(shellOutput, ["file"])
        msg = "Error invalid file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)
      
    def test_open_open(self):
        ''' 
        Attemt to open a file multiple.
        Check for correct behaviour in file handeling when opening a file multiple times.
        '''
        # Create a file to open.
        shellOutput = run_commands(["cat file"])

        # Creating the file shoud not cause any problems
        errorCode = validate_output(shellOutput, [])
        msg = "Error creating file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)
        
        # Attempt to open the file with the more command.
        shellOutput = run_commands(["more file"])

        # Should not cause any errors, e.g. error list empty.
        errorCode = validate_output(shellOutput, ["file"])
        msg = "Error opening file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)
        
        # Attempt to open the file again using the more command.
        shellOutput = run_commands(["more file"])

        # Should not cause any errors, e.g. error list empty.
        errorCode = validate_output(shellOutput, ["file"])
        msg = "Error opening file: {}".format(errorCode)
        self.assertEqual(errorCode, 0, msg)

    def test_open_close_open(self):
        '''
        Attemt to create and open a deletet file
        Check for correct behaviour.
        '''
        # Create a file to open.
        shellOutput = run_commands(["cat file"])
        
        # Attempt to open the file with the more command.
        shellOutput = run_commands(["more file"])

        # Attempt to close the file with the close command.
        shellOutput = run_commands(["rm file"])

        # Attempt to open the file with the more command.
        shellOutput = run_commands(["more file"])

        # Should cause a file does not exist error.
        errorCode = validate_output(shellOutput, [])
        msg = "Error opening file: {}".format(errorCode)
        self.assertEqual(errorCode, -5, msg)
    
    # End of self made tests.
    def tearDown(self):
        cfg.cleanup()

if __name__ == '__main__':
    unittest.main()