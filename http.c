/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "winsock2.h"   //ws2_32.lib
#include "log.h"
#include "time.h"       // asctime
#include "Array.h"
#include "net.h"
#include "errno.h"
#include "abstd.h"
#include "timer.h"
#include "http.h"
#include "abhash.h"
#ifdef UTILS_LIB
#include "fileio.h"
#else
#include "file.h"		// fileAlloc() // FIXME: replace this
#endif

#undef LOG_SYSTEM
#define LOG_SYSTEM "NET"

//    HTTP/1.1 defines the sequence CR LF as the end-of-line marker for all
//    protocol elements except the entity-body (see appendix 19.3 for
//    tolerant applications). The end-of-line marker within an entity-body
//    is defined by its associated media type, as described in section 3.7.
//        CRLF           = CR LF

//    HTTP/1.1 header field values can be folded onto multiple lines if the
//    continuation line begins with a space or horizontal tab. All linear
//    white space, including folding, has the same semantics as SP. A
//    recipient MAY replace any linear white space with a single SP before
//    interpreting the field value or forwarding the message downstream.
//        LWS            = [CRLF] 1*( SP | HT )

//    Many HTTP/1.1 header field values consist of words separated by LWS
//    or special characters. These special characters MUST be in a quoted
//    string to be used within a parameter value (as defined in section
//    3.6).
//        token          = 1*<any CHAR except CTLs or separators>
//        separators     = "(" | ")" | "<" | ">" | "@"
//                       | "," | ";" | ":" | "\" | <">
//                       | "/" | "[" | "]" | "?" | "="
//                       | "{" | "}" | SP | HT
//
//        CTL            = <any US-ASCII control character
//                        (octets 0 - 31) and DEL (127)>

//    Comments can be included in some HTTP header fields by surrounding
//    the comment text with parentheses. Comments are only allowed in
//    fields containing "comment" as part of their field value definition.
//    In all other fields, parentheses are considered part of the field
//    value.
//        comment        = "(" *( ctext | quoted-pair | comment ) ")"
//        ctext          = <any TEXT excluding "(" and ")">

//    A string of text is parsed as a single word if it is quoted using
//    double-quote marks.
//        quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
//        qdtext         = <any TEXT except <">>

// 3 Protocol Parameters
// 3.1 HTTP Version
//    HTTP uses a "<major>.<minor>" numbering scheme to indicate versions
//    of the protocol. 
// HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

// 3.2 Uniform Resource Identifiers
//    URIs have been known by many names: WWW addresses, Universal Document
//    Identifiers, Universal Resource Identifiers [3], and finally the
//    combination of Uniform Resource Locators (URL) [4] and Names (URN)
//    [20]. As far as HTTP is concerned, Uniform Resource Identifiers are
//    simply formatted strings which identify--via name, location, or any
//    other characteristic--a resource.

// 3.2.1 General Syntax
//    URIs in HTTP can be represented in absolute form or relative to some
//    known base URI [11], depending upon the context of their use. The two
//    forms are differentiated by the fact that absolute URIs always begin
//    with a scheme name followed by a colon. For definitive information on
//    URL syntax and semantics, see "Uniform Resource Identifiers (URI):
//    Generic Syntax and Semantics," RFC 2396 [42] (which replaces RFCs
//    1738 [4] and RFC 1808 [11]). This specification adopts the
//    definitions of "URI-reference", "absoluteURI", "relativeURI", "port",
//    "host","abs_path", "rel_path", and "authority" from that
//    specification.
//
// 3.2.2 http URL
//    The "http" scheme is used to locate network resources via the HTTP
//    protocol. This section defines the scheme-specific syntax and
//    semantics for http URLs.
//    http_URL = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
//
//    If the port is empty or not given, port 80 is assumed. 
// ...

// // 3.3.1 Full Date
//    HTTP applications have historically allowed three different formats
//    for the representation of date/time stamps:
//
//       Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
//       Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
//       Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
// 
//    The first format is preferred as an Internet standard and represents
//    a fixed-length subset of that defined by RFC 1123 [8] (an update to
//    RFC 822 [9]). The second format is in common use, but is based on the
//    obsolete RFC 850 [12] date format and lacks a four-digit year.
//    HTTP/1.1 clients and servers that parse the date value MUST accept
//    all three formats (for compatibility with HTTP/1.0), though they MUST
//    only generate the RFC 1123 format for representing HTTP-date values
//    in header fields. See section 19.3 for further information.
//
//    All HTTP date/time stamps MUST be represented in Greenwich Mean Time
//    (GMT), without exception. For the purposes of HTTP, GMT is exactly
//    equal to UTC (Coordinated Universal Time). This is indicated in the
//    first two formats by the inclusion of "GMT" as the three-letter
//    abbreviation for time zone, and MUST be assumed when reading the
//    asctime format. HTTP-date is case sensitive and MUST NOT include
//    additional LWS beyond that specifically included as SP in the
//    grammar.

// 3.3.2 Delta Seconds
//    Some HTTP header fields allow a time value to be specified as an
//    integer number of seconds, represented in decimal, after the time
//    that the message was received.
//        delta-seconds  = 1*DIGIT

// 3.4 Character Sets
//    HTTP uses the same definition of the term "character set" as that
//    described for MIME:
// ...

// 3.5 Content Codings

//    Content coding values indicate an encoding transformation that has
//    been or can be applied to an entity. Content codings are primarily
//    used to allow a document to be compressed or otherwise usefully
//    transformed without losing the identity of its underlying media type
//    and without loss of information. Frequently, the entity is stored in
//    coded form, transmitted directly, and only decoded by the recipient.
//        content-coding   = token

// 3.6 Transfer Codings
//    Transfer-coding values are used to indicate an encoding
//    transformation that has been, can be, or may need to be applied to an
//    entity-body in order to ensure "safe transport" through the network.
//    This differs from a content coding in that the transfer-coding is a
//    property of the message, not of the original entity.
// ... 

// 3.7 Media Types
//    HTTP uses Internet Media Types [17] in the Content-Type (section
//    14.17) and Accept (section 14.1) header fields in order to provide
//    open and extensible data typing and type negotiation.
//        media-type     = type "/" subtype *( ";" parameter )
//        type           = token
//        subtype        = token

