from threading import Thread
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


# SERVER_URL = "http://192.168.1.14:3000"
SERVER_URL = "http://10.55.21.164:3000"
SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_TOP_DIR = os.path.normpath(SERVER_DIR + "/../../")
DATA_DIR = PROJECT_TOP_DIR + "/.data"
SOLVERS_DIR = PROJECT_TOP_DIR + "/algo/bin"
DEFAULT_WAIT_TIME = 1


def is_num(x):
    try:
        int(x)
    except ValueError:
        return False
    else:
        return True


def calc_penalty_and_cost(procs_path) -> (int, int):
    result = subprocess.run(f"{SOLVERS_DIR}/procedure_checker {prob_dir} {procs_path}", stdout=subprocess.PIPE, shell=True)
    if result.returncode != 0:
        return None, None
    else:
        return int(result.stdout.split()[0]), int(result.stdout.split()[1])


class InitialProcsManager:
    new_id = 0
    initial_procs = []

    @classmethod
    def update_initial_procs(cls):
        ini_procs_path = prob_dir + "/ini_procs"
        files = os.listdir(ini_procs_path)
        excepts = '\n'.join(files)
        result = requests.post(SERVER_URL + "/get_initial_procedures", data=excepts)

        zip_path = ini_procs_path + "/tmp.zip"
        with open(zip_path, 'wb') as f:
            f.write(result.content)
        with zipfile.ZipFile(zip_path) as zipf:
            zipf.extractall(ini_procs_path)
            new_files = zipf.namelist()
        for file_name in new_files:
            penalty, cost = calc_penalty_and_cost(ini_procs_path + "/" + file_name)
            cls.initial_procs.append((cls.new_id, file_name, penalty, cost))
            cls.new_id += 1
        os.remove(zip_path)

    @classmethod
    def init(cls):
        ini_procs_path = prob_dir + "/ini_procs"
        if os.path.exists(ini_procs_path):
            shutil.rmtree(ini_procs_path)
        os.mkdir(ini_procs_path)
        cls.update_initial_procs()

    @classmethod
    def show(cls, limit=10, sort_by_cost=True):
        tmp = cls.initial_procs.copy()
        if sort_by_cost:
            tmp.sort(key=lambda x: x[3])
        else:
            tmp.sort(key=lambda x: x[2])

        print("{:>3}, {:>8}, {:>8}, {:>8}".format("id", "hash(5)", "penalty", "cost"))
        for i in range(min(len(tmp), limit)):
            procs = tmp[i]
            print("{:>3}, {:>8}, {:>8}, {:>8}".format(procs[0], procs[1][:5], procs[2], procs[3]))

    @classmethod
    def get_file_name_by_id(cls, id) -> (bool, str):
        if len(cls.initial_procs) > id and cls.initial_procs[id][0] == id:
            return True, cls.initial_procs[id][1]
        else:
            for proc in cls.initial_procs:
                if proc[0] == id:
                    return True, proc[1]
        return False, f"id is not found: {id}"


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


def input_option_and_show_procs():
    print("line limit, sort_by_cost")
    print(" >>> ", end="")
    opt = input().split(',')
    opt.extend(['', ''])
    line_limit = int(opt[0]) if is_num(opt[0]) else 10
    sort_by_cost = int(opt[1]) if is_num(opt[1]) else 0
    sort_by_cost = sort_by_cost == 1
    InitialProcsManager.show(line_limit, sort_by_cost)


def input_id_and_return_file_name():
    print("\n id")
    print(" >>> ")
    id = input()
    if not is_num(id):
        print("this id is not integer: ", id)
        return None
    id = int(id)
    succeeded, s = InitialProcsManager.get_file_name_by_id(id)
    if not succeeded:
        print(s)
        return None
    return s


