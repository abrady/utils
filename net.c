/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "abstd.h"
#include "stream.h"
#include "array.h"
#include "net.h"

#undef LOG_SYSTEM
#define LOG_SYSTEM "NET"

#define NET_VERSION 20080910
static NetCtxt g_netctxt = {0}; // global context

#define apak_create(capacity) apak_create_dbg(capacity  DBG_PARMS_INIT)
#define apak_destroy(ha, fp) apak_destroy_dbg(ha, fp DBG_PARMS_INIT)
#define apak_size(ha) apak_size_dbg(ha  DBG_PARMS_INIT)
#define apak_setsize(ha,size) apak_setsize_dbg(ha, size  DBG_PARMS_INIT)
#define apak_push(ha) apak_push_dbg(ha  DBG_PARMS_INIT)
#define apak_pop(ha) apak_pop_dbg(ha  DBG_PARMS_INIT)
#define apak_cp(hdest, hsrc, n) apak_cp_dbg(hdest, hsrc, n  DBG_PARMS_INIT)
#define apak_append(hdest, src, n) apak_append_dbg(hdest, src, n  DBG_PARMS_INIT)
#define apak_rm(ha, offset, n) apak_rm_dbg(ha, offset, n  DBG_PARMS_INIT)

#define TYPE_T Pak*
#define TYPE_FUNC_PREFIX apak 
#include "Array_def.h"
#include "Array_def.c"


#define alnk_create(capacity) alnk_create_dbg(capacity  DBG_PARMS_INIT)
#define alnk_destroy(ha, fp) alnk_destroy_dbg(ha, fp DBG_PARMS_INIT)
#define alnk_size(ha) alnk_size_dbg(ha  DBG_PARMS_INIT)
#define alnk_setsize(ha,size) alnk_setsize_dbg(ha, size  DBG_PARMS_INIT)
#define alnk_push(ha) alnk_push_dbg(ha  DBG_PARMS_INIT)
#define alnk_pop(ha) alnk_pop_dbg(ha  DBG_PARMS_INIT)
#define alnk_cp(hdest, hsrc, n) alnk_cp_dbg(hdest, hsrc, n  DBG_PARMS_INIT)
#define alnk_append(hdest, src, n) alnk_append_dbg(hdest, src, n  DBG_PARMS_INIT)
#define alnk_rm(ha, offset, n) alnk_rm_dbg(ha, offset, n  DBG_PARMS_INIT)
#define alnk_find(ha, b, cmp_fp, ctxt) alnk_find_dbg(ha, b, cmp_fp, ctxt DBG_PARMS_INIT)

#define TYPE_T NetLink*
#define TYPE_FUNC_PREFIX alnk
#include "Array_def.h"
#include "Array_def.c"

#define asvr_create(capacity) asvr_create_dbg(capacity  DBG_PARMS_INIT)
#define asvr_destroy(ha, fp) asvr_destroy_dbg(ha, fp DBG_PARMS_INIT)
#define asvr_size(ha) asvr_size_dbg(ha  DBG_PARMS_INIT)
#define asvr_setsize(ha,size) asvr_setsize_dbg(ha, size  DBG_PARMS_INIT)
#define asvr_push(ha) asvr_push_dbg(ha  DBG_PARMS_INIT)
#define asvr_pop(ha) asvr_pop_dbg(ha  DBG_PARMS_INIT)
#define asvr_cp(hdest, hsrc, n) asvr_cp_dbg(hdest, hsrc, n  DBG_PARMS_INIT)
#define asvr_append(hdest, src, n) asvr_append_dbg(hdest, src, n  DBG_PARMS_INIT)
#define asvr_rm(ha, offset, n) asvr_rm_dbg(ha, offset, n  DBG_PARMS_INIT)
#define asvr_find(ha, b, cmp_fp, ctxt) asvr_find_dbg(ha, &b, cmp_fp, ctxt DBG_PARMS_INIT)

#define TYPE_T NetServer*
#define TYPE_FUNC_PREFIX asvr
#include "Array_def.h"
#include "Array_def.c"


// helper
static char *str_from_ip(struct sockaddr *addr)
{
    struct sockaddr_in *a;
    if(!addr)
        return NULL;
    a = (struct sockaddr_in*)addr;
    return inet_ntoa(a->sin_addr);
}

