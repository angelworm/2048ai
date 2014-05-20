# -*- coding: utf-8 -*-
import random
import copy
import urllib as urllib2
import urllib
import urllib.request as urllib2
import json
import math
import taas as T
    
def cmv(b,d):
    return b != T.mov(b,d)

def inp(b, vs):
    "vsの要素を埋め込んだbを量産"
    ret = []
    for y in range(4):
        for x in range(4):
            if b[y][x] == 0:
                for v in vs:
                    b2 = [l[:] for l in b]
                    b2[y][x] = v
                    ret.append(b2)
    return ret

def nxt(b):
    "inpしてmovしたbを量産"
    ret = set()
    for b2 in inp(b, [2,4]):
        for d in range(4):
            ret.add(finalize(T.mov(b2, d)))
    return ret

def finalize(b):
    return tuple(tuple(i) for i in b)
def definelize(b):
    return list(list(i) for i in b)
        

def PM(x):
    for i in x:
        for j in i:
            print(j,end="\t")
        print()
    print

def norm(l):
    l = list(l)
    return list(i / sum(l) for i in l)

def ev_score(b):
    return sum(sum(x * int(math.log(x, 2) - 1) for x in l if x != 0) for l in b)

def ev_sclg(b):
    return sum(sum(int(math.log(x, 2) - 1) * x for x in l if x != 0) for l in b)

def ev_max(b):
    def sl(x):
        if x <= 2:
            return 0
        else:
            return int(math.log(x, 2) - 1) 
    return max(max(sl(x) for x in l) for l in b)

def ev_step(b):
    def cost(l, r):
        """ l < r """
        if l == 0:
            l = 1
        if l * 2 == r:
            return 4
        if l * 4 == r:
            return 2
        if l     <= r:
            return 1
        return 0

    b2 = [b, T.rotC(b)]
    
    ret = 0
    for b in b2:
        lc, rc, lrc, rrc = 0, 0, 0, 0 
        lv, rv = 0, 0
        for line in b:
            for l in line:
                lc += cost(l,lv)
                lrc += cost(lv, l)
                lv = l
            for r in reversed(line):                
                rc += cost(r, rv)
                rrc += cost(rv, r)
                rv = r
            lc, rc = rc, lc
            lrc, rrc = rrc, lrc
            lv, rv = rv, lv
        ret = max(ret, rc, lc, rrc, lrc)
    return ret

def ev_eq(b):
    b2 = [b, T.rotC(b)]
    
    ret = 0
    for b in b2:
        for l in b:
            lv = 0
            for i in l:
                if lv == 0:
                    lv = i
                elif i == 0:
                    continue
                elif lv == i:
                    ret += 1
                    lv = i
                else:
                    lv = i
    return ret
              
def ev_cm(b):
    mv = max(max(l) for l in b)
    if mv in [b[0][0], b[0][3], b[3][0], b[3][3]]:
        return 10
    else:
        return 0

def ev_cmmax(b):
    return ev_max(b) * ev_cm(b)

def ev_hole(b):
    return sum(sum(1 for x in l if x == 0) for l in b)

def ev_di(b):
    b2 = [b, T.rotC(b)]
    
    ret = 0
    for b in b2:
        for l in b:
            ib = 0
            for i in l:
                if ib == 0:
                    ib = i
                elif i == 0:
                    pass
                else:
                    a = math.log(ib, 2)
                    b = math.log(i , 2)
                    ret += min(a, b) - max(a,b) + 1
    return ret

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

