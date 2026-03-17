#!/bin/sh

APP="pomotmr"
VERSION=0.1.0

ROOT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/.."
BUILD_DIR="${ROOT_DIR}/builddir"
STAGING_DIR="${ROOT_DIR}/dist/staging"
BUNDLE_DIR="${ROOT_DIR}/dist/${APP}"

# setup meson if needed
if [ ! -f ${BUILD_DIR}/meson-private/coredata.dat ]; then
  meson setup "${BUILD_DIR}" "${ROOT_DIR}"
fi

# install to staging
rm -rf "${STAGING_DIR}"
meson install -C "${BUILD_DIR}" --destdir="${STAGING_DIR}"

# compose bundle
rm -rf "${BUNDLE_DIR}"
mkdir -p "${BUNDLE_DIR}/resource"

# copy executable
cp "${STAGING_DIR}/usr/local/bin/${APP}" "${BUNDLE_DIR}/"

# copy resources
cp -r "${STAGING_DIR}/usr/local/share/${APP}/resource/"* "${BUNDLE_DIR}/resource/"

# optional: strip
if command -v strip >/dev/null 2>&1; then
  strip "${BUNDLE_DIR}/${APP}" || true
fi

# create tarball
tar -C "${ROOT_DIR}/dist" -czf "${ROOT_DIR}/dist/pomotmr-${VERSION}-linux.tar.gz" "$(basename "${BUNDLE_DIR}")"

echo "Bundle created:"
echo "  dist/${APP}/"
echo "  dist/${APP}-${VERSION}-linux.tar.gz"