static char*linkip(NetLink *c)
{
    if(!c)
        return NULL;
    return str_from_ip(&c->address);
}

typedef struct LinkHandshake
{
    U32 vers;
    U32 flags;  // @todo -AB: add endianness optimization :10/22/08
    // ...
} LinkHandshake;

//----------------------------------------
// helper for raw sockets that are 
// nonblocking, closes the passed socket
// on error
// TODO: optimize for win32 specific, e.g. async accept
//----------------------------------------
SOCKET socket_accept(SOCKET *hlisten_sock, struct sockaddr *client_address)
{
    SOCKET sock;
    int addr_len = sizeof(*client_address);    
    if(!hlisten_sock || *hlisten_sock == INVALID_SOCKET)
        return INVALID_SOCKET;
    sock = accept(*hlisten_sock,
                  client_address,
                  client_address?&addr_len:NULL);
    if(sock < 0)
    {
        if(last_error() != EWOULDBLOCK)
        {
            LOG("accept failed on socket %i: %s\n",*hlisten_sock,last_error_str());
            closesocket(*hlisten_sock);
        }
        return *hlisten_sock = INVALID_SOCKET;
    }
    SOCK_CLEAR_DELAY(sock); 
    return sock;
}

//----------------------------------------
//  receive helper.
// this really could use a dose of win32 
// optimizations. I should figure out how
// to schedule and defer receives...
// @todo -AB: get this optimized for async reads :09/24/08
//----------------------------------------
BOOL sock_recv(SOCKET sock, char **hrecv_frame, BOOL *closed)
{
    int total_read = 0;    
    if(closed)
        *closed = FALSE;
    if(sock == INVALID_SOCKET || !hrecv_frame)
        return FALSE;
    // apparently there is no way to know how much data is available to read...
    // I find this very hard to believe
#if 0
    {
        long cur = ftell(sock);
        long n_avil;
        int cur_size = achr_size(hrecv_frame);
        int n_read;
        
        assertm(0==fseek(sock,0,SEEK_END),"failed to seek.");
        n_avil = ftell(sock);
        assertm(0==fseek(sock,cur,SEEK_CUR));
        achr_setsize(hrecv_frame,cur_size+n_avil);
        n_read = recv(c->sock,(*hrecv_frame)+cur_size,n_avil,0);
        if(n_read<0)
        {
            if(last_error() != EWOULDBLOCK) 
                return FALSE;
        }
        else if(n_read == 0)                // closed on other end
        {
            if(closed)
                *closed = TRUE;
        }
        return TRUE;
    }
#endif
    for(;;)
    {
        int n_read;
        int cur_size = achr_size(hrecv_frame); 
        achr_setsize(hrecv_frame,cur_size+1024);
        n_read = recv(sock,(*hrecv_frame)+cur_size,1024,0);
        if(n_read<0)
        {
            if(last_error() != EWOULDBLOCK) 
                return FALSE;
            else
            {
                achr_setsize(hrecv_frame,cur_size);
                break;                      // out of data to read.
            }
        }
        else if(n_read == 0)                // closed on other end
        {
            if(closed)
                *closed = TRUE;
            achr_setsize(hrecv_frame,cur_size);
            break;
        }        
        achr_setsize(hrecv_frame,cur_size+n_read);
        total_read += n_read;
    }
    return TRUE;
}

BOOL sock_canrecv(SOCKET sock, int wait_milliseconds)
{
    struct timeval tv = {0};    
 	fd_set set = {0};
    int rc;
    tv.tv_sec = wait_milliseconds/1000;
    tv.tv_usec = (wait_milliseconds%1000)/1000;
    FD_ZERO(&set);
    FD_SET(sock,&set);
    rc = select(1,&set,NULL,NULL,&tv);
    return rc > 0 && FD_ISSET(sock,&set);
}

BOOL sock_cansend(SOCKET sock, int wait_milliseconds)
{
    struct timeval tv = {0};    
 	fd_set set = {0};
    int rc;
    tv.tv_sec = wait_milliseconds/1000;
    tv.tv_usec = (wait_milliseconds%1000)/1000;
    FD_ZERO(&set);
    FD_SET(sock,&set);
    rc = select(1,NULL,&set,NULL,&tv);
    return rc > 0 && FD_ISSET(sock,&set);
}

