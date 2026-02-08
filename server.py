'''
    Road-E Project - Electronics

    author: Ariel Gal
    date: 08-02-2026

    Code For Connection Between ESP32 And MySQL Server

    Code Changes As Date 08-02:
    1. finish code from gemini
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

@app.route('/data', methods=['POST'])
def receive_data():
    try:
        content = request.json
        print(f"Received: {content}")
        
        temperature = content['temperature']
        humidity = content['humidity']

        # Connect to MySQL and insert data
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        
        query = "INSERT INTO sensor_readings (temperature, humidity) VALUES (%s, %s)"
        cursor.execute(query, (temperature, humidity))
        
        conn.commit()
        cursor.close()
        conn.close()

        return jsonify({"status": "success"}), 200

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    # 0.0.0.0 is required to make the server accessible to other devices (ESP32)
    app.run(host='0.0.0.0', port=5000)