// 3.7.1 Canonicalization and Text Defaults

//    Internet media types are registered with a canonical form. An
//    entity-body transferred via HTTP messages MUST be represented in the
//    appropriate canonical form prior to its transmission except for
//    "text" types, as defined in the next paragraph.

// 3.7.2 Multipart Types

//    MIME provides for a number of "multipart" types -- encapsulations of
//    one or more entities within a single message-body. All multipart
//    types share a common syntax, as defined in section 5.1.1 of RFC 2046

// 4 HTTP Message

// 4.1 Message Types

//    HTTP messages consist of requests from client to server and responses
//    from server to client.

//        HTTP-message   = Request | Response     ; HTTP/1.1 messages

//    Request (section 5) and Response (section 6) messages use the generic
//    message format of RFC 822 [9] for transferring entities (the payload
//    of the message). Both types of message consist of a start-line, zero
//    or more header fields (also known as "headers"), an empty line (i.e.,
//    a line with nothing preceding the CRLF) indicating the end of the
//    header fields, and possibly a message-body.
//         generic-message = start-line
//                           *(message-header CRLF)
//                           CRLF
//                           [ message-body ]
//         start-line      = Request-Line | Status-Line

// header:
//        message-header = field-name ":" [ field-value ]
//        field-name     = token
//        field-value    = *( field-content | LWS )
//        field-content  = <the OCTETs making up the field-value
//                         and consisting of either *TEXT or combinations
//                         of token, separators, and quoted-string>

// 4.3 Message Body

//    The message-body (if any) of an HTTP message is used to carry the
//    entity-body associated with the request or response. The message-body
//    differs from the entity-body only when a transfer-coding has been
//    applied, as indicated by the Transfer-Encoding header field (section
//    14.41).

//        message-body = entity-body
//                     | <entity-body encoded as per Transfer-Encoding>

// ab: pay no attention to transfer-coding (see chunked above)

//    The presence of a message-body in a request is signaled by the
//    inclusion of a Content-Length or Transfer-Encoding header field in
//    the request's message-headers. A message-body MUST NOT be included in
//    a request if the specification of the request method (section 5.1.1)
//    does not allow sending an entity-body in requests. A server SHOULD
//    read and forward a message-body on any request; if the request method
//    does not include defined semantics for an entity-body, then the
//    message-body SHOULD be ignored when handling the request.

// 4.4 Message Length

//    The transfer-length of a message is the length of the message-body as
//    it appears in the message; that is, after any transfer-codings have
//    been applied. When a message-body is included with a message, the
//    transfer-length of that body is determined by one of the following
//    (in order of precedence):

// 4.5 General Header Fields

//    There are a few header fields which have general applicability for
//    both request and response messages, but which do not apply to the
//    entity being transferred. These header fields apply only to the
//    message being transmitted.

//        general-header = Cache-Control            ; Section 14.9
//                       | Connection               ; Section 14.10 - send 'close' when done
//                       | Date                     ; Section 14.18 - always send
//                       | Pragma                   ; Section 14.32 - 'no-cache', redundant.
//                       | Trailer                  ; Section 14.40 - chunked xfer only
//                       | Transfer-Encoding        ; Section 14.41 - chunked, gzip, compress, deflate
//                       | Upgrade                  ; Section 14.42 - transition from 1.1 to e.g. SHTTP
//                       | Via                      ; Section 14.45 - for proxies
//                       | Warning                  ; Section 14.46 - this is crap

// 5 Request

//    A request message from a client to a server includes, within the
//    first line of that message, the method to be applied to the resource,
//    the identifier of the resource, and the protocol version in use.

//         Request       = Request-Line              ; Section 5.1
//                         *(( general-header        ; Section 4.5
//                          | request-header         ; Section 5.3
//                          | entity-header ) CRLF)  ; Section 7.1
//                         CRLF
//                         [ message-body ]          ; Section 4.3

// 5.1 Request-Line

//    The Request-Line begins with a method token, followed by the
//    Request-URI and the protocol version, and ending with CRLF. The
//    elements are separated by SP characters. No CR or LF is allowed
//    except in the final CRLF sequence.

//         Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

//    The Method  token indicates the method to be performed on the
//    resource identified by the Request-URI. The method is case-sensitive.

//        Method         = "OPTIONS"                ; Section 9.2
//                       | "GET"                    ; Section 9.3
//                       | "HEAD"                   ; Section 9.4
//                       | "POST"                   ; Section 9.5
//                       | "PUT"                    ; Section 9.6
//                       | "DELETE"                 ; Section 9.7
//                       | "TRACE"                  ; Section 9.8
//                       | "CONNECT"                ; Section 9.9
//                       | extension-method
//        extension-method = token

// 5.1.2 Request-URI

//    The Request-URI is a Uniform Resource Identifier (section 3.2) and
//    identifies the resource upon which to apply the request.

//        Request-URI    = "*" | absoluteURI | abs_path | authority

//    The four options for Request-URI are dependent on the nature of the
//    request. The asterisk "*" means that the request does not apply to a
//    particular resource, but to the server itself, and is only allowed
//    when the method used does not necessarily apply to a resource. One
//    example would be

//        OPTIONS * HTTP/1.1

//    The absoluteURI form is REQUIRED when the request is being made to a
//    proxy. The proxy is requested to forward the request or service it
//    from a valid cache, and return the response. Note that the proxy MAY
//    forward the request on to another proxy or directly to the server
//    specified by the absoluteURI. In order to avoid request loops, a
//    proxy MUST be able to recognize all of its server names, including
//    any aliases, local variations, and the numeric IP address. An example
//    Request-Line would be:

//        GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1

//    To allow for transition to absoluteURIs in all requests in future
//    versions of HTTP, all HTTP/1.1 servers MUST accept the absoluteURI
//    form in requests, even though HTTP/1.1 clients will only generate
//    them in requests to proxies.

