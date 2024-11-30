from flask import Flask, request, jsonify, render_template
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # Enable cross-origin requests (optional)
user_locations = {}  # In-memory storage for user locations

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

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001)
