from flask import Flask, request, jsonify
from flask_cors import CORS
import time

app = Flask(__name__)
CORS(app)  # Allow cross-origin requests for testing

# In-memory storage for user locations and last update time
user_locations = {}
user_last_update = {}  # Keeps track of the last update time for each user

TIMEOUT = 60  # 60 seconds timeout for marker removal

@app.route('/update_location', methods=['POST'])
def update_location():
    data = request.json
    user_id = data.get('user_id')
    lat = data.get('lat')
    lng = data.get('lng')

    if not user_id or lat is None or lng is None:
        return jsonify({"error": "Invalid data"}), 400

    # Update the user's location and timestamp
    user_locations[user_id] = {"lat": lat, "lng": lng}
    user_last_update[user_id] = time.time()  # Store the current time as last update time

    return jsonify({"message": "Location updated"}), 200

@app.route('/get_locations', methods=['GET'])
def get_locations():
    # Clean up users who have timed out (no update in the last TIMEOUT seconds)
    current_time = time.time()
    for user_id in list(user_locations.keys()):
        if current_time - user_last_update.get(user_id, current_time) > TIMEOUT:
            del user_locations[user_id]
            del user_last_update[user_id]

    return jsonify(user_locations), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001)
