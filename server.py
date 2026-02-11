'''
    Road-E Project - Electronics

    author: Ariel Gal
    date: 11-02-2026

    Code For Connection Between ESP32 And MySQL Server

    Code Changes As Date 11-02:
    1. update code for sending real data from esp32 to server
    2. change code to be match as the columns in DB
    3. change code to be match to the database and table name in DB
'''

from flask import Flask, request, jsonify
import mysql.connector

app = Flask(__name__)

# --- CONFIGURATION ---
DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',           # MySQL Username
    'password': '12345678',   # MySQL Password
    'database': 'esp32_iot'   # Database Name
}

def save_to_db(data):
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        
        # UPDATED QUERY to match your screenshot exactly
        # Note: We removed 'created_at' because the database does it automatically now!
        query = """
            INSERT INTO sensor_readings
            (temperature, humidity, distance, light) 
            VALUES (%s, %s, %s, %s)
        """
        
        values = (
            data.get('temp'),  # Matches ESP32 JSON key "temp"
            data.get('hum'),   # Matches ESP32 JSON key "hum"
            data.get('dist'),  # Matches ESP32 JSON key "dist"
            data.get('ldr')    # Matches ESP32 JSON key "ldr"
        )
        
        cursor.execute(query, values)
        conn.commit()
        cursor.close()
        conn.close()
        return True
    except Exception as e:
        print(f"Error: {e}")
        return False

@app.route('/upload', methods=['POST'])
def handle_sensor_data():
    content = request.json
    print(f"Incoming Data: {content}") # Print to console for debugging
    
    if save_to_db(content):
        return jsonify({"status": "success"}), 201
    else:
        return jsonify({"status": "failed"}), 500

if __name__ == '__main__':
    # '0.0.0.0' allows external access (essential for ESP32 connection)
    app.run(host='0.0.0.0', port=5000)