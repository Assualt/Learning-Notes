#pragma once
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

#define HTTP1_1 "HTTP/1.1"
#define HTTP1_0 "HTTP/1.0"

#define TempFile ".httpclient.download"

// 如果 接收数据大小大于 50M 如果没有指定 output,则提示用户不会展示
#define MAX_RECV_BYTES 50 * 1024 * 1024
