import subprocess
from flask import Flask, jsonify, render_template
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

@app.route('/')
def home():
    return render_template('website chatgpt.html')

@app.route('/hand-gesture')
def hand_gesture():
    try:
        # Path to your Python script
        script_path = r'D:\Semester 7\SIH\handges.py'  # Update path as needed
        result = subprocess.run(['python', script_path], capture_output=True, text=True)
        output = result.stdout.strip()
        return jsonify(output=output)
    except Exception as e:
        return jsonify(error=str(e))

@app.route('/open-exe')
def open_exe():
    try:
        # Specify the path to your .exe file
        subprocess.Popen([r'C:\Users\Home\Desktop\Unity\Hand tracking hardware\exe\Hand tracking hardware.exe'])
        return "Executable file opened successfully."
    except Exception as e:
        return f"An error occurred: {str(e)}"

if __name__ == '__main__':
    app.run(debug=True, port=5001)
