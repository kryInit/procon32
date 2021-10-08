# 1: initial, 2: complete, 3:submit,

# initial
# 1: new, 2: continue, 3: show-procs, 4: show-log
# a: auto, r: redo, c: change mode, enter: reload
#  >>>
# depth, loose_time_limit, strict_time_limit, first_selected_pos

# complete
# 1: new, 2: continue, 3: show-procs, 4: show-log
# a: auto, r: redo, c: change mode, enter: reload
#  >>>
# search_type, promptly, parallel_deg


# ctrl+cで一つ前の改装の選択肢に戻るとか
# 現在の状態(回答送信数, 前回回答時のコスト, 現在の最小コスト,

import subprocess
import requests
import zipfile
import hashlib
import inspect
import shutil
import random
import math
import time
import sys
import os


SERVER_URL = "http://192.168.1.7:3000" if len(sys.argv) > 1 and sys.argv[1] == "local" else sys.argv[1]
SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_TOP_DIR = os.path.normpath(SERVER_DIR + "/../")
DATA_DIR = PROJECT_TOP_DIR + "/.data"
SOLVERS_DIR = PROJECT_TOP_DIR + "/algo/bin"
DEFAULT_WAIT_TIME = 1


def print_match_info():
    result = requests.get(f"{SERVER_URL}/get_match_info")
    print(result.text)


def castable(data_type, data):
    try:
        data_type(data)
    except (ValueError, TypeError):
        return False
    return True


def all_castable(data_type, data):
    try:
        map(data_type, data)
    except (ValueError, TypeError):
        return False
    return True


def tautology(x):
    return True


def calc_penalty_and_cost(procs_path) -> (int, int):
    result = subprocess.run(f"{SOLVERS_DIR}/procedure_checker {prob_dir} {procs_path}", stdout=subprocess.PIPE, shell=True)
    if result.returncode != 0:
        return None, None
    else:
        return int(result.stdout.split()[0]), int(result.stdout.split()[1])


