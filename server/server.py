from flask import Flask, make_response, request
from threading import Thread
import subprocess
import requests
import sqlite3
import zipfile
import hashlib
import shutil
import math
import time
import sys
import os

app = Flask(__name__)


class Const:
    TOKEN = "020b5f50092c10ede13278e08c59b9085410fd1ef1f4d705db43b8fefc7b5b7a"
    PRODUCTION_SERVER_URL = "https://procon32-akita.kosen.work"
    SUB_SERVER_URL = "https://procon32-sub.kosen.work"
    PRACTICE_SERVER_URL = "https://procon32-practice.kosen.work"
    LOCAL_SERVER_URL = "http://192.168.1.7:5000"

    SERVER_TYPE = sys.argv[1] if len(sys.argv) > 1 else "undefined"
    SERVER_URL = PRODUCTION_SERVER_URL if SERVER_TYPE == "production" \
        else SUB_SERVER_URL if SERVER_TYPE == "sub" \
        else PRACTICE_SERVER_URL if SERVER_TYPE == "practice" \
        else LOCAL_SERVER_URL if SERVER_TYPE == "local" \
        else SERVER_TYPE

    IMAGE_URL = SERVER_URL + "/problem.ppm"
    PING_URL = SERVER_URL + "/test"
    SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
    PROJECT_TOP_DIR = os.path.normpath(SERVER_DIR + "/../")
    WORKPLACE = SERVER_DIR + "/tmp"
    DATA_DIR = SERVER_DIR + "/data"
    ANSWER_DIR = DATA_DIR + "/answers"
    RESTORE_ANSWER_DIR = ANSWER_DIR + "/restoration"
    INITIAL_PROCS_DIR = ANSWER_DIR + "/initial_procs"
    BEST_PROCEDURE_PATH = ANSWER_DIR + "/best_procedure.txt"

    IMAGE_PATH = DATA_DIR + "/prob.ppm"
    SQL_PATH = DATA_DIR + '/answer.sqlite'
    SOLVERS_DIR = PROJECT_TOP_DIR + "/algo/bin"
    PORT_NUM = 3000
    DEFAULT_WAIT_TIME = 10


def castable(data_type, data):
    try:
        data_type(data)
    except (ValueError, TypeError):
        return False
    return True


class Util:
    @classmethod
    def convert_restore2procs(cls, answer):
        with open(Const.DATA_DIR + "/original_state.txt", mode='w') as f:
            f.write(answer)
        result = subprocess.run(f"{Const.SOLVERS_DIR}/build_procedure {Const.DATA_DIR} simply cout procedure", stdout=subprocess.PIPE, shell=True)
        return result.stdout.decode('utf-8').rstrip('\r\n')

    @classmethod
    def get_interval(cls, status_code, text, default_interval) -> int:
        if status_code != 400:
            return default_interval
        tokens = text.split(" ")
        if tokens[0] != "AccessTimeError" or len(tokens) != 2 or not castable(int, tokens[1]):
            return default_interval
        return int(tokens[1])

    @classmethod
    def get_answer_hash(cls, answer) -> str:
        return hashlib.sha256(answer.encode('utf-8')).hexdigest()

    @classmethod
    def calc_penalty_and_cost(cls, path) -> (int, int):
        result = subprocess.run(f"{Const.SOLVERS_DIR}/procedure_checker {Const.DATA_DIR} {path}", stdout=subprocess.PIPE, shell=True)
        if result.returncode != 0:
            return None, None
        else:
            return int(result.stdout.split()[0]), int(result.stdout.split()[1])