def exec_initial_procedure_builder_and_send_to_server(cmd, tmp_procs_file_path):
    try:
        result = subprocess.run(cmd, shell=True)
    except KeyboardInterrupt:
        print("keyboard interrupt")
        return
    if result.returncode != 0:
        print("solver failed")
        return
    with open(tmp_procs_file_path, mode='r') as f:
        procs = f.read()

    result = requests.post(SERVER_URL + "/answer/initial_procedure", data=procs)
    print(f"status_code, text: {result.status_code}, {result.text}")


def get_options(source, search_type, searched, div_num):
    if search_type == "a":
        options = [
            (x,y,X,Y) for x in range(2,div_num[0]+1) for y in range(2,div_num[1]+1) for X in range(div_num[0]) for Y in range(div_num[1])
            if not f"{source} {' '.join(map(str, (x,y,X,Y)))}" in searched
        ]
        options.sort(key=lambda x: x[0]+x[1])
    elif search_type == "p":
        rect_sizes = [
            (x,y) for x in range(2,div_num[0]+1) for y in range(2,div_num[1]+1)
            if not max(x,y) > min(x,y)*2
        ]
        rect_sizes.sort(key=lambda x: x[0]+x[1])
        pruned_rect_sizes = []
        dns = set()
        for x,y in rect_sizes:
            dn = (int((div_num[0] + x-2) / x), int((div_num[1] + y-2) / y))
            if dn not in dns:
                pruned_rect_sizes.append((x,y))
                dns.add(dn)
        options = [
            (x,y,X,Y) for (x,y) in pruned_rect_sizes for X in range(div_num[0]) for Y in range(div_num[1])
            if f"{source} {' '.join(map(str, (x,y,X,Y)))}" not in searched
        ]
        options.sort(key=lambda x: x[0]+x[1])
    else:
        rect_size = search_type
        options = [
            (rect_size[0],rect_size[1],X,Y) for X in range(div_num[0]) for Y in range(div_num[1])
            if f"{source} {' '.join(map(str, (rect_size[0],rect_size[1],X,Y)))}" not in searched
        ]
    return search_type == "p", options


def exec_search(parallel_deg, promptly, source, pruning, options):
    found = False
    best_procs = ""
    best_cost = 1e10
    processes = []
    search_log = []
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
                processes.append((f"{source} {str_opt}", new_proc))
            else:
                wrote = False
                while True:
                    for k in range(parallel_deg):
                        if processes[k][1].poll() is not None:
                            if processes[k][1].returncode == 0:
                                cost = int(processes[k][1].stdout.readline())
                                if best_cost > cost:
                                    rect_size = int(processes[k][0].split()[1]) + int(processes[k][0].split()[2])
                                    min_rect_size_sum = min(min_rect_size_sum, rect_size)
                                    best_cost = cost
                                    if promptly:
                                        best_procs = processes[k][1].stdout.read().decode("utf-8")
                                        result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
                                        print(f"status_code, text: {result.status_code}, {result.text}")
                                    else:
                                        found = True
                                        best_procs = processes[k][1].stdout.read()
                            search_log.append(processes[k][0])
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
                if best_cost > cost:
                    best_cost = cost
                    if promptly:
                        best_procs = proc[1].stdout.read().decode("utf-8")
                        result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
                        print(f"status_code, text: {result.status_code}, {result.text}")
                    else:
                        found = True
                        best_procs = proc[1].stdout.read()
            search_log.append(proc[0])
    except KeyboardInterrupt:
        print("keyboard interrupt")

    if not promptly:
        if found:
            best_procs = best_procs.decode("utf-8")
            result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
            print(f"status_code, text: {result.status_code}, {result.text}")
        else:
            print("not found")

    return search_log


def complete_procedure_and_send_to_server(parallel_deg, promptly, source, search_type, searched, div_num):
    pruning, options = get_options(source, search_type, searched, div_num)
    if len(options) == 0:
        print("this type has already been searched completely")
        return

    search_log = exec_search(parallel_deg, promptly, source, pruning, options)

    searched |= set(search_log)


