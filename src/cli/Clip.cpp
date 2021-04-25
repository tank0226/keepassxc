/*
 *  Copyright (C) 2019 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Clip.h"

#include "Utils.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/Tools.h"

const QCommandLineOption Clip::AttributeOption = QCommandLineOption(
    QStringList() << "a"
                  << "attribute",
    QObject::tr("Copy the given attribute to the clipboard. Defaults to \"password\" if not specified.",
                "Don't translate \"password\", it refers to the attribute."),
    "attr",
    "password");

const QCommandLineOption Clip::TotpOption =
    QCommandLineOption(QStringList() << "t"
                                     << "totp",
                       QObject::tr("Copy the current TOTP to the clipboard (equivalent to \"-a totp\")."));

const QCommandLineOption Clip::BestMatchOption =
    QCommandLineOption(QStringList() << "b"
                                     << "best-match",
                       QObject::tr("Must match only one entry, otherwise a list of possible matches is shown."));

Clip::Clip()
{
    name = QString("clip");
    description = QObject::tr("Copy an entry's attribute to the clipboard.");
    options.append(Clip::AttributeOption);
    options.append(Clip::TotpOption);
    options.append(Clip::BestMatchOption);
    positionalArguments.append(
        {QString("entry"), QObject::tr("Path of the entry to clip.", "clip = copy to clipboard"), QString("")});
    optionalArguments.append(
        {QString("timeout"), QObject::tr("Timeout in seconds before clearing the clipboard."), QString("[timeout]")});
}

int Clip::executeWithDatabase(QSharedPointer<Database> database, QSharedPointer<QCommandLineParser> parser)
{
    auto& out = parser->isSet(Command::QuietOption) ? Utils::DEVNULL : Utils::STDOUT;
    auto& err = Utils::STDERR;

    const QStringList args = parser->positionalArguments();
    QString bestEntryPath;

    QString timeout;
    if (args.size() == 3) {
        timeout = args.at(2);
    }

    if (parser->isSet(Clip::BestMatchOption)) {
        QStringList results = database->rootGroup()->locate(args.at(1));
        if (results.count() > 1) {
            err << QObject::tr("Multiple entries matching:") << endl;
            for (const QString& result : asConst(results)) {
                err << result << endl;
            }
            return EXIT_FAILURE;
        } else {
            bestEntryPath = (results.isEmpty()) ? args.at(1) : results[0];
            out << QObject::tr("Used matching entry: %1").arg(bestEntryPath) << endl;
        }
    } else {
        bestEntryPath = args.at(1);
    }

    const QString& entryPath = bestEntryPath;

    int timeoutSeconds = 0;
    if (!timeout.isEmpty() && timeout.toInt() <= 0) {
        err << QObject::tr("Invalid timeout value %1.").arg(timeout) << endl;
        return EXIT_FAILURE;
    } else if (!timeout.isEmpty()) {
        timeoutSeconds = timeout.toInt();
    }

    Entry* entry = database->rootGroup()->findEntryByPath(entryPath);
    if (!entry) {
        err << QObject::tr("Entry %1 not found.").arg(entryPath) << endl;
        return EXIT_FAILURE;
    }

    if (parser->isSet(AttributeOption) && parser->isSet(TotpOption)) {
        err << QObject::tr("ERROR: Please specify one of --attribute or --totp, not both.") << endl;
        return EXIT_FAILURE;
    }

    QString selectedAttribute = parser->value(AttributeOption);
    QString value;
    bool found = false;
    if (parser->isSet(TotpOption) || selectedAttribute == "totp") {
        if (!entry->hasTotp()) {
            err << QObject::tr("Entry with path %1 has no TOTP set up.").arg(entryPath) << endl;
            return EXIT_FAILURE;
        }

        found = true;
        value = entry->totp();
    } else {
        QStringList attrs = Utils::findAttributes(*entry->attributes(), selectedAttribute);
        if (attrs.size() > 1) {
            err << QObject::tr("ERROR: attribute %1 is ambiguous, it matches %2.")
                       .arg(selectedAttribute, QLocale().createSeparatedList(attrs))
                << endl;
            return EXIT_FAILURE;
        } else if (attrs.size() == 1) {
            found = true;
            selectedAttribute = attrs[0];
            value = entry->attributes()->value(selectedAttribute);
        }
    }

    if (!found) {
        out << QObject::tr("Attribute \"%1\" not found.").arg(selectedAttribute) << endl;
        return EXIT_FAILURE;
    }

    int exitCode = Utils::clipText(value);
    if (exitCode != EXIT_SUCCESS) {
        return exitCode;
    }

    out << QObject::tr("Entry's \"%1\" attribute copied to the clipboard!").arg(selectedAttribute) << endl;

    if (!timeoutSeconds) {
        return exitCode;
    }

    QString lastLine = "";
    while (timeoutSeconds > 0) {
        out << '\r' << QString(lastLine.size(), ' ') << '\r';
        lastLine = QObject::tr("Clearing the clipboard in %1 second(s)…", "", timeoutSeconds).arg(timeoutSeconds);
        out << lastLine << flush;
        Tools::sleep(1000);
        --timeoutSeconds;
    }
    Utils::clipText("");
    out << '\r' << QString(lastLine.size(), ' ') << '\r';
    out << QObject::tr("Clipboard cleared!") << endl;

    return EXIT_SUCCESS;
}