class InitialProcsManager:
    new_id = 0
    file_name_set = set()
    initial_procs = []

    @classmethod
    def register(cls, file_name):
        ini_procs_path = prob_dir + "/ini_procs"
        penalty, cost = calc_penalty_and_cost(ini_procs_path + "/" + file_name)
        cls.initial_procs.append((cls.new_id, file_name, penalty, cost))
        cls.file_name_set.add(file_name)
        cls.new_id += 1

    @classmethod
    def update_initial_procs(cls):
        ini_procs_path = prob_dir + "/ini_procs"
        files = os.listdir(ini_procs_path)
        for file_name in files:
            if file_name not in cls.file_name_set:
                cls.register(file_name)
        excepts = '\n'.join(files)
        result = requests.post(SERVER_URL + "/get_initial_procedures", data=excepts)

        zip_path = ini_procs_path + "/tmp.zip"
        with open(zip_path, 'wb') as f:
            f.write(result.content)
        with zipfile.ZipFile(zip_path) as zipf:
            zipf.extractall(ini_procs_path)
            new_files = zipf.namelist()
        for file_name in new_files:
            cls.register(file_name)
        os.remove(zip_path)

    @classmethod
    def init(cls):
        ini_procs_path = prob_dir + "/ini_procs"
        if os.path.exists(ini_procs_path):
            shutil.rmtree(ini_procs_path)
        os.mkdir(ini_procs_path)
        cls.update_initial_procs()

    @classmethod
    def show(cls, limit=10, sort_by_penalty=False, search_log=None, filtering=True):
        if search_log is None:
            search_log = dict()
        tmp = cls.initial_procs.copy()
        if sort_by_penalty:
            tmp.sort(key=lambda x: (x[2], x[3]))
        else:
            tmp.sort(key=lambda x: (x[3], x[2]))

        print("{:>3}, {:>8}, {:>8}, {:>8}".format("id", "hash(5)", "penalty", "cost"))
        if filtering:
            print_count = 0
            min_val = 1e20
            for i in range(len(tmp)):
                procs = tmp[i]
                val = procs[3] if sort_by_penalty else procs[2]
                if min_val > val:
                    min_val = val
                    print_count += 1
                    log = ' '.join(search_log[procs[1]]) if procs[1] in search_log else ""
                    print("{:>3}, {:>8}, {:>8}, {:>8},".format(procs[0], procs[1][:5], procs[2], procs[3]), log)

                    if print_count == limit:
                        break
        else:
            for i in range(min(len(tmp), limit)):
                procs = tmp[i]
                log = ' '.join(search_log[procs[1]]) if procs[1] in search_log else ""
                print("{:>3}, {:>8}, {:>8}, {:>8},".format(procs[0], procs[1][:5], procs[2], procs[3]), log)

    @classmethod
    def get_file_name_by_id(cls, proc_id) -> (bool, str):
        if len(cls.initial_procs) > proc_id and cls.initial_procs[proc_id][0] == proc_id:
            return True, cls.initial_procs[proc_id][1]
        else:
            for proc in cls.initial_procs:
                if proc[0] == proc_id:
                    return True, proc[1]
        return False, f"id is not found: {proc_id}"

    @classmethod
    def input_option_and_show(cls, search_log=None):
        if search_log is None:
            search_log = dict()
        opt = input("line limit, sort_by_penalty, filter\n >>> ").split(',')
        opt.extend(['', ''])
        line_limit = int(opt[0]) if castable(int, opt[0]) else 10
        sort_by_penalty = int(opt[1]) if castable(int, opt[1]) else 0
        sort_by_penalty = sort_by_penalty == 1
        filtering = int(opt[2]) if castable(int, opt[2]) else 1
        filtering = filtering == 1

        InitialProcsManager.show(line_limit, sort_by_penalty, search_log, filtering)

    @classmethod
    def input_id_and_get_file_path(cls) -> (bool, str):
        proc_id = input("id\n >>> ")
        if not castable(int, proc_id):
            return False, f"this id is not integer: {proc_id}"
        proc_id = int(proc_id)
        succeeded, s = InitialProcsManager.get_file_name_by_id(proc_id)
        if not succeeded:
            return False, s
        return True, f"{prob_dir}/ini_procs/{s}"


def get_status():
    print(f"[{inspect.currentframe().f_code.co_name}] start")
    result = requests.get(SERVER_URL + "/status")
    print(f"[{inspect.currentframe().f_code.co_name}] status_code, text: {result.status_code}, {result.text}")
    return result.text.split()[0], result.text.split()[1:]


def get_original_state():
    result = requests.get(SERVER_URL + "/original_state")
    with open(prob_dir + "/original_state.txt", mode='w') as f:
        f.write(result.text)


def wait_until_match_start() -> str:
    while True:
        status, opt = get_status()
        if status != "waiting":
            return status
        print(f"[{inspect.currentframe().f_code.co_name}] sleep for {opt[0]} sec")
        time.sleep(float(opt[0]))


def get_prob_and_processing_for_solver() -> str:
    print(f"[{inspect.currentframe().f_code.co_name}] start")
    result = requests.get(SERVER_URL + "/problem.ppm")
    dir_name = hashlib.sha256(result.content).hexdigest()
    print(f"[{inspect.currentframe().f_code.co_name}] status_code, hash: {result.status_code}, {dir_name}")
    prob_dir = f"{DATA_DIR}/{dir_name}"
    if not os.path.exists(prob_dir):
        os.mkdir(prob_dir)
    img_path = f"{prob_dir}/prob.ppm"
    open(img_path, 'wb').write(result.content)
    subprocess.run(f"python3 {PROJECT_TOP_DIR}/utility/image_divider.py {img_path}", shell=True)
    return prob_dir


def wait_until_procedure_building_start():
    while True:
        status, opt = get_status()
        if status == "building":
            return

        print(f"[{inspect.currentframe().f_code.co_name}] sleep for {DEFAULT_WAIT_TIME} sec")
        time.sleep(DEFAULT_WAIT_TIME)