def complete_procedure_by_rough_sorter_and_send_to_server(source):
    process = subprocess.run(
        f'{SOLVERS_DIR}/build_procedure {prob_dir} complete cout both {source}',
        stdout=subprocess.PIPE,
        shell=True
    )
    best_procs = (b'\n'.join(process.stdout.splitlines()[1:])).decode("utf-8")
    result = requests.post(SERVER_URL + "/answer/complete_procedure", data=best_procs)
    print(f"status_code, text: {result.status_code}, {result.text}")


def build_initial_procs():
    tmp_procs_file_path = prob_dir + "/tmp_procs.txt"

    searched = set()
    search_log = []

    with open(prob_dir + "/prob.txt", mode='r') as f:
        settings = f.read().split()
        dnx = int(settings[0])
        dny = int(settings[1])

    while True:
        InitialProcsManager.update_initial_procs()
        print("\n1: new, 2:improve, 3: redo, 4: show-procs, 5: show-log")
        print(" >>> ", end="")
        mode = input()
        if mode == "1":
            print("depth, loose_time_limit, strict_time_limit, first_selected_pos")
            print(" >>> ", end="")
            opt = input().split(',')
            opt.extend(['', '', '', ''])
            depth = int(opt[0]) if is_num(opt[0]) else 12
            loose_time_limit = int(opt[1]) if is_num(opt[1]) else 5000
            strict_time_limit = int(opt[2]) if is_num(opt[2]) else 120000
            if len(opt[3].split()) >= 2 and is_num(opt[3].split()[0]) and is_num(opt[3].split()[1]):
                fsp = (int(opt[3].split()[0]), int(opt[3].split()[1]))
                if fsp[0] < 0 or fsp[0] >= dnx or fsp[1] < 0 or fsp[1] >= dny:
                    print(f"fsp is invalid: {fsp}, (div_num: {dnx}, {dny})")
                    continue
            else:
                candidate = list()
                for y in range(dny):
                    for x in range(dnx):
                        if not '-'.join(map(str, [depth, x, y])) in searched:
                            candidate.append((x, y))

                if len(candidate) == 0:
                    print(f"explored all this depth: {depth}")
                    continue
                else:
                    fsp = candidate[random.randint(0, len(candidate)-1)]
            params = '-'.join(map(str, [depth, fsp[0], fsp[1]]))
            if params in searched:
                print("already searched")
                continue

            searched.add(params)
            search_log.append(params)

            cmd = f"{SOLVERS_DIR}/build_procedure {prob_dir} initial {tmp_procs_file_path} procedure new {fsp[0]} {fsp[1]} {loose_time_limit} {strict_time_limit} {depth}"
            exec_initial_procedure_builder_and_send_to_server(cmd, tmp_procs_file_path)

            print("searched: ", depth, loose_time_limit, strict_time_limit, fsp)
        elif mode == "2":
            input_option_and_show_procs()
            file_name = input_id_and_return_file_name()
            if file_name is None:
                continue

            print("depth, loose_time_limit, strict_time_limit, first_selected_pos")
            print(" >>> ", end="")
            opt = input().split(',')
            opt.extend(['', '', '', ''])
            depth = int(opt[0]) if is_num(opt[0]) else 12
            loose_time_limit = int(opt[1]) if is_num(opt[1]) else 5000
            strict_time_limit = int(opt[2]) if is_num(opt[2]) else 120000
            if len(opt[3].split()) >= 2 and is_num(opt[3].split()[0]) and is_num(opt[3].split()[1]):
                fsp = (int(opt[3].split()[0]), int(opt[3].split()[1]))
            else:
                fsp = (-1, -1)

            print("file name: ", file_name)
            cmd = f"{SOLVERS_DIR}/build_procedure {prob_dir} initial {tmp_procs_file_path} procedure {prob_dir}/ini_procs/{file_name} {fsp[0]} {fsp[1]} {loose_time_limit} {strict_time_limit} {depth}"
            exec_initial_procedure_builder_and_send_to_server(cmd, tmp_procs_file_path)
            print("searched: ", depth, loose_time_limit, strict_time_limit, fsp)
        elif mode == "3":
            if len(search_log) == 0:
                print("not yet explored")
                continue

            print("depth, loose_time_limit, strict_time_limit, first_selected_pos")
            print(" >>> ", end="")
            opt = input().split(',')
            opt.extend(['', '', '', ''])
            depth = int(opt[0]) if is_num(opt[0]) else 12
            loose_time_limit = int(opt[1]) if is_num(opt[1]) else 5000
            strict_time_limit = int(opt[2]) if is_num(opt[2]) else 120000
            if len(opt[3].split()) >= 2 and is_num(opt[3].split()[0]) and is_num(opt[3].split()[1]):
                fsp = (int(opt[3].split()[0]), int(opt[3].split()[1]))
            else:
                fsp = (-1, -1)

            cmd = f"{SOLVERS_DIR}/build_procedure {prob_dir} initial {tmp_procs_file_path} procedure {tmp_procs_file_path} {fsp[0]} {fsp[1]} {loose_time_limit} {strict_time_limit} {depth}"
            exec_initial_procedure_builder_and_send_to_server(cmd, tmp_procs_file_path)
            print("searched: ", depth, loose_time_limit, strict_time_limit, fsp)
        elif mode == "4":
            input_option_and_show_procs()
        elif mode == "5":
            if len(search_log) == 0:
                print("not yet explored")
            else:
                print("depth, fsp.x, fsp.y")
                for params in search_log:
                    depth, fsp_x, fsp_y = params.split('-')
                    print("{:>5}, {:>5}, {:>5}".format(depth, fsp_x, fsp_y))