class AnswerManager:
    table_nameR = "restored_image"
    table_nameB = "initial_procedure"
    now_min_cost = None

    @classmethod
    def init(cls):
        if os.path.exists(Const.ANSWER_DIR):
            shutil.rmtree(Const.ANSWER_DIR)
        os.mkdir(Const.ANSWER_DIR)
        os.mkdir(Const.RESTORE_ANSWER_DIR)
        os.mkdir(Const.INITIAL_PROCS_DIR)

        conn = sqlite3.connect(Const.SQL_PATH)

        cur = conn.cursor()

        cur.execute(f'drop table if exists {cls.table_nameR}')
        cur.execute(f'drop table if exists {cls.table_nameB}')

        cur.execute(f'create table {cls.table_nameR}(r_id integer primary key, file_name text unique, sent integer, coord_mismatch_cnt integer, dir_mismatch_cnt integer)')
        cur.execute(f'create table {cls.table_nameB}(b_id integer primary key, file_name text unique, sent integer)')

        conn.commit()
        conn.close()

    @classmethod
    def restart(cls):
        if os.path.exists(Const.BEST_PROCEDURE_PATH):
            penalty, cost = Util.calc_penalty_and_cost(Const.BEST_PROCEDURE_PATH)
            cls.now_min_cost = cost

    @classmethod
    def save_restored_image(cls, answer, sent=False, coord_mismatch_cnt=None, dir_mismatch_cnt=None):
        file_name = Util.get_answer_hash(answer)

        with open(Const.RESTORE_ANSWER_DIR + "/" + file_name, mode='w') as f:
            f.write(answer)

        sent = int(sent)
        coord_mismatch_cnt = coord_mismatch_cnt if coord_mismatch_cnt is not None else -1
        dir_mismatch_cnt = dir_mismatch_cnt if dir_mismatch_cnt is not None else -1

        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        cur.execute(f'insert into {cls.table_nameR}(file_name, sent, coord_mismatch_cnt, dir_mismatch_cnt) values(?,?,?,?)', (file_name, sent, coord_mismatch_cnt, dir_mismatch_cnt))
        conn.commit()
        conn.close()

    @classmethod
    def save_initial_procedure(cls, procs):
        file_name = Util.get_answer_hash(procs)

        with open(Const.INITIAL_PROCS_DIR + "/" + file_name, mode='w') as f:
            f.write(procs)

        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        cur.execute(f'insert into {cls.table_nameB}(file_name, sent) values(?,?)', (file_name, 0))
        conn.commit()
        conn.close()

    @classmethod
    def save_complete_procedure(cls, procs) -> str:

        tmp_path = Const.WORKPLACE + "/tmp_procs.txt"
        with open(tmp_path, mode='w') as f:
            f.write(procs)
        new_penalty, new_cost = Util.calc_penalty_and_cost(tmp_path)
        os.remove(tmp_path)

        if not os.path.exists(Const.BEST_PROCEDURE_PATH):
            with open(Const.BEST_PROCEDURE_PATH, mode='w') as f:
                f.write(procs)
                cls.now_min_cost = new_cost
                return f"first save, cost: {new_cost}"
        else:
            old_cost = cls.now_min_cost
            if old_cost >= new_cost:
                cls.now_min_cost = new_cost
                print(f"update cost: {old_cost} -> {new_cost}")
                with open(Const.BEST_PROCEDURE_PATH, mode='w') as f:
                    f.write(procs)
                return f"upgraded, cost: {old_cost} -> {new_cost}"
            else:
                return f"not best, cost: {old_cost} <= {new_cost}"

    @classmethod
    def update_restored_image_state(cls, answer, coord_mismatch_cnt, dir_mismatch_cnt):
        file_name = Util.get_answer_hash(answer)
        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        cur.execute(f'update {cls.table_nameR} set sent = 1, coord_mismatch_cnt = ?, dir_mismatch_cnt = ? where file_name = ?', (coord_mismatch_cnt, dir_mismatch_cnt, file_name))
        conn.commit()
        conn.close()

    @classmethod
    def get_restored_image_info(cls, answer):
        file_name = Util.get_answer_hash(answer)
        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        result = cur.execute(f'select sent, coord_mismatch_cnt, dir_mismatch_cnt from {cls.table_nameR} where file_name = ?', (file_name,)).fetchone()
        conn.close()
        return result

    @classmethod
    def get_all_initial_procedure_file_name(cls):
        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        result = cur.execute(f'select file_name from {cls.table_nameB}').fetchall()
        conn.close()
        return list(map(lambda x: x[0], result))

    @classmethod
    def already_exists_restored_image(cls, answer) -> bool:
        file_name = Util.get_answer_hash(answer)
        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        result = cur.execute(f'select count(0) from {cls.table_nameR} where file_name = ? limit 1', (file_name,)).fetchone()
        conn.close()
        return not result[0] == 0

    @classmethod
    def already_exists_initial_procedure(cls, procs) -> bool:
        file_name = Util.get_answer_hash(procs)
        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        result = cur.execute(f'select count(0) from {cls.table_nameB} where file_name = ? limit 1', (file_name,)).fetchone()
        conn.close()
        return not result[0] == 0

    @classmethod
    def validate_restored_image(cls, answer) -> (bool, str):
        # todo: sent?????????answer???mismatch count???????????????????????????????????????
        exists_same_answer = cls.already_exists_restored_image(answer)
        if exists_same_answer:
            return False, "the answer is already exists."
        else:
            return True, ""

    @classmethod
    def validate_initial_procedure(cls, procs):
        exists_same_answer = cls.already_exists_initial_procedure(procs)
        if exists_same_answer:
            return False, "the answer is already exists."
        else:
            return True, ""

    @classmethod
    def validate_complete_procedure(cls, procs):
        tmp_path = Const.WORKPLACE + "/tmp_procs.txt"
        with open(tmp_path, mode='w') as f:
            f.write(procs)

        penalty, cost = Util.calc_penalty_and_cost(tmp_path)

        os.remove(tmp_path)

        if penalty != 0:
            return False, "this procedure is NOT completed"

        return True, ""

    @classmethod
    def get_sent_count(cls):
        sent_count = 0
        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        result = cur.execute(f'select count(0) from {cls.table_nameR} where sent = 1').fetchone()
        sent_count += result[0]
        result = cur.execute(f'select count(0) from {cls.table_nameB} where sent = 1').fetchone()
        sent_count += result[0]
        conn.close()
        return sent_count

    @classmethod
    def restoration_completed(cls):
        conn = sqlite3.connect(Const.SQL_PATH)
        cur = conn.cursor()
        result = cur.execute(f'select count(0) from {cls.table_nameR} where coord_mismatch_cnt = 0 and dir_mismatch_cnt = 0 limit 1').fetchone()
        conn.close()
        return not result[0] == 0


