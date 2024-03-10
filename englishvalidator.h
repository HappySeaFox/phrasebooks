/*
 * This file is part of phrasebooks.
 *
 * phrasebooks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * phrasebooks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with phrasebooks.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENGLISHVALIDATOR_H
#define ENGLISHVALIDATOR_H

#include <QValidator>

/*
 *  Validator to convert a ticker name to the upper case
 */
class EnglishValidator : public QValidator
{
public:
    explicit EnglishValidator(QObject *parent = 0);
    virtual ~EnglishValidator();

    virtual QValidator::State validate(QString &input, int &pos) const;
};

#endif // ENGLISHVALIDATOR_H
