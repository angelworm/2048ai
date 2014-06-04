#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <map>
#include <cassert>
#include <ctime>
#include <limits>
#include <queue>
#include <set>
#include <cstdint>
#include <functional>
#include <getopt.h>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <fstream>

#include "taas.h"

enum class ev_name{
  e_0 = 0, e_1, e_2, e_3, e_4, e_5, e_6, e_7, e_8, e_9,
    e_b0, e_b1, e_b2, e_b3,
    e_b4, e_b5, e_b6, e_b7,
    e_b8, e_b9, e_b10,e_b11,
    e_b12,e_b13,e_b14,e_b15,
	ev_score, ev_sclg, ev_max, ev_cm, ev_cm_max, ev_step, ev_hole, ev_eq, ev_di, 
	
    e_log, e_abs, e_po2,
    e_mul, e_add, e_sub, e_div,
    size
    };
static const std::vector<std::string> ev_name_str = {
  "e_0", "e_1", "e_2", "e_3", "e_4", "e_5", "e_6", "e_7", "e_8", "e_9", "e_b0", "e_b1", "e_b2", "e_b3", "e_b4", "e_b5", "e_b6", "e_b7", "e_b8", "e_b9", "e_b10", "e_b11", "e_b12", "e_b13", "e_b14", "e_b15", "ev_score", "ev_sclg", "ev_max", "ev_cm", "ev_cm_max", "ev_step", "ev_hole", "ev_eq", "ev_di", "e_log", "e_abs", "e_po2", "e_mul", "e_add", "e_sub", "e_div"
};

namespace std {
  template<>
  struct hash<ev_name>{
	size_t operator()(const ev_name& e) const {
	  return static_cast<size_t>(e);
	}
  };
}

using score = double;

double ev_score(const taas::board& b) {
  return taas::score(b);
}

double ev_sclg(const taas::board& b) {
  double ret = 0;
  for(auto const& l:b) {
	for(auto const& x:l) {
	  int x2 = x == 0 ? 1 : x;
	  double s = x * std::log(x2) / std::log(2);
	  s = s == 0 ? 1 : s;
	  ret += log(s) / std::log(2);
	}
  }
  return ret;
}

double ev_max(const taas::board &b) {
  double ret = 0;
  for(auto const& l:b) {
    ret= std::fmax(ret, *std::max_element(l.begin(), l.end()));
  }
  return ret;
}

double ev_cm(const taas::board &b) {
  double max = ev_max(b);
  for(auto const& bx:{b[0][0], b[3][0], b[0][3], b[3][3]})
	if(bx == max) return 10;
  return 0;
}

double ev_cm_max(const taas::board &b) {
  return ev_max(b) * ev_cm(b);
}

void ev_step_sub_board_rev(taas::board &b) {
  for(int i = 0; i < 8; i++) {
	int ir = 15-i;
	std::swap(b[i/4][i%4], b[ir/4][ir%4]);
  }
}

double ev_step_sub(const taas::board &b) {
  int bx = -1;
  int score = 0;
  
  for(auto const& l:b) {
	for(auto const& x:l) {
	  if(bx == -1) {
		// n.t.d
	  } else if(bx == 0) {
		return score;
	  } else if (bx == x) {
		score += std::log(bx) / std::log(2) * 2;
	  } else if (bx == x * 2) {
		score += std::log(bx) / std::log(2);
	  } else {
		return score;
	  }
	  bx = x;
	}
  }
  return score;
}

double ev_step(const taas::board &b) {
  int ret = 0;
  taas::board br;
  for(int i = 0; i < 4; i++) {
	taas::rot(b, br, i);
	ret = std::fmax(ret, ev_step_sub(br));
	ev_step_sub_board_rev(br);
	ret = std::fmax(ret, ev_step_sub(br));
  }
  return ret;
}

double ev_hole(const taas::board& b) {
  int r = 0;
  for(auto const& l:b) {
	for(auto const& x:l) {
	  if(x == 0) {
		r += 1;
	  }
	}
  }
  return r;
}

double ev_eq(const taas::board& b) {
  int r = 0;
  std::array<int, 4> lb = {-1, -1, -1, -1};
  for(int y = 0; y < 4; y++) {
	int xb = -1;
	for(int x = 0; x < 4; x++) {
	  if(b[y][x] == xb) {
		r += 1;
	  }
	  if(b[y][x] == lb[x]) {
		r += 1;
	  }
	  xb = b[y][x];
	}
	lb = b[y];
  }
  return r;
}

double ev_di(const taas::board& b) {
  int r = 0;
  std::array<int, 4> lb = {-1, -1, -1, -1};
  for(int y = 0; y < 4; y++) {
	int xb = -1;
	for(int x = 0; x < 4; x++) {
	  r -= xb == -1    ? 0 : std::abs(b[y][x] == xb);
	  r -= lb[x] == -1 ? 0 : std::abs(b[y][x] == lb[x]);
	  xb = b[y][x];
	}
	lb = b[y];
  }
  return r;
}

static std::map<ev_name, std::function<double(const taas::board&)> > ev_primitive_table {
  {ev_name::ev_score, ev_score},
  {ev_name::ev_sclg, ev_sclg},
  {ev_name::ev_max, ev_max},
  {ev_name::ev_cm, ev_cm},
  {ev_name::ev_cm_max, ev_cm_max},
  {ev_name::ev_step, ev_step},
  {ev_name::ev_hole, ev_hole},
  {ev_name::ev_eq, ev_eq},
  {ev_name::ev_di, ev_di},
};

#pragma mark - EV Trees

struct Ev {
  ev_name name;
  std::vector<std::shared_ptr<Ev> > ch;
};

