/*
 *  Copyright (C) 2008  Alexandre Courbot
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

#ifndef __CORE_JMDICT_ENTRY_H_
#define __CORE_JMDICT_ENTRY_H_

#include <QList>
#include <QStringList>
#include <QMap>

#include "core/Entry.h"
#include "core/jmdict/JMdictDefs.h"

#define JMDICTENTRY_GLOBALID 1

class QFont;
class KanaReading;

class KanjiReading
{
private:
	QString reading;
	quint32 info;
	QList<qint32> validReadings;
	quint8 _frequency;

public:
	KanjiReading(const QString &reading, quint32 info, quint8 frequency);
	const QString &getReading() const { return reading; }
	const QList<qint32> &getKanaReadings() const { return validReadings; }
	quint8 frequency() const { return _frequency; }

	friend class JMdictEntry;
};

class KanaReading
{
private:
	QString reading;
	QList<qint32> kanjiReadings;
	qint32 info;
	quint8 _frequency;

public:
	KanaReading(const QString &reading, qint32 info, quint8 frequency);
	const QString &getReading() const { return reading; }
	const QList<qint32> &getKanjiReadings() const { return kanjiReadings; }
	quint8 frequency() const { return _frequency; }

	void addKanjiReading(qint32 readingIndex);

	friend class JMdictEntry;
};

class Gloss
{
private:
	QString _lang;
	QString _gloss;

public:
	// Required for QMap
	Gloss() {}
	Gloss(const QString &lang, const QString &gloss);
	const QString &lang() const { return _lang; }
	const QString &gloss() const { return _gloss; }
};

class Sense
{
private:
	QMap<QString, Gloss> glosses;
	QStringList infos;
	QList<qint32> _stagK;
	QList<qint32> _stagR;
	JMdictPosTagType _partOfSpeech;
	JMdictMiscTagType _misc;
	JMdictDialTagType _dialect;
	JMdictFieldTagType _field;

	static QList<int> listOfSetBits(quint64 value);

public:
	Sense(JMdictPosTagType partOfSpeech, JMdictMiscTagType misc, JMdictDialTagType dialect, JMdictFieldTagType field);
	const QMap<QString, Gloss> &getGlosses() const { return glosses; }
	const Gloss gloss(const QString &lang) const;
	const QStringList &getInfos() const { return infos; }

	JMdictPosTagType partOfSpeech() const { return _partOfSpeech; }
	QList<int> partsOfSpeech() const;
	JMdictMiscTagType misc() const { return _misc; }
	QList<int> miscs() const;
	JMdictDialTagType dialect() const { return _dialect; }
	QList<int> dialects() const;
	JMdictFieldTagType field() const { return _field; }
	QList<int> fields() const;
	const QList<qint32> &stagK() const { return _stagK; }
	void addStagK(qint32 index) { _stagK << index; }
	const QList<qint32> &stagR() const { return _stagR; }
	void addStagR(qint32 index) { _stagR << index; }

	void addGloss(const Gloss &gloss);

	QString senseText() const;
};

class JMdictEntry : public Entry
{
	Q_OBJECT
private:
	QList<KanjiReading> kanjis;
	QList<KanaReading> kanas;
	QList<Sense> senses;
	qint8 _jlpt;

	static QFont printFont;

	void addKanaReading(const KanaReading &reading);

public:
	// Needed to register type
	JMdictEntry() : Entry() {}
	JMdictEntry(int id);
	virtual ~JMdictEntry();

	const QList<KanjiReading> &getKanjiReadings() const { return kanjis; }
	bool hasKanjiReadings() const { return (!kanjis.isEmpty()); }
	const QList<KanaReading> &getKanaReadings() const { return kanas; }
	const QList<Sense> &getSenses() const { return senses; }
	/**
	 * This method performs like getSenses, but only returns senses that are
	 * not filtered implicitly.
	 *
	 * The counterFilter argument may contain a mask of tags that are normally filtered,
	 * but that we want to get anyway.
	 */
	QList<const Sense *> getRelevantSenses(JMdictMiscTagType counterFilter = 0) const;

	qint8 jlpt() const { return _jlpt; }

	virtual QStringList writings() const;
	virtual QStringList readings() const;
	virtual QStringList meanings() const;

	friend class JMdictEntrySearcher;
};

#endif