def restore_image():
    subprocess.run(f"{PROJECT_TOP_DIR}/algo/bin/image_restorer {prob_dir}", shell=True)
    answer = open(prob_dir + "/original_state.txt", mode='rb').read()
    result = requests.post(SERVER_URL + "/answer/restoration", data=answer)
    print(f"[{inspect.currentframe().f_code.co_name}] status_code, text", result.status_code, result.text)
    if result.status_code != 200:
        print("", file=sys.stderr)
        print(" ===== ! CAUTION ! ===== ", file=sys.stderr)
        print(" 復元が落ちたので対処すること ", file=sys.stderr)
        print(" ======================= ", file=sys.stderr)
        sys.exit(-1)


def get_div_num() -> (int, int):
    with open(prob_dir + "/prob.txt", mode='r') as f:
        settings = f.read().split()
    return int(settings[0]), int(settings[1])


def get_settings_str() -> (int, int):
    with open(prob_dir + "/prob.txt", mode='r') as f:
        settings = f.read().split()
    return f"div_num, selectable_times, select_cost, swap_cost: ({settings[0]}, {settings[1]}), {settings[2]}, {settings[3]}, {settings[4]}"


class InitialProcsBuilder:
    mode_types = ['1', '2', '3', 'a', 'r', 'c', '']

    tmp_procs_file_path = ""
    labels = ["depth", "loose_time_limit", "strict_time_limit", "first_select_pos"]
    casters = [int, int, int, lambda x: x if x == "random" else tuple(map(int, tuple(x.split())))]
    checkers = [lambda x: x >= 4, lambda x: x > 0, lambda x: x > 0, lambda x: x == "random" or (len(x) == 2 and x[0] < dnx and x[1] < dny)]
    default_values = ["14", "5000", "120000", "random"]
    searched = set()

    @classmethod
    def init(cls):
        cls.tmp_procs_file_path = prob_dir + "/tmp_procs.txt"

    @classmethod
    def get_params_hash(cls, source, params):
        return ' '.join(map(str, [source, params[0], params[3][0], params[3][1]]))

    @classmethod
    def extract_params(cls, source, s):
        s = s.split(',')
        s.extend(cls.default_values[len(s):])

        s = [cls.default_values[i] if s[i] == '' else s[i] for i in range(len(cls.labels))]

        for i in range(len(cls.labels)):
            if not castable(cls.casters[i], s[i]):
                return False, f"invalid {cls.labels[i]}: {s[i]}"
            else:
                s[i] = cls.casters[i](s[i])

            if not cls.checkers[i](s[i]):
                return False, f"invalid {cls.labels[i]}: {s[i]}"

        if s[3] == "random":
            candidate = list()
            for y in range(dny):
                for x in range(dnx):
                    s[3] = (x, y)
                    params_hash = cls.get_params_hash(source, s)
                    if params_hash not in cls.searched:
                        candidate.append((x, y))

            if not candidate:
                return False, f"explored all this depth: {s[0]}"
            else:
                s[3] = candidate[random.randint(0, len(candidate)-1)]

        params_hash = cls.get_params_hash(source, s)
        if params_hash in cls.searched:
            return False, f"this params is already explored: {params_hash}"

        return True, s

    @classmethod
    def exec_solver_and_send_server(cls, source, params) -> bool:
        depth, loose_tl, strict_tl, fsp = params
        cmd = f"{SOLVERS_DIR}/build_procedure {prob_dir} initial {cls.tmp_procs_file_path} procedure {source} {fsp[0]} {fsp[1]} {loose_tl} {strict_tl} {depth}"
        try:
            result = subprocess.run(cmd, shell=True)
        except KeyboardInterrupt:
            print("keyboard interrupt")
            return False
        if result.returncode != 0:
            print("solver failed")
            return False
        with open(cls.tmp_procs_file_path, mode='r') as f:
            procs = f.read()
        penalty, cost = calc_penalty_and_cost(cls.tmp_procs_file_path)
        print(f"penalty, cost: {penalty}, {cost}")
        if penalty == 0:
            result = requests.post(SERVER_URL + "/answer/complete_procedure", data=procs)
        else:
            result = requests.post(SERVER_URL + "/answer/initial_procedure", data=procs)
        cls.searched.add(cls.get_params_hash(source, params))
        print(f"status_code, text: {result.status_code}, {result.text}")
        return True

    @classmethod
    def build(cls):
        while True:
            print()
            print_match_info()

            mode = input(('\n'
                          '1: new,  2: continue, 3: show-procs\n'
                          'a: auto, r: repeat,   c: change mode, Enter: reload\n'
                          ' >>> '))
            if mode not in cls.mode_types:
                print("invalid mode: ", mode)
                continue

            InitialProcsManager.update_initial_procs()

            if mode == '1' or mode == '2':
                if mode == '1':
                    source = "new"
                else:
                    InitialProcsManager.input_option_and_show()
                    succeeded, result = InitialProcsManager.input_id_and_get_file_path()
                    if not succeeded:
                        print(result)
                        continue
                    else:
                        source = result
                s = input('\n' + ', '.join(cls.labels) + '\n >>> ')
                succeeded, result = cls.extract_params(source, s)
                if not succeeded:
                    print(result)
                    continue
                cls.exec_solver_and_send_server(source, result)
            elif mode == '3':
                InitialProcsManager.input_option_and_show()
            elif mode == 'a':
                s = input("depth range >>> ")
                depth_range = s.split()
                if not castable(lambda x: tuple(map(int, x)), depth_range):
                    print("invalid depth_range: ", s)
                    continue
                depth_range = tuple(map(int, depth_range))
                dl = depth_range[0]
                dr = depth_range[1]
                if dl < 4 or dl > dr:
                    print("invalid depth_range: ", s)
                    continue
                depths = list(range(dl, dr+1))

                ss = input('\n' + ', '.join(cls.labels[1:]) + '\n >>> ')
                source = "new"

                while True:
                    depth = random.choice(depths)
                    s = f"{depth},{ss}"
                    succeeded, result = cls.extract_params(source, s)
                    if not succeeded:
                        depths.remove(depth)
                        if not depths:
                            break
                        else:
                            continue

                    succeeded = cls.exec_solver_and_send_server(source, result)
                    if not succeeded:
                        break
            elif mode == 'r':
                s = input('\n' + ', '.join(cls.labels) + '\n >>> ')
                source = "new"
                while True:
                    succeeded, result = cls.extract_params(source,  s)
                    if not succeeded:
                        print(result)
                        break
                    succeeded = cls.exec_solver_and_send_server(source, result)
                    if not succeeded:
                        break
            elif mode == 'c':
                return
            elif mode == '':
                continue


