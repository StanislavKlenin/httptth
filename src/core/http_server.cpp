#include "http_server.hpp"
#include "exceptions.hpp"

#include <string.h>

using namespace std;

namespace httptth
{

//
// http_server
//
http_server::http_server(http_server::handler_type h) :
    handler(h),
    underlying_server(internal_handler(h))
{
    dprintf(logger::DEBUG, "http_server constructed");
}
/*
http_server::http_server(http_server::handler_type h) :
    handler(h),
    underlying_server(
        std::unique_ptr<text_server::extended_handler>(
            new internal_handler(handler)))
    
     
    //underlying_server(internal_handler(handler))
    // ^ custom trigger condition must be passed
    //   (and this trigger condition will have to refer to the handler)
{
    dprintf(logger::DEBUG, "http_server constructed");
}
*/

http_server::~http_server()
{
}

void http_server::listen(const char *address, int port)
{
    underlying_server.listen(address, port);
}

//
// request
//
void http_server::request::clear()
{
    method.clear();
    url.clear();
    version.clear();
    headers.clear();
    body.clear();
}

//
// response
//
http_server::response::response(writable &dst) :
    destination(dst),
    status(200),
    status_written(false),
    body_started(false),
    write_ended(false)
{
}

http_server::response::~response()
{
}

void http_server::response::add_header(const string &name, const string &value)
{
    headers.emplace_back(name, value);
}

void http_server::response::write_header(const string &name,
                                         const string &value)
{
    if (body_started) {
        throw write_error();
    }
    
    ensure_status();
    
    write_header(destination, name, value);
}

void http_server::response::write(const char *data, size_t length)
{
    if (!body_started) {
        write_headers();
        body_started = true;
    }
    
    destination.write(data, length);
}

void http_server::response::flush()
{
    if (!body_started) {
        write_headers();
        body_started = true;
    }
    destination.flush();
}

void http_server::response::end()
{
    if (!body_started) {
        write_headers();
        body_started = true;
    }
    destination.flush();
    destination.end();
    write_ended = true;
}

void http_server::response::write_status(writable &destination,
                                         unsigned status)
{
    string phrase;
    try {
        phrase = status_phrases.at(status);
    } catch (out_of_range) {
        phrase = undefined_status;
    }
    
    destination << "HTTP/1.1 " << status << ' ' << phrase << crlf;
}

void http_server::response::ensure_status()
{
    if (!status_written) {
        write_status(destination, status);
        status_written = true;
    }
}

void http_server::response::write_headers()
{
    if (body_started) {
        throw write_error();
    }
    
    ensure_status();
    
    for (auto I : headers) {
        write_header(destination, I.first, I.second);
    }
    destination << crlf;
}

//
// internal_handler
//
http_server::internal_handler::internal_handler(const handler_type &h) :
    handler(h),
    headers_completed(false),
    body_completed(false)
{
    id = ++last;
    dprintf(logger::DEBUG,
            "##\tinternal_handler %zu constructed; %p",
            id,
            this);
}

http_server::internal_handler::internal_handler(
    const http_server::internal_handler &that) :
        handler(that.handler),
        headers_completed(that.headers_completed),
        body_completed(that.body_completed),
        // ...
        id(++last)
{
    dprintf(logger::DEBUG,
            "##\tinternal_handler %zu copied from %zu; %p",
            this->id,
            that.id,
            this);
}

http_server::internal_handler::internal_handler(
    http_server::internal_handler &&that) :
        handler(std::move(that.handler)),
        headers_completed(that.headers_completed),
        body_completed(that.body_completed),
        // ...
        id(that.id)
{
    that.id = 0;
    dprintf(logger::DEBUG,
            "##\tinternal_handler %zu *moved*; %p",
            this->id,
            this);
}

//bool http_server::internal_handler::trigger(const char *data, size_t length)
bool http_server::internal_handler::operator()(const char *data, size_t length)
{
    //dprintf(logger::DEBUG, "trigger: %.*s", (int)length, data);
    //dprintf(logger::DEBUG, "internal_handler trigger: this=%p", this);
    if (length > 1 &&
        data[length - 2] == '\r' &&
        data[length - 1] == '\n')
    {
        return true;
    }
    
    if (headers_completed &&
        request.content_length &&
        request.content_length == request.body.length() + length)
    {
        dprintf(logger::DEBUG, "@! triggering handler for content_length");
        return true;
    }
    
    return false;
}

//void http_server::internal_handler::process(const char *data,
//                                            size_t length,
//                                            writable &destination)
void http_server::internal_handler::operator()(const char *data,
                                               size_t length,
                                               writable &destination)
{
    dprintf(logger::DEBUG, "internal_handler: this=%p", this);
    if (!headers_completed) {
        // still parsing headers
        if (length > 1 &&
            data[length - 2] == '\r' &&
            data[length - 1] == '\n')
        {
            length -= 2;
        } else {
            // trigger error
            dprintf(logger::INFO, "error: no crlf");
            cut_short(destination, 400);
            return;
        }
        
        if (request.method.empty()) {
            // not started yet
            parse_start_line(data, length);
            if (request.method.empty()) {
                // still not set, this is an error
                dprintf(logger::NOTICE, "could not parse method");
                cut_short(destination, 400);
                return;
            }
        } else {
            // header or blank expected
            if (!length) {
                dprintf(logger::DEBUG, "blank! l=%u", request.content_length);
                headers_completed = true;
                if (!request.content_length) {
                    // expect no content and respond right away
                    //dprintf(logger::DEBUG, "responding with 200");
                    //cut_short(destination, 200);
                    serve(destination);
                    // actually, call a handler // later
                }
                return;
            }
            
            raw_header h = parse_header(data, length);
            if (h.first.empty()) {
                //dprintf(logger::DEBUG,
                //        "error: could not parse header %.*s",
                //        length,
                //        data);
                cut_short(destination, 400);
                return;
            }
            //request.headers.push_back(h);
            auto it = request.headers.find(h.first);
            if (it != request.headers.end()) {
                it->second.append(1, ',');
                it->second.append(h.second);
            } else {
                request.headers.insert(h);
            }
            
            // fill dedicated fields for some headers
            if (h.first == "Content-Length") {
                dprintf(logger::DEBUG, "[%s]", h.second.c_str());
                request.content_length = stoul(h.second);
                dprintf(logger::DEBUG,
                        "contentlength %u",
                        request.content_length);
            }
        }
    } else {
        // TODO: cut off \r and \n?
        request.body.append(data, length);
        // and if line is empty, trigger the handler?
        
        // detecting body end in general case?
        // chunked request: not now
        // content length header: SHOULD be present, but not guaranteed
        // empty line? this is not correct
        // line-based text server will not work as a base of http server, alas
        // but now we can install a custom trigger
        
        
        if (request.content_length &&
            request.content_length == request.body.length())
        {
            dprintf(logger::DEBUG, "body ended?");
            //cut_short(destination, 200);
            serve(destination);
        }
        
        /*
        if (request.content_length) {
            // ...
        } else {
            
            if (length > 1 &&
                data[length - 2] == '\r' &&
                data[length - 1] == '\n')
            {
                fprintf(stderr, "body ended?\n");
                cut_short(destination, 200);
            }
            
        }
        */
        
    }
    
    //if (true) {
    //    response.destination = &destination;
    //    handler(request, response);
    //    if (connection: close header set) {
    //        response.end() // or destination.end
    //    }
    //}
}

void http_server::internal_handler::parse_start_line(const char *data,
                                                     size_t length)
{
    //A request-line begins with a method token, followed by a single space
    //(SP), the request-target, another single space (SP), the protocol
    //version, and ends with CRLF.
    //  request-line   = method SP request-target SP HTTP-version CRLF
    // so we need to split it, with minimal copying
    const char *p = data, *q;
    
    // method
    for (q = p; q < data + length && !isspace(*q); q++) ;
    if (q == p || q == data + length) {
        return; // method remains unset
    } else {
        request.method.assign(p, q - p);
    }
    
    // url
    for (p = q + 1; p < data + length && isspace(*p); p++) ;
    for (q = p; q < data + length && !isspace(*q); q++) ;
    if (q == p || q == data + length) {
        request.method.clear();
        return;
    } else {
        request.url.assign(p, q - p);
    }
    
    // version
    for (p = q + 1; p < data + length && isspace(*p); p++) ;
    for (q = p; q < data + length && !isspace(*q); q++) ;
    request.version.assign(p, q - p);
}

http_server::raw_header http_server::internal_handler::parse_header(
    const char *data,
    size_t length)
{
    const char *p, *q, *r;
    
    //p = static_cast<const char *>(memchr(data, length, ':'));
    //if (!p) {
    //    return raw_header();
    //}
    for (p = data; p < data + length && *p != ':'; p++) ;
    if (p == data + length) {
        return raw_header();
    }
    
    for (q = p + 1; q < data + length && isspace(*q); q++) ;
    for (r = data + length - 1; isspace(*r) && r >= q; r--) ;
    
    return raw_header(string(data, p - data), string(q, r - q + 1));
}

void http_server::internal_handler::clear()
{
    headers_completed = false;
    request.clear();
    //response.clear();
}

/*
void http_server::internal_handler::respond(writable &destination,
                                            const std::string &message,
                                            const std::string &)
{
    destination << message;
    //destination << "Content-Length: 0\r\n";
    destination << crlf;
    // ignore body for now
    
    destination.flush();
    // if (keep alive ) destination.close()
}
*/
void http_server::internal_handler::cut_short(writable &destination,
                                              unsigned status)
{
    response::write_status(destination, status);
    destination << crlf;
    destination.flush();
    // if (not keep alive) destination.end();
    destination.end(); // no keep alive for now
}

void http_server::internal_handler::serve(writable &destination)
{
    dprintf(logger::DEBUG, "serve");
    
    // construct the response
    class response response(destination);
    
    // and pass it to the handler
    handler(request, response);
    
    // some postprocessing: end if client did not, maybe set keepalive
    if (!response.write_ended) {
        response.flush();
        response.end();
    }
}

const string http_server::undefined_status = "Undefined";
const string http_server::crlf = "\r\n";

const map<unsigned, string> http_server::status_phrases =
{
    {   0, undefined_status },
    { 200, "OK" },
    { 400, "Bad Request" },
    { 404, "Not Found" },
    { 500, "Internal Server Error" }
    // ...
};




size_t http_server::internal_handler::last = 0;


} // namespace httptth
