from threading import Thread
import subprocess
from Queue import Queue

num_threads = 1
queue = Queue()
ips = ["10.0.0.1"]
#wraps system ping command
def pinger(i, q):
    """Pings subnet"""
    while True:
        ip = q.get()
        ret = subprocess.call("sudo ping -c 10000 -i 0.01 %s" % ip,
            shell=True,
            stdout=open('/Users/DeyuHan/Documents/Graduate/Summer_2016/research/experiment/baselineTest_Fog.txt', 'w'),
            stderr=subprocess.STDOUT)
        # if ret == 0:
        #     print "%s: is alive" % ip
        # else:
        #     print "%s: did not respond" % ip
        q.task_done()
#Spawn thread pool
for i in range(num_threads):

    worker = Thread(target=pinger, args=(i, queue))
    worker.setDaemon(True)
    worker.start()
#Place work in queue
for ip in ips:
    queue.put(ip)
#Wait until worker threads are done to exit    
queue.join()