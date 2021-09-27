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
    ANSWER_URL = "https://procon32-practice.kosen.work"
    IMAGE_URL = "https://procon32-practice.kosen.work/problem.ppm"
    SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
    PROJECT_TOP_DIR = os.path.normpath(SERVER_DIR + "/../../")
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
    TOKEN = "020b5f50092c10ede13278e08c59b9085410fd1ef1f4d705db43b8fefc7b5b7a"
    DEFAULT_WAIT_TIME = 10


class Util:
    @classmethod
    def convert_restore2procs(cls, answer):
        with open(Const.DATA_DIR + "/original_state.txt", mode='w') as f:
            f.write(answer)
        result = subprocess.run(f"{Const.SOLVERS_DIR}/build_procedure {Const.DATA_DIR} simply cout procedure", stdout=subprocess.PIPE, shell=True)
        return result.stdout.decode('utf-8').rstrip('\r\n')

    @classmethod
    def get_interval(cls, status_code, text, interval) -> int:
        if status_code != 400:
            return interval
        tokens = text.split(" ")
        if tokens[0] != "AccessTimeError":
            return interval
        if len(tokens) != 2:
            return interval
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
            f.write (procs)
        new_penalty, new_cost = Util.calc_penalty_and_cost(tmp_path)
        os.remove(tmp_path)

        if not os.path.exists(Const.BEST_PROCEDURE_PATH):
            with open(Const.BEST_PROCEDURE_PATH, mode='w') as f:
                f.write(procs)
                return f"first save, cost: {new_cost}"
        else:
            old_penalty, old_cost = Util.calc_penalty_and_cost(Const.BEST_PROCEDURE_PATH)

            if old_cost >= new_cost:
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
        # todo: sent済みのanswerのmismatch countから正しいかをチェックする
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

    @classmethod
    def submit(cls, answer) -> (int, int):
        r = requests.post(Const.ANSWER_URL, headers={"procon-token": Const.TOKEN}, data=answer)

        if r.status_code != 200:
            print("[send answer] failed (debug時以外は起こり得ないはずなのでこの文章が見えたら確認すること)")
            return None, None

        print(f"[submit] status_code, text: {r.status_code}, {r.text}")

        tmp = r.text.strip().split()

        coord_mismatch_cnt = int(tmp[1])
        dir_mismatch_cnt = int(tmp[2])

        return coord_mismatch_cnt, dir_mismatch_cnt

    @classmethod
    def get_status_code_and_content(cls) -> (int, bytes):
        r = requests.get(Const.IMAGE_URL, headers={"procon-token": Const.TOKEN})
        return r.status_code, r.content


class GameManager:
    status = "waiting"  # waiting, restoring, building
    start_time = None  # 開始時間をUNIX時間で, Noneならまだわからないかサーバー起動時に開始されていた
    answered_cnt = int(0)

    @classmethod
    def init(cls):
        if os.path.exists(Const.DATA_DIR):
            shutil.rmtree(Const.DATA_DIR)
        if os.path.exists(Const.WORKPLACE):
            shutil.rmtree(Const.WORKPLACE)
        os.mkdir(Const.DATA_DIR)
        os.mkdir(Const.WORKPLACE)
        os.mkdir(Const.ANSWER_DIR)

    @classmethod
    def restart(cls):
        if AnswerManager.restoration_completed():
            cls.status = "building"
            cls.answered_cnt = AnswerManager.get_sent_count()

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
            status_code, content = Operator.get_status_code_and_content()

            if status_code == 200:
                # 本来はここで処理すべきではないと思うけど何度もサーバーへ取りに行くのは申し訳なさがあるのでここで
                with open(Const.IMAGE_PATH, 'wb') as f:
                    f.write(content)
                subprocess.run(f"python3 {Const.PROJECT_TOP_DIR}/utility/image_divider.py {Const.IMAGE_PATH}", shell=True)
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

        if cls.answered_cnt == 0:
            cls.answered_cnt = 1
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

        cls.answered_cnt = cls.answered_cnt+1
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
        penalty, cost = Util.calc_penalty_and_cost(Const.BEST_PROCEDURE_PATH)
        with open(Const.BEST_PROCEDURE_PATH, mode='r') as f:
            procs = f.read()

        coord_mismatch_cnt, dir_mismatch_cnt = Operator.submit(procs)
        cls.answered_cnt += 1
        if coord_mismatch_cnt is None or dir_mismatch_cnt is None:
            return False, "FormatError"

        if coord_mismatch_cnt == 0 and dir_mismatch_cnt == 0:
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


@app.route('/answered_times', methods=['GET'])
def get_answered_times():
    return str(GameManager.answered_cnt), 200


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


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "restart":
        GameManager.restart()
    else:
        GameManager.init()
        AnswerManager.init()
    if GameManager.status == "waiting":
        Thread(target=GameManager.update_status_while_waiting).start()
    app.run(host='0.0.0.0', port=3000, threaded=False)
