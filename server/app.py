from flask import Flask, request, jsonify, render_template
from flask_cors import CORS
from time import time

game_state=False
global_queries_timer = 0
user_locations = {}  # In-memory storage for user locations
last_seen = {}
free_hint_interval = 24

app = Flask(__name__)
CORS(app)  # Enable cross-origin requests (optional)
#initial_hider_send_interval =


display_hider_threshold = 2 #TODO: test for sane speed

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
    return jsonify({"message": "Hider location updated"}), 200

# API to fetch all users' locations
@app.route('/get_locations', methods=['GET'])
def get_locations():
    global global_queries_timer
    local_query_timer = global_queries_timer + 1
    global_queries_timer = local_query_timer
    if global_queries_timer % free_hint_interval == 0: #check whether we send a location hint anyway
        pass
    elif "hider" in user_locations and float(user_locations["hider"]["speed"]) < display_hider_threshold:
        user_locations.pop("hider")
    return jsonify(user_locations), 200

@app.route('/set_game_state', methods=['POST'])
def set_game_state():
    global game_state
    global global_queries_timer
    global_queries_timer = 0
    game_state= not game_state
    return str(game_state)

@app.route('/get_game_state', methods=['GET'])
def get_game_state():
    return jsonify({'gamestate': game_state})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001)