class Operator:
    now_min_cost = None
    submission_times = int(0)

    @classmethod
    def submit(cls, answer) -> (int, int):
        cls.submission_times += 1

        r = requests.post(Const.SERVER_URL, headers={"procon-token": Const.TOKEN}, data=answer)

        print(f"[submit] status_code, text: {r.status_code}, {r.text}")

        if r.status_code != 200:
            print("[send answer] failed (debug??????????????????????????????????????????????????????????????????????????????????????????)")
            return None, None

        tmp = r.text.strip().split()

        coord_mismatch_cnt = int(tmp[1])
        dir_mismatch_cnt = int(tmp[2])

        if coord_mismatch_cnt == 0 and dir_mismatch_cnt == 0:
            tmp_path = Const.WORKPLACE + "/tmp_procs.txt"
            with open(tmp_path, mode='w') as f:
                f.write(answer)
            penalty, cost = Util.calc_penalty_and_cost(tmp_path)
            os.remove(tmp_path)
            if cls.now_min_cost is None:
                cls.now_min_cost = cost
            else:
                cls.now_min_cost = min(cls.now_min_cost, cost)

        return coord_mismatch_cnt, dir_mismatch_cnt

    @classmethod
    def get_status_code_and_content_and_headers(cls) -> (int, bytes, map):
        r = requests.get(Const.IMAGE_URL, headers={"procon-token": Const.TOKEN})
        return r.status_code, r.content, r.headers

    @classmethod
    def get_end_time(cls) -> int:
        r = requests.get(Const.IMAGE_URL, headers={"procon-token": Const.TOKEN})
        return r.headers['Procon-End-At']

    @classmethod
    def ping(cls):
        r = requests.get(Const.PING_URL, headers={"procon-token": Const.TOKEN})
        return r.text, r.status_code