//    The most common form of Request-URI is that used to identify a
//    resource on an origin server or gateway. In this case the absolute
//    path of the URI MUST be transmitted (see section 3.2.1, abs_path) as
//    the Request-URI, and the network location of the URI (authority) MUST
//    be transmitted in a Host header field. For example, a client wishing
//    to retrieve the resource above directly from the origin server would
//    create a TCP connection to port 80 of the host "www.w3.org" and send
//    the lines:

//        GET /pub/WWW/TheProject.html HTTP/1.1
//        Host: www.w3.org

//    followed by the remainder of the Request. Note that the absolute path
//    cannot be empty; if none is present in the original URI, it MUST be
//    given as "/" (the server root).

// 5.2 The Resource Identified by a Request

//    The exact resource identified by an Internet request is determined by
//    examining both the Request-URI and the Host header field.

//    An origin server that does not allow resources to differ by the
//    requested host MAY ignore the Host header field value when
//    determining the resource identified by an HTTP/1.1 request. (But see
//    section 19.6.1.1 for other requirements on Host support in HTTP/1.1.)

//    An origin server that does differentiate resources based on the host
//    requested (sometimes referred to as virtual hosts or vanity host
//    names) MUST use the following rules for determining the requested
//    resource on an HTTP/1.1 request:

//    1. If Request-URI is an absoluteURI, the host is part of the
//      Request-URI. Any Host header field value in the request MUST be
//      ignored.

//    2. If the Request-URI is not an absoluteURI, and the request includes
//      a Host header field, the host is determined by the Host header
//      field value.

//    3. If the host as determined by rule 1 or 2 is not a valid host on
//      the server, the response MUST be a 400 (Bad Request) error message.

//    Recipients of an HTTP/1.0 request that lacks a Host header field MAY
//    attempt to use heuristics (e.g., examination of the URI path for
//    something unique to a particular host) in order to determine what
//    exact resource is being requested.

// 5.3 Request Header Fields

//    The request-header fields allow the client to pass additional
//    information about the request, and about the client itself, to the
//    server. These fields act as request modifiers, with semantics
//    equivalent to the parameters on a programming language method
//    invocation.

//        request-header = Accept                   ; Section 14.1
//                       | Accept-Charset           ; Section 14.2
//                       | Accept-Encoding          ; Section 14.3
//                       | Accept-Language          ; Section 14.4
//                       | Authorization            ; Section 14.8
//                       | Expect                   ; Section 14.20
//                       | From                     ; Section 14.22
//                       | Host                     ; Section 14.23
//                       | If-Match                 ; Section 14.24
//                       | If-Modified-Since        ; Section 14.25
//                       | If-None-Match            ; Section 14.26
//                       | If-Range                 ; Section 14.27
//                       | If-Unmodified-Since      ; Section 14.28
//                       | Max-Forwards             ; Section 14.31
//                       | Proxy-Authorization      ; Section 14.34
//                       | Range                    ; Section 14.35
//                       | Referer                  ; Section 14.36
//                       | TE                       ; Section 14.39
//                       | User-Agent               ; Section 14.43

//    Request-header field names can be extended reliably only in
//    combination with a change in the protocol version. However, new or
//    experimental header fields MAY be given the semantics of request-
//    header fields if all parties in the communication recognize them to
//    be request-header fields. Unrecognized header fields are treated as
//    entity-header fields.

// 6 Response

//    After receiving and interpreting a request message, a server responds
//    with an HTTP response message.

//        Response      = Status-Line               ; Section 6.1
//                        *(( general-header        ; Section 4.5
//                         | response-header        ; Section 6.2
//                         | entity-header ) CRLF)  ; Section 7.1
//                        CRLF
//                        [ message-body ]          ; Section 7.2

// 6.1 Status-Line

//    The first line of a Response message is the Status-Line, consisting
//    of the protocol version followed by a numeric status code and its
//    associated textual phrase, with each element separated by SP
//    characters. No CR or LF is allowed except in the final CRLF sequence.

//        Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

// 6.1.1 Status Code and Reason Phrase

//    The Status-Code element is a 3-digit integer result code of the
//    attempt to understand and satisfy the request. These codes are fully
//    defined in section 10. The Reason-Phrase is intended to give a short
//    textual description of the Status-Code. The Status-Code is intended
//    for use by automata and the Reason-Phrase is intended for the human
//    user. The client is not required to examine or display the Reason-
//    Phrase.

//    The first digit of the Status-Code defines the class of response. The
//    last two digits do not have any categorization role. There are 5
//    values for the first digit:

//       - 1xx: Informational - Request received, continuing process
//       - 2xx: Success - The action was successfully received,
//         understood, and accepted
//       - 3xx: Redirection - Further action must be taken in order to
//         complete the request
//       - 4xx: Client Error - The request contains bad syntax or cannot
//         be fulfilled
//       - 5xx: Server Error - The server failed to fulfill an apparently
//         valid request

//    The individual values of the numeric status codes defined for
//    HTTP/1.1, and an example set of corresponding Reason-Phrase's, are
//    presented below. The reason phrases listed here are only
//    recommendations -- they MAY be replaced by local equivalents without
//    affecting the protocol.

