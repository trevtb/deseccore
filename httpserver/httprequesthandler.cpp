#include "httprequesthandler.h"

HttpRequestHandler::HttpRequestHandler(QObject* parent) : QObject(parent) {
} //endconstructor

HttpRequestHandler::~HttpRequestHandler() {
} //enddestructor

void HttpRequestHandler::service(HttpRequest&, HttpResponse& response) {
    response.setStatus(501,"not implemented");
    response.write("501 not implemented",true);
} //endfunction service

void HttpRequestHandler::stop() {
    emit doStop();
} //endfunction stop