class GameManager:
    status = "waiting"  # waiting, restoring, building
    start_time = None  # ???????????????UNIX?????????, None???????????????????????????????????????????????????????????????????????????
    end_time = None
    last_submitted_procs_hash = None

    @classmethod
    def init(cls):
        if os.path.exists(Const.DATA_DIR):
            shutil.rmtree(Const.DATA_DIR)
        if os.path.exists(Const.WORKPLACE):
            shutil.rmtree(Const.WORKPLACE)
        os.mkdir(Const.DATA_DIR)
        os.mkdir(Const.WORKPLACE)
        os.mkdir(Const.ANSWER_DIR)
        AnswerManager.init()

    @classmethod
    def restart(cls):
        AnswerManager.restart()
        if AnswerManager.restoration_completed():
            cls.end_time = Operator.get_end_time()
            cls.status = "building"
            Operator.submission_times = AnswerManager.get_sent_count()
            status_code, content, headers = Operator.get_status_code_and_content_and_headers()
            cls.end_time = int(headers['Procon-End-At'])

    @classmethod
    def get_waiting_time(cls):
        if cls.status == "waiting":
            return Const.DEFAULT_WAIT_TIME if cls.start_time is None else max(0.3, math.ceil((cls.start_time - time.time()) * 10.) / 10.)
        else:
            return 0

    @classmethod
    def now_status_is(cls, *expected_statuses) -> (bool, str):
        if cls.status in expected_statuses:
            return True, ""

        if cls.status == "waiting":
            return False, f"StatusError waiting {cls.get_waiting_time()}"
        else:
            return False, f"StatusError {cls.status}"

    @classmethod
    def update_status_while_waiting(cls):
        if cls.status != "waiting":
            return

        while True:
            status_code, content, headers = Operator.get_status_code_and_content_and_headers()

            if status_code == 200:
                # ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                with open(Const.IMAGE_PATH, 'wb') as f:
                    f.write(content)
                subprocess.run(f"python3 {Const.PROJECT_TOP_DIR}/utility/image_divider.py {Const.IMAGE_PATH}", shell=True)
                cls.end_time = int(headers['Procon-End-At'])
                break

            text = content.decode('utf-8')
            interval = Util.get_interval(status_code, text, -1)
            if interval >= 0:
                cls.start_time = time.time() + interval
            else:
                interval = Const.DEFAULT_WAIT_TIME

            print(f"wait {interval} sec")
            time.sleep(interval)

        cls.status = "restoring"
        print("connection completed")

    @classmethod
    def verify_and_save_restoration_answer(cls, answer) -> (bool, str):
        is_valid, msg = AnswerManager.validate_restored_image(answer)
        if not is_valid:
            return False, msg

        if Operator.submission_times == 0:
            coord_mismatch_cnt, dir_mismatch_cnt = Operator.submit(Util.convert_restore2procs(answer))
            AnswerManager.save_restored_image(answer, True, coord_mismatch_cnt, dir_mismatch_cnt)
            if coord_mismatch_cnt is None or dir_mismatch_cnt is None:
                return False, "FormatError"

            if coord_mismatch_cnt == 0 and dir_mismatch_cnt == 0:
                cls.status = "building"
                return True, "correct"
            else:
                return False, f"incorrect {coord_mismatch_cnt} {dir_mismatch_cnt}"
        else:
            AnswerManager.save_restored_image(answer)
            return False, "pending"

    @classmethod
    def verify_and_save_initial_procedure(cls, procs) -> (bool, str):
        is_valid, msg = AnswerManager.validate_initial_procedure(procs)
        if not is_valid:
            return False, msg

        AnswerManager.save_initial_procedure(procs)

        return True, "saved"

    @classmethod
    def verify_and_save_complete_procedure(cls, procs):
        is_valid, msg = AnswerManager.validate_complete_procedure(procs)
        if not is_valid:
            return False, msg

        msg = AnswerManager.save_complete_procedure(procs)
        return True, msg

    @classmethod
    def submit_restoration_answer(cls, file_name) -> (bool, str):
        file_path = f"{Const.RESTORE_ANSWER_DIR}/{file_name}"
        if not os.path.isfile(file_path):
            return False, f"this file is not exists: {file_name}"

        with open(file_path, 'r') as f:
            answer = f.read()

        sent, coord_mismatch_cnt, dir_mismatch_cnt = AnswerManager.get_restored_image_info(answer)
        if sent:
            return False, f"incorrect {coord_mismatch_cnt} {dir_mismatch_cnt}"

        coord_mismatch_cnt, dir_mismatch_cnt = Operator.submit(Util.convert_restore2procs(answer))
        AnswerManager.update_restored_image_state(answer, coord_mismatch_cnt, dir_mismatch_cnt)
        if coord_mismatch_cnt is None or dir_mismatch_cnt is None:
            return False, "FormatError"

        if coord_mismatch_cnt == 0 and dir_mismatch_cnt == 0:
            cls.status = "building"
            return True, "correct"
        else:
            return False, f"incorrect {coord_mismatch_cnt} {dir_mismatch_cnt}"

    @classmethod
    def submit_best_procedure(cls) -> (bool, str):
        if not os.path.exists(Const.BEST_PROCEDURE_PATH):
            return False, f"procedure is not exists"

        with open(Const.BEST_PROCEDURE_PATH, mode='r') as f:
            procs = f.read()

        procs_hash = hashlib.sha256(procs.encode('utf-8')).hexdigest()
        if cls.last_submitted_procs_hash == procs_hash:
            return False, "this procedure is already submitted"

        cls.last_submitted_procs_hash = procs_hash

        print(procs)

        coord_mismatch_cnt, dir_mismatch_cnt = Operator.submit(procs)
        if coord_mismatch_cnt is None or dir_mismatch_cnt is None:
            return False, "FormatError"

        if coord_mismatch_cnt == 0 and dir_mismatch_cnt == 0:
            penalty, cost = Util.calc_penalty_and_cost(Const.BEST_PROCEDURE_PATH)
            return True, f"correct, cost: {cost}"
        else:
            return False, f"incorrect {coord_mismatch_cnt} {dir_mismatch_cnt}"

    @classmethod
    def get_initial_procs_zip(cls, excepts):
        if os.path.exists(Const.WORKPLACE + "/initial_procedures.zip"):
            os.remove(Const.WORKPLACE + "/initial_procedures.zip")

        file_names = set(AnswerManager.get_all_initial_procedure_file_name())
        for x in excepts:
            file_names.remove(x)

        with zipfile.ZipFile(Const.WORKPLACE + '/initial_procedures.zip', 'w', compression=zipfile.ZIP_DEFLATED) as zipf:
            for file_name in file_names:
                zipf.write(Const.INITIAL_PROCS_DIR + '/' + file_name, arcname=file_name)

        with open(Const.WORKPLACE + "/initial_procedures.zip", mode='rb') as f:
            result = f.read()
        return result


