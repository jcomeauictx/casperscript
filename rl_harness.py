#!/usr/bin/env python3
"""
Harness to drive rl_test through pexpect + pyte.
Sends commands to build history, then scrolls with up-arrow and renders
the virtual screen to detect prompt corruption.

Usage: python3 rl_harness.py
"""
import os
import pexpect
import pyte
import sys
import time

os.chdir('/home/jcclaw/src/jcomeauictx/casperscript')

UP    = '\x1b[A'
ENTER = '\r'
COLS, ROWS = 80, 24
PROMPT = 'GS> '
PROMPT_WIDTH = len(PROMPT)  # 4


class ScreenLogger:
    """Feed pexpect output into a pyte screen for analysis."""
    def __init__(self, cols, rows):
        self.screen = pyte.Screen(cols, rows)
        self.stream = pyte.ByteStream(self.screen)

    def write(self, data):
        if isinstance(data, str):
            data = data.encode('utf-8', errors='replace')
        self.stream.feed(data)
        return len(data)

    def flush(self):
        pass

    def row(self, r):
        s = self.screen
        return ''.join(s.buffer[r][c].data for c in range(s.columns)).rstrip()

    def cursor_row_text(self):
        return self.row(self.screen.cursor.y)

    def show(self, label=''):
        if label:
            print(f"\n--- {label} ---")
        for r in range(self.screen.lines):
            text = self.row(r)
            if text:
                marker = ' <-- cursor' if self.screen.cursor.y == r else ''
                print(f"  [{r:2d}] |{text}|{marker}")
        print(f"       cursor col: {self.screen.cursor.x}")


def run():
    sl = ScreenLogger(COLS, ROWS)

    child = pexpect.spawn(
        f'./rl_test {PROMPT_WIDTH}',
        encoding='utf-8',
        timeout=5,
        dimensions=(ROWS, COLS),
    )
    # Route all output through pyte screen AND stdout
    child.logfile_read = sl

    history = ['showpage', '1 2 add =', '(hello) print', '/x 42 def']

    print("=== Building history ===")
    for cmd in history:
        child.expect_exact(PROMPT)
        child.send(cmd + ENTER)
        child.expect_exact(f'got: [{cmd}]')
        print(f"  stored: {cmd!r}")

    # Now at prompt, ready to scroll
    child.expect_exact(PROMPT)
    sl.show("after history built")

    def drain(timeout=0.3):
        """Read all pending output so pyte screen is up to date."""
        buf = ''
        while True:
            try:
                chunk = child.read_nonblocking(size=1024, timeout=timeout)
                buf += chunk
                timeout = 0.05  # shorter for subsequent chunks
            except pexpect.TIMEOUT:
                break
            except pexpect.EOF:
                break
        return buf

    print("\n=== Scrolling through history ===")
    for i in range(len(history) + 1):
        child.send(UP)
        drain()

        sl.show(f"after up-arrow #{i+1}")

        cur_text = sl.cursor_row_text()
        if not cur_text.startswith(PROMPT):
            print(f"  *** CORRUPTION: line is {cur_text!r}, does not start with {PROMPT!r} ***")
        elif sl.screen.cursor.x < PROMPT_WIDTH:
            print(f"  *** CURSOR in prompt area at col {sl.screen.cursor.x} ***")
        else:
            print(f"  OK — line: {cur_text!r}, cursor col: {sl.screen.cursor.x}")

    child.send('\x04')  # EOF
    child.close()
    print("\nDone.")


if __name__ == '__main__':
    run()
