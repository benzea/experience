#!/bin/sh

if ! [ -d 'ac-helpers' ] ; then
  if ! mkdir 'ac-helpers' ; then
    echo "failed to create ac-helpers directory" >&2
    exit 1
  fi
fi


aclocal-1.8

autoconf
libtoolize --copy --force
automake-1.8 -a --copy

rm -rf autom4te.cache

echo ""
echo "##################################################"
echo ""
echo "You can now run ./configure"
echo "--"
echo "Or create a directory for building everything. (Useful for distributions)"
echo "eg. $ mkdir builddir; cd builddir; ../configure"
