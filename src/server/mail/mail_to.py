import os
import smtplib
from dotenv import load_dotenv
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart


def send_mail(emailto: str, subject: str, text_content: str):
    load_dotenv()
    APP_NAME = os.getenv('APP_NAME')
    APP_PASSWORD = os.getenv('APP_PASSWORD')
    USER_MAIL = os.getenv('USER_MAIL')
    smtpobj = smtplib.SMTP('smtp.gmail.com', 587)
    smtpobj.ehlo()
    smtpobj.starttls()

    smtpobj.login(
        USER_MAIL,
        APP_PASSWORD,
    )

    msg = MIMEMultipart('alternative')
    msg['Subject'] = subject
    msg['From'] = USER_MAIL
    msg['To'] = emailto
    text = f'''AUTOMATED MESSAGE FROM: {APP_NAME}\n{text_content}'''
    mimepart = MIMEText(text, 'plain')

    msg.attach(mimepart)

    smtpobj.sendmail(USER_MAIL, emailto, msg.as_string())
    smtpobj.quit()
