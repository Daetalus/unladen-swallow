# Test that gdb can debug JITted Python code.

import os
import re
import subprocess
import sys
import unittest

from test.test_support import run_unittest, TestSkipped

try:
    import _llvm
except ImportError:
    raise TestSkipped("Built without JIT support")

try:
    gdb_version, _ = subprocess.Popen(["gdb", "--version"],
                                      stdout=subprocess.PIPE).communicate()
except OSError:
    # This is what "no gdb" looks like.  There may, however, be other
    # errors that manifest this way too.
    raise TestSkipped("Couldn't find gdb on the path")
gdb_version_number = re.search(r"^GNU gdb [^\d]*(\d+)\.", gdb_version)
if int(gdb_version_number.group(1)) < 7:
    raise TestSkipped("gdb versions before 7.0 didn't support JIT debugging."
                      " Saw:\n" + gdb_version)


class DebuggerTests(unittest.TestCase):

    """Test that the debugger can debug Python."""

    def run_gdb(self, *args):
        """Runs gdb with the command line given by *args. Returns its stdout.

        Forwards stderr to the current process's stderr.
        """
        # err winds up empty.
        out, err = subprocess.Popen(
            args, stdout=subprocess.PIPE,
            stderr=None,  # Forward stderr to the current process's stderr.
            env=dict(PYTHONLLVMFLAGS="-jit-emit-debug", **os.environ)
            ).communicate()
        return out

    def test_gets_stack_trace(self):
        gdb_output = self.run_gdb("gdb", "--batch",
                                  "--eval-command=break PyObject_Print",
                                  "--eval-command=run",
                                  "--eval-command=backtrace",
                                  "--eval-command=continue",
                                  "--args",
                                  sys.executable, "-c", """
import _llvm
def foo(): bar()
def bar(): baz()
def baz(): print 'Hello, World!'
for function in (foo, bar, baz):
    _llvm.compile(function.__code__, 2)
foo()""")
        # Get the indices of each function in the stack trace.
        foo, bar, baz, output = map(
            gdb_output.find, ("_23_u_23_foo", "_23_u_23_bar",
                              "_23_u_23_baz", "Hello, World!"))
        # str.find returns -1 on failure, so this makes sure each
        # string is in the output.
        self.assertTrue(-1 not in (foo, bar, baz, output), msg=gdb_output)
        # And now we make sure they're in the right order in the backtrace.
        self.assertTrue(baz < bar < foo < output, msg=gdb_output)



def test_main():
    run_unittest(DebuggerTests)


if __name__ == "__main__":
    test_main()
