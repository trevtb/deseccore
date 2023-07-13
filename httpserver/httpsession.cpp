#include "httpsession.h"

HttpSession::HttpSession(bool canStore) {
    if (canStore) {
        dataPtr = new HttpSessionData();
        dataPtr->refCount = 1;
        dataPtr->lastAccess = QDateTime::currentMSecsSinceEpoch();
        dataPtr->id = QUuid::createUuid().toString().toLocal8Bit();
    } else {
        dataPtr = 0;
    } //endif
} //endconstructor

HttpSession::HttpSession(const HttpSession& other) {
    dataPtr = other.dataPtr;
    if (dataPtr) {
        dataPtr->lock.lockForWrite();
        dataPtr->refCount++;
        dataPtr->lock.unlock();
    } //endif
} //endconstructor

HttpSession& HttpSession::operator= (const HttpSession& other) {
    HttpSessionData* oldPtr = dataPtr;
    dataPtr = other.dataPtr;
    if (dataPtr) {
        dataPtr->lock.lockForWrite();
        dataPtr->refCount++;
        dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
        dataPtr->lock.unlock();
    } //endif
    if (oldPtr) {
        int refCount;
        oldPtr->lock.lockForRead();
        refCount = oldPtr->refCount--;
        oldPtr->lock.unlock();
        if (refCount == 0) {
            delete oldPtr;
        } //endif
    } //endif
    return *this;
} //endfunction operator=

HttpSession::~HttpSession() {
    if (dataPtr) {
        int refCount;
        dataPtr->lock.lockForRead();
        refCount =-- dataPtr->refCount;
        dataPtr->lock.unlock();
        if (refCount == 0) {
            delete dataPtr;
        } //endif
    } //endif
} //enddestructor

QByteArray HttpSession::getId() const {
    if (dataPtr) {
        return dataPtr->id;
    } else {
        return QByteArray();
    } //endif
} //endfunction getId

bool HttpSession::isNull() const {
    return dataPtr == 0;
} //endfunction isNull

void HttpSession::set(const QByteArray& key, const QVariant& value) {
    if (dataPtr) {
        dataPtr->lock.lockForWrite();
        dataPtr->values.insert(key,value);
        dataPtr->lock.unlock();
    } //endif
} //endfunction set

void HttpSession::remove(const QByteArray& key) {
    if (dataPtr) {
        dataPtr->lock.lockForWrite();
        dataPtr->values.remove(key);
        dataPtr->lock.unlock();
    } //endif
} //endfunction remove

QVariant HttpSession::get(const QByteArray& key) const {
    QVariant value;
    if (dataPtr) {
        dataPtr->lock.lockForRead();
        value=dataPtr->values.value(key);
        dataPtr->lock.unlock();
    } //endif
    return value;
} //endfunction get

bool HttpSession::contains(const QByteArray& key) const {
    bool found=false;
    if (dataPtr) {
        dataPtr->lock.lockForRead();
        found=dataPtr->values.contains(key);
        dataPtr->lock.unlock();
    } //endif
    return found;
} //endfunction contains

QMap<QByteArray,QVariant> HttpSession::getAll() const {
    QMap<QByteArray,QVariant> values;
    if (dataPtr) {
        dataPtr->lock.lockForRead();
        values = dataPtr->values;
        dataPtr->lock.unlock();
    } //endif
    return values;
} //endfunction getAll

qint64 HttpSession::getLastAccess() const {
    qint64 value = 0;
    if (dataPtr) {
        dataPtr->lock.lockForRead();
        value = dataPtr->lastAccess;
        dataPtr->lock.unlock();
    } //endif
    return value;
} //endfunction getLastAccess

void HttpSession::setLastAccess() {
    if (dataPtr) {
        dataPtr->lock.lockForRead();
        dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
        dataPtr->lock.unlock();
    } //endif
} //endfunction setLastAccess
