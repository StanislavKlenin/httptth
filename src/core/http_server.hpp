#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <map>
#include <string>
#include <vector>

#include "text_server.hpp"

namespace httptth
{

// a first iteration, proof of concept thing
// is it itself a handler?
class http_server
{
protected:
    class internal_handler;
    
public:
    enum class method
    {
        NONE = 0, // not set / unknown
        GET,
        HEAD,
        POST,
        PUT,
        DELETE
        // others later
    };
    
    // not so "raw" headers anymore;
    // duplicate values are concatenated into comma-separated list
    using raw_header_map = std::map<std::string, std::string>;
    using raw_header = raw_header_map::value_type;
    
    // maybe better move them up
    struct request
    {
        enum method             method = http_server::method::NONE;
        std::string             url;
        std::string             version;
        std::string             body;
        raw_header_map          headers;
        //std::vector<raw_header> headers;
        
        // some dedicated fields, like Content-Length ?
        unsigned                content_length;
        
        request() : content_length(0) {}
        void clear();
        //bool complete();
    };
    
    class response : public writable
    {
    public:
        response(writable &dst);
        ~response();
        
        // header management (later)
        // set_header(...):   save it, will be written later
        // write_header(...): write immediately (triggering status line write)
        
        // status: message is optional, default will be used if empty
        //void set_status(unsigned code, const std::string &phrase = "");
        //void set_version(const std::string &version);
        
        //void commence();
        
        // writable implementation
        void write(const char *data, size_t length) override;
        void flush() override;
        void end() override;
        
        static void write_status(writable &destination, unsigned status);
        
    protected:
        void write_headers(); // and status
    private:
        // need a state: what is already written
        
        writable                &destination;
        unsigned                 status;
        bool                     write_started, write_ended;
        std::string              version;
        std::string              phrase;
        std::string              body;
        std::vector<raw_header>  headers;
        
        friend class internal_handler;
    };
    
    using handler_type = std::function<void(const request &, response &)>;
    
public:
    explicit http_server(handler_type h);
    http_server(const http_server &) = delete;
    http_server(http_server &&) = delete;
    ~http_server();
    
    void listen(const char *address, int port);
    
    inline void stop() { underlying_server.stop(); }
    
protected:
    // handler for text_server
    class internal_handler// : public text_server::extended_handler
    {
    public:
        explicit internal_handler(const handler_type &h);
        internal_handler(const internal_handler &h);
        internal_handler(internal_handler &&h);
        ~internal_handler() {}
        
        // will collect headers and body and call http handler
        //void operator()(const char *data,
        //                size_t length,
        //                writable &destination);
        
        //void process(const char *data,
        //             size_t      length,
        //             writable   &destination) override;
        
        //bool trigger(const char *data, size_t length) override;
        
        void operator()(const char *data, size_t length,writable &destination);
        bool operator()(const char *data, size_t length);
        
        //std::unique_ptr<text_server::extended_handler> clone() override
        //{
        //    return std::unique_ptr<text_server::extended_handler>(
        //        new internal_handler(handler));
        //}
        
    private:
        void clear();
        void parse_start_line(const char *data, size_t length);
        raw_header parse_header(const char *data, size_t length);
        
        // simple state-aware respond method (with single message and no body?)
        // (fast-track or something)
        //void respond(writable &destination,
        //             const std::string &message,
        //             const std::string &body = "");
        
        // TODO: better name
        // TODO: body
        // prematurely stop processing current request
        // and issue a simplified response
        void cut_short(writable &destination, unsigned status);
        
        void serve(writable &destination);
        
    private:
        // handler
        handler_type    handler;
        
        // and state
        bool            headers_completed, body_completed;
        struct request  request;
        //struct response response;
        // keep alive: not sure, maybe a part of the header
        // logic: on by default in http 1.1, off by default in 1.0
        
        size_t id = 0;
        static size_t last;
        
        
        
        
        
        //friend class internal_trigger;
    };
    
private:
    handler_type handler;
    
    text_server<internal_handler> underlying_server;
    
    // TODO: custom trigger condition
    //text_server  underlying_server;
    
public:
    static const std::string crlf;
    static const std::string undefined_status;
    static const std::map<std::string, method> methods;
    static const std::map<unsigned, std::string> status_phrases;
};


} // namespace httptth
# endif // __HTTP_SERVER_H__
