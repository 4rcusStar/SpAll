// StartupTrace.cpp — implementation of the opt-in startup milestone log.
#include "StartupTrace.h"

#include <QCoreApplication>
#include <QFile>
#include <QString>
#include <QTextStream>

namespace spall {

void startupTrace(const char* milestone) {
    static const bool enabled = !qEnvironmentVariableIsEmpty("SPALL_STARTUP_TRACE");
    if (!enabled) {
        return;
    }
    QFile file(QCoreApplication::applicationDirPath() +
               QStringLiteral("/spall-startup-trace.log"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << milestone << QLatin1Char('\n');
    stream.flush();
}

}  // namespace spall
