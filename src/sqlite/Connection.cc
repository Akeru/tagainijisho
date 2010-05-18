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

#include "sqlite/Connection.h"

using namespace sqlite;

Connection::Connection(QObject *parent) : QObject(parent)
{
}

Connection::~Connection()
{
}

bool Connection::connect(const QString &dbFile)
{
	return true;
}

bool Connection::attach(const QString &dbFile, const QString &alias)
{
	return true;
}

bool Connection::detach(const QString &alias)
{
	return true;
}

Error Connection::lastError()
{
	return Error();
}
