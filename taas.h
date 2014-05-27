#ifndef _TAAS_H_
#define _TAAS_H_

#include <array>
#include <vector>

namespace taas {
  using board = std::array<std::array<int, 4>, 4>;
  
  template<class T>
  T rwc(std::vector<std::pair<T, double> > l){
    std::vector<double> ps;
    double total = 0;
    for(auto x:l) {
      total += x.second;
      ps.push_back(total);
    }
    
    for(auto& x:ps) {
      x /= total;
    }
    
    double top = std::rand() * 1.0 / RAND_MAX;
    for(int i = 0; i < l.size(); i++) {
      if(ps[i] >= top) {
        return l[i].first;
      }
    }
    return l.back().first;
  }

  board rot(const board&, int);
  board mov(const board&, int);
  std::vector<std::pair<int, int>> hole(const board&);
  void inp(board &, const std::vector<std::pair<int, double>>&);
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
  };
    
  class API : public Local {
  public:
    API(std::string serv);
    virtual ~API() = default;
    
    virtual bool move(int d) override;
  };
}

#endif
