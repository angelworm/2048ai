# -*- coding: utf-8 -*-
import random
import copy
import urllib as urllib2
import urllib.request as urllib2
import json
import math

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
            else:
                ret[y][minl+1] = b[y][x]
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
    
def cmv(b,d):
    return b != mov(b,d)

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
            ret.add(finalize(mov(b2, d)))
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
    
class API:
    def __init__(self, serv):
        self.serv = serv
        st = self.start()
        self.sess = st['session_id']
        self.update(st)

    def start(self):
        return json.loads(str(urllib2.urlopen(self.serv + "/hi/start/json").read(), 'ascii'))

    def move(self, d):
        st = json.loads(str(urllib2.urlopen(self.serv + "/hi/state/" + self.sess + "/move/" + d + "/json").read(), 'ascii'))
        self.update(st)
        return self.board

    def update(self, st):
        self.board = st['grid']
        self.over = st['over']
        self.moved = st['moved']
        self.score = st['score']

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

    b2 = [b, rotC(b)]
    
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
    b2 = [b, rotC(b)]
    
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
    b2 = [b, rotC(b)]
    
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

def guessN(b, n=3, w=defaultweight):
    def acc(bcs):
        bcs = list(bcs)
        if len(bcs) == 0:
            return 0
        return sum(bcs) / len(bcs)

    b2 = inp(definelize(b), [2,4])
    
    if n == 0:
        #return acc(evf(x, w) for x in b2)
        return evf(b, w)
    
    ret = 0
    for d in range(4):
        dcs = []
        b2s = set(finalize(mov(x, d)) for x in b2 if cmv(x, d))
        for b3 in b2s:
            dcs.append(guessN(b3, n-1, w))
        ret = max(ret, acc(dcs))
    return ret

def guess(b, n=2, w=defaultweight):
    b2 = [(guessN(mov(b, d), n=n, w=w), d) for d in range(4) if cmv(b, d)]
    if b2 == []:
        return -1,0
    else:
        return max(b2, key=lambda x:x[0])

def di(d):
    return ["↑", "→", "↓", "←"][d]

def run2048(w=defaultweight, p=True):
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

def proc(Q):
    while True:
        mode, data = Q.get()
        pass
        Q.task_done()

def main():
    run2048()

def mkb(s):
    return [[int(x) for x in i.split()] for i in s.split(',')]

if __name__ == '__main__':
    pass
    #main()

