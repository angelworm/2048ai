# -*- coding: utf-8 -*-

import random
import supercalifragilisticexpialidocious as S
from multiprocessing import Process, Queue, JoinableQueue
import collections
import sys

def worker(Q, Qout):
    while True:
        action = Q.get()
        scores = []

        score = S.run2048(w=S.norm(action), p=False)
        scores.append(score)
        
        avg = score
        print("score: " + str(avg) + " \tact:" + str(action))   
        sys.stdout.flush()         
        Qout.put((action, avg))
        Q.task_done()

def main():
    threadMax = 8
    genMax = 100

    act = [500, 500, 300, 100, 500, 100]
    score = 0
    Q = JoinableQueue()
    rQ = Queue()

    for i in range(threadMax):
        w = Process(target=worker, args=(Q,rQ,))
        w.start()

    for gen in range(genMax):
        print(str(gen) + " generation start *************")
        sys.stdout.flush()
            
        task = []

        # rand-small
        for i in range(11):
            ac = act[:]
            for i in random.sample(range(len(act)), 2):
                ac[i] += random.randint(-10,10)
            task.append(ac)

        # coll
        for i in range(5):
            task.append([a+b for a, b in zip(ac, [random.randint(-100, 100) for i in range(6)]) ])
        for t in task * 10:
            Q.put(t)
        
        Q.join()
        
        d = collections.defaultdict(list)
        while not rQ.empty():
            a, s = rQ.get_nowait()
            d[tuple(a)].append(s)

        old = score

        for k,v in d.items():
            s = sum(sorted(v[-3:]))/len(v[-3:])
            if score < s:
                score = s
                act = list(k)

        print("Max Score: " + str(score) + "(" + str(score-old)+ ")" + " \tact:" + str(act))
        sys.stdout.flush()
        
if __name__ == '__main__':
    main()
