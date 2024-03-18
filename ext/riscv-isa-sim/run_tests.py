import argparse
import os
import subprocess
def getPaths() -> (str, str, str):
    proj_root = os.getenv('PROJ_ROOT')
    if proj_root == None:
        raise 'please source env.sh'
    isatest_root = proj_root + '/isatest'
    main_root = f"{proj_root}/build/playground/playground_main"
    if not os.path.exists(main_root):
        raise "please compile and generate playground_main"
    return proj_root, isatest_root, main_root

def getTests(isatest_root:str) -> list[str]:
    tests = os.listdir(isatest_root)
    return tests

def gen_bin():
    proj_root, isatest_root, main_root = getPaths()
    tests = getTests(isatest_root)
    for test in tests:
        if "." in test:
            continue
        os.system(f"riscv64-unknown-elf-objcopy -I elf64-littleriscv -O binary {isatest_root}/{test} {isatest_root}/{test}.bin")    

    
    
def __main__():
    proj_root, isatest_root, main_root = getPaths()
    tests = getTests(isatest_root)

    parser = argparse.ArgumentParser()
    parser.add_argument('-n', '--name', type=str, help="Name of the test")
    parser.add_argument('-v', '--virtual', default=False, action='store_true', help="run all -v-")
    parser.add_argument('-p', '--physical', default=False, action='store_true', help="run all -p-")
    parser.add_argument('-a', '--all', default=False, action='store_true', help="run all tests")
    parser.add_argument('-o', '--output', type=str,default=proj_root + "/isatest_logs", help="Output dir")
    parser.add_argument("--gen_bin", default=False, action='store_true', help="generate bin in isatest")
    args = parser.parse_args()

    if args.gen_bin:
        gen_bin()
        return

    test_name = ""
    if(args.name == None and args.virtual == False and args.physical == False and args.all == False):
        raise "tests must be selected. use -v/-p/--all or -n to specify a test"
    if args.name != None:
        test_name = args.name
    if not os.path.exists(args.output):
        raise f"output path {args.output} does not exist"
    test_list = []
    if(test_name != ""):
        if test_name not in tests:
            raise f"test name {test_name} is invalid"
        test_list.append(test_name)
    elif(args.virtual):
        for test in tests :
            if "-v-" not in test or ".bin" not in test or test == "Makefile":
                continue
            test_list.append(test.replace(".bin", ""))
    elif(args.physical):
        for test in tests :
            if "-p-" not in test or ".bin" not in test or test == "Makefile":
                continue
            test_list.append(test.replace(".bin", ""))
    elif(args.all):
        for test in tests or test == "Makefile":
            if ".bin" not in test:
                continue
            test_list.append(test.replace(".bin", ""))
    else:
        raise "unexcepted case"
    
    if test_name == "":
        os.system(f"rm -rf {args.output}/*.log")
        
    for test in test_list:
        result = subprocess.run([f"{main_root}", test], stdout=subprocess.PIPE, text=True)
        print(result)
        if result.returncode == 1: # pass
            continue
        elif result.returncode == 3 or result.returncode == -6 : # fail. -6 is the magic number of wfi
            os.system(f"{main_root} {test} --trace > {args.output}/fail.{test}.log")
        elif result.returncode == 2: # timeout
            os.system(f"echo \"timeout\" > {args.output}/timeout.{test}.log")
        else:
            raise "unexcepted return value"
            
        
    
__main__()
        