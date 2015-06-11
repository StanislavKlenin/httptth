#include "http_server.hpp"

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
    fprintf(stderr, "http_server constructed\n");
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
    fprintf(stderr, "http_server constructed\n");
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
    method = http_server::method::NONE;
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
    write_started(false),
    write_ended(false)
{
}

http_server::response::~response()
{
}

void http_server::response::write(const char *data, size_t length)
{
    if (!write_started) {
        write_headers();
        write_started = true;
    }
    
    // TODO: write headers, unless hey have been already written
    destination.write(data, length);
}

void http_server::response::flush()
{
    if (!write_started) {
        write_headers();
        write_started = true;
    }
    destination.flush();
}

void http_server::response::end()
{
    if (!write_started) {
        write_headers();
        write_started = true;
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

void http_server::response::write_headers()
{
    if (write_started) {
        // TODO: throw something
    }
    write_started = true;
    
    write_status(destination, status);
    
    for (auto I : headers) {
        destination << I.first << ": " << I.second << crlf;
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
    fprintf(stderr, "##\tinternal_handler %zu constructed; %p\n", id, this);
}

http_server::internal_handler::internal_handler(
    const http_server::internal_handler &that) :
        handler(that.handler),
        headers_completed(that.headers_completed),
        body_completed(that.body_completed),
        // ...
        id(++last)
{
    fprintf(stderr,
            "##\tinternal_handler %zu copied from %zu; %p\n",
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
    fprintf(stderr, "##\tinternal_handler %zu *moved*; %p\n", this->id, this);
}

//bool http_server::internal_handler::trigger(const char *data, size_t length)
bool http_server::internal_handler::operator()(const char *data, size_t length)
{
    //fprintf(stderr, "trigger: %.*s\n", (int)length, data);
    //fprintf(stderr, "internal_handler trigger: this=%p\n", this);
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
        fprintf(stderr, "@! triggering handler because of content_length\n");
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
    fprintf(stderr, "internal_handler: this=%p\n", this);
    if (!headers_completed) {
        // still parsing headers
        if (length > 1 &&
            data[length - 2] == '\r' &&
            data[length - 1] == '\n')
        {
            length -= 2;
        } else {
            // trigger error
            fprintf(stderr, "error: no crlf\n");
            cut_short(destination, 400);
            return;
        }
        
        if (request.method == method::NONE) {
            // not started yet
            parse_start_line(data, length);
            if (request.method == method::NONE) {
                // still not set, this is an error
                fprintf(stderr, "error: could not parse method\n");
                cut_short(destination, 400);
                return;
            }
        } else {
            // header or blank expected
            if (!length) {
                fprintf(stderr, "blank line! cl=%u\n", request.content_length);
                headers_completed = true;
                if (!request.content_length) {
                    // expect no content and respond right away
                    //fprintf(stderr, "responding with 200\n");
                    //cut_short(destination, 200);
                    serve(destination);
                    // actually, call a handler // later
                }
                return;
            }
            
            raw_header h = parse_header(data, length);
            if (h.first.empty()) {
                //fprintf(stderr,
                //        "error: could not parse header %.*s\n",
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
                fprintf(stderr, "[%s]\n", h.second.c_str());
                request.content_length = stoul(h.second);
                fprintf(stderr, "contentlength %u\n", request.content_length);
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
            fprintf(stderr, "body ended?\n");
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
    // so we need to split it, avoiding copying
    const char *p = data, *q;
    
    try {
        // method
        for (q = p; q < data + length && !isspace(*q); q++) ;
        if (q == p || q == data + length) {
            return; // method remains unset
        }
        request.method = methods.at(string(p, q - p));
        
        // url
        for (p = q + 1; p < data + length && isspace(*p); p++) ;
        for (q = p; q < data + length && !isspace(*q); q++) ;
        if (q == p || q == data + length) {
            return; // method remains unset
        }
        request.url.assign(p, q - p);
        
        // version
        for (p = q + 1; p < data + length && isspace(*p); p++) ;
        for (q = p; q < data + length && !isspace(*q); q++) ;
        request.version.assign(p, q - p);
        
    } catch (out_of_range) {
        // do nothing; or set method to NONE just in case?
    }
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
    fprintf(stderr, "serve\n");
    
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

const map<string, http_server::method> http_server::methods =
{
    { "GET",    http_server::method::GET },
    { "HEAD",   http_server::method::HEAD },
    { "POST",   http_server::method::POST },
    { "PUT",    http_server::method::PUT },
    { "DELETE", http_server::method::DELETE }
};

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