//       Status-Code    =
//             "100"  ; Section 10.1.1: Continue
//           | "101"  ; Section 10.1.2: Switching Protocols
//           | "200"  ; Section 10.2.1: OK
//           | "201"  ; Section 10.2.2: Created
//           | "202"  ; Section 10.2.3: Accepted
//           | "203"  ; Section 10.2.4: Non-Authoritative Information
//           | "204"  ; Section 10.2.5: No Content
//           | "205"  ; Section 10.2.6: Reset Content
//           | "206"  ; Section 10.2.7: Partial Content
//           | "300"  ; Section 10.3.1: Multiple Choices
//           | "301"  ; Section 10.3.2: Moved Permanently
//           | "302"  ; Section 10.3.3: Found
//           | "303"  ; Section 10.3.4: See Other
//           | "304"  ; Section 10.3.5: Not Modified
//           | "305"  ; Section 10.3.6: Use Proxy
//           | "307"  ; Section 10.3.8: Temporary Redirect
//           | "400"  ; Section 10.4.1: Bad Request
//           | "401"  ; Section 10.4.2: Unauthorized
//           | "402"  ; Section 10.4.3: Payment Required
//           | "403"  ; Section 10.4.4: Forbidden
//           | "404"  ; Section 10.4.5: Not Found
//           | "405"  ; Section 10.4.6: Method Not Allowed
//           | "406"  ; Section 10.4.7: Not Acceptable
//           | "407"  ; Section 10.4.8: Proxy Authentication Required
//           | "408"  ; Section 10.4.9: Request Time-out
//           | "409"  ; Section 10.4.10: Conflict
//           | "410"  ; Section 10.4.11: Gone
//           | "411"  ; Section 10.4.12: Length Required
//           | "412"  ; Section 10.4.13: Precondition Failed
//           | "413"  ; Section 10.4.14: Request Entity Too Large
//           | "414"  ; Section 10.4.15: Request-URI Too Large
//           | "415"  ; Section 10.4.16: Unsupported Media Type
//           | "416"  ; Section 10.4.17: Requested range not satisfiable
//           | "417"  ; Section 10.4.18: Expectation Failed
//           | "500"  ; Section 10.5.1: Internal Server Error
//           | "501"  ; Section 10.5.2: Not Implemented
//           | "502"  ; Section 10.5.3: Bad Gateway
//           | "503"  ; Section 10.5.4: Service Unavailable
//           | "504"  ; Section 10.5.5: Gateway Time-out
//           | "505"  ; Section 10.5.6: HTTP Version not supported
//           | extension-code

//       extension-code = 3DIGIT
//       Reason-Phrase  = *<TEXT, excluding CR, LF>

// 6.2 Response Header Fields

//    The response-header fields allow the server to pass additional
//    information about the response which cannot be placed in the Status-
//    Line. These header fields give information about the server and about
//    further access to the resource identified by the Request-URI.

//        response-header = Accept-Ranges           ; Section 14.5
//                        | Age                     ; Section 14.6
//                        | ETag                    ; Section 14.19
//                        | Location                ; Section 14.30
//                        | Proxy-Authenticate      ; Section 14.33
//                        | Retry-After             ; Section 14.37
//                        | Server                  ; Section 14.38
//                        | Vary                    ; Section 14.44
//                        | WWW-Authenticate        ; Section 14.47

// 7 Entity

//    Request and Response messages MAY transfer an entity if not otherwise
//    restricted by the request method or response status code. An entity
//    consists of entity-header fields and an entity-body, although some
//    responses will only include the entity-headers.

//    In this section, both sender and recipient refer to either the client
//    or the server, depending on who sends and who receives the entity.

// 7.1 Entity Header Fields

//    Entity-header fields define metainformation about the entity-body or,
//    if no body is present, about the resource identified by the request.
//    Some of this metainformation is OPTIONAL; some might be REQUIRED by
//    portions of this specification.

//        entity-header  = Allow                    ; Section 14.7
//                       | Content-Encoding         ; Section 14.11
//                       | Content-Language         ; Section 14.12
//                       | Content-Length           ; Section 14.13
//                       | Content-Location         ; Section 14.14
//                       | Content-MD5              ; Section 14.15
//                       | Content-Range            ; Section 14.16
//                       | Content-Type             ; Section 14.17
//                       | Expires                  ; Section 14.21
//                       | Last-Modified            ; Section 14.29
//                       | extension-header

//        extension-header = message-header

//    The extension-header mechanism allows additional entity-header fields
//    to be defined without changing the protocol, but these fields cannot
//    be assumed to be recognizable by the recipient. Unrecognized header
//    fields SHOULD be ignored by the recipient and MUST be forwarded by
//    transparent proxies.


// 8 Connections

// 8.1 Persistent Connections

// 8.1.1 Purpose
// ...
// 8.1.2 Overall Operation
//    Persistent connections provide a mechanism by which a client and a
//    server can signal the close of a TCP connection. This signaling takes
//    place using the Connection header field (section 14.10). Once a close
//    has been signaled, the client MUST NOT send any more requests on that
//    connection.

// 8.1.2.1 Negotiation

//    An HTTP/1.1 server MAY assume that a HTTP/1.1 client intends to
//    maintain a persistent connection unless a Connection header including
//    the connection-token "close" was sent in the request. If the server
//    chooses to close the connection immediately after sending the
//    response, it SHOULD send a Connection header including the
//    connection-token close.

//    An HTTP/1.1 client MAY expect a connection to remain open, but would
//    decide to keep it open based on whether the response from a server
//    contains a Connection header with the connection-token close. In case
//    the client does not want to maintain a connection for more than that
//    request, it SHOULD send a Connection header including the
//    connection-token close.

//    If either the client or the server sends the close token in the
//    Connection header, that request becomes the last one for the
//    connection.

//    Clients and servers SHOULD NOT assume that a persistent connection is
//    maintained for HTTP versions less than 1.1 unless it is explicitly
//    signaled. See section 19.6.2 for more information on backward
//    compatibility with HTTP/1.0 clients.

//    In order to remain persistent, all messages on the connection MUST
//    have a self-defined message length (i.e., one not defined by closure
//    of the connection), as described in section 4.4.

// 8.1.2.2 Pipelining

//    A client that supports persistent connections MAY "pipeline" its
//    requests (i.e., send multiple requests without waiting for each
//    response). A server MUST send its responses to those requests in the
//    same order that the requests were received.

//    Clients which assume persistent connections and pipeline immediately
//    after connection establishment SHOULD be prepared to retry their
//    connection if the first pipelined attempt fails. If a client does
//    such a retry, it MUST NOT pipeline before it knows the connection is
//    persistent. Clients MUST also be prepared to resend their requests if
//    the server closes the connection before sending all of the
//    corresponding responses.

//    Clients SHOULD NOT pipeline requests using non-idempotent methods or
//    non-idempotent sequences of methods (see section 9.1.2). Otherwise, a
//    premature termination of the transport connection could lead to
//    indeterminate results. A client wishing to send a non-idempotent
//    request SHOULD wait to send that request until it has received the
//    response status for the previous request.

// 9.3 GET

//    The GET method means retrieve whatever information (in the form of an
//    entity) is identified by the Request-URI. If the Request-URI refers
//    to a data-producing process, it is the produced data which shall be
//    returned as the entity in the response and not the source text of the
//    process, unless that text happens to be the output of the process.

