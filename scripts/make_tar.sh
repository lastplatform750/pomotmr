#!/bin/sh


ROOT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/.."

BUILD_DIR="$ROOT_DIR/builddir"

STAGING="$ROOT_DIR/dist/staging"
BUNDLE_DIR="$ROOT_DIR/dist/pomotmr"
VERSION=0.1.0
APP=pomotmr

# setup meson if needed
if [ ! -f ${BUILD_DIR}/meson-private/coredata.dat ]; then
  meson setup "$BUILD_DIR" "$ROOT_DIR"
fi

# install to staging
rm -rf "$STAGING"
meson install -C "$BUILD_DIR" --destdir="$STAGING"

# compose bundle
rm -rf "$BUNDLE_DIR"
mkdir -p "$BUNDLE_DIR/resource"

# copy executable
cp "$STAGING/usr/local/bin/$APP" "$BUNDLE_DIR/"

# copy resources
cp -r "$STAGING/usr/local/share/$APP/resource/"* "$BUNDLE_DIR/resource/"

# optional: strip
if command -v strip >/dev/null 2>&1; then
  strip "$BUNDLE_DIR/$APP" || true
fi

# create tarball
tar -C "$ROOT_DIR/dist" -czf "${ROOT_DIR}/dist/pomotmr-${VERSION}-linux.tar.gz" "$(basename "$BUNDLE_DIR")"

echo "Bundle created:"
echo "  dist/pomotmr/"
echo "  dist/pomotmr-${VERSION}-linux.tar.gz"