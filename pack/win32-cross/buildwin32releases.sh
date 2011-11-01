#!/bin/bash
curpath=`pwd`
BASEDIR=`readlink -f \`dirname $0\``
SRCDIR=`readlink -f $BASEDIR/../..`
BUILDDIR=$BASEDIR/build-win32
VERSION=`grep "set(VERSION " $SRCDIR/CMakeLists.txt |sed "s/set(VERSION \(.*\))/\1/"`
TOOLCHAIN=$BASEDIR/Toolchain-win32.cmake
QTPATH=`grep "^set(QT_ROOT" $TOOLCHAIN |sed "s/set(QT_ROOT \(.*\))/\1/"`
NSIFILE=$BASEDIR/tagainijisho.nsi
MINGWDLLPATH=${MINGWDLLPATH-/usr/i486-mingw32/lib}

mkdir -p $BUILDDIR/i18n
for lang in fr de es ru nl cs;
do
	touch $BUILDDIR/i18n/qt_$lang.qm
	lrelease $QTPATH/translations/qt_$lang.ts -qm $BUILDDIR/i18n/qt_$lang.qm
done

cd $BUILDDIR
cmake -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN -DCMAKE_BUILD_TYPE=Release -DEMBED_SQLITE=1 $SRCDIR
make tagainijisho -j5
rm -f CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release $SRCDIR
make i18n databases docs -j5

cd $BASEDIR
makensis -DVERSION=$VERSION -DBUILDDIR=$BUILDDIR -DSRCDIR=$SRCDIR -DQTPATH=$QTPATH -DMINGWDLLPATH=$MINGWDLLPATH $NSIFILE
mv install.exe tagainijisho-$VERSION.exe
rm -Rf $BUILDDIR
cd $curpath