using Ev_p = std::shared_ptr<Ev>;

Ev_p ev_gen() {
  Ev_p ret = std::make_shared<Ev>(Ev());
  std::vector<ev_name> g;
  for(auto const& i: {ev_name::e_0, ev_name::e_1, ev_name::e_2, ev_name::e_3, ev_name::e_4, ev_name::e_5, ev_name::e_6, ev_name::e_7, ev_name::e_8, ev_name::e_9, ev_name::e_b0, ev_name::e_b1, ev_name::e_b2, ev_name::e_b3, ev_name::e_b4, ev_name::e_b5, ev_name::e_b6, ev_name::e_b7, ev_name::e_b8, ev_name::e_b9, ev_name::e_b10, ev_name::e_b11, ev_name::e_b12, ev_name::e_b13, ev_name::e_b14,ev_name::e_b15, ev_name::ev_score, ev_name::ev_sclg, ev_name::ev_max, ev_name::ev_cm, ev_name::ev_cm_max, ev_name::ev_step, ev_name::ev_hole, ev_name::ev_eq, ev_name::ev_di} )
    g.push_back(i);
  for(int c = 0; c < 6; c++)
    for(auto const& i: {ev_name::e_log, ev_name::e_abs, ev_name::e_po2, ev_name::e_mul, ev_name::e_add, ev_name::e_sub, ev_name::e_div} )
      g.push_back(i);

  ret->name = g[std::rand() % g.size()];
  switch(ret->name) {
  case ev_name::e_mul: case ev_name::e_add:
  case ev_name::e_sub: case ev_name::e_div:
    (ret->ch).push_back(ev_gen());
  case ev_name::e_log: case ev_name::e_abs:
  case ev_name::e_po2:
    (ret->ch).push_back(ev_gen());
  default:
    break;
  }
  return ret;
}

Ev_p ev_copy(Ev_p a) {
  Ev_p ret = std::make_shared<Ev>(Ev());
  ret->name = a->name;
  for(auto const& i:a->ch) {
    ret->ch.push_back(ev_copy(i));
  }
  return ret;
}

int ev_size(Ev_p a) {
  int ret = 1;
  for(auto const& x:a->ch) {
    ret += ev_size(x);
  }
  return ret;
}

Ev_p ev_select(Ev_p a, int ind) {
  assert(ind < ev_size(a));

  if(ind == 0) {
    return a;
  } else {
    ind -= 1;
    for(auto const& i:a->ch) {
      if(ind < ev_size(i)) {
        return ev_select(i, ind);
      }
      ind = ind - ev_size(i);
    }
	std::cout << "ev_select index overflow" << std::endl;
    return ev_copy(a);
  }
}

Ev_p ev_inp(Ev_p a, Ev_p b, int n) {
  if(n < 0) {
	return ev_copy(a);
  } else if(n == 0) {
    return ev_copy(b);
  } else {
    Ev_p ret = std::make_shared<Ev>(Ev());
    ret->name = a->name;
    n = n-1;
    for(auto const& i:a->ch) {
       ret->ch.push_back(ev_inp(i, b, n));
	   n = n - ev_size(i);
    }
    return ret;
  }
}

template<class Gen>
std::array<Ev_p, 2> ev_cross1(Ev_p a, Ev_p b, Gen& g) {
  int ai = std::uniform_int_distribution<int>(0, ev_size(a) - 1)(g);
  int bi = std::uniform_int_distribution<int>(0, ev_size(b) - 1)(g);
  return {{ev_inp(a, ev_select(b, bi), ai), ev_inp(b, ev_select(a, ai), bi)}};
}

template<class Gen>
std::array<Ev_p, 2> ev_mut(Ev_p a, Gen& g) {
  Ev_p b = ev_gen();
  int ai = std::uniform_int_distribution<int>(0, ev_size(a) - 1)(g);
  int bi = std::uniform_int_distribution<int>(0, ev_size(b) - 1)(g);

  return {{ev_inp(a, ev_select(b, bi), ai), ev_inp(b, ev_select(a, ai), bi)}};
}

using ev_env = std::unordered_map<ev_name, double>;

double ev_eval_sub(Ev_p a, ev_env& ev_t) {
  double v;
  switch(a->name) {
  case ev_name::e_0: case ev_name::e_1: case ev_name::e_2: case ev_name::e_3: case ev_name::e_4: case ev_name::e_5: case ev_name::e_6: case ev_name::e_7: case ev_name::e_8: case ev_name::e_9:
  case ev_name::e_b0: case ev_name::e_b1: case ev_name::e_b2: case ev_name::e_b3: case ev_name::e_b4: case ev_name::e_b5: case ev_name::e_b6: case ev_name::e_b7: case ev_name::e_b8: case ev_name::e_b9: case ev_name::e_b10: case ev_name::e_b11: case ev_name::e_b12: case ev_name::e_b13: case ev_name::e_b14: case ev_name::e_b15: case ev_name::ev_score: case ev_name::ev_sclg: case ev_name::ev_max: case ev_name::ev_cm: case ev_name::ev_cm_max: case ev_name::ev_step: case ev_name::ev_hole: case ev_name::ev_eq: case ev_name::ev_di: 
    return ev_t[a->name];
  case ev_name::e_log: 
    v = ev_eval_sub(a->ch[0], ev_t);
    v = (v <= 1 ? 1 : v);
    return std::log(v) / std::log(2);
  case ev_name::e_abs: return std::abs(ev_eval_sub(a->ch[0], ev_t));
  case ev_name::e_po2: return std::pow(ev_eval_sub(a->ch[0], ev_t), 2);
  case ev_name::e_mul: return ev_eval_sub(a->ch[0], ev_t) * ev_eval_sub(a->ch[1], ev_t);
  case ev_name::e_add: return ev_eval_sub(a->ch[0], ev_t) + ev_eval_sub(a->ch[1], ev_t);
  case ev_name::e_sub: return ev_eval_sub(a->ch[0], ev_t) - ev_eval_sub(a->ch[1], ev_t);
  case ev_name::e_div: 
    v = ev_eval_sub(a->ch[1], ev_t);
    if(v == 0)
      return 0;
    else
      return ev_eval_sub(a->ch[0], ev_t) / v;
  default:
    assert(std::string("no such ev name") == "");
    return 0;
  }
}

