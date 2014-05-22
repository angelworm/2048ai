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
import operator as op
import sys

sys.setrecursionlimit(100000)

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

def safe_log(x):
    if x <= 0:
        ev2 = x
    return math.log(x, 2)


ev_binop = {
    '*':     op.mul,
    '**':    op.pow,
    '+':     op.add,
    '-':     op.sub,
    '/':     op.truediv,
    'max':   max,
    'min':   min
}

board_index_variable = ['b' + str(x) for x in range(16)]
number_variable = [str(x) for x in range(20)]

def ev_gen():
    values = ['log', 'abs']
    values += board_index_variable
    values += list(ev_binop.keys()) * 4
    values += number_variable
    v = random.choice(values)
    if v in board_index_variable:
        return [v]
    elif v in number_variable:
        return [v]
    elif v in ['log', 'abs']:
        return [v, ev_gen()]
    elif v in ev_binop.keys():
        return [v, ev_gen(), ev_gen()]

def ev_eval_sub(ev, e_var):
    if not isinstance(ev, list) and ev in number_variable:
        return float(ev)
    elif not isinstance(ev, list) and ev in board_index_variable:
        return e_var[int(ev[1:])]
    elif ev[0] in ['abs']:
        return abs(ev_eval_sub(ev[1], e_var))
    elif ev[0] in ['log']:
        return safe_log(ev_eval_sub(ev[1], e_var))
    elif ev[0] in ev_binop.keys():
        return ev_binop[ev[0]](ev_eval_sub(ev[1], e_var), ev_eval_sub(ev[2], e_var))
    else:
        return ev_eval_sub(ev[0], e_var)
 
def ev_eval(ev, b):
    e_var = list(b.flat)
    return ev_eval_sub(ev, e_var)

def ev_choice(ev, p=0.3):
    if random.randrange(2) == 1:
        return ev
    elif not isinstance(ev, list):
        return [ev]
    elif len(ev) == 1:
        return ev
    else:
        return random.choice(list(ev_choice(x) for x in ev[1:]))

def ev_cross(ev1, ev2, p=0.3):
    if p > random.random():
        return ev_choice(ev2, p)
    elif not isinstance(ev1, list):
        return [ev1]
    elif len(ev1) == 1:
        return ev1
    else:
        ret = [ev1[0]]
        for e in ev1[1:]:
            ret.append(ev_cross(e, ev2, p))
        return ret

nullcache = lambda:{"a":{}, "b":{}, "mv":{}, "ev":{}}

def guessN(b1, evf, n=4, player=True, a=float('-inf'), b=float('inf'), c=nullcache()):
    h = hash(str(b1) + str(n))
    
    if n == 0:
        if h not in c["ev"]:
            c["ev"][h] = ev_eval(evf, b1)
        return c["ev"][h]
    
    if player:
        if h not in c["a"]:
            for b2 in [T.mov(b1, d) for d in range(4) if cmv(b1, d)]:
                a = max(a, guessN(b2, evf, n=n-1, player=not player, a=a, b=b, c=c))
                if a >= b:
                    c["a"][h] = b
                    break
            else:
                c["a"][h] = a
        return c["a"][h]

    else:
        if h not in c["b"]:
            for b2 in inp(b1, [2,4]):
                b = min(b, guessN(b2, evf, n=n-1, player=not player, a=a, b=b,c=c))
                if a >= b:
                    c["b"][h] = a
                    break
            else:
                c["b"][h] = b
        return c["b"][h]

def guess(b, evf, n=4):
    cache = nullcache()
    b2 = []
    for d in range(4):
        if cmv(b, d):
            b2.append((guessN(T.mov(b, d), evf, n=n, player=False, c=cache), d))
    
    if b2 == []:
        return -1,0
    else:
        return max(b2, key=lambda x:x[0])

def di(d):
    return ["↑", "→", "↓", "←"][d]

def run2048(evf=ev_gen(), n=2, p=True, API=T.API):
    a = API("http://localhost:8080")
    
    try:
        while not a.over:
            gv, gd = guess(a.board, evf, n=n)
            if p:
                print("EV: " + str(evf) + " guess: " + di(gd) + "(" + str(gv) + ")")
                PM(a.board)
            a.move(str(gd))
            if not a.moved:
                print("failed to move: " + di(gd))
                PM(a.board)
    except Exception as e:
        if p:
            print(e)
            print("everr:" + str(evf))
        return 0
                
    if p:
        print("over:" + str(a.score))
        PM(a.board)
    return a.score

def lr(evf=ev_gen()):
    run2048(evf=evf, API=T.Local)

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