BOOL sock_send(SOCKET sock, char **hhdr, char **hbuf)
{
    if(!hhdr || !hbuf)
        return FALSE;
    return sock_send_raw(sock,*hhdr,achr_size(hhdr),*hbuf,achr_size(hbuf));
}

BOOL sock_send_raw(SOCKET sock, char *hdr, int nhdr, char *buf, int nbuf)
{
#ifdef WIN32
    DWORD n_sent = 0;
    WSABUF bufs[2] = {0};
    if(!hdr || !buf || sock == INVALID_SOCKET)
        return FALSE;
    bufs[0].len = nhdr;
    bufs[0].buf = hdr;
    bufs[1].len = nbuf;
    bufs[1].buf = buf;
    if(0!=WSASend(sock,bufs,ARRAY_SIZE(bufs),&n_sent,0,NULL,NULL))
        return FALSE;
    if((int)n_sent != nhdr + nbuf)
        return FALSE;
    return TRUE;
#else
#       error "not defined for non win32 yet"
    int n;
    if(sock == INVALID_SOCKET || !hbuf)
        return FALSE;
    n = achr_size(hbuf);
    return n == send(sock,*hbuf,n,0); // don't worry about partial sends and sends failing
#endif // WIN32
}

static BOOL connect_to_url(char *url, SOCKET *res)
{
	struct addrinfo *aiList = NULL;

	if(!url || !res)
		return FALSE;
	if(strstr(url,"http://"))
		url+=strlen("http://");
	
	*res = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(*res == INVALID_SOCKET)
		return FALSE;
	
	{
		char* ip = url;
		char* port = "80";
		struct addrinfo aiHints;
		int retVal;
		struct hostent* remoteHost;
		
		memset(&aiHints, 0, sizeof(aiHints));
		aiHints.ai_family = AF_INET;
		aiHints.ai_socktype = SOCK_STREAM;
		aiHints.ai_protocol = IPPROTO_TCP;
		
		remoteHost = gethostbyname(ip);
		if ((retVal = getaddrinfo(ip, port, &aiHints, &aiList)) != 0)
			return FALSE;
	}

	if(SOCKET_ERROR == connect(*res, aiList->ai_addr, (int)aiList->ai_addrlen))
		{
			closesocket(*res);
			freeaddrinfo(aiList);
			return FALSE;
		}
	freeaddrinfo(aiList);
	return TRUE;
}

// *************************************************************************
//  
// *************************************************************************

#define Pak_Create(c ) Pak_Create_dbg(c  DBG_PARMS_INIT)
static Pak *Pak_Create_dbg(NetLink *c DBG_PARMS);
static void Pak_Destroy(Pak **hp);
static void Pak_Cleanup(Pak *hp);

static NetLink* NetLink_Create(NetCtxt *ctxt)
{
    NetLink *p = MALLOCP(p);
    if(!ctxt)
        ctxt = &g_netctxt;
    p->id = ctxt->n_lnks++;
    p->ctxt = ctxt;
    return p;
}

static void NetLink_Destroy(NetLink **hl)
{
    NetLink *l;
    if(!hl || !*hl)
        return;
    l = *hl;
    apak_destroy(&l->recvs,Pak_Destroy);
    apak_destroy(&l->sends,Pak_Destroy);
    achr_destroy(&l->recv_frame);
    Pak_Destroy(&l->recv_pak);
    HFREE(hl);
}

static NetServer* NetServer_Create(NetCtxt *ctxt)
{
    NetServer*p = MALLOCP(p);
    if(!ctxt)
        ctxt = &g_netctxt;
    p->ctxt = ctxt;
    p->id = ctxt->n_svrs++;
    *asvr_push(&ctxt->servers) = p;
    return p;
}

static void NetServer_Destroy(NetServer **hs)
{
    NetCtxt *ctxt;
    NetServer *s;
    int i;
    if(!hs || !*hs)
        return;
    s = *hs;
    alnk_destroy(&s->clients,NetLink_Destroy);
    ctxt = s->ctxt;
    i = asvr_find(&ctxt->servers,s,NULL,NULL);
    if(i>=0)
        asvr_rm(&ctxt->servers,i,1);
    free(s);
}


