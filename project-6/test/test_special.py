#!/usr/bin/python3

"""
Important note: This test is NOT in any way writen by me. All SELF MADE TEST is clearly marked with a comment (inspired by the original auther).
Original author: Isak Kjerstad (discord user name: cornersatan42, github user name: isakkjerstad)
Published: Discord server INF-2201-1 23V general the 05.05.2023 9:18 PM
Download from: Github: isakkjerstad /inf2201-p6-tests, got it from the provided discord link.

Modification: test_inodes_and_directs(self) (line: 113)
Made by me in the style and logic of original auther (1): test_multi_level_dirs(self) (line: 83), test_cd_direct_to_root(self) (line: 243)

Source:
    Written by: Isak Kjerstad.
    Purpose: functions handeling simulator communication and output validation.
    Date: 24.05.22
"""

import unittest
import config as cfg
from sim_comms import run_commands, validate_output, construct_multi_command, cat_write


class TestSpecialCases(unittest.TestCase):
    """
    Advanced file system tests to validate that inodes, data blocks,
    directory blocks, bitmaps and so on works correctly. The tests does
    not cover all cases, but should give a good indication on correct behaviour.
    All tests in this class expects that simple functionality already works.
    """

    def setUp(self):
        cfg.compile()

    def test_cat_multiblock(self):
        """Test read and write with more than one data block."""

        stringList = []

        # Create a string list with unique words.
        for num in range(0, 100):
            stringList.append("Hello World! Check out myWord_" + str(num))

        # Writes more than 512 bytes to a file.
        error = cat_write("myTextFile", stringList)
        msg = "Cat multi-block write error: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Check output when reading the file.
        output = run_commands(["more myTextFile", "stat myTextFile"])

        # Validate the stat command output, mainly check file size.
        error = validate_output(output, ["myTextFile:", "1", "0", "3290"])
        msg = "Cat invalid file size with more: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Get list of words from stringList.
        searchWords = []
        for string in stringList:
            for word in string.split():
                searchWords.append(word)

        # Validate the more command output, expect to find the text.
        error = validate_output(output, searchWords)
        msg = "Invalid multi-block file read: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Truncate the large file with a new input.
        error = cat_write("myTextFile", ["The file has been truncated!"])
        msg = "Error truncating multi-block file: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Check size of the truncated file.
        output = run_commands(["stat myTextFile"])
        error = validate_output(output, ["myTextFile:", "1", "0", "29"])
        msg = "Wrong size of truncated multi-block file: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Read and verify the output of the truncated file.
        output = run_commands(["more myTextFile"])
        error = validate_output(output, ["The", "file", "has", "been", "truncated!"])
        msg = "Invalid multi-block truncated file read: {}".format(error)
        self.assertEqual(error, 0, msg)

    # Stript the original test and made it my own following the provided logic.
    def test_multi_level_dirs(self):
        """Verify that the file system works with multi-level directories."""

        depth = 6
        mkdirStr = "mkdir"
        cdStr = "cd"
        multiCmd = ""

        # Create command and cd strings.
        for dir in range(depth + 1):
            mkdirStr += "d" + str(dir) + " "
            cdStr += "d" + str(dir) + " "
            multiCmd += mkdirStr + "\n" + cdStr + "\n"

        # Build the multi-level directory structure.
        output = run_commands([multiCmd])
        error = validate_output(output, [])
        msg = "Failed to create multi-level directories: {}".format(error)
        self.assertEqual(error, 0, msg)

        multiCmd = construct_multi_command(["cd /", "stat d0"])
        output = run_commands([multiCmd])
        # Update the expected attributes of the "d0" directory based on your file system
        expected_output = ["d0: ", "2", "0", "40"]
        error = validate_output(output, expected_output)
        msg = "Failed to find the first directory in the multi-level directory structure: {}".format(error)
        self.assertEqual(error, 0, msg)
     
    def test_inodes_and_directs(self):
        """
        Test that inodes are freed and handeled correctly by the file system.
        In addition, test that the direct pointers work as expected.
        """

        dirs = 20
        multiCmd = ""
        dirNames = []

        # Build command for creating 200 directories.
        for dir in range(3, dirs + 1):
            multiCmd += "mkdir d{}".format(dir) + "\n"
            dirNames.append("d{}".format(dir))

        # Create all the directories.
        output = run_commands([multiCmd])
        error = validate_output(output, [])
        msg = "Failed to create 20 directories: {}".format(error)
        self.assertEqual(error, 0, msg)

        # List all directories.
        output = run_commands(["ls"])
        error = validate_output(output, dirNames)
        msg = "Failed to list all 200 directories: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Attempt to create one more directory, should fail.
        output = run_commands(["mkdir d21"])
        error = validate_output(output, [])
        msg = "Exceeded the direct pointer limit: {}".format(error)
        self.assertEqual(error, -17, msg)

        # Remove and create the last directory.
        output = run_commands(["rmdir d20", "mkdir d20", "ls"])
        error = validate_output(output, ["d20"])
        msg = "Remove does not free direct pointers: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Enter a directory and create 50 more directories, uses all inodes.
        multiCmd = "cd d17\n"
        dirNames = []
        subDirs = 5

        # Build command for creating 50 directories.
        for subDir in range(3, subDirs + 1):
            multiCmd += "mkdir sd{}".format(subDir) + "\n"
            dirNames.append("sd{}".format(subDir))

        # Create more directories.
        output = run_commands([multiCmd])
        error = validate_output(output, [])
        msg = "Failed to create 50 directories in sub-directory: {}".format(error)
        self.assertEqual(error, 0, msg)

        # List all directories.
        multiCmd = construct_multi_command(["cd d173", "ls"])
        output = run_commands([multiCmd])
        error = validate_output(output, dirNames)
        msg = "Failed to list all 50 sub-directories: {}".format(error)
        self.assertEqual(error, 0, msg)

        # Attempt to create one more directory, should fail.
        multiCmd = construct_multi_command(["cd d173", "mkdir sd51"])
        output = run_commands([multiCmd])
        error = validate_output(output, [])
        msg = "Exceeded the data block limit: {}".format(error)
        self.assertEqual(error, -16, msg)

        # Remove and create the last sub-directory.
        multiCmd = construct_multi_command(
            ["cd d173", "rmdir sd50", "mkdir sd50", "ls"]
        )
        output = run_commands([multiCmd])
        error = validate_output(output, ["sd50"])
        msg = "Remove does not free data blocks: {}".format(error)
        self.assertEqual(error, 0, msg)

    def test_bitmaps(self):
        """Test that bitmaps does not cause any errors."""

        # Command for making directories.
        multiCmdMkdirs = construct_multi_command(
            [
                "mkdir dirOne",
                "mkdir dirTwo",
                "mkdir dirThree",
                "mkdir dirFour",
                "mkdir dirFive",
                "mkdir dirSix",
            ]
        )

        # Command for removing directories.
        multiCmdRmdirs = construct_multi_command(
            [
                "rmdir dirOne",
                "rmdir dirTwo",
                "rmdir dirThree",
                "rmdir dirFour",
                "rmdir dirFive",
                "rmdir dirSix",
            ]
        )

        # Attempt to exhaust bitmap/data blocks.
        for steps in range(0, 50):
            # Create directories
            output = run_commands([multiCmdMkdirs])
            error = validate_output(output, [])
            msg = "Could not create directories: {}".format(error)
            self.assertEqual(error, 0, msg)

            # Remove directories, test free.
            output = run_commands([multiCmdRmdirs])
            error = validate_output(output, [])
            msg = "Could not remove directories: {}".format(error)
            self.assertEqual(error, 0, msg)

        # Check empty but valid root folder.
        output = run_commands(["ls"])
        error = validate_output(output, [".", ".."])
        msg = "Root directory is damaged: {}".format(error)
        self.assertEqual(error, 0, msg)

    # This 1 function is added and made by me, following the same pattern and style/logic as the other (not mabe by me) tests
    def test_cd_direct_to_root(self):
        """Check that changing directory to root works on one move."""

        # Attempt to cd from a folder back to root.
        multiCmd = construct_multi_command(
            [
                "mkdir dir1",
                "cd dir1",
                "mkdir dir2",
                "cd dir2",
                "mkdir dir3",
                "cd dir3",
                "mkdir dir4",
                "cd dir4",
                "cd /",
            ]
        )
        output = run_commands([multiCmd])

        # Expected output no error.
        error = validate_output(output, ["dir1"])
        msg = "Changing directory to root does not work: {}".format(error)
        self.assertEqual(error, 0, msg)

    # End of self made tests

    def tearDown(self):
        cfg.cleanup()


if __name__ == "__main__":
    unittest.main()
