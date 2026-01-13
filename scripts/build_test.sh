#!/bin/bash

ROOT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/.."

echo $ROOT_DIR

meson compile -C builddir

chmod +x "${ROOT_DIR}/builddir/src/pomotmr"