void net_init(void)
{
	WSADATA wsaData;
    if(0!=WSAStartup(MAKEWORD(2,2), &wsaData))
		abErrorFatal("couldn't startup winsock: %s\n",last_error_str());
    g_netctxt.vers = NET_VERSION;
}



NetServer* netserver_create(NetCtxt *ctxt, U16 port, LinkConnFp conn_callback, LinkDiscoFp disco_callback, MessageFp msg_callback)
{
    struct sockaddr_in service = {0}; // bind
    NetServer *s = NetServer_Create(ctxt);
    s->port = port;
    s->listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s->conn_callback = conn_callback;
    s->disco_callback = disco_callback;
    s->msg_callback = msg_callback;
    
    if(s->listen_sock == INVALID_SOCKET)
    {
        printf("error creating socket %s\n", last_error_str());
        goto error;
    }
    
    if(!sock_set_noblocking(s->listen_sock))
    {
        printf("error setting listen socket to async: %s\n", last_error_str());
        goto error;
    }
    
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(port);
    if(bind( s->listen_sock, (SOCKADDR*)&service,sizeof(service)) == SOCKET_ERROR)
    {
        printf("bind failed %s\n",last_error_str());
        goto error;
    }
    
    SOCK_CLEAR_DELAY(s->listen_sock);
    
    if(listen(s->listen_sock,1) == SOCKET_ERROR)
    {
        printf("listen failed %s\n",last_error_str());
        goto error;
    }
    
    return s;
error:
    NetServer_Destroy(&s);
    return NULL;
}

void netserver_destroy(NetServer **hs)
{
    NetServer *s;
    if(!hs || !*hs)
        return;
    s = *hs;
    if(alnk_size(&s->clients))
        WARN("destroying server with active clients.");
    NetServer_Destroy(hs);
    *hs = NULL;
}


static void net_process_frame(NetLink *c)
{
    if(!c || c->sock == INVALID_SOCKET)
        return;
    for(;;)
    {
        if(!INRANGE0(c->recv_state,NetRecvState_Count))
        {
            ERR("link %s:%p in bad recv state. resetting.", linkip(c),c);
            c->recv_state = 0;
            Pak_Destroy(&c->recv_pak);
        }
        
        if(c->recv_state == NetRecvState_None)
        {
            assert(!c->recv_pak);
            c->recv_pak = Pak_Create(c);
            achr_setsize(&c->recv_frame,0);
            c->recv_state = NetRecvState_Hdr;
        }
        if(c->recv_state == NetRecvState_Hdr)
        {
            int i_hdr = 0;
            U32 tmp;
            if(achr_size(&c->recv_frame) < sizeof(PakHdr))
                break;
            i_hdr = 0;
            memcpy(&tmp,c->recv_frame+i_hdr,sizeof(tmp));
            i_hdr += sizeof(tmp);
            c->recv_pak->hdr.msgid = ntohl(tmp);

            memcpy(&tmp,c->recv_frame+i_hdr,sizeof(tmp));
            i_hdr += sizeof(tmp);
            c->recv_pak->hdr.id = ntohl(tmp);

            memcpy(&tmp,c->recv_frame+i_hdr,sizeof(tmp));
            i_hdr += sizeof(tmp);
            c->recv_pak->hdr.size = ntohl(tmp);
            
            achr_rm(&c->recv_frame,0,i_hdr);                             

            // validate
            // TODO: encrypt/unencrypt
            c->recv_state = NetRecvState_Body;        
        }
        if(c->recv_state == NetRecvState_Body)
        {
            int n = c->recv_pak->hdr.size;
            if(achr_size(&c->recv_frame) < n)
                break;
            achr_setsize(&c->recv_pak->body,n);
            memmove(c->recv_pak->body,c->recv_frame,n);
            achr_rm(&c->recv_frame,0,n);
            *apak_push(&c->recvs) = c->recv_pak;
            c->recv_pak = NULL;
            c->recv_state = 0;
            c->recv_state = NetRecvState_None;
        }
    }
}


