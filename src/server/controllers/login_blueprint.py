from flask import Blueprint, request
from http import HTTPStatus
from server.models.user import User
from dataclasses import asdict
from server.database.json_db import JSONDatabase


TABLE_NAME = 'users'

login_blueprint = Blueprint('login', __name__)


@login_blueprint.route('/', methods=["POST"])
def login():
    auth_obj = User(**request.json)
    db = JSONDatabase()
    users = db.select_by(TABLE_NAME, 'username', auth_obj.username)
    for _, user in users.items():
        if user['password'] == auth_obj.password:
            username = user['username']
            return username, HTTPStatus.OK
    return "unauthorized", HTTPStatus.UNAUTHORIZED


@login_blueprint.route('/new', methods=["POST"])
def new_user():
    user = User(**request.json)
    db = JSONDatabase()
    db.insert(TABLE_NAME, **asdict(user))
    return "ok", HTTPStatus.CREATED
