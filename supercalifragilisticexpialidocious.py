# -*- coding: utf-8 -*-
import random
import copy
import urllib as urllib2
import urllib
import urllib.request as urllib2
import json
import math
import taasNum as T
import numpy as np
import hashlib

def cmv(b,d):
    return not np.all(b == T.mov(b,d))

def inp(b, vs):
    "vsの要素を埋め込んだbを量産"
    ret = []
    for y, x in T.hole_index(b):
        for v in vs:
            b2 = np.copy(b)
            b2[y, x] = v
            ret.append(b2)
    return ret

def nxt(b):
    "inpしてmovしたbを量産"
    def elem(d, li):
        any(np.array_equal(d, x) for x in li)

    ret = np.empty()
    for b2 in inp(b, [2,4]):
        for d in range(4):
            if not elem(d, ret):
                ret.append(T.mov(b2, d))
    return ret

def PM(x):
    for i in x:
        for j in i:
            print(j,end="\t")
        print()
    print

def norm(l):
    return list(i / sum(l) for i in list(l))

def ev_score(b):
    b = b[b.nonzero()]
    return int(sum(b * (np.log2(b) - 1)))

def ev_sclg(b):
    b = b[b.nonzero()]
    return int(sum((np.log2(b) - 1) ** 2))

def ev_max(b):
    return int(np.log2(np.max(b)))

def ev_step(b):
    def cost(l, r):
        """ l < r """
        if l == 0:
            l = 1
        if l     == r:
            return 4
        elif l * 2 == r:
            return 2
        elif l     <= r:
            return 1
        return 0
        
    bt = T.rotC(b)
    b2 = [[x, np.fliplr(x), np.flipud(x), np.fliplr(np.flipud(x))] for x in [b, bt]]
    
    ret = 0
    for bi in sum(b2, []):
        bx = -1
        c = 0
        for x in bi.flat:
            if bx == -1:
                pass
            elif bx >= x:
                c += cost(x,bx)
            else:
                break
            bx = x
        ret = max(ret, c)
    return ret

def ev_eq(b):
    d1 = b[1:].nonzero()
    d2 = b[...,1:].nonzero()
    l1 = len(np.nonzero(b[1:][d1] == b[:-1][d1])[0])
    l2 = len(np.nonzero(b[...,1:][d2] == b[...,:-1][d2])[0])
    return l1 + l2
              
def ev_cm(b):
    mv = np.max(b)
    if mv in [b[0, 0], b[0, 3], b[3, 0], b[3, 3]]:
        return 10
    else:
        return 0

def ev_cmmax(b):
    return ev_max(b) * ev_cm(b)

def ev_hole(b):
    return len((b == 0).nonzero()[0])

def ev_di(b):
    b = b.copy()
    b[np.nonzero(b == 0)] = 1
    b = np.log2(b)

    d1 = np.abs(b[1:] - b[:-1])
    d2 = np.abs(b[..., 1:] - b[..., :-1]) 
    return 0 - np.sum(d1) - np.sum(d2)

defaultweight = norm([50, 50, 30, 10, 50, 10, 10])
evs = [ev_sclg, ev_max, ev_step, ev_eq, ev_cmmax, ev_hole, ev_di]

def evf(b, w=defaultweight, evs=evs):
    """ 評価関数が入る """
    return sum(w * ev(b) for w, ev in zip(w, evs))

def evs(b, w=defaultweight, evs=evs):
    def pp(x):
        return str(int(x * 10000) / 100.00)  + "%"
    ret = str(evs[0](b)) + "*" + pp(w[0])

    for ev, wv in zip(evs[1:], w[1:]):
        ret += " + " + str(ev(b)) + "*" + pp(wv)

    ret += " = " + str(int(evf(b,w=w)))
    return ret

def guessN(b1, n=4, w=defaultweight, player=True, a=float('-inf'), b=float('inf')):
    if n == 0:
        return evf(b1, w)
    
    if player:
        for b2 in [T.mov(b1, d) for d in range(4) if cmv(b1, d)]:
            a = max(a, guessN(b2, n=n-1, w=w, player=not player, a=a, b=b))
            if a >= b:
                return b
        return a
    else:
        for b2 in inp(b1, [2,4]):
            b = min(b, guessN(b2, n=n-1, w=w, player=not player, a=a, b=b))
            if a >= b:
                return a
        return b

def guess(b, n=5, w=defaultweight):
    b2 = [(guessN(T.mov(b, d), n=n, w=w, player=False), d) for d in range(4) if cmv(b, d)]
    if b2 == []:
        return -1,0
    else:
        return max(b2, key=lambda x:x[0])

def di(d):
    return ["↑", "→", "↓", "←"][d]

def run2048(w=defaultweight, p=True, API=T.API):
    a = API("http://localhost:8080")
    
    try:
        while not a.over:
            gv, gd = guess(a.board)
            if p:
                print("EV: " + evs(a.board, w) + " guess: " + di(gd) + "(" + str(gv) + ")")
                PM(a.board)
            a.move(str(gd))
            if not a.moved:
                print("failed to move: " + di(gd))
                PM(a.board)
    except urllib.error.HTTPError as e:
        print("!!! token expired !!!")
        PM(a.board)
                
    if p:
        print("over:" + str(a.score))
        PM(a.board)
    return a.score

def lr():
    run2048(API=T.Local)

def proc(Q):
    while True:
        mode, data = Q.get()
        pass
        Q.task_done()

def main():
    #run2048()
    run2048(API=T.Local)

def mkb(s):
    return [[int(x) for x in i.split()] for i in s.split(',')]

if __name__ == '__main__':
    pass
    #main()

