import subprocess
import multiprocessing

def command(): 
    try:
        cmd = ['python3', 'client.py', 'ssl', 'noout']
        subprocess.call(cmd, shell=False)
    except: 
        print("Maybe kia")

if __name__ == '__main__':
    try:
        for i in range(0, 10):
            p = multiprocessing.Process(target=command, args=[])
            p.start()
    except:
        print("Closed")