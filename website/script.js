// Global variables
let map;
let marker;
let reconnectInterval = 3000;

// Initialize Leaflet Map
function initMap() {
    // Default position (0,0) with zoom level 2
    map = L.map('map').setView([0, 0], 2);

    // Add OpenStreetMap tiles
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '© OpenStreetMap contributors'
    }).addTo(map);

    // Initialize marker
    marker = L.marker([0, 0], {
        title: "Road-E location",
        opacity: 0 // Hide initially
    }).addTo(map);

    // Connect to WebSocket after map is ready
    // connectToGPSWebSocket(); // Removed
}

// UI Update Functions
function updateUIOnline() {
    const status = document.getElementById("status-val");
    if (status) {
        status.textContent = "Online";
        status.classList.remove("no-data");
    }
}

function updateUIOffline() {
    const status = document.getElementById("status-val");
    if (status) {
        status.textContent = "Offline";
        status.classList.add("no-data");
    }
}

function setStatusMessage(msg, isError = false) {
    const status = document.getElementById("status-val");
    if (status) {
        status.textContent = msg;
        if (isError) status.classList.add("no-data");
        else status.classList.remove("no-data");
    }
}

function showMapMessage(text) {
    const msg = document.getElementById("map-message");
    if (msg) {
        msg.style.display = "block";
        msg.textContent = text;
    }
}

function hideMapMessage() {
    const msg = document.getElementById("map-message");
    if (msg) msg.style.display = "none";
}

function updateGPS(lat, lng) {
    if (!map || !marker) return;
    const latNum = Number(lat);
    const lngNum = Number(lng);
    
    if (Number.isNaN(latNum) || Number.isNaN(lngNum)) return;
    
    const pos = [latNum, lngNum];
    
    marker.setLatLng(pos);
    marker.setOpacity(1); // Show marker
    map.setView(pos, 17); // Zoom in

    hideMapMessage();
    updateUIOnline();
}

function updateSensors(temp, hum) {
    if (temp !== undefined) {
        const el = document.getElementById("temp-val");
        if (el) el.textContent = temp;
    }
    if (hum !== undefined) {
        const el = document.getElementById("hum-val");
        if (el) el.textContent = hum;
    }
}

function updateStream(url) {
    const img = document.getElementById("stream");
    const ph = document.getElementById("stream-placeholder");
    
    if (url) {
        if (img) {
            img.src = url;
            img.style.display = "block";
        }
        if (ph) ph.style.display = "none";
    } else {
        if (img) img.style.display = "none";
        if (ph) ph.style.display = "block";
    }
}

// Database / Table Logic
let dbRecordCount = 0;

function updateSQLTable(data) {
    const tbody = document.getElementById("db-body");
    const noDataRow = tbody.querySelector(".no-data");

    if (noDataRow) {
        noDataRow.remove();
    }

    const row = document.createElement("tr");
    
    // ID
    dbRecordCount++;
    const idCell = document.createElement("td");
    idCell.textContent = dbRecordCount;
    row.appendChild(idCell);

    // Time
    const timeCell = document.createElement("td");
    timeCell.textContent = new Date().toLocaleTimeString();
    row.appendChild(timeCell);

    // Hazard Name
    const hazNameCell = document.createElement("td");
    hazNameCell.textContent = data.hazard_name || "None";
    row.appendChild(hazNameCell);

    // Hazard Info
    const hazInfoCell = document.createElement("td");
    hazInfoCell.textContent = data.hazard_info || "--";
    row.appendChild(hazInfoCell);

    // GPS Location
    const gpsCell = document.createElement("td");
    if (data.lat !== undefined && data.lng !== undefined) {
        gpsCell.textContent = `${data.lat.toFixed(6)}, ${data.lng.toFixed(6)}`;
    } else {
        gpsCell.textContent = "--";
    }
    row.appendChild(gpsCell);

    // Prepend to show newest first, or append? Table headers usually imply list.
    // Let's prepend to keep latest at top if it scrolls, or append if it's a log.
    // Usually log is appended, but "newest first" is often better for monitoring.
    // Let's use prepend (insertBefore first child).
    tbody.insertBefore(row, tbody.firstChild);

    // Limit to last 20 rows to prevent DOM flooding
    if (tbody.children.length > 20) {
        tbody.removeChild(tbody.lastChild);
    }
}

// HTTP Polling Logic
let currentEspIp = null;
let pollingInterval = null;
const POLL_RATE_MS = 500;

function connectToESP32() {
    const ipInput = document.getElementById('esp-ip');
    let url = ipInput ? ipInput.value.trim() : '';
    
    if (!url) {
        alert("Please enter a valid IP address or URL (e.g., http://192.168.4.1)");
        return;
    }

    // Ensure protocol is present
    if (!url.startsWith('http://') && !url.startsWith('https://')) {
        url = 'http://' + url;
    }

    // Determine if we are already polling this URL
    if (currentEspIp === url && pollingInterval) {
        return;
    }

    // Stop previous polling
    if (pollingInterval) clearInterval(pollingInterval);

    currentEspIp = url;
    setStatusMessage('Connecting (Polling)...');
    
    // Update input
    if (ipInput) ipInput.value = url;

    // Start polling
    startPolling();
}

function startPolling() {
    if (pollingInterval) clearInterval(pollingInterval);

    pollingInterval = setInterval(() => {
        if (!currentEspIp) return;

        fetch(`${currentEspIp}/status`)
            .then(response => {
                if (!response.ok) throw new Error("Network response was not ok");
                return response.json();
            })
            .then(data => {
                setStatusMessage('Online');
                if (data.lat !== undefined && data.lng !== undefined) updateGPS(data.lat, data.lng);
                if (data.temp !== undefined || data.hum !== undefined) updateSensors(data.temp, data.hum);
                // Stream is handled differently in HTTP usually (MJPEG), but for now we might just get a URL or skip it
                if (data.streamUrl !== undefined) updateStream(data.streamUrl);
                
                updateSQLTable(data);
            })
            .catch(err => {
                console.error("Polling error:", err);
                setStatusMessage('Offline', true);
            });
    }, POLL_RATE_MS);
}

// Control Logic
async function sendDrive(cmd) {
    console.log("Sending command: " + cmd);
    if (!currentEspIp) {
        console.warn("No ESP32 connected");
        return;
    }

    try {
        await fetch(`${currentEspIp}/drive?command=${cmd}`);
    } catch (err) {
        console.error("Error sending drive command", err);
    }
}

// Start everything when the page loads
document.addEventListener('DOMContentLoaded', () => {
    // Check if Leaflet is loaded
    if (typeof L !== 'undefined') {
        initMap();
    } else {
        console.error('Leaflet library not loaded');
        showMapMessage('Error: Leaflet library not loaded.');
    }
});

// Expose globals for debugging
window.updateGPS = updateGPS;
window.updateSensors = updateSensors;
window.updateStream = updateStream;
window.connectToESP32 = connectToESP32;
window.sendDrive = sendDrive;
window.initMap = initMap;