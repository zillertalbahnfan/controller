from flask import Flask, render_template
app = Flask(__name__)


COMPONENTS = {
    'controller':{
        'ip_address':'192.168.0.150'
    },
    'sections':{
        'bichl':{
            'controller':{
                'ip_address':'192.168.0.160'
            },
            'sensors':{
                'ip_address':'192.168.0.161'
            }
        }
    }
}

discovery_route = '/status'

@app.route('/')
def home():
    return render_template('home.html', title="homepage")


@app.route('/static/<path:path>')
def send_static(path):
    return send_from_directory('static', path)