double ev_eval(Ev_p a, const taas::board& b) {  
  ev_env t {
	{ev_name::e_0, 0}, {ev_name::e_1, 0}, {ev_name::e_2, 2}, {ev_name::e_3, 3}, {ev_name::e_4, 4}, {ev_name::e_5, 5}, {ev_name::e_6, 6}, {ev_name::e_7, 7}, {ev_name::e_8, 8}, {ev_name::e_9, 9}, {ev_name::e_b0, b[0][0]}, {ev_name::e_b1, b[0][1]}, {ev_name::e_b2, b[0][2]}, {ev_name::e_b3, b[0][3]}, {ev_name::e_b4, b[1][0]}, {ev_name::e_b5, b[1][1]}, {ev_name::e_b6, b[1][2]}, {ev_name::e_b7, b[1][3]}, {ev_name::e_b8, b[2][0]}, {ev_name::e_b9, b[2][1]}, {ev_name::e_b10, b[2][2]}, {ev_name::e_b11, b[2][3]}, {ev_name::e_b12, b[3][0]}, {ev_name::e_b13, b[3][1]}, {ev_name::e_b14, b[3][2]}, {ev_name::e_b15, b[3][3]}};

  for(auto const& pev: ev_primitive_table) {
	double evv = pev.second(b);
	assert(evv != -std::numeric_limits<double>::infinity());
	t[pev.first] = evv;
  }
  
  return ev_eval_sub(a, t);
}

std::string ev_show(Ev_p a) {
  std::string ret = "(" + ev_name_str[static_cast<int>(a->name)];
  for(auto const& i:a->ch) {
    ret += " " + ev_show(i);
  }
  return ret + ")";
}
 
ev_name ev_parse_tok(std::string::const_iterator& begin, std::string::const_iterator end) {
  assert(*begin != '(');
  auto head = begin;
  while(begin != end and *begin != ' ' and *begin != ')' and *begin != '(') begin++;

  std::string name(head, begin);

  for(size_t i = 0; i < ev_name_str.size(); i++) {
	if(ev_name_str[i] == name) {
	  return static_cast<ev_name>(i);
	}
  }
  throw std::runtime_error(std::string("no token: ") + name);
}

Ev_p ev_parse_brak(std::string::const_iterator& begin, std::string::const_iterator end) {
  Ev_p ret = std::make_shared<Ev>(Ev());
  if(*begin != '(') {
	std::stringstream ss;
	ss << "parse error at: " << std::string(begin, end);
	throw std::runtime_error(ss.str());
  }

  begin++;
  ret->name = ev_parse_tok(begin, end);

  while(begin != end and *begin == ' ') {
	begin++;
	ret->ch.push_back(ev_parse_brak(begin, end));
  }
  
  if(begin == end or *begin != ')') {
	std::stringstream ss;
	ss << "parse error at: " << std::string(begin, end);
	throw std::runtime_error(ss.str());
  } else {
	begin++;
  }
  return ret;
}

Ev_p ev_parse(const std::string& str) {
  auto begin = str.cbegin();
  return ev_parse_brak(begin, str.cend());
}

std::uint64_t ev_hash(Ev_p a) {
  std::uint64_t ret = 0;
  std::uint64_t tmp = 0;
  int count = 0;
  std::queue<Ev_p> Q;
  Q.push(a);

  while(not Q.empty()) {
    auto b = Q.front();
    Q.pop();
    tmp *= static_cast<int>(ev_name::size);
    tmp += static_cast<int>(b->name);

    for(auto const& v:b->ch) {
      Q.push(v);
    }

    count++;
    if(count >= 12) {
      count = 0;
      ret ^= tmp;
      tmp = 0;
    }
  }
  return ret ^ tmp;
}

#pragma mark - alpha beta

double guessN(taas::board &b1, Ev_p evf, int n=4, bool player=true, double a = -std::numeric_limits<double>::infinity(), double b=std::numeric_limits<double>::infinity()) {
  if(n == 0)
    return ev_eval(evf, b1);
    
  if(player) {
    for(auto const& d:{0,1,2,3}) {
      taas::board b2; 
	  taas::mov(b1, b2, d);
      double atmp = guessN(b2, evf, n-1, not player, a, b);
      a = std::fmax(a, atmp);
      if(a >= b)
        return b;
    }
    return a;
  } else {
    for(auto const& ind: taas::hole(b1)) {
      for(auto const& v:{2,4}) {
        taas::board b2 = b1;
        b2[ind.first][ind.second] = v;
        double btmp = guessN(b2, evf, n-1, not player, a, b);
        b = std::fmin(b, btmp);
        if(a >= b) {
          return a;
        }
      }
    }
    return b;
  }
}

int guess(taas::board& b, Ev_p ev, int n=4) {
  double score = -std::numeric_limits<double>::infinity();
  int dir = -1;
# pragma omp parallel for
  for(int i = 0; i < 4; i++) {
    taas::board b2;
	taas::mov(b, b2, i);
    if(b2 != b) {
      double scoret = guessN(b2, ev, n, false);
#     pragma omp critical
      {
        if(scoret > score) {
          score = scoret;
          dir = i;
        }
      }
    }
  }
  assert(dir != -1);
  return dir;
}
 