//    The semantics of the GET method change to a "conditional GET" if the
//    request message includes an If-Modified-Since, If-Unmodified-Since,
//    If-Match, If-None-Match, or If-Range header field. A conditional GET
//    method requests that the entity be transferred only under the
//    circumstances described by the conditional header field(s). The
//    conditional GET method is intended to reduce unnecessary network
//    usage by allowing cached entities to be refreshed without requiring
//    multiple requests or transferring data already held by the client.

//    The semantics of the GET method change to a "partial GET" if the
//    request message includes a Range header field. A partial GET requests
//    that only part of the entity be transferred, as described in section
//    14.35. The partial GET method is intended to reduce unnecessary
//    network usage by allowing partially-retrieved entities to be
//    completed without transferring data already held by the client.

//    The response to a GET request is cacheable if and only if it meets
//    the requirements for HTTP caching described in section 13.

//    See section 15.1.3 for security considerations when used for forms.

// 9.4 HEAD
// ab: nobody does this
// 9.5 POST

//    The POST method is used to request that the origin server accept the
//    entity enclosed in the request as a new subordinate of the resource
//    identified by the Request-URI in the Request-Line. POST is designed
//    to allow a uniform method to cover the following functions:
//       - Annotation of existing resources;
//       - Posting a message to a bulletin board, newsgroup, mailing list,
//         or similar group of articles;
//       - Providing a block of data, such as the result of submitting a
//         form, to a data-handling process;
//       - Extending a database through an append operation.

//    The action performed by the POST method might not result in a
//    resource that can be identified by a URI. In this case, either 200
//    (OK) or 204 (No Content) is the appropriate response status,
//    depending on whether or not the response includes an entity that
//    describes the result.

//    If a resource has been created on the origin server, the response
//    SHOULD be 201 (Created) and contain an entity which describes the
//    status of the request and refers to the new resource, and a Location
//    header (see section 14.30).

//    Responses to this method are not cacheable, unless the response
//    includes appropriate Cache-Control or Expires header fields. However,
//    the 303 (See Other) response can be used to direct the user agent to
//    retrieve a cacheable resource.

//    POST requests MUST obey the message transmission requirements set out
//    in section 8.2.

//    See section 15.1.3 for security considerations.

// 11 Access Authentication

//    HTTP provides several OPTIONAL challenge-response authentication
//    mechanisms which can be used by a server to challenge a client
//    request and by a client to provide authentication information. The
//    general framework for access authentication, and the specification of
//    "basic" and "digest" authentication, are specified in "HTTP
//    Authentication: Basic and Digest Access Authentication" [43]. This
//    specification adopts the definitions of "challenge" and "credentials"
//    from that specification.

//    [43] Franks, J., Hallam-Baker, P., Hostetler, J., Lawrence, S.,
//         Leach, P., Luotonen, A., Sink, E. and L. Stewart, "HTTP
//         Authentication: Basic and Digest Access Authentication", RFC
//         2617, June 1999. [jg646]

// 12 Content Negotiation
// http://en.wikipedia.org/wiki/Content_negotiation

// 13 Caching in HTTP
// 13.1.3 Cache-control Mechanisms

//    The basic cache mechanisms in HTTP/1.1 (server-specified expiration
//    times and validators) are implicit directives to caches. In some
//    cases, a server or client might need to provide explicit directives
//    to the HTTP caches. We use the Cache-Control header for this purpose.

//    The Cache-Control header allows a client or server to transmit a
//    variety of directives in either requests or responses. These
//    directives typically override the default caching algorithms. As a
//    general rule, if there is any apparent conflict between header
//    values, the most restrictive interpretation is applied (that is, the
//    one that is most likely to preserve semantic transparency). However,

// 13.2 Expiration Model

// 13.2.1 Server-Specified Expiration

//    HTTP caching works best when caches can entirely avoid making
//    requests to the origin server. The primary mechanism for avoiding
//    requests is for an origin server to provide an explicit expiration
//    time in the future, indicating that a response MAY be used to satisfy
//    subsequent requests. In other words, a cache can return a fresh
//    response without first contacting the server.

//    Our expectation is that servers will assign future explicit
//    expiration times to responses in the belief that the entity is not
//    likely to change, in a semantically significant way, before the
//    expiration time is reached. This normally preserves semantic
//    transparency, as long as the server's expiration times are carefully
//    chosen.

//    The expiration mechanism applies only to responses taken from a cache
//    and not to first-hand responses forwarded immediately to the
//    requesting client.

//    If an origin server wishes to force a semantically transparent cache
//    to validate every request, it MAY assign an explicit expiration time
//    in the past. This means that the response is always stale, and so the
//    cache SHOULD validate it before using it for subsequent requests. See
//    section 14.9.4 for a more restrictive way to force revalidation.

//    If an origin server wishes to force any HTTP/1.1 cache, no matter how
//    it is configured, to validate every request, it SHOULD use the "must-
//    revalidate" cache-control directive (see section 14.9).

//    Servers specify explicit expiration times using either the Expires
//    header, or the max-age directive of the Cache-Control header.

//    An expiration time cannot be used to force a user agent to refresh
//    its display or reload a resource; its semantics apply only to caching
//    mechanisms, and such mechanisms need only check a resource's
//    expiration status when a new request for that resource is initiated.
//    See section 13.13 for an explanation of the difference between caches
//    and history mechanisms.

// 13.2.3 Age Calculations

//    In order to know if a cached entry is fresh, a cache needs to know if
//    its age exceeds its freshness lifetime. We discuss how to calculate
//    the latter in section 13.2.4; this section describes how to calculate
//    the age of a response or cache entry.

//    HTTP/1.1 requires origin servers to send a Date header, if possible,
//    with every response, giving the time at which the response was
//    generated (see section 14.18). We use the term "date_value" to denote
//    the value of the Date header, in a form appropriate for arithmetic
//    operations.



