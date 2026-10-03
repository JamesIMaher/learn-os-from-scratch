#!/usr/bin/env python3
"""
check.py - boot the test kernel in QEMU and report the checkpoint results.

Run it through the Makefile:  make check L=<lesson>

How it works: the test kernel writes a line-based protocol to QEMU's
"debugcon" device (I/O port 0xE9), which QEMU saves to build/debugcon.log.
Your serial port output goes to build/serial.log. This script follows the
debugcon log while QEMU runs, types keys when a test asks for them (through
the QEMU monitor), and finally checks that every string the tests expected on
your serial port actually showed up there.
"""
import argparse
import json
import os
import socket
import subprocess
import sys
import time

GREEN, RED, YELLOW, BOLD, DIM, RESET = "\033[32m", "\033[31m", "\033[33m", "\033[1m", "\033[2m", "\033[0m"
if not sys.stdout.isatty():
    GREEN = RED = YELLOW = BOLD = DIM = RESET = ""


class Monitor:
    """Minimal QMP client, used only to send key presses."""

    def __init__(self, path):
        self.path = path
        self.sock = None

    def connect(self, deadline):
        while time.time() < deadline:
            try:
                s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                s.connect(self.path)
                self.sock = s
                self.file = s.makefile("rw")
                self.file.readline()                       # greeting
                self.cmd({"execute": "qmp_capabilities"})
                return True
            except OSError:
                time.sleep(0.05)
        return False

    def cmd(self, obj):
        self.file.write(json.dumps(obj) + "\n")
        self.file.flush()
        while True:
            reply = json.loads(self.file.readline())
            if "return" in reply or "error" in reply:
                return reply

    def sendkey(self, key):
        self.cmd({"execute": "human-monitor-command",
                  "arguments": {"command-line": "sendkey " + key}})