void netserver_tick(NetServer *s)
{
    int i;
    int n;
    
    if(!s)
        return;
    if(s->listen_sock != INVALID_SOCKET)
    {
        SOCKET as;
        struct sockaddr client_address = {0};
        while((as = socket_accept(&s->listen_sock, &client_address)) != INVALID_SOCKET)
        {
            NetLink *l = NetLink_Create(s->ctxt);
            SOCK_CLEAR_DELAY(as);
            sock_set_noblocking(as);
            l->sock = as;
            l->address = client_address;
            l->s = s;
            *alnk_push(&s->clients) = l;
        }
    }

    n = alnk_size(&s->clients);
    for( i = n - 1; i >= 0; --i ) 
    {
        NetLink *c = s->clients[i];
        if(!netlink_tick(c))
        {
            NetLink_Destroy(&c);
            alnk_rm(&s->clients,i,1);
        }
    }
}


void net_shutdownlink(NetLink *client)
{
    if(!client)
        return;
    client->shutdown_req = TRUE;
}

#define IPADDR(str_addr) ((0==_stricmp(str_addr,"localhost"))?"127.0.0.1":str_addr)

NetLink* net_connect(NetCtxt *ctxt, char *addr, U16 port, LinkConnFp conn_callback, LinkDiscoFp disco_callback, MessageFp msg_callback, BOOL block)
{
    NetLink *c;
    char port_str[32];
    struct addrinfo *aiList = NULL;
    struct addrinfo aiHints = {0};
    if(!addr)
        return NULL;

    aiHints.ai_family = AF_INET;
    aiHints.ai_socktype = SOCK_STREAM;
    aiHints.ai_protocol = IPPROTO_TCP;
    itoa(port,port_str,10);
    if(0!=getaddrinfo(IPADDR(addr), port_str, &aiHints, &aiList))
        return NULL;

    c = NetLink_Create(ctxt);
    c->conn_callback = conn_callback;
    c->disco_callback = disco_callback;
    c->msg_callback = msg_callback;
    
    c->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(!block)
        sock_set_noblocking(c->sock);
    if(connect(c->sock, aiList->ai_addr, (int)aiList->ai_addrlen) == 0)
        c->connected = TRUE;
    else if(block || last_error() != EWOULDBLOCK)
    {
        LOG("failed to connect to %s server",addr);
        NetLink_Destroy(&c);
        freeaddrinfo(aiList);
        return NULL;
    }
    freeaddrinfo(aiList);
    sock_set_noblocking(c->sock);
    *alnk_push(&ctxt->links) = c;
    return c;
}

