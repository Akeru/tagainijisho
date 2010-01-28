#!/bin/sh
SQLITE_VERSION=3.6.22
SQLITE_SOURCE=http://www.sqlite.org/sqlite-amalgamation-${SQLITE_VERSION}.tar.gz
JMDICT_SOURCE=ftp://ftp.monash.edu.au/pub/nihongo/JMdict.gz
KANJIDIC2_SOURCE=http://www.csse.monash.edu.au/~jwb/kanjidic2/kanjidic2.xml.gz
KANJIVG_SOURCE=http://kanjivg.tagaini.net/upload/Main/kanjivg-latest-beta.xml.gz

[ -d 3rdparty ] || mkdir 3rdparty

if [ ! -d 3rdparty/sqlite-${SQLITE_VERSION} ]; then
	pushd 3rdparty
	wget $SQLITE_SOURCE
	tar -x sqlite-$SQLITE_VERSION/sqlite3.c sqlite-$SQLITE_VERSION/sqlite3.h -f sqlite-amalgamation-$SQLITE_VERSION.tar.gz
	ln -sf sqlite-$SQLITE_VERSION sqlite
	popd
fi

if [ ! -f 3rdparty/JMdict ]; then
	wget $JMDICT_SOURCE -O - |gunzip >3rdparty/JMdict
fi

if [ ! -f 3rdparty/kanjidic2.xml ]; then
	wget $KANJIDIC2_SOURCE -O - |gunzip >3rdparty/kanjidic2.xml
fi

if [ ! -f 3rdparty/kanjivg.xml ]; then
	wget $KANJIVG_SOURCE -O - |gunzip >3rdparty/kanjivg.xml
fi

