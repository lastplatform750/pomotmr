#!/bin/sh

ROOT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/.."

${ROOT_DIR}/scripts/build_test.sh

${ROOT_DIR}/builddir/src/pomotmr --test --enable-server --enable-timer-log --timer-log-path ${ROOT_DIR}/timer.log