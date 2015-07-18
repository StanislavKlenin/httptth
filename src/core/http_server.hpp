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
    
    // not so "raw" headers anymore;
    // duplicate values are concatenated into comma-separated list
    using raw_header_map = std::map<std::string, std::string>;
    using raw_header = raw_header_map::value_type;
    
    // maybe better move them up
    struct request
    {
        std::string    method;
        std::string    url;
        std::string    version;
        std::string    body;
        raw_header_map headers;
        
        // some dedicated fields, like Content-Length ?
        unsigned       content_length;
        
        request() : content_length(0) {}
        void clear();
        //bool complete();
    };
    
    class response : public writable
    {
    public:
        response(writable &dst);
        ~response();
        
        // header management
        
        // add a new header and save it (will be written later)
        void add_header(const std::string &name, const std::string &value);
        
        // write this header immediately (triggering status line write)
        void write_header(const std::string &name, const std::string &value);
        
        // status: message is optional, default will be used if empty
        //void set_status(unsigned code, const std::string &phrase = "");
        //void set_version(const std::string &version);
        
        //void commence();
        
        // writable implementation
        void write(const char *data, size_t length) override;
        void flush() override;
        void end() override;
        
        static void write_status(writable &destination, unsigned status);
        static void write_header(writable &destination,
                                 const std::string &name,
                                 const std::string &value)
        {
            destination << name << ": " << value << crlf;
        }
        
    protected:
        void ensure_status();
        void write_headers();
    private:
        // need a state: what is already written
        
        writable                &destination;
        unsigned                 status;
        bool                     status_written, body_started, write_ended;
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
    static const std::map<unsigned, std::string> status_phrases;
};


} // namespace httptth
# endif // __HTTP_SERVER_H__
