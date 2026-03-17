#!/bin/bash

APP="pomotmr"

ROOT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/.."

meson compile -C builddir
chmod +x "${ROOT_DIR}/builddir/src/${APP}"