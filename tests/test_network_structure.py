#!/usr/bin/env python3
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'tests'/'network_structure_audit.cpp'
BIN=ROOT/'bin'/'network_structure_audit'
cmd=['g++','-O2','-std=c++17','-Wall','-Wextra','-Wpedantic','-Werror',str(SRC),'-o',str(BIN)]
p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True)
if p.returncode:
    print(p.stdout)
    print(p.stderr)
    raise SystemExit(p.returncode)
p=subprocess.run([str(BIN)],cwd=ROOT,text=True,capture_output=True)
print(p.stdout,end='')
if p.returncode:
    print(p.stderr)
    raise SystemExit(p.returncode)
if 'STRUCTURE AUDIT PASS' not in p.stdout:
    raise SystemExit('STRUCTURE AUDIT FAIL')
