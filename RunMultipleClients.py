import os
import subprocess
import threading

os.system("cls")


ans:int = int(input("How many extra clients to start?\n>"))

def RunClient():
    subprocess.run(["RoboCatSFMLClient.exe", "127.0.0.1:50000", "Ruby"])
    
threads:list[threading.Thread] = []

os.chdir("GD4RoboCatSFML-master/x64/Debug/")
for i in range(ans):
    newThread:threading.Thread = threading.Thread(target=RunClient)
    newThread.start()
    threads.append(newThread)
    
