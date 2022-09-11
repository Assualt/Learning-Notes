#!/usr/bin/env bash
#-*-encoding: utf-8 -*-
from email.header import Header
from email.mime.text import MIMEText
from email.utils import parseaddr, formataddr
import smtplib

def _format_addr(s):
    # 格式化一个邮件地址，注意不能简单地传入name <addr@example.com>
    # 因为如果包含中文，需要通过Header对象进行编码。
    name, addr = parseaddr(s)
    return formataddr((Header(name, 'utf-8').encode(), addr))

# 配置 邮件发件人、收件人信息
from_addr = "**@**.com"
password = "******"
to_addr = ["**@**.com", "11@11.com" ]  # 输入收件人地址,接收的是字符串而不是list，如果有多个邮件地址，用,分隔即可。
smtp_server = "127.0.0.1"  # 设置邮箱服务器 mail.**.com

# 构造一封简单的文本邮件
msg = MIMEText('Hello, send by Python..', 'plain', 'utf-8')  # plain表示纯文本
msg['From'] = _format_addr("Python爱好者<%s>" % from_addr)
msg['To'] = _format_addr("管理员<%s>" % to_addr)
msg['Subject'] = Header('来着SMTP的问候', 'utf-8').encode()

# 进行邮箱登录、邮件发送
smtpObj = smtplib.SMTP(smtp_server, 8025)
smtpObj.set_debuglevel(1)
#smtpObj.starttls()
smtpObj.login(from_addr, password)
smtpObj.sendmail(from_addr, to_addr, msg.as_string())
smtpObj.quit()
