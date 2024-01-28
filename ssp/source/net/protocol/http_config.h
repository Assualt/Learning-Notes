//
// Created by 侯鑫 on 2024/1/14.
//

#ifndef SSP_TOOLS_HTTP_CONFIG_H
#define SSP_TOOLS_HTTP_CONFIG_H

#define CTRL "\r\n"
#define ContentType "Content-Type"
#define ContentEncoding "Content-Encoding"
#define Accept "Accept"
#define AcceptEncoding "Accept-Encoding"
#define Authorization "Authorization"
#define AcceptLanguage "Accept-Language"
#define UserAgent "User-Agent"

#define ContentLength "Content-Length"
#define TransferEncoding "Transfer-Encoding"
#define AcceptRanges "Accept-Ranges"
#define Location "Location"
#define Cookie "Cookie"
#define Referer "Referer"

#define NOTFOUND "/404"
#define PATTERN_METHOD_NOT_ALLOWED "/405"
#define Date "Date"
#define LastModified "Last-Modified"
#define Connection "Connection"
#define SERVER "Server"
#define SERVERVal "HttpServer/0.1 Linux/GNU gcc/c++"
#define WwwAuthenticate "WWW-Authenticate"

// for websocket
#define Upgrade "Upgrade"
#define SecWebSocketVersion "Sec-WebSocket-Version"
#define SecWebSocketKey "Sec-WebSocket-Key"
#define Origin "Origin"
#define SecWebSocketAccept "Sec-WebSocket-Accept"

// userDefined
#define FilePattern "/#*#/"
#define DefaultPattern "/#@#/"

#define USER_AGENT_DEFAULT \
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.183 Safari/537.36"

#endif //SSP_TOOLS_HTTP_CONFIG_H
