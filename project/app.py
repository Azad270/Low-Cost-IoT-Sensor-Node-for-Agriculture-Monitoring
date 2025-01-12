from flask import Flask, render_template, request, jsonify
from collections import deque
import time

app = Flask(__name__)

# Data storage for both boards
board_1_data = deque(maxlen=20)
board_2_data = deque(maxlen=20)

@app.route('/')
def index():
    return render_template('index.html')  # Serve the HTML file

@app.route('/board1_data', methods=['POST'])
def board1_data():
    data = request.get_json()
    data['timestamp'] = time.time()  # Add timestamp for each entry
    board_1_data.append(data)
    return 'Data received from Board 1', 200

@app.route('/board2_data', methods=['POST'])
def board2_data():
    data = request.get_json()
    data['timestamp'] = time.time()  # Add timestamp for each entry
    board_2_data.append(data)
    return 'Data received from Board 2', 200

@app.route('/get_data', methods=['GET'])
def get_data():
    return jsonify({
        'board_1': list(board_1_data),
        'board_2': list(board_2_data)
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
