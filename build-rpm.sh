#!/bin/bash
# Build the RPM from this directory: ./build-rpm.sh [rpmbuild topdir]
set -euo pipefail
here=$(dirname "$(readlink -f "$0")")
name=plasma-windows-can-cover
version=$(sed -n 's/^Version:[[:space:]]*//p' "$here/$name.spec")
top=${1:-$HOME/rpmbuild}
mkdir -p "$top"/{SOURCES,SPECS,BUILD,RPMS,SRPMS}
tar -C "$(dirname "$here")" --transform "s|^$(basename "$here")|$name-$version|" \
    --exclude-vcs --exclude='*.diff' --exclude=bugreport -czf "$top/SOURCES/$name-$version.tar.gz" "$(basename "$here")"
rpmbuild --define "_topdir $top" -ba "$here/$name.spec"
