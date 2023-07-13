#include "logger.h"

Logger::Logger(QObject *parent) : QObject(parent) {
} //endconstructor

void Logger::log(QString message, int type, QString ctime) {
    QString msg;
    if (ctime == "") {
        QDateTime date = QDateTime::currentDateTime();
        msg = date.toString("dd-MM-yy hh:mm:");
    } else {
        msg = ctime;
    } //endif

    if (type == WARNING) {
        msg += " [WARNING] ";
    } else if (type == ERROR) {
        msg += " [ERROR] ";
    } else {
        msg += " [OK] ";
    } //endif

    msg += message;

    if (type == WARNING) {
        qWarning(msg.toStdString().c_str());
    } else if (type == ERROR) {
        qCritical(msg.toStdString().c_str());
    } else {
        qDebug(msg.toStdString().c_str());
    } //endif
} //endfunction log
