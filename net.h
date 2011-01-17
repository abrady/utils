/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef NET_H
#define NET_H

#include "abstd.h"
#include "array.h"
#include "winsock2.h" //ws2_32.lib
#include "WS2tcpip.h"

ABHEADER_BEGIN

#if WIN32
#define ECONNRESET              WSAECONNRESET
#define ENOTCONN                WSAENOTCONN
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EMSGSIZE                WSAEMSGSIZE    // buffer is full
#endif

// *************************************************************************
// foundational networking helpers
// *************************************************************************

SOCKET socket_accept(SOCKET *hlisten_sock, struct sockaddr *client_address);
BOOL sock_recv(SOCKET sock, char **hrecv_frame, BOOL *closed);
BOOL sock_canrecv(SOCKET sock, int wait_milliseconds);
BOOL sock_send(SOCKET sock, char **hhdr, char **hbuf);
BOOL sock_send_raw(SOCKET sock, char *hdr, int nhdr, char *buf, int nbuf);
BOOL sock_cansend(SOCKET sock, int wait_milliseconds);

// *************************************************************************
// Our networking layer
// *************************************************************************

typedef struct NetServer NetServer;
typedef struct NetLink NetLink;
typedef struct Pak Pak; 

typedef enum NetRecvState
{
    NetRecvState_None,
    NetRecvState_Hdr,
    NetRecvState_Body,    
    NetRecvState_Done,
    NetRecvState_Count,
} NetRecvState;

// ----------------------------------------
// Packet

#define PAK_VALID(P) ((P) && !(P)->error && (!(P)->body || achr_inrange(&(P)->body,(P)->i)))

// 1. client connects to server, sends one time handshake info to server
// 2. server recvs handshake. validates. sends info back to client. server can send and expects recv
// 3. client receives info, client->handshake = 1. client can send and recv
// in business
// typedef enum PakMsg
// {
//     PakMsg_None,
//     PakMsg_HandshakeClient,
//     PakMsg_HandshakeServer,
//     PakMsg_Count,
// } PakMsg;

typedef struct PakHdr
{    
    U32 msgid;
    U32 id;
    U32 size;  // valid on reads
} PakHdr;

typedef struct Pak
{
    PakHdr hdr;
    BOOL done;
    
    BOOL error;
    char *error_function;
    int error_line;

    char *body;
    int i;
} Pak;

#define pak_create(c, msgid ) pak_create_dbg(c, msgid  DBG_PARMS_INIT)
#define pak_send_int(p, a ) pak_send_int_dbg(p, a  DBG_PARMS_INIT)
#define pak_get_int(p ) pak_get_int_dbg(p  DBG_PARMS_INIT)
#define pak_send(c, p ) pak_send_dbg(c, p  DBG_PARMS_INIT)

Pak *pak_create_dbg(NetLink *c,int msgid DBG_PARMS);
void pak_send_int_dbg(Pak *p, int a DBG_PARMS);
int pak_get_int_dbg(Pak *p DBG_PARMS);
void pak_send_dbg(NetLink *c, Pak *p DBG_PARMS);

// ----------------------------------------
// connections

typedef void (*LinkConnFp)(NetLink* link);
typedef void (*LinkDiscoFp)(NetLink* link);
typedef BOOL (*MessageFp)(NetLink* link,Pak *pak,int cmd);

typedef struct NetCtxt
{
    U32 vers;
    int n_paks;
    int n_lnks;
    int n_svrs;

    NetServer **servers;
    NetLink **links;
} NetCtxt;

typedef struct NetLink
{
    NetCtxt *ctxt;
    SOCKET sock;
    BOOL connected;          // for non-blocking connects.   
    BOOL handshaken;         // app-level connect messages exchanged
    BOOL shutdown_req;       // shutdown has been requested   
    BOOL shutdown  ;         // shutdown from our side. can't send any longer
    BOOL closed    ;         // recv'd 0, close from other side
    LinkConnFp  conn_callback;
    LinkDiscoFp disco_callback;
    MessageFp   msg_callback;

    struct sockaddr address;
    char addr[32];              // debug
    DBG_PARMS_MBR;


    Pak **recvs;                // ap
    Pak **sends;
    
    NetServer *s;              // parent server, (optional)

    // recvs
    NetRecvState recv_state;
    char *recv_frame;           // accumulator of one or more reqs
    Pak  *recv_pak;

    // sends
    int n_paks; 

    // debug
    int id;                     // for debugging
} NetLink;

typedef struct NetServer
{
    NetCtxt *ctxt;
    int port;
    SOCKET listen_sock;
    NetLink **clients;
    LinkConnFp  conn_callback;
    LinkDiscoFp disco_callback;
    MessageFp   msg_callback;

    int id;                     // for debugging
} NetServer;

#define net_create() net_create_dbg(DBG_PARMS_INIT_SOLO)

NetCtxt*   net_create_dbg(DBG_PARMS_SOLO);
void       net_tick(NetCtxt *ctxt);
void       net_destroy(NetCtxt *ctxt);

NetServer* netserver_create(NetCtxt *ctxt, U16 port, LinkConnFp conn_callback, LinkDiscoFp disco_callback, MessageFp message_callback); 
void       netserver_destroy(NetServer **hs);
void       netserver_tick(NetServer *s);
void       net_shutdownlink(NetLink *client);
void       net_closelink(NetLink *client);    // NetLink* invalid after
NetLink*   net_connect(NetCtxt *ctxt, char *addr, U16 port, LinkConnFp conn_callback, LinkDiscoFp disco_callback, MessageFp message_callback, BOOL block);
BOOL       netlink_tick(NetLink *c);
void       netlink_destroy(NetLink **hc);
// *************************************************************************
//  misc
// *************************************************************************


// disable Nagle's algo. on by default on Win32, so no need to call this
#define SOCK_CLEAR_DELAY(SOCK) do { int nodelay = 1;                    \
        setsockopt(SOCK,IPPROTO_TCP,TCP_NODELAY,(void *)&nodelay,sizeof(nodelay)); \
    } while(0)
    
__forceinline BOOL sock_set_noblocking(SOCKET s)
{
    u_long on = 1; // for ioctlsocket
    return ioctlsocket(s,FIONBIO,&on) >= 0;
}


void net_init(void); // call once

ABHEADER_END
#endif //NET_H
