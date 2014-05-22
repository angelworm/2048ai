# -*- coding: utf-8 -*-

import random
import gp as S
import taasNum as T
from multiprocessing import Process, Queue, JoinableQueue
import collections
import sys
import bisect
import gpknown as K

def worker(Q, Qout):
    while True:
        ev = Q.get()
        scores = []

        for _ in range(10):
            score = S.run2048(evf=ev, p=False, API=T.Local, n=0)
            scores.append(score)
        
        if 0 in scores:
            avg = 0
        else:
            avg = sum(scores) / 10

        #print("score: " + str(avg) + " \tact:" + str(ev))   
        if avg != 0:
            sys.stdout.flush()         
            Qout.put((ev, avg))
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

def main():
    def gh(g):
        return hash(str(g))
    
    threadMax = 8
    genMax = 10000
    geneMax = 350
    actions = [(0, 0.20), (1, 0.75), (2, 0.05)]
    
    gene = [(S.ev_gen(), 1/geneMax) for _ in range(geneMax - len(K.known))] + [(x, 1/geneMax) for x in K.known]
    score = 0
    Q = JoinableQueue()
    rQ = Queue()

    for i in range(threadMax):
        w = Process(target=worker, args=(Q,rQ,))
        w.start()

    for gen in range(genMax):
        print(str(gen) + " generation start *************")
        sys.stdout.flush()
            
        task = set()
        count = 0
        while count < geneMax:
            action = rwc(actions)
            if action == 0:
                g = rwc(gene)
                if gh(g) not in task:
                    task.add(gh(g))
                    count += 1
                    Q.put(g)
            elif action == 1:
                a = S.ev_cross(rwc(gene), rwc(gene))
                if gh(a) not in task:
                    task.add(gh(a))
                    count += 1
                    Q.put(a)
            elif action == 2:
                g = rwc(gene)
                a = S.ev_cross(g, S.ev_gen())
                if gh(a) not in task:
                    task.add(gh(a))
                    count += 1
                    Q.put(a)
                
        Q.join()
        
        gene = []
        scoresum = 0
        while not rQ.empty():
            a, s = rQ.get_nowait()
            scoresum += s
            gene.append((a,s))

        for i in range(len(gene)):
            gene[i] = (gene[i][0], gene[i][1] / float(scoresum))

        gene = sorted(gene, key=lambda x:x[1])
        for i in range(1,4):
            print(str(i) + ": " + str(int(gene[-i][1] * scoresum)) + " act:" + str(gene[-i][0]))
        sys.stdout.flush()
        
if __name__ == '__main__':
    main()
