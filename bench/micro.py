import subprocess
import time
import threading
import os
import random
import sys
import grandet

DEVNULL = open(os.devnull, 'wb')

last_begin = 0

def randstr(length):
    ret = ''
    for _ in xrange(length):
        ret += random.choice('0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ')
    return ret

class AtomicInt(object):
    def __init__(self, initval=0):
        self.value = initval
        self.lock = threading.Lock()

    def inc(self, val=1):
        with self.lock:
            self.value += val
            return self.value

    def dec(self, val=1):
        with self.lock:
            self.value -= val
            return self.value

    def get(self):
        with self.lock:
            return self.value

class Worker(threading.Thread):
    def __init__(self, count, tid, req_size, finished, started, ended, worker_count, server, keys, job_type):
        threading.Thread.__init__(self)
        self.tid = tid
        self.used = -1
        self.count = count
        self.key_len = 32
        self.value_len = req_size*1024
        self.value = '#' * self.value_len
        self.daemon = True
        self.finished = finished
        self.ended = ended
        self.started = started
        self.worker_count = worker_count
        self.server = server
        self.keys = keys
        self.job_type = job_type

    def stat(self, cur, tot):
        if self.tid != 0:
            return
        if cur % (tot / 10) == 0:
            print "%d%%" % (cur * 100 / tot),
            sys.stdout.flush()
        if cur == tot:
            print

    def run(self):
#        cmds = ["build/console/console"]
        begin = time.time()
        client = grandet.Grandet(self.server)
        global last_begin
        if self.started.inc() == self.worker_count:
            last_begin = time.time()

        mykeys = []
        if self.job_type == 'p':
            for i in xrange(self.count):
                key = randstr(self.key_len)
                if client.put(key, self.value, latency_required=100) != 1:
                    raise Exception("PUT Error")
    #            cmds.append("p %s %s" % (key, self.value))
                mykeys.append(key)
                self.stat(i + 1, self.count)
                if last_begin > 1:
                    self.finished.inc()
            self.keys[self.tid] = mykeys
        elif self.job_type == 'g':
            mykeys = self.keys[self.tid]
            for i in xrange(self.count):
                client.get(mykeys[i])
                if last_begin > 1:
                    self.finished.inc()
                self.stat(i + 1, self.count)
        elif self.job_type == 'd':
            mykeys = self.keys[self.tid]
            for i in xrange(self.count):
                client.delete(mykeys[i])
                if last_begin > 1:
                    self.finished.inc()
                self.stat(i + 1, self.count)
#                cmds.append("g %s" % keys[i])

#        subprocess.check_call(cmds, stdout=DEVNULL)
        end = time.time()
        self.used = end - begin
        self.end = end
        if self.ended.inc() == 1:
            completed = self.finished.get()
            period = time.time() - last_begin
            print("\nthread %d first. rps: %f" % (self.tid, completed / period))

wall_begin = time.time()

num_workers = int(sys.argv[4])
keys = [1] * num_workers

def do_work(job_type, keys):
    server = sys.argv[1]
    total_count = int(sys.argv[2])
    reqsize = int(sys.argv[3])
    count = total_count / reqsize
    workers = []
    finished = AtomicInt()
    ended = AtomicInt()
    started = AtomicInt()
    global last_begin
    last_begin = 0

    for tid in xrange(num_workers):
        worker = Worker(count, tid, reqsize, finished, started, ended, num_workers, server, keys, job_type)
        worker.start()
        workers.append(worker)

    total = 0
    for worker in workers:
        worker.join()
        total += worker.used

    wall_end = time.time()
    wall_used = wall_end - wall_begin

    print "avg: ", total / num_workers
    print "wall: ", wall_used
    print "rps: ", 1.0 * count * num_workers / wall_used
    print "avg rps: ", 1.0 * count * num_workers / (total / num_workers)

do_work('p', keys)
do_work('g', keys)
do_work('d', keys)