class ProcsCompleter:
    mode_types = ['1', '2', '3', 'a', 'r', 'c', '']

    labels = ["search_type", "promptly", "parallel_deg"]
    casters = [lambda x: x if x in ['a', 'p', 's'] else tuple(map(int, tuple(x.split()))), lambda x: bool(int(x)), int]
    checkers = [lambda x: x in ['a', 'p', 's'] or len(x) == 2, tautology, lambda x: 1 <= x <= 12]
    default_values = ["p", "0", "12"]
    searched = set()
    search_log = dict()

    @classmethod
    def get_params_hash(cls, source, params):
        return f"{source} {' '.join(map(str, params))}"

    @classmethod
    def extract_params(cls, s):
        s = s.split(',')
        s.extend(cls.default_values[len(s):])

        s = [cls.default_values[i] if s[i] == '' else s[i] for i in range(len(cls.labels))]

        for i in range(len(cls.labels)):
            if not castable(cls.casters[i], s[i]):
                return False, f"invalid {cls.labels[i]}: {s[i]}"
            else:
                s[i] = cls.casters[i](s[i])

            if not cls.checkers[i](s[i]):
                return False, f"invalid {cls.labels[i]}: {s[i]}"

        return True, s

    @classmethod
    def get_options(cls, search_type, source):
        if search_type == "a":
            options = [
                (x, y, X, Y) for x in range(2, div_num[0]+1) for y in range(2, div_num[1]+1) for X in range(div_num[0]) for Y in range(div_num[1])
                if cls.get_params_hash(source, (x, y, X, Y)) not in cls.searched
            ]
            options.sort(key=lambda x: x[0]+x[1])
        elif search_type == "p":
            rect_sizes = [
                (x, y) for x in range(2, div_num[0]+1) for y in range(2, div_num[1]+1)
                if not max(x, y) > min(x, y)*2
            ]
            rect_sizes.sort(key=lambda x: x[0]+x[1])
            pruned_rect_sizes = []
            dns = set()
            for x, y in rect_sizes:
                dn = (int((div_num[0] + x-2) / x), int((div_num[1] + y-2) / y))
                if dn not in dns:
                    pruned_rect_sizes.append((x, y))
                    dns.add(dn)
            options = [
                (x, y, X, Y) for (x, y) in pruned_rect_sizes for X in range(div_num[0]) for Y in range(div_num[1])
                if cls.get_params_hash(source, (x, y, X, Y)) not in cls.searched
            ]
            options.sort(key=lambda x: x[0]+x[1])
        else:
            rect_size = search_type
            options = [
                (rect_size[0], rect_size[1], X, Y) for X in range(div_num[0]) for Y in range(div_num[1])
                if cls.get_params_hash(source, (rect_size[0], rect_size[1], X, Y)) not in cls.searched
            ]
        return search_type == "p", options

    @classmethod
    def exec_solver_and_send_server(cls, source, pruning, options, params):
        _, promptly, parallel_deg = params

        best_procs = ""
        best_cost = None
        processes = []
        min_rect_size_sum = 1e10

        try:
            for opt in options:
                if pruning and opt[0] + opt[1] > min_rect_size_sum+3:
                    continue
                str_opt = ' '.join(map(str, opt))
                new_proc = subprocess.Popen(
                    f'{SOLVERS_DIR}/build_procedure {prob_dir} complete cout both {source} {str_opt}',
                    stdout=subprocess.PIPE,
                    shell=True
                )
                if len(processes) < parallel_deg:
                    processes.append((cls.get_params_hash(source, opt), new_proc))
                else:
                    wrote = False
                    while True:
                        for k in range(parallel_deg):
                            if processes[k][1].poll() is not None:
                                if processes[k][1].returncode == 0:
                                    cost = int(processes[k][1].stdout.readline())
                                    if best_cost is None or best_cost > cost:
                                        rect_size = int(processes[k][0].split()[1]) + int(processes[k][0].split()[2])
                                        min_rect_size_sum = min(min_rect_size_sum, rect_size)
                                        best_cost = cost
                                        if promptly:
                                            best_procs = processes[k][1].stdout.read().decode("utf-8")
                                            result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
                                            print(f"status_code, text: {result.status_code}, {result.text}")
                                        else:
                                            best_procs = processes[k][1].stdout.read()
                                cls.searched.add(processes[k][0])
                                processes[k] = (f"{source} {str_opt}", new_proc)
                                wrote = True
                                break
                        if wrote:
                            break
                        time.sleep(1/1000)

            for proc in processes:
                proc[1].wait()
                if proc[1].returncode == 0:
                    cost = int(proc[1].stdout.readline())
                    if best_cost is None or best_cost > cost:
                        best_cost = cost
                        if promptly:
                            best_procs = proc[1].stdout.read().decode("utf-8")
                            result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
                            print(f"status_code, text: {result.status_code}, {result.text}")
                        else:
                            best_procs = proc[1].stdout.read()

                cls.searched.add(proc[0])
        except KeyboardInterrupt:
            print("keyboard interrupt")

        if not promptly:
            if best_cost is not None:
                best_procs = best_procs.decode("utf-8")
                result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
                print(f"status_code, text: {result.status_code}, {result.text}")
            else:
                print("not found")

    @classmethod
    def exec_simple_solver_and_send_to_server(cls, source):
        process = subprocess.run(
            f'{SOLVERS_DIR}/build_procedure {prob_dir} complete cout both {source}',
            stdout=subprocess.PIPE,
            shell=True
        )
        if process.returncode == 0:
            best_procs = (b'\n'.join(process.stdout.splitlines()[1:])).decode("utf-8")
            result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
            print(f"status_code, text: {result.status_code}, {result.text}")
        else:
            print("[simple solver] couldn't build a procedure")

    @classmethod
    def exec_obvious_solver_and_send_to_server(cls, source):
        process = subprocess.run(
            f'{SOLVERS_DIR}/build_procedure {prob_dir} obviously cout both {source}',
            stdout=subprocess.PIPE,
            shell=True
        )
        if process.returncode == 0:
            best_procs = (b'\n'.join(process.stdout.splitlines()[1:])).decode("utf-8")
            result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
            print(f"status_code, text: {result.status_code}, {result.text}")
        else:
            print("[simple solver] couldn't build a procedure")

    @classmethod
    def complete(cls):
        while True:
            print()
            print_match_info()

            mode = input(('1: new,  2: continue, 3: show-procs\n'
                          'c: change mode,   Enter: reload\n'
                          ' >>> '))
            if mode not in cls.mode_types:
                print("invalid mode: ", mode)
                continue

            InitialProcsManager.update_initial_procs()

            if mode == '1' or mode == '2':
                if mode == '1':
                    source = "new"
                else:
                    InitialProcsManager.input_option_and_show(cls.search_log)
                    succeeded, result = InitialProcsManager.input_id_and_get_file_path()
                    if not succeeded:
                        print(result)
                        continue
                    else:
                        source = result
                s = input('\n' + ', '.join(cls.labels) + '\n >>> ')
                succeeded, result = cls.extract_params(s)
                if not succeeded:
                    print(result)
                    continue
                search_type = result[0]

                if os.path.basename(source) not in cls.search_log:
                    cls.search_log[os.path.basename(source)] = [search_type]
                else:
                    cls.search_log[os.path.basename(source)].append(search_type)

                if search_type in ['a', 'p', 's']:
                    cls.exec_simple_solver_and_send_to_server(source)
                    cls.exec_obvious_solver_and_send_to_server(source)

                if search_type == 's':
                    continue

                pruning, options = cls.get_options(search_type, source)
                if not options:
                    print("this type has already been searched completely")
                    continue
                cls.exec_solver_and_send_server(source, pruning, options, result)
            elif mode == '3':
                InitialProcsManager.input_option_and_show(cls.search_log)
            elif mode == 'c':
                return
            elif mode == '':
                continue


