#include "httpcookie.h"

HttpCookie::HttpCookie() {
    version = 1;
    maxAge = 0;
    secure = false;
} //endconstructor

HttpCookie::HttpCookie(const QByteArray name, const QByteArray value, const int maxAge, const QByteArray path, const QByteArray comment, const QByteArray domain, const bool secure) {
    this->name = name;
    this->value = value;
    this->maxAge = maxAge;
    this->path = path;
    this->comment = comment;
    this->domain = domain;
    this->secure = secure;
    this->version = 1;
} //endconstructor

HttpCookie::HttpCookie(const QByteArray source) {
    version = 1;
    maxAge = 0;
    secure = false;
    QList<QByteArray> list=splitCSV(source);
    foreach(QByteArray part, list) {
        // Split the part into name and value
        QByteArray name;
        QByteArray value;
        int posi=part.indexOf('=');
        if (posi) {
            name=part.left(posi).trimmed();
            value=part.mid(posi+1).trimmed();
        } else {
            name=part.trimmed();
            value = "";
        } //endif

        // Set fields
        if (name == "Comment") {
            comment = value;
        } else if (name == "Domain") {
            domain = value;
        } else if (name == "Max-Age") {
            maxAge = value.toInt();
        } else if (name == "Path") {
            path = value;
        } else if (name == "Secure") {
            secure = true;
        } else if (name == "Version") {
            version = value.toInt();
        } else {
            if (this->name.isEmpty()) {
                this->name = name;
                this->value = value;
            } //endif
        } //endif
    } //endforeach
} //endconstructor

QByteArray HttpCookie::toByteArray() const  {
    QByteArray buffer(name);
    buffer.append('=');
    buffer.append(value);
    if (!comment.isEmpty()) {
        buffer.append("; Comment=");
        buffer.append(comment);
    } //endif
    if (!domain.isEmpty()) {
        buffer.append("; Domain=");
        buffer.append(domain);
    } //endif
    if (maxAge!=0) {
        buffer.append("; Max-Age=");
        buffer.append(QByteArray::number(maxAge));
    } //endif
    if (!path.isEmpty()) {
        buffer.append("; Path=");
        buffer.append(path);
    } //endif
    if (secure) {
        buffer.append("; Secure");
    } //endif
    buffer.append("; Version=");
    buffer.append(QByteArray::number(version));
    return buffer;
} //endfunction toByteArray

void HttpCookie::setName(const QByteArray name){
    this->name = name;
} //endfunction setName

void HttpCookie::setValue(const QByteArray value){
    this->value = value;
} //endfunction setValue

void HttpCookie::setComment(const QByteArray comment){
    this->comment = comment;
} //endfunction setComment

void HttpCookie::setDomain(const QByteArray domain){
    this->domain = domain;
} //endfunction setDomain

void HttpCookie::setMaxAge(const int maxAge){
    this->maxAge = maxAge;
} //endfunction setMaxAge

void HttpCookie::setPath(const QByteArray path){
    this->path = path;
} //endfunction setPath

void HttpCookie::setSecure(const bool secure){
    this->secure = secure;
} //endfunction setSecure

QByteArray HttpCookie::getName() const {
    return name;
} //endfunction getName

QByteArray HttpCookie::getValue() const {
    return value;
} //endfunction getValue

QByteArray HttpCookie::getComment() const {
    return comment;
} //endfunction getComment

QByteArray HttpCookie::getDomain() const {
    return domain;
} //endfunction getDomain

int HttpCookie::getMaxAge() const {
    return maxAge;
} //endfunction getMaxAge

QByteArray HttpCookie::getPath() const {
    return path;
} //endfunction getPath

bool HttpCookie::getSecure() const {
    return secure;
} //endfunction getSecure

int HttpCookie::getVersion() const {
    return version;
} //endfunction getVersion

QList<QByteArray> HttpCookie::splitCSV(const QByteArray source) {
    bool inString = false;
    QList<QByteArray> list;
    QByteArray buffer;
    for (int i=0; i<source.size(); ++i) {
        char c = source.at(i);
        if (inString == false) {
            if (c == '\"') {
                inString=true;
            } else if (c == ';') {
                QByteArray trimmed=buffer.trimmed();
                if (!trimmed.isEmpty()) {
                    list.append(trimmed);
                } //endif
                buffer.clear();
            } else {
                buffer.append(c);
            } //endif
        } else {
            if (c == '\"') {
                inString=false;
            } else {
                buffer.append(c);
            } //endif
        } //endif
    } //endfor
    QByteArray trimmed = buffer.trimmed();
    if (!trimmed.isEmpty()) {
        list.append(trimmed);
    } //endif
    return list;
} //endfunction splitCSV
