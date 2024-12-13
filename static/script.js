document.addEventListener("DOMContentLoaded", function () {
  const map = L.map("map").setView([0, 0], 13); // Default view
  const markers = {}; // Store markers for each user
  const markerTimeouts = {}; // Store timeout ids for marker cleanup
  var gamestate = false

  L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    maxZoom: 19,
    attribution: '© OpenStreetMap contributors',
  }).addTo(map);

  var redIcon = new L.Icon({
    iconUrl: 'https://raw.githubusercontent.com/pointhi/leaflet-color-markers/master/img/marker-icon-2x-red.png',
    shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
    iconSize: [25, 41],
    iconAnchor: [12, 41],
    popupAnchor: [1, -34],
    shadowSize: [41, 41]
  });

  // Function to update markers on the map
  function updateMarkers(locations) {
    for (const userId in locations) {
      if (userId == "hider") {
        continue;
      }

      const { lat, lng } = locations[userId];

      // If the marker for the user doesn't exist, create it
      if (!markers[userId]) {
        markers[userId] = L.marker([lat, lng])
          .addTo(map)
          .bindPopup(`User ${userId}`);
      } else {
        markers[userId].setLatLng([lat, lng]);
      }

      // Reset the timeout to remove the marker after a certain period
      if (markerTimeouts[userId]) {
        clearTimeout(markerTimeouts[userId]);
      }
      markerTimeouts[userId] = setTimeout(() => {
        map.removeLayer(markers[userId]);
        delete markers[userId];
        delete markerTimeouts[userId];
      }, 60000); // Marker is removed after 60 seconds of inactivity
    }

    const userId = "hider"

    if (userId in locations) {
      const {speed, lat, lng} = locations[userId]
      if (!markers[userId]) {
        markers[userId] = L.marker([lat, lng], {icon:redIcon})
          .addTo(map)
      } else {
        markers[userId].setLatLng([lat, lng], {icon:redIcon});
      }
    }
    else {
      delete markers[userId]
    }
  }

  // Function to send the user's location to the server
  function sendLocation(lat, lng) {
    const userId = localStorage.getItem("userId") || generateUserId();
    localStorage.setItem("userId", userId);

    fetch("/update_location", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ user_id: userId, lat, lng }),
    }).catch(console.error);
  }

  // Function to fetch all locations from the server
  function fetchLocations() {
    fetch("/get_locations")
      .then((res) => res.json())
      .then(updateMarkers)
      .catch(console.error);
  }

  async function fetchButtonState() {
    response = await fetch("/get_game_state")
    const buttonJson = await response.json()
    var new_gamestate = buttonJson["gamestate"]
    console.log(new_gamestate)
    if (new_gamestate != gamestate && gamestate==false) {alert('Game Starts NOW!!!')}
    if (new_gamestate != gamestate && gamestate==true) {alert('Game Over!!!')}
    gamestate = new_gamestate
  }

  // Generate a unique user ID if one doesn't exist
  function generateUserId() {
    return "user_" + Math.random().toString(36).substr(2, 9);
  }

  // Handle geolocation
  if (navigator.geolocation) {
    navigator.geolocation.watchPosition(
      (position) => {
        const { latitude, longitude } = position.coords;

        // Center the map the first time it loads
        if (!localStorage.getItem("mapInitialized")) {
          map.setView([latitude, longitude], 15);
          localStorage.setItem("mapInitialized", true);
        }

        // Send the user's location to the server
        sendLocation(latitude, longitude);
      },
      (err) => console.error("Error fetching location:", err.message)
    );
  } else {
    alert("Geolocation is not supported by your browser.");
  }

  // Periodically fetch all users' locations
  setInterval(fetchLocations, 5000); // Fetch every 5 seconds
  setInterval(fetchButtonState, 5000);



  // Set the map size and position once
  const savedCenter = localStorage.getItem("mapCenter");
  const savedZoom = localStorage.getItem("mapZoom");

  if (savedCenter && savedZoom) {
    const [lat, lng] = savedCenter.split(",").map(Number);
    map.setView([lat, lng], parseInt(savedZoom));
  }

  map.on("moveend", () => {
    const center = map.getCenter();
    const zoom = map.getZoom();
    localStorage.setItem("mapCenter", `${center.lat},${center.lng}`);
    localStorage.setItem("mapZoom", zoom);
  });
});
