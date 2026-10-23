#!/bin/bash
# /*
#    FILE: run_tests.sh
#    DESC: Run the XPR test driver from any working directory.
#    AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
#    DATE: October, 2026
#    LICENSE: GPL-v3
# */
set -eu
exec python3 "$(dirname -- "$0")/run_tests.py" "$@"
