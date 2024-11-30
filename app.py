from flask import Flask, request, jsonify, render_template
from flask_cors import CORS
from time import time

game_state=False

app = Flask(__name__)
CORS(app)  # Enable cross-origin requests (optional)
user_locations = {}  # In-memory storage for user locations
last_seen = {}

display_hider_threshold = 5 #TODO: test for sane speed

# Serve the main HTML page
@app.route('/')
def index():
    return render_template('index.html')

# API to update a user's location
@app.route('/update_location', methods=['POST'])
def update_location():
    data = request.json
    user_id = data.get('user_id')
    lat = data.get('lat')
    lng = data.get('lng')

    if not user_id or lat is None or lng is None:
        return jsonify({"error": "Invalid data"}), 400

    user_locations[user_id] = {"lat": lat, "lng": lng}
    last_seen[user_id] = int(time())
    to_delete = []
    for u in user_locations:
        if u == "hider":
            continue
        tim = int(time())
        print(tim)
        print(last_seen[u])
        if tim - last_seen[u] > 60:
            to_delete.append(u)
    for u in to_delete:
        del last_seen[u]
        del user_locations[u]
            
    return jsonify({"message": "Location updated"}), 200

# API to update hider's location
@app.route('/hider', methods=['POST'])
def hider():
    user_locations["hider"] = request.json

    print(user_locations["hider"])

# API to fetch all users' locations
@app.route('/get_locations', methods=['GET'])
def get_locations():
    #user_locations["hider"] = {"speed": 10, "lat": 33.6472, "lng": -117.8411}
    if "hider" in user_locations and user_locations["hider"]["speed"] < display_hider_threshold:
        user_locations.pop(hider)
    if game_state:
        return jsonify(user_locations), 200
    else:
        return jsonify({}), 200

@app.route('/set_game_state', methods=['POST'])
def set_game_state():
    global game_state
    if request.form.get("state") == "True":
        print("Setting game_state to True")
        game_state=True
    elif request.form.get("state") == "False":
        print("Setting game_state to False")
        game_state=False
    return str(game_state)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001)
