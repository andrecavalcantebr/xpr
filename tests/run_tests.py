#!/usr/bin/env python3
# /*
#    FILE: run_tests.py
#    DESC: Check exact output and structured failures of XPR test cases.
#    AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
#    DATE: October, 2026
#    LICENSE: GPL-v3
# */

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys
import tomllib


ROOT = Path(__file__).resolve().parent.parent
TESTS = ROOT / "tests"


def collect_cases(suite):
    cases = []
    if suite in ("lexer", "all"):
        for source in sorted(TESTS.glob("*.x")):
            cases.append(("lexer", source, source.with_suffix(".expected"), None))
    if suite in ("runtime", "all"):
        runtime = TESTS / "runtime"
        for source in sorted((runtime / "cases").glob("*.x")):
            expected = runtime / "expected" / (source.stem + ".expected")
            cases.append(("runtime", source, expected, None))
        entries = tomllib.loads((runtime / "errors.toml").read_text())["case"]
        names = [entry["file"] for entry in entries]
        sources = {path.name for path in (runtime / "errors").glob("*.x")}
        if len(names) != len(set(names)) or set(names) != sources:
            raise ValueError("Negative sources and manifest entries must match exactly")
        for entry in entries:
            source = runtime / "errors" / entry["file"]
            lines = source.read_text().splitlines()
            if not 1 <= entry["line"] <= len(lines):
                raise ValueError(f"Invalid diagnostic line for {source.name}")
            line = lines[entry["line"] - 1]
            if not 1 <= entry["column"] <= len(line) + 1:
                raise ValueError(f"Invalid diagnostic column for {source.name}")
            cases.append(("runtime", source, None, entry))
    for _, source, expected, _ in cases:
        if expected is not None and not expected.is_file():
            raise ValueError(f"Missing expected output for {source.name}")
    return cases


def check_result(result, expected, error, source):
    problems = []
    if result.stdout != expected:
        problems.append(f"stdout: expected {expected!r}, got {result.stdout!r}")
    if error is None:
        if result.returncode != 0:
            problems.append(f"expected exit 0, got {result.returncode}")
        if result.stderr:
            problems.append(f"unexpected stderr: {result.stderr!r}")
        return problems
    # A crash is not evidence of the requested language error.
    if result.returncode <= 0 or result.returncode >= 128:
        problems.append(f"expected an ordinary error exit (1..127), got {result.returncode}")
    stderr = result.stderr.decode("utf-8", errors="replace")
    code_pattern = r"\[(?:syntax-error|wrong-assignment|empty-program|size-mismatch|type-mismatch|undefined-identifier|div-by-zero|not-a-function|stack-overflow|index-out-of-range)\]"
    codes = re.findall(code_pattern, stderr)
    if codes != [f"[{error['error']}]"]:
        problems.append(f"expected one [{error['error']}] diagnostic, got {codes!r}")
    location = rf"(?:^|[/\\\s]){re.escape(source.name)}:{error['line']}:{error['column']}:"
    matching_lines = [line for line in stderr.splitlines() if re.search(location, line)]
    if not any(f"[{error['error']}]" in line for line in matching_lines):
        problems.append("expected file:line:column and error code on the same diagnostic line")
    if problems:
        problems.append(f"stderr: {result.stderr!r}")
    return problems


def main():
    parser = argparse.ArgumentParser(description="Run XPR lexer and runtime contract tests.")
    parser.add_argument("--suite", choices=("lexer", "runtime", "all"), default="lexer")
    parser.add_argument("--binary", default=os.environ.get("XPR", str(ROOT / "xpr")))
    parser.add_argument("--lexer-arg", action="append", default=[], help="future lexer mode, e.g. --lexer-arg=--lex")
    parser.add_argument("--case", help="select an exact source stem")
    parser.add_argument("--list", action="store_true", help="validate and list fixtures without executing XPR")
    parser.add_argument("--timeout", type=float, default=5.0)
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    try:
        cases = collect_cases(args.suite)
    except (OSError, ValueError, KeyError) as error:
        parser.error(str(error))
    if args.case:
        cases = [case for case in cases if case[1].stem == args.case]
    if not cases:
        parser.error("no matching tests")
    if args.list:
        for suite, source, _, error in cases:
            kind = error["error"] if error else "exact stdout"
            print(f"{suite}: {source.relative_to(ROOT)} ({kind})")
        print(f"Total: {len(cases)}")
        return 0
    binary = str(Path(args.binary).expanduser().resolve())
    failed = 0
    for suite, source, output_file, error in cases:
        expected = output_file.read_bytes() if output_file else b""
        command = [binary, *(args.lexer_arg if suite == "lexer" else []), str(source)]
        try:
            result = subprocess.run(command, cwd=ROOT, input=b"", capture_output=True, timeout=args.timeout)
            problems = check_result(result, expected, error, source)
        except (OSError, subprocess.TimeoutExpired) as exception:
            problems = [str(exception)]
        if problems:
            failed += 1
            print(f"FAIL {suite}/{source.stem}")
            for problem in problems:
                print(f"  {problem}")
        else:
            print(f"PASS {suite}/{source.stem}")
    print(f"Total: {len(cases)}; passed: {len(cases) - failed}; failed: {failed}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
