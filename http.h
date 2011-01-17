/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef HTTP_H
#define HTTP_H
#include "abstd.h"
//#include "net.h"
#include "Str.h"
#include <WinSock2.h>

typedef enum HTTPReqType
{
    HTTPReqType_None,
    HTTPReqType_Get,
    HTTPReqType_Post,
    HTTPReqType_Count
} HTTPReqType;

typedef enum HTTPVer
{
    HTTPVer_None,
    HTTPVer_10,
    HTTPVer_11,
    HTTPVer_Count
} HTTPVer;

typedef struct HTTPServer HTTPServer;
typedef struct HTTPLink HTTPLink;
typedef struct HTTPReq HTTPReq;

typedef void (http_conn_fp)(HTTPServer *s, HTTPLink *l);
typedef void (http_disco_fp)(HTTPServer *s, HTTPLink *l, BOOL on_error);
typedef BOOL (http_req_fp)(HTTPServer *s, HTTPLink *l, HTTPReq *r);

typedef struct HTTPReq
{
    HTTPReqType type;
    char *body;
    char *uri;

    char *path;
    char **query_keys;
    char **query_values;
    int query_count;
    char *fragment;

    void *context; // usable by request handler
    BOOL done;          // set to TRUE when req is serviced
} HTTPReq;

typedef struct HTTPHdr
{
    HTTPVer ver;
    U32 keep_alive_until;       // ss2k
    BOOL close_link;    // Connection: close, the sender is closing this link
    int content_length;
} HTTPHdr;

typedef struct HTTPLink
{
    SOCKET sock;
    BOOL shutdown;      // shutdown from our side. can't send any longer
    BOOL closed;        // recv'd 0, close from other side

    struct sockaddr address;
    HTTPReq **reqs;              // ap
    http_disco_fp *disco_callback;
    HTTPServer *s;              // parent server, (optional)
    // ----------
    // request accumulation records

    char *recv_frame; // accumulator of one or more reqs
    
    // ab: stupid working from home checkin
    int recv_state;   // track what we're getting right now  
    char *hdr_buf;
    char *body_buf;
    HTTPHdr hdr;
    HTTPReqType req_type;
    char *req_uri;
} HTTPLink;


typedef struct HTTPServer
{
    int port;
    SOCKET listen_sock;
    int error_id;
    HTTPLink **clients;                         // ap
    http_conn_fp *conn_callback;
    http_disco_fp *disco_callback;
    http_req_fp *req_callback;
} HTTPServer;


HTTPServer* http_server_create(U16 port,http_req_fp req_callback,http_conn_fp conn_callback,http_disco_fp disco_callback); // create a listening socket
void http_server_tick(HTTPServer *s);
void http_server_shutdown(HTTPServer *s);
void http_server_destroy(HTTPServer **hs);

void http_explodereq(HTTPReq *request); // destructive to uri

// *************************************************************************
// send functions
// *************************************************************************

BOOL http_sendstr(HTTPLink *client,char *str);
BOOL http_sendfile(HTTPLink *client, const char *path, const char *mimetype); // NULL mimetype = application/octet-stream
BOOL http_sendbytes(HTTPLink *client, void *bytes, size_t bytes_len, const char *mimetype); // NULL mimetype = application/octet-stream
BOOL http_sendbadreq(HTTPLink *client);
void http_shutdownlink(HTTPLink *client, BOOL on_error);
#endif //HTTP_H
