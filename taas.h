#ifndef _TAAS_H_
#define _TAAS_H_

#include <array>
#include <vector>
#include <random>
#include <algorithm>

namespace taas {
  using board = std::array<std::array<int, 4>, 4>;

  template<class T, class Gen>
  T rwc(const std::vector<T>& data, const std::vector<double>& ps, Gen& g){
    std::discrete_distribution<int> gen(ps.begin(), ps.end());
    return data[gen(g)];
  }

  board rot(const board&, int);
  board mov(const board&, int);
  std::vector<std::pair<int, int>> hole(const board&);
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
}

#endif
