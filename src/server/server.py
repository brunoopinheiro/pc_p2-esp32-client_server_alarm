from flask import Flask
from server.controllers.login_blueprint import login_blueprint


app = Flask(__name__)
app.register_blueprint(login_blueprint, url_prefix='/login')
