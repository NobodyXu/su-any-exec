#!/usr/bin/python3

import sys
import os

def assertEqual(expr1, expr2):
    result1 = eval(expr1)
    result2 = eval(expr2)
    if result1 != result2:
        msg = "{} '{}' != {}  '{}'".format(expr1, result1, expr2, result2)
        raise AssertionError(msg)

def main():
    assertEqual("int(sys.argv[1])", "os.getuid()")
    assertEqual("int(sys.argv[2])", "os.getgid()")
    assertEqual("sys.argv[3]", "str(os.getgroups())")
    
    assertEqual("sys.argv[4]", "os.getenv('USER')")
    assertEqual("sys.argv[5]", "os.getenv('LOGNAME')")
    assertEqual("sys.argv[6]", "os.getenv('HOME')")

if __name__ == "__main__":
    try:
        main()
    except AssertionError as e:
        print("AssertionError:", e)
        sys.exit(1)
