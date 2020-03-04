#!/usr/bin/python3

import os
import sys
import subprocess

def run(command, **kwargs):
    print("Running", command)
    return subprocess.run(command, **kwargs)

#def usage_test(exe, test):
#    stderr = run([exe], stderr = subprocess.PIPE, text = True, check = True).stderr
#    usage  = "{}: Usage: {} user-spec command [args]".format(os.path.basename(exe), exe)
#    if stderr != usage:
#        raise AssertionError("stderr '{}' != usage '{}'".format(stderr, usage))

def spec_test(exe, test):
    for spec in test["specs"]:
        run([exe, spec, "./Assert.py"] + test["asserted"], check = True)

tests = [
            {
                "specs": ["", ":", "0",  "root", "0:", "root:", ":0", ":root", "root:root", "root:0", "0:root"],
                "asserted": ["0", "0", "[0]", "root", "root", "/root"],
                "func": spec_test
            },
            {
                "specs": ["test1", "1000"],
                "asserted": ["1000", "1000", "[1000, 1002]", "test1", "test1", "/home/test1"],
                "func": spec_test
            },
            {
                "specs": ["1000:1000", "test1:1000", "1000:test1", "test1:test1"],
                "asserted": ["1000", "1000", "[1000]", "test1", "test1", "/home/test1"],
                "func": spec_test
            },
            {
                "specs": ["test2", "test2:test2", "1001:1001", "test2:1001", "1001:test2"],
                "asserted": ["1001", "1001", "[1001]", "test2", "test2", "/home/test2"],
                "func": spec_test
            },
            {
                "specs": ["test1:test2", "1000:1001"],
                "asserted": ["1000", "1001", "[1001]", "test1", "test1", "/home/test1"],
                "func": spec_test
            },
            {
                "specs": ["10293:29304"],
                "asserted": ["10293", "29304", "[29304]", "10293", "10293", "/"],
                "func": spec_test
            }
        ]

def main():
    for exe in sys.argv[1 : ]:
        for test in tests:
            test["func"](exe, test)

if __name__ == "__main__":
    try:
        main()
    except AssertionError as e:
        print("AssertionError:", e)
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print("subprocess.CalledProcessError", e)
        sys.exit(1)
