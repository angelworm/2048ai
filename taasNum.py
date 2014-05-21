# -*- coding: utf-8 -*-
import urllib as urllib2
import urllib.request as urllib2
import json
import random
import bisect
import math
import numpy as np

def rwc(l):
    total = 0
    ps = []
    it = []

    for item, p in l:
        total += p
        ps.append(total)
        it.append(item)

    return it[bisect.bisect(ps, random.random() * total)]

def rotC(b):
    """ 時計回りに回転 """
    return np.rot90(b, 3)
def rotA(b):
    """ 反時計回りに回転 """
    return np.rot90(b, 1)
def rotR(b):
    """ 左右反転 """
    return np.fliplr(b)

def movL(b):
    """ 左に動く """
    ret = np.zeros((4,4), dtype=np.int)
    for y in range(4):
        minl = 0
        for x in range(minl, 4):
            if b[y,x] == 0:
                pass
            elif ret[y, minl] == 0:
                ret[y, minl] = b[y, x]
            elif ret[y, minl] == b[y, x]:
                ret[y, minl] = b[y, x] * 2
                minl += 1
            else:
                minl += 1
                ret[y, minl] = b[y, x]
                
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

def over(b):
    yb = [0,0,0,0]
    for y in range(4):
        xb = 0
        for x in range(4):
            bi = b[y, x]
            if bi == 0:
                return False
            if xb == bi:
                return False
            if yb[x] == bi:
                return False
            yb[x] = bi
            xb = bi
    return True

def score(b):
    return sum(int(math.log(x, 2) - 1) * x for x in np.nditer(b) if x != 0)

def hole_index(b):
    return list(zip(*np.nonzero(b == 0)))

def inp(b):
    y, x = random.choice(hole_index(b))
    v = rwc([(2, 0.9), (4, 0.1)])
    b[y, x] = v
    return b

class Local:
    def __init__(self, serv="N/A"):
        self.start()
        self.score_update()
        self.sess = "N/A"
        self.over = False
        self.moved = False
    
    def score_update(self):
        self.score = score(self.board)
        return self.score

    def start(self):
        self.board = np.zeros((4,4), dtype=np.int)
        
        for _ in range(2):
            inp(self.board)
        
        return self.board
        
    def move(self, d):
        
        b2 = mov(self.board, int(d))
        self.moved = not np.all(b2 == self.board)
        if not self.moved:
            return b2
        b2 = inp(b2)
        self.board = b2
        self.score_update()
        self.over = over(self.board)
        return b2

class API:
    def __init__(self, serv):
        self.serv = serv
        st = self.start()
        self.sess = st['session_id']
        self.update(st)

    def start(self):
        return json.loads(str(urllib2.urlopen(str(self.serv) + "/hi/start/json").read(), 'ascii'))

    def move(self, d):
        st = json.loads(str(urllib2.urlopen(self.serv + "/hi/state/" + self.sess + "/move/" + str(d) + "/json").read(), 'ascii'))
        self.update(st)
        return self.board

    def update(self, st):
        self.board = np.array(st['grid'], dtype=np.int)
        self.over = st['over']
        self.moved = st['moved']
        self.score = st['score']