//----------------------------------------
// connect : TCP/UDP layer
// handshake : 
//  - once connected, send handshake
//  - wait for handshake response
//  - process (fail: close)
//----------------------------------------
BOOL netlink_tick(NetLink *c)
{
    MessageFp msg_callback;
    int i;
    char *hdr;
    int n;
    
    if(!c || c->sock == INVALID_SOCKET)
        return FALSE;
    if(!c->connected)
    {
        if(c->s) // server side sends handshake
        {
            LinkHandshake hs = {0};
            
            // remember to add sends below
            assert_infunc_static(sizeof(hs) == 8);
            
            hs.vers = htonl(c->ctxt->vers);
            hs.flags = htonl(0); // todo

            n = 0;
            if(n>=0) n = send(c->sock, (char*)&hs.vers, sizeof(hs.vers), 0);
            if(n>=0) n = send(c->sock, (char*)&hs.flags, sizeof(hs.flags), 0);
            if(n < 0)
            {
                if(last_error() == ENOTCONN) // okay, still connecting
                    return TRUE;
                else
                {
                    LOG("error handshaking client %s",last_error_str());
                    goto netlinktick_error;
                }
            }
            c->connected = TRUE;
        }
        else
        {
            if(sock_canrecv(c->sock,0))
                c->connected = TRUE;
        }
    }
    
    if(!c->handshaken)
    {
        if(!sock_recv(c->sock, &c->recv_frame, &c->closed))
        {
            ERR("error in link %s:%p recv during handshake %s", linkip(c),c,last_error_str());
            goto netlinktick_error;
        }

        if(c->s) // server-side handling of handshake response
        {
            U32 vers_res;
            
            if(achr_size(&c->recv_frame) < sizeof(vers_res))
                return TRUE;
            achr_mv(&vers_res,&c->recv_frame,sizeof(vers_res));
            vers_res = ntohl(vers_res);
            if(vers_res != c->ctxt->vers)
            {
                ERR("%s:%p link handshake responded with wrong version %u should be %u",linkip(c),c,vers_res,c->ctxt->vers);
                goto netlinktick_error;
            }
        }
        else
        {
            U32 tmp;
            LinkHandshake lh = {0};        
            if(achr_size(&c->recv_frame) < sizeof(lh))
                return TRUE;

            assert_infunc_static(sizeof(lh) == 8);
            achr_mv(&lh.vers,&c->recv_frame,sizeof(lh.vers));
            lh.vers = ntohl(lh.vers);
            achr_mv(&lh.flags,&c->recv_frame,sizeof(lh.flags));
            lh.flags = ntohl(lh.flags);

            if(lh.vers != c->ctxt->vers)
            {
                ERR("%s:%p version mismatch between client(%c) and server(%c)",linkip(c),c,c->ctxt->vers,lh.vers);
                goto netlinktick_error;
            }

            tmp = htonl(c->ctxt->vers);
            n = send(c->sock,(char*)&tmp,sizeof(c->ctxt->vers),0);
            if(n < 0)
            {
                ERR("%s:%p couldn't send version response",linkip(c),c);
                goto netlinktick_error;
            }
        }
        c->handshaken = TRUE;
        if(c->conn_callback)
            c->conn_callback(c);
        else if(SAFE_MEMBER(c->s,conn_callback))
            c->s->conn_callback(c);
    }

    // ----------------------------------------
    // recv and turn into packets 

    if(!sock_recv(c->sock, &c->recv_frame, &c->closed))
    {
        ERR("%s:%p recv failed %s",linkip(c),c,last_error_str());
        goto netlinktick_error;
    }
    net_process_frame(c);

    // ----------------------------------------
    // dispatch
    
    n = apak_size(&c->recvs);
    if(n)
    {
        msg_callback = c->msg_callback;
        if(!msg_callback && c->s)
            msg_callback = c->s->msg_callback;
        if(!msg_callback)
        {
            ERR("%s:%p no msg handler for link",linkip(c),c);
            goto netlinktick_error;    
        }
        
        for(i = 0;i < n; ++i)
        {
            Pak *m = c->recvs[i];
            if(!msg_callback(c,m,m->hdr.msgid))
                continue;
            Pak_Destroy(&m);
            apak_rm(&c->recvs,i,1);
        }
    }
    
    n = apak_size(&c->sends);
    if(n)
    {
        hdr = achr_create(sizeof(PakHdr));
        for( i = 0; i < n; ++i ) 
        {
            U32 tmp;
            Pak *p = c->sends[i];
            
            // @todo -AB: encrypted, etc. :09/26/08
            achr_setsize(&hdr,0); 
            tmp = htonl(p->hdr.msgid);
            achr_append(&hdr,(char*)&tmp,sizeof(tmp));
            
            tmp = htonl(p->hdr.id);
            achr_append(&hdr,(char*)&tmp,sizeof(tmp));
            
            tmp = htonl(achr_size(&p->body));
            achr_append(&hdr,(char*)&tmp,sizeof(tmp));        
            
            if(!sock_send(c->sock,&hdr,&p->body))
            {
                if(last_error() == EMSGSIZE) // out of room to send. try again later
                {
                    ERR("%s:%p send buffer full for socket: %s. waiting one tick",linkip(c),c,last_error_str());
                    break;
                }
                ERR("%s:%p failed to send on socket: %s",linkip(c),c,last_error_str());
                net_shutdownlink(c);
                Pak_Destroy(&p);
                break;
            }
            Pak_Destroy(&p);
        }
        achr_destroy(&hdr);
        apak_rm(&c->sends,0,i); // cleanup happens above
    }
    
    // ----------
    // cleanup

    // what are we doing here? The point is to allow a graceful
    // cleanup. At some point one side or the other is supposed
    // to initiate termination by calling 'shutdown'. there are
    // several ways this happens:
    // 1) link recvs 0 bytes : closed. all recvd packets are processed. sends are allowed. when all recv'd paks handled, shutdown is called
    // 2) link calls shutdown: no sends allowed, server should respond with FIN
    // e.g.
    // A: shutdown(send). sends FIN to B
    // B: recv returns 0. sets 'closed' flag. if no msgs to send
    //    will call shutdown itself and close the socket.
    // A: recv returns 0. closesocket

    if(apak_size(&c->sends) == 0 && c->shutdown_req && !c->shutdown)
    {
        shutdown(c->sock,SD_SEND);
        c->shutdown = TRUE;
    }
        
    if(!c->closed)     // @todo -AB: timeout code :09/25/08
        return TRUE;
    else if(apak_size(&c->recvs))
        return TRUE;

    if(!c->shutdown)
    {
        shutdown(c->sock,SD_SEND);
        c->shutdown = TRUE;
    }

    // if this link was made visibile above this layer
    if(c->handshaken)
    {
        if(c->disco_callback)
            c->disco_callback(c);
        else if(SAFE_MEMBER(c->s,disco_callback))
            c->s->disco_callback(c);
    }
    return TRUE;
netlinktick_error:
    closesocket(c->sock);
    c->sock = INVALID_SOCKET;
    return FALSE;
}

