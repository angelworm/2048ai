# -*- coding: utf-8 -*-

import random
import copy
import urllib as urllib2
import urllib.request as urllib2
import json
import queue
from threading import Thread
import bisect
import taasNum as T

def rotC(b):
    """ 時計回りに回転 """
    return [list(reversed(a)) for a in zip(*b)]
def rotA(b):
    """ 反時計回りに回転 """
    return [list(a) for a in zip(*[reversed(x) for x in b])]
def rotR(b):
    """ 左右反転 """
    return [list(reversed(a)) for a in b]


def movL(b):
    """ 左に動く """
    ret = [[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]]
    for y in range(4):
        minl = 0
        for x in range(minl, 4):
            if b[y][x] == 0:
                pass
            elif ret[y][minl] == 0:
                ret[y][minl] = b[y][x]
            elif ret[y][minl] == b[y][x]:
                ret[y][minl] = b[y][x] * 2
                minl += 1
    return ret

def mov(b, d):
    """ 上右下左: 0123 """
    if d == 0:
        return rotC(movL(rotA(b)))
    elif d == 1:
        return rotR(movL(rotR(b)))
    elif d == 2:
        return rotA(movL(rotC(b)))
    elif d == 3:
        return movL(b)
    else:
        print("none move")
    
def inp(b, vs):
    "vsの要素を埋め込んだbを量産"
    ret = []
    for y in range(4):
        for x in range(4):
            if b[y][x] == 0:
                for v in vs:
                    b2 = copy.deepcopy(b)
                    b2[y][x] = v
                    ret.append(b2)
    return ret

def nxt(b):
    "inpしてmovしたbを量産"
    ret = set()
    for b2 in inp(b, [2,4]):
        for d in range(4):
            ret.add(finalize(mov(b2, d)))
    return ret

def finalize(b):
    return tuple(tuple(i) for i in b)

def PM(x):
    for i in x:
        for j in i:
            print(j, end="\t")
        print()
    print()


class API:
    def __init__(self, serv):
        self.serv = serv
        st = self.start()
        self.sess = st['session_id']
        self.update(st)

    def start(self):
        return json.loads(urllib2.urlopen(self.serv + "/hi/start/json").read())

    def move(self, d):
        st = json.loads(urllib2.urlopen(self.serv + "/hi/state/" + self.sess + "/move/" + d + "/json").read())
        self.update(st)
        return self.board

    def update(self, st):
        self.board = st['grid']
        self.over = st['over']
        self.moved = st['moved']
        self.score = st['score']

def evf(b):
    """ 評価関数が入る """
    pass

def isVaildAction(action):
    return all(d in action for d in range(4))

def run2048(action, API=T.API):
    if not isVaildAction(action):
        return -1
    
    a = API("http://localhost:8080")
    while not a.over:
        for d in action:
            a.move(int(d))
            if a.over:
                return a.score
    return a.score

def worker(Q, Qout):
    while True:
        action = Q.get()
        score = [run2048(action) for i in range(10)]
        avg = sorted(score)[4]
        print("score: " + str(avg))
        Qout.put((action, avg))
        Q.task_done()

def rwc(l):
    total = 0
    ps = []
    it = []

    for item, p in l:
        total += p
        ps.append(total)
        it.append(item)

    return it[bisect.bisect(ps, random.random() * total)]

def cross(a, b):
    x, y = sorted(random.sample(range(len(a)), 2))
    return (a[:x] + b[x:y] + a[y:], b[:x] + a[x:y] + b[y:])

def main():
    genMax = 100
    chlidMax = 100
    sampleMax = 10
    geneMax = 400
    threadMax = 50
    actions = [(0, 0.30), (1, 0.69), (2, 0.01)]

    #act = [0, 2, 2, 3, 0, 2, 2, 1, 3, 3, 2, 3, 0, 1, 2, 1, 3, 2, 3, 3, 0, 2, 0, 1, 3, 1, 2, 3, 3, 0, 3, 3, 1, 3, 3, 3, 1, 2, 2, 3, 3, 1, 2, 3, 1, 0, 3, 3, 2, 2, 3, 0, 3, 2, 3, 1, 3, 0, 1, 0, 1, 2, 1, 2, 3, 3, 3, 2, 1, 2, 0, 3, 1, 1, 2, 3, 0, 1, 3, 3, 1, 0, 3, 2, 3, 0, 3, 2, 3, 0, 2, 0, 2, 3, 2, 2, 2, 2, 3, 3, 3, 1, 0, 2, 1, 3, 2, 0, 2, 0, 1, 3, 3, 0, 1, 3, 2, 3, 0, 1, 0, 1, 3, 2, 0, 0, 1, 2, 2, 0, 1, 1, 2, 2, 1, 3, 2, 1, 0, 2, 0, 0, 0, 1, 1, 2, 2, 3, 2, 0, 3, 2, 1, 1, 0, 1, 2, 1, 0, 0, 2, 2, 1, 0, 0, 3, 0, 3, 3, 3, 0, 3, 2, 0, 3, 0, 0, 1, 1, 2, 3, 2, 2, 3, 3, 0, 1, 1, 2, 1, 2, 3, 2, 1, 2, 0, 3, 0, 2, 1]
    acts = [([random.randrange(4) for i in range(geneMax)], 1.0/chlidMax) for i in range(chlidMax)]
    Q = queue.Queue()
    rQ = queue.Queue()

    for i in range(threadMax):
        w = Thread(target=worker, args=(Q,rQ,))
        w.setDaemon(True)
        w.start()

    for gen in range(genMax):
        print(str(gen) + " generation start *************")
        
        for i in range(chlidMax):
            action = rwc(actions)
            if action == 0:
                Q.put(rwc(acts))
            elif action == 1:
                a, b = cross(rwc(acts), rwc(acts))
                Q.put(a)
                Q.put(b)
            elif action == b:
                a = rwc(acts)[:]
                for i in range(len(a)):
                    if random.randrange(1) == 0:
                        a[i] = random.randrange(4)
                Q.put(a)
            
        Q.join()
    
        acts = []
        scoresum = 0
        while not rQ.empty():
            a, s = rQ.get_nowait()
            scoresum += s
            acts.append((a,s))

        for i in range(len(acts)):
            acts[i] = (acts[i][0], acts[i][1] / float(scoresum))

        acts = sorted(acts, key=lambda x:x[1])
        print("Max Score: " + str(int(acts[-1][1] * scoresum)) + " act:" + str(acts[-1][0]))
        
if __name__ == '__main__':
    main()
