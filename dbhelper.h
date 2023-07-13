#ifndef DBHELPER_H
#define DBHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>
#include <QSettings>

#include "logger.h"

class DbHelper : public QObject {
    Q_OBJECT
public:
    explicit DbHelper(QSettings *settings, QObject *parent = 0);

private:
    QSettings *settings;
    Logger logger;
    QString dbhost;
    int dbport;
    QString dbname;
    QString dbuser;
    QString dbpass;
    QSqlDatabase db;
private slots:
    bool supportsMySQL();
public slots:
    bool connectToDb();
    void closeDbConnection();
    QList<QList<QString> > getTable(QString name);
    void update(QString table, int id, QString attribute, QString value);
    QMap<QString, QString> getUnit(int id);
};

#endif // DBHELPER_H