// Notes: 
// - CRLF+space/tab should be turned into space
// - tokens are CHAR outside of 0-31, 127, and separators.
// - comments in header are in parentheses ???
// - strings are en quoted, and parsed as a single word (but word is undefined? token?)
// - use 'asctime()' to generate date/time stamps. use GMT. use delta-seconds if possible
// - I'm not going to worry about character sets for now (if ever)
// - not going to worry about transfer codings, and chunked data.
// - transfer-length is the octect count after tranformations (e.g. compression)
// - only need to support 'GET' and 'HEAD' methods ('POST' too)
// - if using a proxy with GET, must send 'absoluteURI'
// - if connecting directly, must use 'abs_path' plus 'Host: www.w3.org'
// - for servers, extract host from Request-URI always, if present, ignore host
// - response line is 'status: OK' plus general header
// - Entity: meta info about what is transmitted, optional.
// * per-connection: pipelined requests must be answered IN ORDER
// - do not persist connections with HTTP/1.0 clients (I say: don't support)
// * timeout of conns on server: don't close in the middle of transporting data
// * cache control

// *************************************************************************
//  Example Headers
// *************************************************************************

// ----------------------------------------
// from client http://localhost

// Firefox:
// recv GET / HTTP/1.1
// Host: localhost
// User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.0; en-US; rv:1.9.0.1) Gecko/2008070208 Firefox/3.0.1
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Accept-Language: en-us,en;q=0.5
// Accept-Encoding: gzip,deflate
// Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
// Keep-Alive: 300
// Connection: keep-alive

// IE
// recv GET / HTTP/1.1
// Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-ms-application, application/vnd.ms-xpsdocument, application/xaml+xml, application/x-ms-xbap, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, application/x-shockwave-flash, */*
// Accept-Language: en-us
// UA-CPU: x86
// Accept-Encoding: gzip, deflate
// User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0; SLCC1; .NET CLR 2.0.50727; .NET CLR 3.0.04506)
// Host: localhost
// Connection: Keep-Alive


// telnet www.yahoo.com 80
// >GET / HTTP 1.1
// response:
// HTTP/1.1 400 Bad Request
// Date: Tue, 19 Aug 2008 04:36:49 GMT
// Cache-Control: private
// Connection: close
// Content-Type: text/html; charset=iso-8859-1
// 
// ...

// Fields of Header
// HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
// content-coding = token ; gzip, compress (LZW), deflate (zlib), identity (default, nothing)
// Content-Length : N ; length of body, tells server/client there is a body 
// Date : ; required (age for response header?)

// message format
// - start-line
// - zero+ header fields
// - empty line (for end of header)
// - message body

void HTTPReq_destroy(HTTPReq **hr)
{
    HTTPReq *r;
    if(!hr || !*hr)
        return;
    r = *hr;
    achr_destroy(&r->body);
    achr_destroy(&r->uri);
    ap_destroy(&r->query_keys, 0);
    ap_destroy(&r->query_values, 0);
    HFREE(hr);
}

void HTTPLink_destroy(HTTPLink **hl)
{
    int i;
    HTTPLink *l;
    if(!hl || !*hl)
        return;
    l = *hl;
    closesocket(l->sock);
    for( i = 0; i < ap_size(&l->reqs); ++i ) 
        HTTPReq_destroy(&l->reqs[i]);
    achr_destroy(&l->recv_frame);
    achr_destroy(&l->hdr_buf);
    achr_destroy(&l->body_buf);
    achr_destroy(&l->req_uri);
    HFREE(hl);
}

void HTTPServer_Destroy(HTTPServer **hs)
{
    int i;
    HTTPServer *s;
    if(!hs || !*hs)
        return;
    s = *hs;
    closesocket(s->listen_sock);
    for( i = 0; i < ap_size(&s->clients); ++i ) 
        HTTPLink_destroy(&s->clients[i]);
    HFREE(hs);
}

BOOL disco_error()
{
    return errno == ENOTCONN || errno == ECONNRESET;  
}

HTTPServer* http_server_create(U16 port,http_req_fp req_callback,http_conn_fp conn_callback,http_disco_fp disco_callback)
{
    struct sockaddr_in service = {0}; // bind
    HTTPServer *s = MALLOCP(s);
    s->port = port;
    s->listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s->conn_callback = conn_callback;
    s->disco_callback = disco_callback;
    s->req_callback = req_callback;
    
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
    service.sin_addr.s_addr = ADDR_ANY; // inet_addr("127.0.0.1");
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
    HTTPServer_Destroy(&s);
    return NULL;
}

void http_server_destroy(HTTPServer **hs)
{
    HTTPServer_Destroy(hs);
}

static BOOL http_is_token_char(char c)
{
    static S32 lookup[256/32] = {0}; 
    if(!lookup[0])
    {
        lookup[0] = 0xffffffff;
        bit_set(lookup,'(');
        bit_set(lookup,')');
        bit_set(lookup,'<');
        bit_set(lookup,'>');
        bit_set(lookup,'@');
        bit_set(lookup,',');
        bit_set(lookup,';');
        bit_set(lookup,':');
        bit_set(lookup,' ');
        bit_set(lookup,'<');
        bit_set(lookup,'>');
        bit_set(lookup,'/');
        bit_set(lookup,'[');
        bit_set(lookup,']');
        bit_set(lookup,'?');
        bit_set(lookup,'=');
        bit_set(lookup,'{');
        bit_set(lookup,'}');
        bit_set(lookup,' ');
        bit_set(lookup,'\t');
        bit_set(lookup,127); // del
    }
    return !bit_tst(lookup,c);
}

ABINLINE int xdigit_to_i(char c) { return isdigit((U8)c) ? c-'0' : toupper(c)-'A'+10; }

static void http_cleanuri(char *uri)
{
	char *in, *out = uri;
	for(in = uri; *in; in++)
	{
		if(*in == '+')
		{
			*(out++) = ' ';
		}
		else if(in[0] == '%' && isxdigit((U8)in[1]) && isxdigit((U8)in[2]))
		{
			*(out++) = (char)(16*xdigit_to_i(in[1]) + xdigit_to_i(in[2]));
			in += 2;
		}
		else
		{
			*(out++) = *in;
		}
	}
	*out = *in;
}

