from flask import Flask, make_response, request
import subprocess
import hashlib
import math
import time
import sys
import os

app = Flask(__name__)


class Const:
    TOKEN = "020b5f50092c10ede13278e08c59b9085410fd1ef1f4d705db43b8fefc7b5b7a"

    SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
    PROJECT_TOP_DIR = os.path.normpath(SERVER_DIR + "/../")
    SOLVER_PATH = f"{PROJECT_TOP_DIR}/algo/bin"

    PROB_DIR = sys.argv[1]
    IMG_PATH = f"{PROB_DIR}/prob.ppm"

    time_to_boot_server = int(time.time()) + int(sys.argv[2])
    match_start_time = time_to_boot_server + int(sys.argv[3])
    match_end_time = match_start_time + int(sys.argv[4])

    PORT_NUM = 5000


def get_states() -> (int, str):
    if time.time() < Const.time_to_boot_server:
        return 502, ""
    elif time.time() < Const.match_start_time:
        return 400, f"AccessTimeError {math.ceil(Const.match_start_time - time.time())}"
    elif time.time() > Const.match_end_time:
        return 400, "AccessTimeError"
    else:
        return 200, ""


@app.route('/problem.ppm', methods=['GET'])
def get_image():
    if request.headers.get('procon-token') != Const.TOKEN:
        return "InvalidToken", 400

    status_code, msg = get_states()
    if status_code != 200:
        return msg, status_code

    response = make_response()
    with open(Const.IMG_PATH, "rb") as f:
        response.data = f.read()
    response.headers['Content-Type'] = 'image/x-portable-pixmap'
    response.headers['Procon-End-At'] = Const.match_end_time
    return response


@app.route('/test', methods=['GET'])
def ping():
    if request.headers.get('procon-token') != Const.TOKEN:
        return "InvalidToken", 400

    status_code, msg = get_states()
    if status_code != 200:
        return msg, status_code
    return "ok", status_code


@app.route('/', methods=['POST'])
def validate_answer():
    answer = request.get_data().decode('utf-8')
    file_name = hashlib.sha256(answer.encode('utf-8')).hexdigest()
    file_path = f"{Const.PROB_DIR}/{file_name}"

    with open(file_path, mode='w') as f:
        f.write(answer)
    result = subprocess.run(
        f"{Const.SOLVER_PATH}/procs_grader {Const.PROB_DIR} {file_path}",
        stdout=subprocess.PIPE,
        shell=True
    )
    os.remove(file_path)

    if result.returncode != 0:
        return "FormatError", 400
    return f"ACCEPTED {result.stdout.decode('utf-8')}", 200


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=Const.PORT_NUM, threaded=False)