def input_mode_and_exec():
    mode_types = ['1', '2', '3']
    while True:
        print("\n", get_settings_str())
        print_match_info()

        mode = input('1: initial, 2: complete, 3: submit\n >>> ')
        if mode not in mode_types:
            print("invalid mode: ", mode)
            continue
        try:
            if mode == '1':
                InitialProcsBuilder.build()
            elif mode == '2':
                ProcsCompleter.complete()
            else:
                result = requests.get(SERVER_URL + "/submit/procedure")
                print(f"status_code, text: {result.status_code}, {result.text}")
        except KeyboardInterrupt:
            print("keyboard interrupt")
            continue


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("two arguments must be required.")
        sys.exit(-1)
    if sys.argv[2] not in ["primary", "ordinary"]:
        print("2nd arguments must be 'primary' or 'ordinary'")
        sys.exit(-1)

    print(f"server url: {SERVER_URL}")

    status = wait_until_match_start()
    prob_dir = get_prob_and_processing_for_solver()
    dnx, dny = get_div_num()
    div_num = (dnx, dny)

    if status == "restoring" and sys.argv[2] == "primary":
        restore_image()

    wait_until_procedure_building_start()

    get_original_state()

    InitialProcsManager.init()
    InitialProcsBuilder.init()

    input_mode_and_exec()