void http_explodereq(HTTPReq *request)
{
	char *cur, *s;

	if(!request || !request->uri)
		return;

	cur = request->uri;
	if(cur[0] == '/')
		cur++;
	request->path = cur;

	request->fragment = strchr(cur, '#');
	if(request->fragment)
	{
		*request->fragment = '\0';
		request->fragment++;
		http_cleanuri(request->fragment);
	}

	s = strchr(cur, '?');
	if(s)
	{
		*s = '\0';
		cur = s+1;
		while(cur)
		{
			char *key = cur;
			char *value = strchr(cur, '=');
			cur = strchr(cur, '&');

			if(value && (value < cur || !cur))
				*(value++) = '\0'; // truncate key, value is next
			else if(cur)
				value = cur; // set value to the next '\0'
			else
				value = key + strlen(key); // set value to the next '\0'

			if(cur)
				*(cur++) = '\0'; // truncate value

			http_cleanuri(key);
			http_cleanuri(value);

			ap_push(&request->query_keys, key);
			ap_push(&request->query_values, value);
			request->query_count++;
		}
	}

	http_cleanuri(request->path);
}


//----------------------------------------
//  hres is a Str
//----------------------------------------
static char* http_next_token(Str *res, char **s)
{
    if(!s)
        return NULL;
    while(**s && !http_is_token_char(**s))
        (*s)++;
    if(!**s)
        return NULL;
    achr_clear(res);
    while(http_is_token_char(**s))
    {
        achr_push(res,**s);
        (*s)++;
    }
    
    achr_push(res,0); // NULL terminator
    return *res;
}

// read the argument to a token
static char* http_read_token_arg(Str *res, char **s_arg)
{
    unsigned char **s = (unsigned char**)s_arg; // compiler warning in VC
    while(isspace(**s))
        (*s)++;
    
    achr_clear(res);
    while(!isspace(**s))
    {
        achr_push(res,**s);
        (*s)++;
    }
    achr_push(res,0);
    return *res;
}


HTTPVer HTTPVer_from_str(char *s)
{
    if(!s)
        return HTTPVer_11;
    STRSTR_ADVANCE(s,"HTTP/");
    if(!s)
        return HTTPVer_11; // default
    else if(strcmp(s,"1.1"))
        return HTTPVer_11;
    else if(strcmp(s,"1.0"))
        return HTTPVer_10;
    return HTTPVer_11; // default.
}


//----------------------------------------
// see if we've gotten an entire
// entity
//----------------------------------------
static void http_process_frame(HTTPLink *c)
{
    char *s;
    if(!c)
        return;
    if(c->recv_state == NetRecvState_None)
    {
        int n;
        s=c->recv_frame;
        if(!(STRSTR_ADVANCE(s,"\r\n\r\n")))
            return;
        // move the header from the frame
        n = (int)(s-c->recv_frame); 
        achr_cp(&c->hdr_buf,&c->recv_frame,n);
        Str_setsize(&c->hdr_buf,n);
        Str_rm(&c->recv_frame,0,n);
        c->recv_state = NetRecvState_Hdr;
        
    }
        
    if(c->recv_state == NetRecvState_Hdr)
    {
        // Fields of Header
        // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
        // content-coding = token ; gzip, compress (LZW), deflate (zlib), identity (default, nothing)
        // Content-Length : N ; length of body, tells server/client there is a body 
        // Date : ; required (age for response header?)
        char *tok = NULL;
        assert(c->hdr_buf);
#define GET_TOKEN_VAL(STR) ((s=c->hdr_buf,STRSTR_ADVANCE(s,STR)) && http_next_token(&tok,&s))
#define GET_TOKEN_ARG(STR) ((s=c->hdr_buf,STRSTR_ADVANCE(s,STR)) && http_read_token_arg(&tok,&s))
        if(GET_TOKEN_VAL("HTTP-Version"))
            c->hdr.ver = HTTPVer_from_str(tok);
        if(GET_TOKEN_ARG("GET "))
        {
            Str_printf(&c->req_uri, "%s", tok);
            c->hdr.ver = HTTPVer_from_str(tok);
            c->req_type = HTTPReqType_Get;
        }
        if(GET_TOKEN_ARG("POST"))
        {
            Str_printf(&c->req_uri, "%s", tok);
            c->hdr.ver = HTTPVer_from_str(tok);
            c->req_type = HTTPReqType_Post;
        }
        if(GET_TOKEN_VAL("Keep-Alive:"))    // firefox sends this
            c->hdr.keep_alive_until = time_ss2k() + atoi(tok);
        if(GET_TOKEN_VAL("Connection:") && 0==_stricmp(tok,"close"))
            c->hdr.close_link = TRUE;
//     if(GET_TOKEN_VAL("Accept-Encoding"))
//     {
//     }
        if(GET_TOKEN_VAL("Content-Length:"))
            c->hdr.content_length = atoi(tok);
        
        if(c->hdr.content_length)
            c->recv_state = NetRecvState_Body;
        else
            c->recv_state = NetRecvState_Done;
        
        Str_destroy(&tok);
    }
        
    if(c->recv_state == NetRecvState_Body)
    {
        assert(c->hdr.content_length);
        if(Str_len(&c->recv_frame) < c->hdr.content_length)
            return;
        achr_ncp(&c->body_buf,c->recv_frame,c->hdr.content_length);
        achr_rm(&c->recv_frame,0,c->hdr.content_length); 
        c->recv_state = NetRecvState_Done;
    }
        
    if(c->recv_state == NetRecvState_Done)
    {
        if(c->req_type)
        {
            HTTPReq *r = MALLOCP(r);
            r->type = c->req_type;
            COPY_CLEAR(r->uri, c->req_uri);
            COPY_CLEAR(r->body, c->body_buf);
            ap_push(&c->reqs,r);
        }
        
        // cleanup 
        achr_clear(&c->hdr_buf);
        achr_clear(&c->body_buf);
        achr_clear(&c->req_uri);
        c->recv_state = 0;
        c->req_type = 0;
        ClearStruct(&c->hdr);
    }
}