template<class API>
int run2048(API& a, Ev_p evf, bool show=true) {
   try {
     while(not a.over) {
       int d = guess(a.b, evf, 2);
       if(show) {
         std::cout << d << std::endl;
         taas::pb(a.b);
       }
       a.move(d);
	   assert(a.moved);
     }
   }catch(const std::exception& e) {
	 std::cout << e.what() << std::endl;
     return 0;
   }
   if(show) {
	 std::cout << std::endl;
	 taas::pb(a.b);
	 std::cout << "score:" << a.score << std::endl;
   }
   return a.score;
}

#pragma mark - avg guess

using ev_cache = std::unordered_map<std::size_t, double>;

double guess_avg_sub(const taas::board& b, Ev_p evf, int n, ev_cache& ca) {
  static std::hash<taas::board> hash;
  if(n == 0) {
	auto h = hash(b);
	auto it = ca.find(h);
	if(it == ca.end()) {
	  auto e = ev_eval(evf, b);
#     pragma omp critical
      {
		ca.emplace(h, e);
	  }
	  return e;
	} else {
	 return it->second;
	}
  }

  std::array<std::unordered_set<taas::board>, 4> nbs;

  for(auto const& ind: taas::hole(b)) {
	for(auto const& v:{2,4}) {
	  taas::board b2 = b;
	  b2[ind.first][ind.second] = v;

	  for(int d = 0; d < 4; d++) {
		taas::board b3;
		taas::mov(b2, b3, d);
		if(b2 != b3)
		  nbs[d].insert(b3);
	  }
	}
  } 

  double ret = 0;
  for(auto& bs:nbs) {
    double score_sum = 0;
	int count = 0;
	for(const taas::board& bn:bs) {
	  score_sum += guess_avg_sub(bn, evf, n-1, ca);
	  count++;
	}
    ret = std::fmax(ret, score_sum / count);
  }
  return ret;
}

double guess_avg(taas::board& b, Ev_p evf, int n, ev_cache& ca) {
  double score = -std::numeric_limits<double>::infinity();
  int dir = -1;
# pragma omp parallel for shared(ca)
  for(int i = 0; i < 4; i++) {
	ev_cache ca2;
    taas::board b2;
	taas::mov(b, b2, i);
    if(b2 != b) {
      double scoret = guess_avg_sub(b2, evf, n, ca2);
#     pragma omp critical
      {
        if(scoret > score) {
          score = scoret;
          dir = i;
        }
      }
    }
  }
  assert(dir != -1);
  return dir;
}

template<class API>
int run2048_avg(API& a, Ev_p evf, bool show=true) {
   ev_cache ca;
   try {
     while(not a.over) {
       int d = guess_avg(a.b, evf, 1, ca);
       if(show) {
         std::cout << d << "(" << ca.size() << ")" << std::endl;
         taas::pb(a.b);
       }
       a.move(d);
	   assert(a.moved);
     }
   }catch(const std::exception& e) {
	 std::cout << e.what() << std::endl;
     return 0;
   }
   if(show) {
	 std::cout << std::endl;
	 taas::pb(a.b);
	 std::cout << "score:" << a.score << std::endl;
   }
   return a.score;
}


#pragma mark - gene

void addgene(Ev_p a, std::vector<Ev_p>& list, std::set<std::uint64_t>& hash){
  std::uint64_t h = ev_hash(a);
  if(hash.find(h) == hash.end()) {
    list.push_back(ev_copy(a));
    hash.insert(h);
  }
}

void analyze(const std::vector<Ev_p>& g, const std::vector<double>& w, const int score_sum) {
  std::vector<std::pair<Ev_p, double> > gene;

  for(size_t i = 0; i < g.size(); i++) {
    gene.push_back(std::make_pair(g[i], w[i]));
  }

  std::sort(gene.begin(), gene.end(), [](std::pair<Ev_p, double> x,
                                         std::pair<Ev_p, double> y) {
        return x.second > y.second;
    });

  for(int i = 0; i < 3; i++) {
    std::cout << "score: " << gene[i].second << "\tEV: " << ev_show(gene[i].first) << std::endl;
  }

  auto it = std::remove_if(gene.begin(), gene.end(), [](std::pair<Ev_p, double> x) {
      return x.second == 0;
    });  

  auto minmax = std::minmax_element(gene.begin(), it, [](std::pair<Ev_p, double> x,
														 std::pair<Ev_p, double> y) {
      return x.second > y.second;
    });
  int size_sum = 0;
  std::for_each(gene.begin(), it, [&](std::pair<Ev_p, double> x){
      size_sum += ev_size(x.first);
    });
  std::cout << "max: " << minmax.first->second
            << "\tmed: " << gene[(it - gene.begin()) / 2].second
            << "\tavg: " << score_sum / (it - gene.begin()) 
            << "\tmin: " << minmax.second->second
            << "\tts: "  << size_sum * 1.0 / (it - gene.begin()) << std::endl;
  
}

template<class Gen>
Ev_p tournament(const std::vector<Ev_p>& list, const std::vector<double>& score,
				const int n, Gen g) {
  std::uniform_int_distribution<std::size_t> dis(0, list.size() -1);
  std::unordered_set<std::size_t> selected;
  Ev_p ret = nullptr;
  int max_score = -1;
  while(selected.size() < n) {
	std::size_t i = dis(g);
	if(selected.find(i) != selected.end()) 
	  continue;
	selected.insert(i);
	if(max_score < score[i]) {
	  max_score = score[i];
	  ret = list[i];
	}
  }
  return ret;
}

