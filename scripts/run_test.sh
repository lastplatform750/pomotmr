#!/bin/sh

ROOT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/.."

${ROOT_DIR}/scripts/build_test.sh

${ROOT_DIR}/builddir/src/pomotmr short 2> "${ROOT_DIR}/test.log"