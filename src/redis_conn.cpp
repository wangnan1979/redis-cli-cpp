#include "redis_conn.h"
#include "redis_def.h"
#include <assert.h>
#include <sstream>


#ifdef _WIN32

#include <WinSock2.h>
#pragma comment(lib,"Ws2_32.lib")

//#pragma comment(lib,"my_redis/hiredis.lib")
#endif // _WIN32

void redis_initialize()
{

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err) {
        throw err;
    }
#endif

}

void redis_uninitialize()
{

#ifdef _WIN32
    if (SOCKET_ERROR == WSACleanup()) {
        throw WSAGetLastError();
    }
#endif

}

redis_conn_t::redis_conn_t(void)
:context(NULL),server_port(0)
{
}


redis_conn_t::~redis_conn_t(void)
{
}

bool redis_conn_t::open( const char *srv_ip, unsigned short port, int *err_num/*=NULL*/ )
{
    assert(srv_ip);
    
    context = redisConnect(srv_ip,port);
    if (context == NULL) {
        if (err_num!=NULL) {
            *err_num = NEW_ERR(ERR_REDIS_CONTEX,0xff);
        }
        return false;
    }
    else if(context->err) {
        if(err_num!=NULL) {
            *err_num = NEW_ERR(ERR_REDIS_CONTEX,context->err);
        }
        redisFree(context);
        return false;
    }

    server_ip.assign(srv_ip);
    server_port = port;

    return true;
}

bool redis_conn_t::set_string( const char *key, const char *value, int *err_num )
{
    int *err_ptr = err_num;
    int err;

    std::ostringstream cmd;
    cmd<<"set "<<key<<" "<<value;

    if(err_ptr==NULL) {
        err_ptr = &err;
    }
    bool is_success = set_string(cmd, err_ptr);

    return is_success;
}

bool redis_conn_t::get_string( const char *key, std::string &value, int *err_num/*=NULL*/ )
{
    std::string cmd("get ");
    cmd.append(key);
    redisReply *reply = (redisReply *)redisCommand(context, cmd.c_str());
    if (reply == NULL) {
        if (err_num!=NULL) {
            *err_num=NEW_ERR(ERR_REDIS_CONTEX,context->err);
        }
        return false;
    }
    else if(reply->type != REDIS_REPLY_STRING) {
        if (err_num!=NULL) {
            *err_num=NEW_ERR(ERR_REDIS_REPLY,reply->type);
        }
        return false;
    }
    else {
        value.assign(reply->str);
        return true;
    }
}

bool redis_conn_t::set_number( const char *key, int64_t value, int *err_num/*=NULL*/ )
{
    int *err_ptr = err_num;
    int err;

    std::ostringstream cmd;
    cmd<<"set "<<key<<" "<<value;    
    
    if (err_ptr==NULL) {
        err_ptr = &err;
    }
    bool is_success = set_string(cmd,err_ptr);

    return is_success;
}


int64_t redis_conn_t::get_number( const char *key, int *err_num/*=NULL*/ )
{
    int *err_ptr = err_num;
    int err;
    
    if (err_ptr==NULL) {
        err_ptr = &err;
    }

    std::string str_val;
    bool is_success = get_string(key,str_val,err_ptr);
    int64_t int_val;

    if (is_success)
    {
        std::istringstream iss(str_val);
        iss>>int_val;
    }    
    else if (err_num==NULL && !is_success) {
        throw err_ptr;
    }

    return int_val;
}


int64_t redis_conn_t::incrby( const char *key, int64_t incr_val, int *err_num/*=NULL*/ )
{
    int *err_ptr = err_num;
    int err;

    std::ostringstream cmd;
    cmd<<"incrby "<<key<<" "<<incr_val;

    if (err_ptr==NULL) {
        err_ptr = &err;
    }

    int64_t value;
    bool is_success = number_operation(cmd,&value,err_ptr);
    if (err_num==NULL && !is_success) {
        throw err_ptr;
    }

    return is_success;
}

int64_t redis_conn_t::decrby( const char *key, int64_t decr_val, int *err_num/*=NULL*/ )
{
    int *err_ptr = err_num;
    int err;

    std::ostringstream cmd;
    cmd<<"decrby "<<key<<" "<<decr_val;

    int64_t value;
    bool is_success = number_operation(cmd,&value,err_ptr);
    if (err_num==NULL && !is_success) {
        throw err_ptr;
    }

    return is_success;
}

void redis_conn_t::err_handle( int context_err )
{
    switch (context_err)
    {
    case REDIS_ERR_IO:
    case REDIS_ERR_EOF:
    case REDIS_ERR_PROTOCOL:
    case REDIS_ERR_OOM:
    case REDIS_ERR_OTHER:
        redisFree(context);
        context = NULL;
        open(server_ip.c_str(),server_port);
        break;
    default:
        break;
    }
}

bool redis_conn_t::number_operation( const std::ostringstream &cmd, int64_t *value, int *err_num )
{
    assert(value);
    assert(err_num);

    redisReply *reply = (redisReply *)redisCommand(context, cmd.str().c_str());
    if (reply == NULL) {
        *err_num=NEW_ERR(ERR_REDIS_CONTEX,context->err);
        return false;
    } 
    if(reply->type != REDIS_REPLY_INTEGER) {       
        *err_num=NEW_ERR(ERR_REDIS_REPLY,reply->type);
        return false;
    }
    else {
        *value = reply->integer;
        return true;
    }

}

bool redis_conn_t::set_string( std::ostringstream &cmd, int *err_num )
{
    assert(err_num);

    redisReply *reply = (redisReply *)redisCommand(context, cmd.str().c_str());
    if (reply == NULL) {
        *err_num=NEW_ERR(ERR_REDIS_CONTEX,context->err);
        return false;
    }
    else if(reply->type == REDIS_REPLY_ERROR) {
        *err_num=NEW_ERR(ERR_REDIS_REPLY,reply->type);
        return false;
    }

    return true;
}



