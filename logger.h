#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QDateTime>
#include <QDebug>

class Logger : public QObject {
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);
    static const int OK = 0;
    static const int WARNING = 1;
    static const int ERROR = 2;
    void log (QString message, int type, QString ctime="");
};

#endif // LOGGER_H
