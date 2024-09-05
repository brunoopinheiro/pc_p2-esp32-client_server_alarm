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
    db.insert(TABLE_NAME, **asdict(auth_obj))
    return "ok", HTTPStatus.OK
