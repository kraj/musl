#!/bin/sh

if test -d .git ; then
if type git >/dev/null 2>&1 ; then
v=$(git describe --tags --match 'v[0-9]*' 2>/dev/null \
| sed -e 's/^v//' -e 's/-/-git-/')
fi
# git describe finds nothing in a clone without tags (e.g. a build
# system checkout); fall back to the VERSION file then too.
if test -n "$v" ; then
echo "$v"
else
sed 's/$/-git/' < VERSION
fi
else
cat VERSION
fi