@app.route('/problem.ppm', methods=['GET'])
def get_image():
    is_valid_status_now, msg = GameManager.now_status_is('restoring', 'building')
    if not is_valid_status_now:
        return msg, 400

    response = make_response()
    with open(Const.IMAGE_PATH, "rb") as f:
        response.data = f.read()
    response.headers['Content-Type'] = 'image/x-portable-pixmap'
    return response


@app.route('/ping', methods=['GET'])
def ping():
    return Operator.ping()


@app.route('/end_time', methods=['GET'])
def get_end_time():
    is_valid_status_now, msg = GameManager.now_status_is('restoring', 'building')
    if not is_valid_status_now:
        return msg, 400

    return str(GameManager.end_time), 200


@app.route('/res_time', methods=['GET'])
def get_res_time():
    is_valid_status_now, msg = GameManager.now_status_is('restoring', 'building')
    if not is_valid_status_now:
        return msg, 400

    return str(GameManager.end_time - time.time()), 200


@app.route('/status', methods=['GET'])
def get_status():
    if GameManager.status == "waiting":
        return f"waiting {GameManager.get_waiting_time()}", 200
    else:
        return GameManager.status, 200


@app.route('/original_state', methods=['GET'])
def get_original_state():
    is_valid_status_now, msg = GameManager.now_status_is('building')
    if not is_valid_status_now:
        return msg, 400

    with open(Const.DATA_DIR + "/original_state.txt") as f:
        result = f.read()
    return result, 200