def unescape(token):
    return token.replace("\\r", "\r").replace("\\n", "\n").replace("\\t", "\t")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--qemu", default="qemu-system-i386")
    ap.add_argument("--kernel", required=True)
    ap.add_argument("--lesson", type=int, required=True)
    ap.add_argument("--timeout", type=float, default=None)
    ap.add_argument("qemu_args", nargs="*")
    args = ap.parse_args()

    build = os.path.dirname(os.path.dirname(os.path.abspath(args.kernel)))
    debug_log = os.path.join(build, "debugcon.log")
    serial_log = os.path.join(build, "serial.log")
    qemu_log = os.path.join(build, "qemu.log")
    qmp_path = os.path.join(build, "qmp.sock")
    for p in (debug_log, serial_log, qmp_path):
        if os.path.exists(p):
            os.remove(p)

    timeout = args.timeout or (30 + 6 * args.lesson)
    cmd = [args.qemu, "-kernel", args.kernel, "-append", "lesson=%d" % args.lesson,
           "-display", "none",
           "-serial", "file:" + serial_log,
           "-debugcon", "file:" + debug_log,
           "-device", "isa-debug-exit,iobase=0xf4,iosize=0x04",
           "-qmp", "unix:%s,server=on,wait=off" % qmp_path,
           "-d", "cpu_reset,guest_errors", "-D", qemu_log] + args.qemu_args

    print(f"{BOLD}Booting the test kernel for lessons 1..{args.lesson}{RESET}")
    print(DIM + " ".join(cmd) + RESET)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    mon = Monitor(qmp_path)
    mon_ok = mon.connect(time.time() + 5)

    expected, passed, failed = [], [], []
    current_test, current_lesson, done = None, None, False
    last_finished = None
    pos, partial = 0, ""
    start = time.time()
    timed_out = False
    panicked_at = None

    def handle(line):
        nonlocal current_test, current_lesson, done, last_finished
        if not line.startswith("@@"):
            return
        kind, _, rest = line[2:].partition(" ")
        if kind == "LESSON":
            current_lesson = rest
            print(f"\n{BOLD}Lesson {rest}{RESET}")
        elif kind == "BEGIN":
            current_test = rest
        elif kind == "PASSED":
            passed.append(rest)
            print(f"  {GREEN}PASS{RESET}  {rest}")
            current_test, last_finished = None, rest
        elif kind == "FAILED":
            failed.append(rest)
            print(f"  {RED}FAIL{RESET}  {rest}")
            current_test, last_finished = None, rest
        elif kind == "FAIL":
            print(f"        {RED}{rest}{RESET}")
        elif kind == "NOTE":
            print(f"        {YELLOW}note: {rest}{RESET}")
        elif kind == "EXPECT-SERIAL":
            expected.append((current_test, rest))
        elif kind == "SENDKEYS":
            if not mon_ok:
                print(f"        {RED}could not reach the QEMU monitor to type keys{RESET}")
                return
            for key in rest.split():
                mon.sendkey(key)
                time.sleep(0.04)
        elif kind == "DONE":
            done = True

    while True:
        if os.path.exists(debug_log):
            with open(debug_log, "rb") as f:
                f.seek(pos)
                chunk = f.read()
                pos += len(chunk)
            partial += chunk.decode("latin-1")
            while "\n" in partial:
                line, partial = partial.split("\n", 1)
                handle(line)
        if proc.poll() is not None:
            if os.path.exists(debug_log) and os.path.getsize(debug_log) > pos:
                continue
            break
        if panicked_at is None and os.path.exists(serial_log):
            with open(serial_log, "rb") as f:
                if b"KERNEL PANIC" in f.read():
                    panicked_at = time.time()
        if panicked_at and time.time() - panicked_at > 1.0:
            proc.kill()
            proc.wait()
            break
        if time.time() - start > timeout:
            timed_out = True
            proc.kill()
            proc.wait()
            break
        time.sleep(0.02)

    out = proc.stdout.read().decode(errors="replace").strip() if proc.stdout else ""
    serial = ""
    if os.path.exists(serial_log):
        with open(serial_log, "rb") as f:
            serial = f.read().decode("latin-1")

    if expected:
        print(f"\n{BOLD}Serial output checks{RESET}")
    for test, token in expected:
        if unescape(token) in serial:
            print(f"  {GREEN}PASS{RESET}  serial output contains {token!r}")
        else:
            failed.append("serial: " + token)
            print(f"  {RED}FAIL{RESET}  serial output should contain {token!r}  {DIM}(from: {test}){RESET}")

    print()
    problem = None
    if not done:
        reset = os.path.exists(qemu_log) and "CPU Reset" in open(qemu_log, errors="replace").read()
        if current_test:
            where = f" during {BOLD}{current_test}{RESET}"
        elif last_finished:
            where = f" right after {BOLD}{last_finished}{RESET} (in the next lesson's setup?)"
        else:
            where = ""
        if pos == 0 and ("PVH" in out or "multiboot" in out.lower()):
            problem = (f"QEMU could not boot the kernel:\n    {out}\n"
                       f"    That means QEMU found no valid Multiboot header (lesson 01).")
        elif pos == 0 and out:
            problem = f"QEMU could not boot the kernel:\n    {out}"
        elif pos == 0:
            problem = "the kernel never reached the test runner (is your boot code done? lesson 01)"
        elif panicked_at:
            problem = f"your kernel panicked{where}"
        elif timed_out:
            problem = f"the run timed out after {timeout:.0f}s{where} (a hang? an infinite loop? interrupts off?)"
        elif reset:
            problem = (f"the CPU reset itself{where}: almost certainly a TRIPLE FAULT.\n"
                       f"    See docs/debugging.md; build/qemu.log may help.")
        else:
            problem = f"QEMU exited unexpectedly{where}"
            if out:
                problem += ":\n    " + out
    if problem:
        print(f"{RED}{BOLD}Run did not finish:{RESET} {problem}")
        tail = [l for l in serial.replace("\r", "").split("\n") if l.strip()][-6:]
        if tail:
            print(f"{DIM}    last lines of your serial output:{RESET}")
            for l in tail:
                print(f"    | {l}")
    total = len(passed) + len(failed)
    colour = GREEN if not failed and not problem else RED
    print(f"{colour}{BOLD}{len(passed)}/{total} checks passed{RESET}"
          + (f"  {DIM}(serial log: {serial_log}){RESET}" if failed else ""))
    sys.exit(0 if (done and not failed) else 1)


if __name__ == "__main__":
    main()
