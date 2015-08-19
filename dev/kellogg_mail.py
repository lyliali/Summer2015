# smtplib module send mail
# hint password is our advisor's first name no caps
import sys
import smtplib

def sxor(s1,s2):    
    return ''.join(chr(ord(s)^ord(c)) for s,c in zip(s1,s2*100))

TO = 'thedevingardella@gmail.com'
SUBJECT = 'TEST Email'
TEXT = 'Here is a message from python.'

# Gmail Sign In
gmail_sender = 'dpg3@williams.edu'
gmail_passwd = '\x1f\x03\x04\x04\x1d\x0e\x0f\x1f\x1c\x03\x14\r\x07\x15\x00\x08'
app_pass = sys.argv[1]

gmail_passwd = sxor(gmail_passwd, app_pass)

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