@app.route('/submission_times', methods=['GET'])
def get_submission_times():
    return str(Operator.submission_times), 200


@app.route('/answer/restoration', methods=['POST'])
def receive_restoration_answer():
    is_valid_status_now, msg = GameManager.now_status_is('restoring')
    if not is_valid_status_now:
        return msg, 400

    data = request.get_data().decode('utf-8')
    is_correct, msg = GameManager.verify_and_save_restoration_answer(data)
    return msg, (200 if is_correct else 400)


@app.route('/answer/initial_procedure', methods=['POST'])
def receive_initial_procedure():
    is_valid_status_now, msg = GameManager.now_status_is('building')
    if not is_valid_status_now:
        return msg, 400

    data = request.get_data().decode('utf-8')
    is_correct, msg = GameManager.verify_and_save_initial_procedure(data)
    return msg, (200 if is_correct else 400)


@app.route('/answer/complete_procedure', methods=['POST'])
def receive_complete_procedure():
    is_valid_status_now, msg = GameManager.now_status_is('building')
    if not is_valid_status_now:
        return msg, 400

    data = request.get_data().decode('utf-8')
    is_correct, msg = GameManager.verify_and_save_complete_procedure(data)
    return msg, (200 if is_correct else 400)


@app.route('/submit/restoration', methods=['GET'])
def submit_restoration_answer():
    is_valid_status_now, msg = GameManager.now_status_is('restoring')
    if not is_valid_status_now:
        return msg, 400

    file_name = request.args.get('hash')

    is_correct, msg = GameManager.submit_restoration_answer(file_name)
    return msg, (200 if is_correct else 400)


@app.route('/submit/procedure', methods=['GET'])
def submit_building_answer():
    is_valid_status_now, msg = GameManager.now_status_is('building')
    if not is_valid_status_now:
        return msg, 400

    is_correct, msg = GameManager.submit_best_procedure()
    return msg, (200 if is_correct else 400)


@app.route('/get_initial_procedures', methods=['POST'])
def get_initial_procedure():
    is_valid_status_now, msg = GameManager.now_status_is('building')
    if not is_valid_status_now:
        return msg, 400

    excepts = request.get_data().decode('utf-8').split()

    response = make_response()
    response.data = GameManager.get_initial_procs_zip(excepts)
    response.headers['Content-Type'] = 'application/zip'
    return response


@app.route('/get_match_info', methods=['GET'])
def get_match_info():
    is_valid_status_now, msg = GameManager.now_status_is('restoring', 'building')
    if not is_valid_status_now:
        return msg, 400
    sent_min_cost = Operator.now_min_cost
    saved_min_cost = AnswerManager.now_min_cost
    sent_min_cost = "inf" if sent_min_cost is None else sent_min_cost
    saved_min_cost = "inf" if saved_min_cost is None else saved_min_cost
    return f"res_time, min_cost(sent, saved), res_submission: {int(GameManager.end_time - time.time())}, ({sent_min_cost}, {saved_min_cost}), {10-Operator.submission_times}", 200


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("two arguments must be required.")
        sys.exit(-1)
    if sys.argv[2] not in ["init", "restart"]:
        print("2nd arguments must be 'init' or 'restart'")
        sys.exit(-1)

    print(f"server url: {Const.SERVER_URL}")

    if sys.argv[2] == "init":
        GameManager.init()
    else:
        GameManager.restart()

    if GameManager.status == "waiting":
        Thread(target=GameManager.update_status_while_waiting).start()
    app.run(host='0.0.0.0', port=Const.PORT_NUM, threaded=False)
