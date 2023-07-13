#include "httpresponse.h"

HttpResponse::HttpResponse(QTcpSocket* socket) {
    this->socket = socket;
    statusCode = 200;
    statusText = "OK";
    sentHeaders = false;
    sentLastPart = false;
} //endconstructor

void HttpResponse::setHeader(QByteArray name, QByteArray value) {
    headers.insert(name,value);
} //endfunction setHeader

void HttpResponse::setHeader(QByteArray name, int value) {
    headers.insert(name,QByteArray::number(value));
} //endfunction setHeader

QMap<QByteArray,QByteArray>& HttpResponse::getHeaders() {
    return headers;
} //endfunction getHeaders

void HttpResponse::setStatus(int statusCode, QByteArray description) {
    this->statusCode=statusCode;
    statusText=description;
} //endfunction setStatus

void HttpResponse::writeHeaders(bool skipRespCode) {
    QByteArray buffer;
    if (!skipRespCode) {
        buffer.append("HTTP/1.1 ");
        buffer.append(QByteArray::number(statusCode));
        buffer.append(' ');
        buffer.append(statusText);
        buffer.append("\r\n");
    } //endif
    foreach(QByteArray name, headers.keys()) {
        buffer.append(name);
        buffer.append(": ");
        buffer.append(headers.value(name));
        buffer.append("\r\n");
    } //endforeach
    foreach(HttpCookie cookie,cookies.values()) {
        buffer.append("Set-Cookie: ");
        buffer.append(cookie.toByteArray());
        buffer.append("\r\n");
    } //endforeach
    buffer.append("\r\n");
    writeToSocket(buffer);
    sentHeaders = true;
} //endfunction writeHeaders

bool HttpResponse::writeToSocket(QByteArray data) {
    int remaining = data.size();
    char* ptr = data.data();
    while (socket->state() == QAbstractSocket::ConnectedState && remaining>0) {
        socket->waitForBytesWritten(-1);
        int written = socket->write(ptr,remaining);
        if (written == -1) {
          return false;
        } //endif
        ptr += written;
        remaining -= written;
    } //endwhile
    return true;
} //endfunction writeToSocket

void HttpResponse::write(QByteArray data, bool lastPart, bool skipRespCode) {
    if (sentHeaders == false) {
        QByteArray connectionMode = headers.value("Connection");
        if (!headers.contains("Content-Length") && !headers.contains("Transfer-Encoding") && connectionMode!="close" && connectionMode!="Close") {
            if (!lastPart) {
                headers.insert("Transfer-Encoding","chunked");
            } else {
                headers.insert("Content-Length",QByteArray::number(data.size()));
            } //endif
        } //endif
        writeHeaders(skipRespCode);
    } //endif
    bool chunked = headers.value("Transfer-Encoding")=="chunked" || headers.value("Transfer-Encoding")=="Chunked";
    if (chunked) {
        if (data.size() > 0) {
            QByteArray buffer = QByteArray::number(data.size(),16);
            buffer.append("\r\n");
            writeToSocket(buffer);
            writeToSocket(data);
            writeToSocket("\r\n");
        } //endif
    } else {
        writeToSocket(data);
    } //endif
    if (lastPart) {
        if (chunked) {
            writeToSocket("0\r\n\r\n");
        } else if (!headers.contains("Content-Length")) {
            socket->disconnectFromHost();
        } //endif
        sentLastPart = true;
    } //endif
} //endfunction write

void HttpResponse::resetSentHeaders() {
    sentHeaders = false;
    headers.clear();
} //endfunction resetSentHeaders

bool HttpResponse::hasSentLastPart() const {
    return sentLastPart;
} //endfunction hasSentLastPart

void HttpResponse::setCookie(const HttpCookie& cookie) {
    if (!cookie.getName().isEmpty()) {
        cookies.insert(cookie.getName(),cookie);
    } //endif
} //endfunction setCookie

QMap<QByteArray,HttpCookie>& HttpResponse::getCookies() {
    return cookies;
} //endfunction getCookies

void HttpResponse::redirect(const QByteArray& url) {
    setStatus(303, "See Other");
    setHeader("Location", url);
    write("Redirect", true);
} //endfunction redirect
