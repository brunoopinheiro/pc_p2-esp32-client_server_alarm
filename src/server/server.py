from flask import Flask
from http import HTTPStatus


app = Flask(__name__)


@app.route('/login/<user>/<password>', methods=['GET'])
def login(user: str, password: str):
    response = "unauthorized", HTTPStatus.UNAUTHORIZED
    if user == "test" and password == "123":
        response = "ok", HTTPStatus.OK
    return response