//----------------------------------------
//  if closing a link:
// - call shutdown.
// - wait until recv returns 0
// - call closesocket
//----------------------------------------
void http_shutdownlink(HTTPLink *client, BOOL on_error)
{
    if(!SAFE_MEMBER(client,sock) || client->sock == INVALID_SOCKET || client->shutdown)
        return;

    if(client->disco_callback)
        client->disco_callback(client->s,client,on_error);
    else if(SAFE_MEMBER(client->s,disco_callback))
        client->s->disco_callback(client->s,client,on_error);
    client->shutdown = TRUE;
    shutdown(client->sock,SD_SEND);
}

void http_server_tick(HTTPServer *s)
{
    int n;
    int i;
    if(!s)
        return;
    
    if(s->listen_sock != INVALID_SOCKET)
    {
        SOCKET as;
        struct sockaddr client_address = {0};
        while((as = socket_accept(&s->listen_sock, &client_address)) != INVALID_SOCKET)
        {
            HTTPLink *l = MALLOCT(*l);
            SOCK_CLEAR_DELAY(as);
            sock_set_noblocking(as);
            l->sock = as;
            l->address = client_address;
            l->disco_callback = s->disco_callback;
            l->s = s;
            ap_push(&s->clients,l);
            if(s->conn_callback)
                s->conn_callback(s,l);
        }
    }

    n = ap_size(&s->clients);
    for( i = 0; i < n; ++i ) 
    {
        HTTPLink *c = s->clients[i];
        if(c->sock == INVALID_SOCKET)
            continue;

        if(!sock_recv(c->sock,&c->recv_frame,&c->closed))
        {
            LOG("error reading from socket %s\n", last_error_str());
            http_shutdownlink(c,TRUE);
            continue;
        }
        http_process_frame(c);
    }

    // ----------
    // dispatch requests

    n = ap_size(&s->clients);
    for( i = 0; i < n; ++i ) 
    {
        HTTPLink *c = s->clients[i];
        int j;
        int n_reqs = ap_size(&c->reqs);
        if(c->shutdown) // not allowed to send
            continue;
        for( j = n_reqs-1; j >= 0; --j ) 
        {
            HTTPReq *r = c->reqs[j];
            if(!s->req_callback || !s->req_callback(s,c,r))
            {
                // no default handler
                continue;
            }
            if(r->done)
            {
                HTTPReq_destroy(&c->reqs[j]);
                ap_rm(&c->reqs,j,1);
            }
        }
    }

    // ----------
    // cleanup

    n = ap_size(&s->clients);
    for( i = n - 1; i >= 0; --i ) 
    {
        HTTPLink *c = s->clients[i];
        // @todo -AB: timeout code :09/25/08
        // what are we doing here?
        // case 1: client is closed. outstanding reqs: wait (forever?)
        // case 2: client is closed. nothing to send: shutdown
        // case 3: client open. server's called shutdown. c->closed should get set above
        // case 4: client keeps socket open forever: do we care?
        if(!c->closed)
            continue;
        else if(ap_size(&c->reqs))   // not receiving any more, may still send
            continue;
        // client closed and shutdown
        http_shutdownlink(c,FALSE);
        HTTPLink_destroy(&c);
        ap_rm(&s->clients,i,1);
    }
}

static BOOL http_send_response(HTTPLink *client, char *status, char *header, S8 *body, int body_len)
{
    // 6 Response
    char *hdr = NULL;
    char tbuf[128];
    if(!client || client->sock == INVALID_SOCKET)
        return FALSE;
    curtime_str(ASTR(tbuf));
    Str_printf(&hdr,
               // response starts with the status line e.g.
               // - HTTP-Version HTTP 1.1
               // - HTTP/1.1 400 Bad Request
               "%s\r\n"                                 // arg 0: status
               // the generic header fields next
               "Date: %s\r\n",                           // arg 1: ctime UTC
               status,                  // arg 0: status
               tbuf                     // arg 1: time
        ); 

    if(header)
        Str_catf(&hdr, "%s", header); // additional header fields
    
    if(body)
    {
        // if a body, send the 'entity' header:
        // Content-Type:                        (required?)
        // Content-Encoding: gzip,compress,...  (default, nothing)
        // Content-Length: N                    (required)
        Str_catf(&hdr,
                "Content-Length: %i\r\n",                   // arg 1: body length
                body_len                                // arg 1: length
        );
    }

    Str_catf(&hdr, "\r\n");      // end of header
    achr_pop(&hdr); // remove NULL term
    if(!sock_send_raw(client->sock,hdr,achr_size(&hdr),body,body_len))
    {
        LOG("send failed");
        http_shutdownlink(client,TRUE);
        return FALSE;
    }
    achr_destroy(&hdr);
    return TRUE;
}


BOOL http_sendstr(HTTPLink *client,char *str)
{
    int n = (int)strlen(str);
    return http_send_response(client,"HTTP/1.1 200 OK",  
                              // i.e how the body is represented (its type)
                              "Content-Type: text/html\r\n", // ; charset=ISO-8859-4 ???
                              str,n);
}

BOOL http_sendbytes(HTTPLink *client, void *bytes, size_t bytes_len, const char *mimetype)
{
	BOOL ret;
	char *header = Str_temp();
	Str_printf(&header, "Content-Type: %s\r\n", mimetype ? mimetype : "application/octet-stream");
	ret = http_send_response(client,"HTTP/1.1 200 OK", header, bytes, bytes_len);
	Str_destroy(&header);
	return ret;
}

BOOL http_sendfile(HTTPLink *client, const char *path, const char *mimetype)
{
	BOOL ret;
	int len;
#ifdef UTILS_LIB
    char *bytes = 0;
    file_alloc(&bytes,path);
    len = achr_size(&bytes);
#else
	void *bytes = fileAlloc(path, &len);
#endif
	ret = http_sendbytes(client, bytes, len, mimetype);

#ifdef UTILS_LIB
    achr_destroy(&bytes);
#else
	SAFE_FREE(bytes);
#endif
	return ret;
}


BOOL http_sendbadreq(HTTPLink *client)
{
    return http_send_response(client,"HTTP/1.1 400 Bad Request","Connection: close",NULL,0);
}



void http_server_shutdown(HTTPServer *s)
{
    int i;
    for( i = 0; i < ap_size(&s->clients); ++i )
        http_shutdownlink(s->clients[i],FALSE);
}
