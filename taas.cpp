#include <iostream>
#include <cmath>
#include <cstdlib>
#include <memory>
#include "taas.h"

static taas::board nullboard{{{{0,0,0,0}}, {{0,0,0,0}}, {{0,0,0,0}}, {{0,0,0,0}}}};

using namespace taas;

board rotC1(const board& b) {
  auto ret = nullboard;
  for(int y = 0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      ret[x][3-y] = b[y][x];
    }
  }
  return ret;
}
board rotC2(const board& b) {
  auto ret = nullboard;
  for(int y = 0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      ret[3-y][3-x] = b[y][x];
    }
  }
  return ret;
}
board rotC3(const board& b) {
  auto ret = nullboard;
  for(int y = 0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      ret[3-x][y] = b[y][x];
    }
  }
  return ret;
}

board taas::rot(const board& b, int d) {
  switch(d) {
  case 0: return b;
  case 1: return rotC1(b);
  case 2: return rotC2(b);
  case 3: return rotC3(b);
  }
  return b;
}

std::pair<board, int> movL(const board& b) {
  board ret = nullboard;
  int score_inc = 0;
  for(int y=0; y < 4; y++) {
    int ml = 0;
    for(int x = ml; x < 4; x++) {
      if(b[y][x] == 0) {
        // n.t.d
      } else if(ret[y][ml] == 0) {
        ret[y][ml] = b[y][x];
      } else if(ret[y][ml] == b[y][x]) {
        ret[y][ml] = b[y][x] * 2;
		score_inc += ret[y][ml];
        ml++;
      } else {
        ret[y][ml+1] = b[y][x];
        ml++;
      }
    }
  }
  return std::make_pair(ret, score_inc);
}

std::pair<board, int> mov_score(const board& b, const int d) {
  int l,r;
  switch(d) {
  case 0: l = 1; r = 3; break;
  case 1: l = 2; r = 2; break;
  case 2: l = 3; r = 1; break;
  case 3: l = 0; r = 0; break;
  }
  auto b2 = movL(rot(b, r));
  b2.first = rot(b2.first, l);
  return b2;
}

board taas::mov(const board& b, const int d) {
  return mov_score(b, d).first;
}

std::vector<std::pair<int, int>> taas::hole(const board& b) {
  std::vector<std::pair<int, int>> ret;
  for(int y=0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      if(b[y][x] == 0) {
        ret.push_back(std::make_pair(y, x));
      }
    }
  }
  return ret;
}

template<class Gen>
void taas::inp(board &b, const std::vector<int>& data, const std::vector<double>& ps, Gen& g) {
  auto h = taas::hole(b);
  if(h.empty()) return;
  int mi = std::rand() % h.size();
  int vi = taas::rwc(data, ps, g);
  auto i = h[mi];
  b[i.first][i.second] = vi;
}

int taas::score(const board& b) {
  int ret = 0;
  for(auto l:b) {
    for(auto x:l) {
      ret += x * std::log(x) / std::log(2);
    }
  }
  return ret;
}

bool taas::over(const board& b) {
  int yb[] = {0,0,0,0};           
  for(auto l:b) {
    int xb = 0, i = 0;
    for(auto x:l) {
      if(x == 0)  return false;
      if(xb == x) return false;
      if(yb[i] == x) return false;
      yb[i] = x;
      xb = x;
      i++;
    }
  }
  return true;
}

void taas::pb(const board& b) {
  for(auto l:b) {
    for(auto x:l) {
      std::cout << x << ' ';
    }
    std::cout << std::endl;
  }
}

taas::Local::Local()
  :over(false), moved(false), score(0), b(nullboard) {
  std::random_device sg;
  this->g.seed(sg());
  auto v = {2, 4};
  auto p = {0.9, 0.1};
  inp(this->b, v, p, this->g);
  inp(this->b, v, p, this->g);
}

bool taas::Local::move(int d) {
  board bb = this->b;
  auto v = {2, 4};
  auto p = {0.9, 0.1};

  auto b = mov_score(this->b, d);
  this->b = b.first;
  this->score = b.second;
  this->moved = (this->b != bb);

  inp(this->b, v, p, this->g);
  this->over  = taas::over(this->b);
  return this->moved;
}
