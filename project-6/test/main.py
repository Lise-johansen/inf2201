#!/usr/bin/python3

'''
Integration test of the file system, via the shell program.

Written by: Isak Kjerstad.
Purpose: runs all the given tests on the file system.
Date: 24.05.22

Supported Python version: 3.8.10
Supported OS: Linux (Ubuntu 18.04.6 LTS)

Run: "python3 main.py" to test the file system.
'''

import unittest

# Import unit tests to run.
from test_open import *
from test_funcs import *
from test_special import *

# Execute all imported tests.
if __name__ == '__main__':
    print("Running file system tests:\n")
    unittest.main()