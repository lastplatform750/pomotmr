#!/bin/sh

ROOT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/.."

APP="pomotmr"
CL="--enable-timer-log --timer-log-path ${ROOT_DIR}/timer.log --task-names a,b,c,d,e"

${ROOT_DIR}/scripts/build_test.sh

${ROOT_DIR}/builddir/src/${APP} ${CL}