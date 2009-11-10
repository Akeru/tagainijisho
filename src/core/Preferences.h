/*
 *  Copyright (C) 2009  Alexandre Courbot
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

#ifndef __CORE_PREFERENCES_H
#define __CORE_PREFERENCES_H

#include <QtDebug>

#include <QObject>
#include <QSettings>
#include <QMutex>

#define __ORGANIZATION_NAME "tagaini.net"
#define __APPLICATION_NAME "Tagaini Jisho"

/**
 * Necessary to put some determinism in the static
 * initialization order.
 */
extern QMutex &_settingsMutex();

/**
 * Necessary to put some determinism in the static
 * initialization order.
 */
extern QSettings &_prefsSettings();

class PreferenceRoot : public QObject
{
	Q_OBJECT
public:
	PreferenceRoot(QObject *parent = 0) : QObject(parent) {}
	virtual ~PreferenceRoot() {}

public slots:
	virtual void setValue(QVariant newValue) = 0;

signals:
	void valueChanged(QVariant newValue);
};

/**
 * A template class that allows to set and access preference values quickly. The current value
 * of the preference is always cached. This class also provides a way to reset a value to its default.
 */
template<class T>
class PreferenceItem : public PreferenceRoot {
private:
	T _value;
	T _defaultValue;
	const QString _group;
	const QString _name;
	bool _isDefault;

public:
	PreferenceItem(const QString &group, const QString &name, const T &defaultValue) : _defaultValue(defaultValue), _group(group), _name(name) {
		_settingsMutex().lock();
		_prefsSettings().beginGroup(_group);
		_value = qVariantValue<T>(_prefsSettings().value(_name, _defaultValue));
		_isDefault = !_prefsSettings().contains(name);
		_prefsSettings().endGroup();
		_settingsMutex().unlock();
	}
	const T &value() const { return _value; }
	const T &defaultValue() const { return _defaultValue; }
	/**
	 * Reset the new value of the preference.
	 */
	void set(T newVal) {
		bool toEmit(value() != newVal);
		_settingsMutex().lock();
		_prefsSettings().beginGroup(_group);
		_prefsSettings().setValue(_name, newVal);
		_prefsSettings().endGroup();
		_value = newVal;
		_isDefault = false;
		_settingsMutex().unlock();
		if (toEmit) emit valueChanged(value());
	}
	/**
	 * Reset the preference to its default value.
	 */
	void reset() {
		_settingsMutex().lock();
		_prefsSettings().beginGroup(_group);
		_prefsSettings().remove(_name);
		_prefsSettings().endGroup();
		_value = _defaultValue;
		_isDefault = true;
		_settingsMutex().unlock();
	}

	void setValue(QVariant newValue) {
		set(qVariantValue<T>(newValue));
	}

	bool isDefault() {
		return _isDefault;
	}
};

#endif