std::string logpath(const std::string& log_dir, int gen) {
  if(log_dir.empty()) {
	return std::string("/dev/null");
  } else {
	return log_dir + "/" + std::to_string(gen) + ".csv";
  }
}

void grown(std::vector<Ev_p> evs={}, const std::string log_dir = {}, const int CHILD_MAX = 100) {
  std::vector<Ev_p> gene; 
  std::vector<double> weight;
  std::set<std::uint64_t> hashs;
  std::random_device sg;
  std::mt19937 g(sg());

  std::vector<int> act = {0, 1, 2};
  std::vector<double > act_w = {0.2, 0.75,  0.05};
  bool dolog = not log_dir.empty();

  for(auto const& v:evs) {
    hashs.insert(ev_hash(v));
    gene.push_back(v);
    weight.push_back(1.0/CHILD_MAX);
  }

  while(gene.size() < CHILD_MAX) {
    Ev_p b = ev_gen();
    std::uint64_t h = ev_hash(b);
    if(hashs.find(h) == hashs.end()) {
      hashs.insert(h);
      gene.push_back(b);
      weight.push_back(1.0/CHILD_MAX);
    }
  }

  for(int gen = 0; gen < 1000; gen++) {
    std::vector<Ev_p> gene2;
    std::set<std::uint64_t> hashs2;
	std::string path = logpath(log_dir, gen); 
	std::ofstream flog(path);
    
    std::cout << "***** " << gen << ":generating *****" << std::endl;
    
    while(gene2.size() < CHILD_MAX) {
      Ev_p a,b;
      switch(taas::rwc(act, act_w, g)) {
      case 0:
        a = taas::rwc(gene, weight, g);
        addgene(a, gene2, hashs2);
        break;
      case 1:
        a = taas::rwc(gene, weight, g);
        b = taas::rwc(gene, weight, g);
		for(auto const& x:ev_cross1(a, b, g))
		  addgene(x, gene2, hashs2);
        break;
      case 2:
        a = taas::rwc(gene, weight, g);
		for(auto const& x:ev_mut(a, g))
		  addgene(x, gene2, hashs2);
        break;
      }
    }
    
    std::cout << "***** " << gen << ":testing *****" << std::endl;

    gene.clear();
    weight.clear();
    int score_sum = 0;
#   pragma omp parallel for reduction(+:score_sum)
    for(int i = 0; i < CHILD_MAX; i++) {
      std::array<int, 10> scores;
      int score = 0;
      for(int j = 0; j < 10; j++) {
		taas::Local a;
        scores[j] = run2048(a, gene2[i], false);
		if(scores[j] == 0) break;
      }

	  if(std::any_of(scores.begin(), scores.end(),[](int x) {return x == 0;})) {
		score = 0;
		std::fill(scores.begin(), scores.end(), 0);
	  } else {
		std::sort(scores.begin(), scores.end(), [](int x,int y) {return x > y;});
		for(int j = 0; j < 3; j++) score += scores[j];
		score /= 3;
	  }
	  score_sum += score;

#     pragma omp critical
      {
        gene.push_back(gene2[i]);
        weight.push_back(score);
		
		flog << ev_show(gene2[i]);
		for(auto const& x:scores) 
		  flog << "," << x;
		flog << std::endl;
      }
    }
    
    analyze(gene, weight, score_sum);
  }
}

#pragma mark - main

std::vector<Ev_p> read_ev(const char* path) {
  std::ifstream ifs(path);
  std::string s;
  std::vector<Ev_p> ret;

  if(!ifs.is_open()){
	std::cerr << "no such file: " << path << std::endl;
	throw std::runtime_error("file not found");
  }
  while(std::getline(ifs, s)){
	ret.push_back(ev_parse(s));
  }
  return ret;
}

