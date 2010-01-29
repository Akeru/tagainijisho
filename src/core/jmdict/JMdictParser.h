/*
 *  Copyright (C) 2010  Alexandre Courbot
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CORE_JMDICT_PARSER
#define __CORE_JMDICT_PARSER

#include "core/XmlParserHelper.h"

#include <QStringList>
#include <QMap>
#include <QSet>
#include <QHash>

class JMdictKanjiWritingItem {
public:
	QString writing;
	// TODO Frequency not yet calculated
	int frequency;
	
	JMdictKanjiWritingItem() : writing(), frequency(0) {}
};

class JMdictKanaReadingItem {
public:
	QString reading;
	bool noKanji;
	// TODO Frequency not yet calculated
	int frequency;
	QList<quint8> restrictedTo;
	
	JMdictKanaReadingItem() : reading(), noKanji(false), frequency(0) {}
};

class JMdictParser;
class JMdictSenseItem {
public:
	QSet<QString> pos;
	QSet<QString> field;
	QSet<QString> misc;
	QSet<QString> dialect;
	QList<quint8> restrictedToKanji;
	QList<quint8> restrictedToKana;
	/// Maps a language to its glosses
	QMap<QString, QStringList> gloss;
	
	JMdictSenseItem() {}
	quint64 posBitField(const JMdictParser &parser) const;
	quint64 fieldBitField(const JMdictParser &parser) const;
	quint64 miscBitField(const JMdictParser &parser) const;
	quint64 dialectBitField(const JMdictParser &parser) const;
};

class JMdictItem {
public:
	int id;
	int frequency;
	QList<JMdictKanjiWritingItem> kanji;
	QList<JMdictKanaReadingItem> kana;
	QList<JMdictSenseItem> senses;
	
	JMdictItem() : id(0), frequency(0) {}
};

class JMdictParser {
protected:
	QStringList languages;

public:
	QHash<QString, quint8> posBitFields;
	int posBitFieldsCount;
	QHash<QString, quint8> fieldBitFields;
	int fieldBitFieldsCount;
	QHash<QString, quint8> miscBitFields;
	int miscBitFieldsCount;
	QHash<QString, quint8> dialectBitFields;
	int dialectBitFieldsCount;
	
	QHash<QString, QString> entities;
	QHash<QString, QString> reversedEntities;
	
	JMdictParser(const QStringList &langs);
	virtual ~JMdictParser() {}
	bool parse(QXmlStreamReader &reader);
	
	// This method can be overloaded by subclasses in order to implement
	// a behavior when an item is finished being parsed.
	// Returns true if the processing completed successfully, false otherwise
	virtual bool onItemParsed(JMdictItem &entry) { return true; }
};

#endif
