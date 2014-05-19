# -*- coding: utf-8 -*-
import urllib2
import json

class API:
    def __init__(self, serv):
        self.serv = serv
        st = self.start()
        self.sess = st['session_id']
        self.update(st)

    def start(self):
        return json.loads(urllib2.urlopen(serv + "/hi/start/json").read())

    def move(self, d):
        st = json.loads(urllib2.urlopen(serv + "/hi/state/" + self.sess + "/move/" + d + "/json").read())
        self.update(st)
        return self.board

    def update(self, st):
        self.board = st['grid']
        self.over = st['over']
        self.moved = st['moved']

