from flask import Flask, Response
from http import HTTPStatus
from server.controllers.login_blueprint import login_blueprint


app = Flask(__name__)
app.register_blueprint(login_blueprint, url_prefix='/login')


@app.route('/', methods=['GET'])
def index():
    return Response('ok', status=HTTPStatus.OK)