int main(int argc, char *argv[]) {
  int mode = 0, result = -1;
  std::string output_dir;
  std::vector<Ev_p> ev_log;
  std::string evstr;
  char serv[500] = {'\0'}; int port = -1;
  srand(time(NULL));

  while((result=getopt(argc,argv,"hrtd:i:e:a:")) != -1){
    switch(result){
	case 'r': // random run mode
	  mode = 1;
	  break;
	case 't': // unit test mode?
	  mode = 3;
	  break;
	case 'd': // output directory
	  output_dir = optarg;
	  break;
	case 'i': // gene file input
	  ev_log = read_ev(optarg);
	  break;
	case 'e': // ev input mode
	  mode = 2;
	  evstr = optarg;
	  break;
	case 'a': // API mode
	  std::sscanf(optarg, "%[^:]:%d", serv, &port);
	  std::cout << serv << ":" << port << std::endl;
	  break;
	case 'h':
    case '?':
	  std::cout << "useage: gp [opt]" << std::endl;
	  std::cout << "\t-h: this option" << std::endl;
	  std::cout << "\t-s: single run mode(default: grown mode)" << std::endl;
	  std::cout << "\t-t: test run mode(default: grown mode)" << std::endl;
	  std::cout << "\t-r: random run mode(default: grown mode)" << std::endl;
	  std::cout << "\t-e ev: ev eval run mode(default: grown mode)" << std::endl;
	  std::cout << "\t-a serv:port: 2048 as a service mode(default: Local mode)" << std::endl;
	  std::cout << "\t-d dir: generations writes into dir(default: none)" << std::endl;
	  std::cout << "\t-i file: read genes files as seed" << std::endl;
	  return -1;
    }
  }

  switch(mode) {
  case 0:
	grown(ev_log, output_dir);
	break;
  case 1:
	if(port == -1) {
	  taas::Local a;
	  run2048(a, ev_gen());
	} else {
	  taas::API a(std::string(serv), port);
	  run2048(a, ev_gen());
	}
	break;
  case 2:
	if(port == -1) {
	  taas::Local a;
	  run2048_avg(a, ev_parse(evstr));
	} else {
	  taas::API a(std::string(serv), port);
	  run2048_avg(a, ev_parse(evstr));
	}
	break;
  case 3:
	{
	  std::string s("(e_mul (e_log (e_abs (e_b7))) (e_add (e_po2 (e_add (e_sub (e_add (e_abs (e_div (e_po2 (e_7)) (e_add (e_add (e_log (e_0)) (e_po2 (e_abs (e_div (e_1) (e_div (e_log (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3))))))) (e_b12)))))) (e_add (e_po2 (e_2)) (e_log (e_abs (e_mul (e_sub (e_9) (e_b11)) (e_mul (e_po2 (e_log (e_b4))) (e_b8))))))))) (e_b3)) (e_po2 (e_div (e_b13) (e_mul (e_sub (e_add (e_add (e_log (e_div (e_log (e_b11)) (e_8))) (e_log (e_sub (e_b1) (e_sub (e_9) (e_b1))))) (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_sub (e_po2 (e_add (e_po2 (e_add (e_sub (e_add (e_abs (e_div (e_po2 (e_7)) (e_add (e_add (e_log (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3))))))) (e_po2 (e_div (e_abs (e_0)) (e_mul (e_div (e_b5) (e_div (e_log (e_b8)) (e_div (e_add (e_6) (e_b9)) (e_mul (e_div (e_abs (e_0)) (e_b6)) (e_b3))))) (e_mul (e_log (e_add (e_abs (e_div (e_po2 (e_7)) (e_add (e_add (e_log (e_po2 (e_po2 (e_0)))) (e_po2 (e_sub (e_abs (e_3)) (e_mul (e_po2 (e_b3)) (e_add (e_log (e_log (e_sub (e_abs (e_po2 (e_sub (e_add (e_5) (e_b7)) (e_b4)))) (e_mul (e_b2) (e_mul (e_b5) (e_po2 (e_7))))))) (e_b13)))))) (e_add (e_po2 (e_2)) (e_log (e_abs (e_add (e_mul (e_b2) (e_b15)) (e_b15)))))))) (e_b3))) (e_div (e_1) (e_b15))))))) (e_add (e_po2 (e_2)) (e_log (e_abs (e_mul (e_sub (e_9) (e_div (e_abs (e_log (e_log (e_po2 (e_div (e_b14) (e_abs (e_0))))))) (e_b14))) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_sub (e_b10) (e_4)))))) (e_b8))))))))) (e_b3)) (e_po2 (e_div (e_div (e_1) (e_b15)) (e_sub (e_abs (e_abs (e_po2 (e_8)))) (e_b5))))) (e_b11))) (e_8))) (e_mul (e_po2 (e_sub (e_b12) (e_b8))) (e_abs (e_po2 (e_sub (e_add (e_abs (e_div (e_po2 (e_7)) (e_add (e_add (e_log (e_po2 (e_po2 (e_5)))) (e_po2 (e_sub (e_abs (e_log (e_b0))) (e_mul (e_po2 (e_b3)) (e_add (e_log (e_log (e_sub (e_abs (e_po2 (e_sub (e_add (e_5) (e_b7)) (e_b4)))) (e_po2 (e_b3))))) (e_b13)))))) (e_add (e_po2 (e_2)) (e_log (e_abs (e_add (e_log (e_mul (e_mul (e_abs (e_div (e_div (e_mul (e_mul (e_div (e_mul (e_2) (e_b3)) (e_8)) (e_b7)) (e_b15)) (e_add (e_b4) (e_b10))) (e_b12))) (e_po2 (e_abs (e_0)))) (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_abs (e_add (e_po2 (e_sub (e_b10) (e_abs (e_log (e_po2 (e_abs (e_div (e_0) (e_b8)))))))) (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_abs (e_add (e_po2 (e_sub (e_b10) (e_4))) (e_8)))) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_po2 (e_po2 (e_po2 (e_3))))))))) (e_log (e_po2 (e_po2 (e_po2 (e_3)))))))) (e_po2 (e_log (e_add (e_mul (e_b14) (e_b3)) (e_log (e_mul (e_mul (e_abs (e_div (e_div (e_mul (e_mul (e_div (e_mul (e_2) (e_b3)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_7)))))) (e_div (e_1) (e_b15)))) (e_b7)) (e_b15)) (e_add (e_b4) (e_b10))) (e_b12))) (e_b6)) (e_mul (e_b14) (e_sub (e_b1) (e_add (e_abs (e_2)) (e_div (e_div (e_b12) (e_b14)) (e_b11)))))))))))))) (e_b1)) (e_mul (e_abs (e_5)) (e_div (e_1) (e_b15))))) (e_po2 (e_sub (e_add (e_abs (e_div (e_po2 (e_7)) (e_add (e_add (e_log (e_b12)) (e_po2 (e_div (e_abs (e_0)) (e_mul (e_sub (e_b14) (e_b2)) (e_log (e_b3)))))) (e_add (e_po2 (e_2)) (e_log (e_abs (e_mul (e_sub (e_abs (e_sub (e_abs (e_abs (e_add (e_po2 (e_sub (e_7) (e_4))) (e_8)))) (e_4))) (e_b1)) (e_mul (e_po2 (e_b14)) (e_b8))))))))) (e_b3)) (e_po2 (e_div (e_mul (e_log (e_abs (e_b7))) (e_add (e_po2 (e_add (e_sub (e_add (e_abs (e_div (e_po2 (e_7)) (e_add (e_add (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_abs (e_add (e_po2 (e_abs (e_0))) (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_abs (e_add (e_po2 (e_sub (e_b10) (e_4))) (e_8)))) (e_b1)) (e_mul (e_po2 (e_3)) (e_div (e_1) (e_b15))))) (e_po2 (e_log (e_add (e_mul (e_b14) (e_b11)) (e_log (e_mul (e_mul (e_abs (e_div (e_div (e_b12) (e_add (e_b4) (e_b10))) (e_b12))) (e_b6)) (e_add (e_add (e_b15) (e_add (e_sub (e_b10) (e_mul (e_b9) (e_b10))) (e_mul (e_add (e_add (e_log (e_div (e_div (e_b2) (e_abs (e_b3))) (e_mul (e_b15) (e_2)))) (e_mul (e_b2) (e_sub (e_b1) (e_abs (e_abs (e_log (e_po2 (e_b14)))))))) (e_div (e_sub (e_b3) (e_b6)) (e_abs (e_b11)))) (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_b11)) (e_b1)) (e_mul (e_po2 (e_div (e_div (e_b12) (e_add (e_abs (e_2)) (e_div (e_div (e_b12) (e_b14)) (e_b11)))) (e_b11))) (e_div (e_1) (e_log (e_b8))))))))) (e_div (e_log (e_log (e_log (e_7)))) (e_abs (e_mul (e_3) (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3))))))))))))))))))) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_add (e_add (e_log (e_0)) (e_po2 (e_div (e_abs (e_0)) (e_mul (e_sub (e_b14) (e_b14)) (e_mul (e_po2 (e_div (e_b14) (e_abs (e_0)))) (e_b14)))))) (e_add (e_log (e_po2 (e_mul (e_9) (e_4)))) (e_log (e_abs (e_mul (e_sub (e_9) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_mul (e_9) (e_b2))))) (e_b8))))))))))) (e_div (e_1) (e_b15))))) (e_po2 (e_log (e_b10)))) (e_po2 (e_div (e_abs (e_div (e_div (e_b0) (e_b14)) (e_b11))) (e_9)))) (e_add (e_po2 (e_2)) (e_log (e_abs (e_mul (e_log (e_7)) (e_mul (e_po2 (e_abs (e_b11))) (e_b8))))))))) (e_b3)) (e_abs (e_add (e_abs (e_div (e_po2 (e_7)) (e_add (e_add (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_po2 (e_abs (e_0))) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3)))))) (e_div (e_1) (e_b15))))) (e_po2 (e_div (e_log (e_log (e_log (e_7)))) (e_abs (e_mul (e_3) (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_sub (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_abs (e_add (e_po2 (e_po2 (e_div (e_b13) (e_sub (e_8) (e_b5))))) (e_po2 (e_div (e_sub (e_div (e_po2 (e_7)) (e_add (e_div (e_1) (e_b15)) (e_sub (e_b10) (e_b3)))) (e_sub (e_abs (e_abs (e_add (e_mul (e_b15) (e_po2 (e_0))) (e_po2 (e_div (e_sub (e_9) (e_sub (e_b11) (e_div (e_add (e_2) (e_sub (e_abs (e_sub (e_add (e_log (e_po2 (e_b2))) (e_div (e_b12) (e_b2))) (e_b0))) (e_add (e_mul (e_sub (e_sub (e_b0) (e_po2 (e_b1))) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_div (e_sub (e_b5) (e_sub (e_b11) (e_log (e_log (e_7))))) (e_sub (e_sub (e_b10) (e_log (e_add (e_0) (e_0)))) (e_b15))))))) (e_b8))) (e_add (e_add (e_log (e_add (e_2) (e_div (e_mul (e_b5) (e_abs (e_po2 (e_po2 (e_po2 (e_4)))))) (e_add (e_0) (e_div (e_b14) (e_div (e_1) (e_b15))))))) (e_po2 (e_b5))) (e_abs (e_4)))))) (e_0)))) (e_sub (e_abs (e_8)) (e_b15))))))) (e_b8))) (e_sub (e_b3) (e_b15))))))) (e_sub (e_mul (e_div (e_abs (e_0)) (e_mul (e_log (e_po2 (e_b12))) (e_7))) (e_b3)) (e_add (e_b3) (e_log (e_b7))))) (e_b2))) (e_po2 (e_abs (e_div (e_1) (e_div (e_log (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3))))))) (e_b12)))))) (e_add (e_abs (e_2)) (e_div (e_b3) (e_b11)))))))))))))) (e_b12)) (e_add (e_po2 (e_2)) (e_log (e_abs (e_mul (e_log (e_7)) (e_mul (e_add (e_po2 (e_log (e_po2 (e_po2 (e_abs (e_b11)))))) (e_log (e_sub (e_b1) (e_add (e_abs (e_2)) (e_b13))))) (e_b8))))))))) (e_b3)))) (e_b11))) (e_8))) (e_mul (e_po2 (e_div (e_abs (e_0)) (e_log (e_sub (e_po2 (e_div (e_b14) (e_abs (e_0)))) (e_b7))))) (e_div (e_1) (e_b15)))))))))) (e_b15)))))))) (e_b3)) (e_po2 (e_po2 (e_b3)))))))))))))) (e_8)) (e_div (e_po2 (e_9)) (e_add (e_mul (e_b14) (e_b11)) (e_div (e_abs (e_0)) (e_mul (e_sub (e_b14) (e_mul (e_po2 (e_sub (e_b10) (e_b3))) (e_div (e_1) (e_sub (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_add (e_log (e_div (e_div (e_log (e_abs (e_b11))) (e_abs (e_b3))) (e_mul (e_b15) (e_2)))) (e_mul (e_b2) (e_sub (e_b1) (e_abs (e_abs (e_log (e_po2 (e_b14))))))))) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3)))))) (e_div (e_1) (e_add (e_b12) (e_b15)))))) (e_po2 (e_log (e_log (e_abs (e_b7)))))) (e_add (e_abs (e_2)) (e_div (e_mul (e_b5) (e_b2)) (e_b11))))))) (e_mul (e_po2 (e_sub (e_b10) (e_b3))) (e_div (e_1) (e_sub (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_sub (e_b1) (e_abs (e_log (e_div (e_1) (e_b15))))) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3)))))) (e_div (e_1) (e_b15))))) (e_po2 (e_log (e_div (e_add (e_2) (e_po2 (e_po2 (e_div (e_mul (e_b5) (e_abs (e_mul (e_mul (e_sub (e_b6) (e_div (e_log (e_3)) (e_sub (e_b14) (e_b15)))) (e_b12)) (e_add (e_div (e_b12) (e_b2)) (e_b3))))) (e_add (e_0) (e_div (e_b14) (e_div (e_1) (e_b15)))))))) (e_abs (e_abs (e_add (e_abs (e_b11)) (e_abs (e_sub (e_log (e_b3)) (e_b1)))))))))) (e_add (e_abs (e_2)) (e_div (e_po2 (e_9)) (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_abs (e_add (e_po2 (e_sub (e_b10) (e_4))) (e_sub (e_abs (e_sub (e_add (e_log (e_po2 (e_log (e_b4)))) (e_div (e_b12) (e_b2))) (e_b6))) (e_b5))))) (e_b1)) (e_mul (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3)))))) (e_div (e_1) (e_b15))))) (e_po2 (e_log (e_add (e_mul (e_b14) (e_b11)) (e_log (e_mul (e_mul (e_abs (e_div (e_div (e_mul (e_mul (e_div (e_mul (e_2) (e_b3)) (e_b12)) (e_b7)) (e_add (e_abs (e_0)) (e_add (e_add (e_log (e_div (e_add (e_2) (e_sub (e_abs (e_mul (e_b15) (e_po2 (e_0)))) (e_add (e_po2 (e_b4)) (e_add (e_add (e_po2 (e_log (e_add (e_mul (e_b14) (e_b11)) (e_log (e_mul (e_mul (e_abs (e_mul (e_sub (e_9) (e_b1)) (e_mul (e_po2 (e_po2 (e_9))) (e_b8)))) (e_b6)) (e_add (e_add (e_b15) (e_add (e_sub (e_b10) (e_mul (e_b9) (e_b10))) (e_mul (e_add (e_add (e_log (e_div (e_div (e_b2) (e_abs (e_b3))) (e_mul (e_b0) (e_2)))) (e_mul (e_b2) (e_sub (e_b1) (e_abs (e_abs (e_log (e_po2 (e_b14)))))))) (e_div (e_sub (e_b3) (e_b6)) (e_abs (e_b11)))) (e_div (e_abs (e_0)) (e_7))))) (e_div (e_log (e_log (e_log (e_7)))) (e_abs (e_mul (e_3) (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3))))))))))))))) (e_log (e_sub (e_b1) (e_div (e_b13) (e_5))))) (e_abs (e_4)))))) (e_8))) (e_log (e_sub (e_b1) (e_div (e_b13) (e_b12))))) (e_abs (e_b10))))) (e_add (e_po2 (e_2)) (e_b10))) (e_b12))) (e_b6)) (e_add (e_add (e_b15) (e_add (e_b14) (e_mul (e_add (e_add (e_log (e_div (e_div (e_b2) (e_abs (e_b3))) (e_mul (e_b15) (e_2)))) (e_mul (e_abs (e_b6)) (e_sub (e_b1) (e_abs (e_div (e_log (e_b8)) (e_div (e_b3) (e_b14))))))) (e_div (e_b14) (e_abs (e_0)))) (e_div (e_abs (e_0)) (e_div (e_sub (e_log (e_add (e_b0) (e_log (e_div (e_5) (e_mul (e_b15) (e_log (e_b4))))))) (e_6)) (e_0)))))) (e_div (e_log (e_log (e_log (e_div (e_add (e_6) (e_b9)) (e_mul (e_div (e_abs (e_0)) (e_mul (e_sub (e_abs (e_abs (e_add (e_div (e_log (e_mul (e_6) (e_log (e_div (e_add (e_po2 (e_mul (e_log (e_mul (e_div (e_sub (e_add (e_abs (e_div (e_po2 (e_b1)) (e_add (e_b11) (e_add (e_po2 (e_2)) (e_log (e_abs (e_mul (e_log (e_7)) (e_mul (e_po2 (e_log (e_po2 (e_sub (e_add (e_abs (e_mul (e_b6) (e_9))) (e_2)) (e_0))))) (e_b8))))))))) (e_b3)) (e_b14)) (e_abs (e_b2))) (e_b2))) (e_abs (e_abs (e_sub (e_abs (e_b11)) (e_0)))))) (e_b13)) (e_b15))))) (e_abs (e_abs (e_po2 (e_abs (e_3)))))) (e_8)))) (e_b1)) (e_2))) (e_b3)))))) (e_abs (e_mul (e_b1) (e_po2 (e_log (e_po2 (e_po2 (e_po2 (e_3))))))))))))))))))))))))))))) (e_b11))) (e_8)))");
	  assert(ev_show(ev_parse(s)) == s);
	}
	break;
  }
  return 0;
}
