from flask import Flask
import subprocess
import time
import mysql.connector


app = Flask(__name__)


@app.route('/')
def index():
    return 'Hello World'
    
@app.route('/test')
def test():
    BDDliste = []
    BDD = mysql.connector.connect(host="localhost",user="root",password="Night52", database="hardware")    
    curseur = BDD.cursor()
    curseur.execute("SELECT * FROM access")
    for ligne in curseur.fetchall():
        BDDliste.append(ligne)    
    BDD.close()
    return BDDliste


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')

