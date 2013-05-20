#ifndef __REDIS_CONN_H__
#define __REDIS_CONN_H__

#ifdef _WIN32
#include "stdint.h"
#else
#include <stdint.h>
#endif

#include <stddef.h>
#include <string>
#include "hiredis.h"

void redis_initialize();
void redis_uninitialize();

class redis_conn_t
{
public:
    redis_conn_t(void);
    ~redis_conn_t(void);
    bool open(const char *srv_ip, unsigned short port, int *err_num=NULL);
    bool set_string(const char *key, const char *value, int *err_num=NULL);
    bool get_string(const char *key, std::string &value, int *err_num=NULL);
    bool set_number(const char *key, int64_t value, int *err_num=NULL);
    int64_t get_number(const char *key, int *err_num=NULL);
    int64_t incrby(const char *key, int64_t incr_val, int *err_num=NULL);
    int64_t decrby(const char *key, int64_t decr_val, int *err_num=NULL);
private:
    void err_handle(int context_err);
    bool set_string( std::ostringstream &cmd, int *err_num );
    bool number_operation(const std::ostringstream &cmd, int64_t *value, int *err_num);
    bool get_simple_value(const char *key);
private:
    redisContext *context;
    std::string server_ip;
    unsigned short server_port;
};


#endif