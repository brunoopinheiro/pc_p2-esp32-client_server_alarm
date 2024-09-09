import os
from dotenv import load_dotenv
from datetime import datetime
from flask import Blueprint, request
from http import HTTPStatus
from server.models.user import User
from dataclasses import asdict
from server.database.json_db import JSONDatabase
from server.mail.mail_to import send_mail


TABLE_NAME = 'users'
LOGIN_ATT_TABLE = 'failedLogin'

login_blueprint = Blueprint('login', __name__)


def __send_mail_notification(subject: str, content: str):
    load_dotenv()
    MAIL_TO = os.getenv('MAIL_TO')
    send_mail(
        emailto=MAIL_TO,
        subject=subject,
        text_content=content,
    )


def __update_attempts(id: str, username: str, success=True):
    attempts = 0
    db = JSONDatabase()
    if not success:
        user = db.select_by(LOGIN_ATT_TABLE, 'username', username)[0]
        attempts = int(user['attempts']) + 1
        if attempts >= 2:
            now = datetime.now()
            dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
            content = f"""WARNING: Maximum password retries reached
            USERNAME: {username}
            TIME: {dt_string}"""
            subject = "[ESP32-ALARM] - Password Failed Attempt"
            __send_mail_notification(subject, content)
    db.update(LOGIN_ATT_TABLE, id, attempts=attempts, username=username)


@login_blueprint.route('/', methods=["POST"])
def login():
    auth_obj = User(**request.json)
    db = JSONDatabase()
    user = db.select_by(TABLE_NAME, 'username', auth_obj.username)
    content = "unauthorized"
    statuscode = HTTPStatus.UNAUTHORIZED
    success = False
    if len(user) > 0:
        username = user[0]['username']
        userid = user[0]['id']
        if user[0]['password'] == auth_obj.password:
            success = True
            content = username
            statuscode = HTTPStatus.OK

        __update_attempts(
            id=userid,
            username=username,
            success=success,
        )
    return content, statuscode


@login_blueprint.route('/new', methods=["POST"])
def new_user():
    user = User(**request.json)
    db = JSONDatabase()
    userid = db.insert(TABLE_NAME, **asdict(user))
    db.insert_known_id(
        LOGIN_ATT_TABLE,
        userid,
        attempts=0,
        username=user.username,
    )
    return "ok", HTTPStatus.CREATED