// relinquish link
void netlink_destroy(NetLink **hc)
{
    NetLink *c;
    if(!hc || !*hc)
        return;
    c = *hc;
    if(!c->shutdown)
        WARN("destroying link that has not been closed gracefully");
    NetLink_Destroy(hc);
}

void net_tick(NetCtxt *ctxt)
{
    int i;
    for( i = 0; i < asvr_size(&ctxt->servers); ++i ) 
        netserver_tick(ctxt->servers[i]);
    for( i = 0; i < alnk_size(&ctxt->links); ++i ) 
        netlink_tick(ctxt->links[i]);
}

void net_destroy(NetCtxt *ctxt)
{
    asvr_destroy(&ctxt->servers,netserver_destroy);
    alnk_destroy(&ctxt->links,netlink_destroy);
}


// *************************************************************************
// Pak 
// *************************************************************************

#define PAK_ERROR(P) do {                       \
        P->error = TRUE;                        \
        P->error_function = __FUNCTION__;       \
        P->error_line = __LINE__;               \
    } while(0); 

#define PAK_NEXT_BYTE(P) (P->body[P->i++])

static void Pak_Cleanup(Pak *p)
{
    if(!p)
        return;
    achr_destroy(&p->body);    
}


static void Pak_Destroy(Pak **p)
{
    if(!p || !*p)
        return;
    Pak_Cleanup(*p);
    HFREE(p);
}

static Pak *Pak_Create_dbg(NetLink *c DBG_PARMS)
{
    Pak *p;
    if(!c)
        return NULL;
    p = MALLOCP_DBG_CALL(p);
    p->hdr.id = c->n_paks++;
    return p;
}

Pak *pak_create_dbg(NetLink *c,int msgid DBG_PARMS)
{
    Pak *p;
    if(c->shutdown || c->shutdown_req)
        return NULL;
    p = Pak_Create_dbg(c DBG_PARMS_CALL);
    if(!p)
        return NULL;
    p->hdr.msgid = msgid;
    return p;
}

void pak_send_int_dbg(Pak *p, int a DBG_PARMS)
{
    if(!PAK_VALID(p))
        return;    
    strm_write_int_dbg(&p->body,a DBG_PARMS_CALL);
}

int pak_get_int_dbg(Pak *p DBG_PARMS)
{
    if(!PAK_VALID(p))
        return 0;
    return strm_read_int_dbg(&p->body,&p->i DBG_PARMS_CALL);
}

// takes ownership of Pak
void pak_send_dbg(NetLink *c, Pak *p DBG_PARMS)
{
    if(!c || c->sock == INVALID_SOCKET || !p)
        return;
    if(c->shutdown)
    {
        WARN("%s:%i packet for client %s:%s(%i) sent on shutdown link. caller %s:%i",caller_fname,line,c->addr,c->caller_fname,c->line DBG_PARMS_CALL);
        Pak_Destroy(&p);
        return;
    }
    *apak_push(&c->sends) = p;
    return;
}

NetCtxt*   net_create_dbg(DBG_PARMS_SOLO)
{
    NetCtxt *c = MALLOCP_DBG_CALL(c);
    c->vers = NET_VERSION;
    return c;
}
