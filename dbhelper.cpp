#include "dbhelper.h"

DbHelper::DbHelper(QSettings *settings, QObject *parent) : QObject(parent) {
    this->dbhost = settings->value("Database/host", "localhost").toString();
    this->dbport = settings->value("Database/port", 3306).toInt();
    this->dbname = settings->value("Database/dbname", "deseccore").toString();
    this->dbuser = settings->value("Database/user", "").toString();
    this->dbpass = settings->value("Database/pass", "").toString();

    if (!supportsMySQL()) {
        logger.log("Database module error: mySQL is not supported - please check if libqt4-sql-mysql is installed.", Logger::ERROR);
        exit(0);
    } //endif
} //endconstructor

bool DbHelper::supportsMySQL() {
    bool retVal = false;
    QStringList drivers = QSqlDatabase::drivers();
    foreach (QString driver, drivers) {
        if (QString::compare(driver, "QMYSQL") == 0) {
            retVal = true;
        } //endif
    } //endforeach

    return retVal;
} //endfunction supportsMySQL

bool DbHelper::connectToDb() {
    db = QSqlDatabase::database();
    if (db.isValid()) {
        return true;
    } //endif

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(dbhost);
    db.setPort(dbport);
    db.setDatabaseName(dbname);
    db.setUserName(dbuser);
    db.setPassword(dbpass);

    if (!db.open()) {
        return false;
    } //endif

   return true;
} //endfunction connectToDb

void DbHelper::closeDbConnection() {
    this->db.close();
    QSqlDatabase::removeDatabase("QMYSQL");
} //endfunction closeDbConnection

QList<QList<QString> > DbHelper::getTable(QString name) {
    QList<QList<QString> > table;
    QSqlQuery query(QString("SELECT * FROM `" + name + "`;"));

    if (query.isActive()) {
        while (query.next()) {
            QList<QString> row;
            for (int i=0; i<query.record().count(); i++) {
                row.append(query.value(i).toString());
            } //endfor

            table.append(row);
        } //endwhile
    } //endif

    return table;
} //endfunction getTable

void DbHelper::update(QString table, int id, QString attribute, QString value) {
    QSqlQuery q;
    QString qs = "SELECT `" + attribute + "` FROM `" + table + "` WHERE `id`=:id;";
    q.prepare(qs);
    q.bindValue(":id", id);
    q.exec();
    if (!q.next()) {
        logger.log("Database module error: Could not read from database.", Logger::ERROR);
    } //endif

    QSqlQuery query;
    query.prepare(QString("SELECT * FROM `" + table + "` WHERE `id`=:id;"));
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        QString qstri = QString("UPDATE `" + table + "` SET `" + attribute + "`=:value WHERE `id`=:id;");
        QSqlQuery q;
        q.prepare(qstri);
        q.bindValue(":value", value);
        q.bindValue(":id", id);
        q.exec();
    } else {
        logger.log("Database module error: can't update entry width id #"+QString::number(id)+", id does not exist.", Logger::ERROR);
    } //endif
} //endfunction update

QMap<QString, QString> DbHelper::getUnit(int id) {
    QList<QList<QString> > unittab = getTable("deseccunit");

    QList<QString> row;
    foreach (QList<QString> unit, unittab) {
        if (QString::compare(unit.at(0), QString::number(id)) == 0) {
            row = unit;
            break;
        } //endif
    } //endforeach

    QMap<QString, QString> retVal;
    retVal.insert("id", row.at(0));
    retVal.insert("name", row.at(1));
    retVal.insert("host", row.at(2));
    retVal.insert("port", row.at(3));
    retVal.insert("streampath", row.at(4));
    retVal.insert("usessl", row.at(5));
    retVal.insert("sslcert", row.at(6));
    retVal.insert("useauth", row.at(7));
    retVal.insert("user", row.at(8));
    retVal.insert("pass", row.at(9));
    retVal.insert("delay", row.at(10));
    retVal.insert("resolution", row.at(11));
    retVal.insert("resize", row.at(12));
    retVal.insert("autoreconnect", row.at(13));

    return retVal;
} //endfunction getUnit
