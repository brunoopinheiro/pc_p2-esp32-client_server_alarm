import smtplib
import json
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart


def send_notification(msgTo: str, ):
    # Conexão com o servidor SMTP
    smtpObj = smtplib.SMTP('smtp.outlook.com', 587)
    smtpObj.ehlo()
    smtpObj.starttls()

    # Autenticação do email no servidor SMTP
    msgFrom, fromPass = read_mail_info()
    smtpObj.login(msgFrom, fromPass)

    # Conteúdo do email
    msg = MIMEMultipart("alternative")
    msg["Subject"] = "Mais um teste"
    msg["From"] = msgFrom
    msg["To"] = msgTo
    text = """Finalmente enviando o corpo do email"""
    part1 = MIMEText(text, "plain")

    # Anexar corpo a mensagem
    msg.attach(part1)

    smtpObj.sendmail(msgFrom, msgTo, msg.as_string())
    smtpObj.quit()
    print("Email enviado com sucesso!")


def read_mail_info():
    with open('config.json', 'r', encoding='utf-8') as file:
        config_data = json.load(file)

    return config_data['email'], config_data['password']


msgTo = "tesouraescolarsemponta@gmail.com"

send_notification(msgTo=msgTo)
