'''
A simple script to send emails.
  See: https://developers.google.com/gmail/api/quickstart/python
'''

import sys
import smtplib

TO = '' #Enter recipient's address
SUBJECT = 'TEST Email'
TEXT = 'Here is a message from python.'

# Gmail Sign In
gmail_sender = "" #Your email
gmail_passwd = "" #Your password

server = smtplib.SMTP('smtp.gmail.com', 587)
server.ehlo()
server.starttls()
server.login(gmail_sender, gmail_passwd)

BODY = '\r\n'.join(['To: %s' % TO,
                    'From: %s' % gmail_sender,
                    'Subject: %s' % SUBJECT,
                    '', TEXT])

try:
    server.sendmail(gmail_sender, [TO], BODY)
    print ('email sent')
except:
    print ('error sending mail')

server.quit()