def complete_initial_procs():
    searched = set()

    with open(prob_dir + "/prob.txt", mode='r') as f:
        settings = f.read().split()
        dnx = int(settings[0])
        dny = int(settings[1])
        div_num = (dnx, dny)

    while True:
        InitialProcsManager.update_initial_procs()

        print("\n1: using initial procs, 2:new, 3: show-procs, 4: show-log")
        print(" >>> ", end="")
        mode = input()
        if mode == "1" or mode == "2":
            if mode == "1":
                input_option_and_show_procs()
                file_name = input_id_and_return_file_name()
                if file_name is None:
                    continue
                source = f"{prob_dir}/ini_procs/{file_name}"
                if source is None:
                    continue
            else:
                source = "new"

            print("search type, promptly, parallel_deg")
            opt = input().split(',')
            opt.extend(['', '', ''])
            if opt[0] != "a" and opt[0] != "p" and opt[0] != "r":
                if len(opt[0].split()) >= 2 and is_num(opt[0].split()[0]) and is_num(opt[0].split()[1]):
                    rect_size = (int(opt[0].split()[0]), int(opt[0].split()[1]))
                    if rect_size[0] < 2 or rect_size[0] >= dnx or rect_size[1] < 2 or rect_size[1] >= dny:
                        print(f"rect_size is invalid: {rect_size}, (div_num: {dnx}, {dny})")
                        continue
                    search_type = rect_size
                elif opt[0] == '':
                    search_type = "p"
                else:
                    print("search type must be 'a(all)' or 'p(pruning)' or r(rough) or (x,y)")
                    print(f"your input: {opt[0]}")
                    continue
            else:
                search_type = opt[0]
            promptly = False if opt[1] == '' or opt[1] == '0' else True
            parallel_deg = int(opt[2]) if is_num(opt[2]) else 10

            if search_type == "r":
                complete_procedure_by_rough_sorter_and_send_to_server(source)
            else:
                complete_procedure_and_send_to_server(parallel_deg, promptly, source, search_type, searched, div_num)
        elif mode == "3":
            input_option_and_show_procs()
        elif mode == "4":
            pass


if __name__ == "__main__":
    status = wait_until_match_start()
    prob_dir = get_prob_and_processing_for_solver()
    if status == "restoring" and sys.argv[1] == "primary":
        restore_image()

    wait_until_procedure_building_start()

    get_original_state()

    InitialProcsManager.init()

    # exit(-1)

    if sys.argv[2] == "initial":
        build_initial_procs()
    else:
        complete_initial_procs()
