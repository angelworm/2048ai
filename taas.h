#ifndef _TAAS_H_
#define _TAAS_H_

#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include "picojson.h"

namespace taas {
  using board = std::array<std::array<int, 4>, 4>;

  template<class T, class Gen>
  T rwc(const std::vector<T>& data, const std::vector<double>& ps, Gen& g){
    std::discrete_distribution<int> gen(ps.begin(), ps.end());
    return data[gen(g)];
  }

  void rot(const board&, board&, const int);
  int mov(const board&, board&, const int );
  std::vector<std::pair<int, int> > hole(const board&);

  template<class Gen>
  void inp(board &b, const std::vector<int>& data, const std::vector<double>& l, Gen& g);
  int score(const board&);
  bool over(const board&);
 
  void pb(const board&);

  class Local {
  public:
    Local();
    virtual ~Local() = default;
    Local(Local const&) = default;
    Local(Local&&) = default;
    Local& operator =(Local const&) = default;
    Local& operator =(Local&&) = default;

    virtual bool move(int d);

    board b;
    int score;
    bool over, moved;
    std::mt19937 g;
  };

  class API {
  public:
    API() = delete;
	API(std::string serv, int port);

    virtual ~API() = default;
    API(API const&) = default;
    API(API&&) = default;
    API& operator =(API const&) = default;
    API& operator =(API&&) = default;
	

    virtual bool move(int d);
	picojson::value js_get(std::string path);

    board b;
    int score, port;
    bool over, moved;
	std::string serv, session;
  };

}

namespace std {
  template<>
  struct hash<taas::board>{
	size_t operator()(const taas::board& b) const {
	  uint64_t ret = 0;
	  for(const auto& l:b) {
		for(const auto& x:l) {
		  ret <<= 4;
		  ret += std::log2(x);
		}
	  }
	  return std::hash<uint64_t>()(ret);
	}
  };
}

#endif
