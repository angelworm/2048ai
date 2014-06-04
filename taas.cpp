#include <iostream>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <cassert>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <regex>

#include "taas.h"
#include "picojson.h"
static taas::board nullboard{{{{0,0,0,0}}, {{0,0,0,0}}, {{0,0,0,0}}, {{0,0,0,0}}}};

using namespace taas;

void rotC0(const board& b, board& ret) {
  ret = b;
}

void rotC1(const board& b, board& ret) {
  for(int y = 0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      ret[x][3-y] = b[y][x];
    }
  }
}
void rotC2(const board& b, board& ret) {
  for(int y = 0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      ret[3-y][3-x] = b[y][x];
    }
  }
}
void rotC3(const board& b, board& ret) {
  for(int y = 0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      ret[3-x][y] = b[y][x];
    }
  }
}

void taas::rot(const board& b, board &ret, const int d) {
  switch(d) {
  case 0: return rotC0(b, ret);
  case 1: return rotC1(b, ret);
  case 2: return rotC2(b, ret);
  case 3: return rotC3(b, ret);
  default:
	assert(0 <= d and d <= 3);
  }
}

int movL(const board& b, board& out) {
  int score_inc = 0;
  for(int y=0; y < 4; y++) {
    int ml = 0;
    for(int x = 0; x < 4; x++) {
      if(b[y][x] == 0) {
        // n.t.d
      } else if(out[y][ml] == 0) {
        out[y][ml] = b[y][x];
      } else if(out[y][ml] == b[y][x]) {
        out[y][ml] = b[y][x] * 2;
		score_inc += out[y][ml];
        ml++;
      } else {
        out[y][ml+1] = b[y][x];
        ml++;
      }
    }
  }
  return score_inc;
}

int taas::mov(const board& b, board& ret, const int d) {
  int l,r;
  auto b2(nullboard);
  auto b3(nullboard);

  switch(d) {
  case 0: l = 1; r = 3; break;
  case 1: l = 2; r = 2; break;
  case 2: l = 3; r = 1; break;
  case 3: l = 0; r = 0; break;
  default:
	assert(0 <= d and d <= 3);
	l = -1; r = -1;
  }
  rot(b, b2, r);
  int score_inc = movL(b2, b3);
  rot(b3, ret, l);
  return score_inc;
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
  for(const auto& l:b) {
    for(const auto& x:l) {
      ret += x * std::log(x) / std::log(2);
    }
  }
  return ret;
}

bool taas::over(const board& b) {
  int yb[] = {0,0,0,0};           
  for(const auto& l:b) {
    int xb = 0, i = 0;
    for(const auto& x:l) {
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
  for(const auto& l:b) {
    for(const auto& x:l) {
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

  this->score += taas::mov(this->b, this->b, d);
  this->moved = (this->b != bb);

  inp(this->b, v, p, this->g);
  this->over = taas::over(this->b);
  return this->moved;
}

taas::API::API(std::string serv, int port)
  :over(false), moved(false), score(0), b(nullboard), serv(serv), port(port) {
  auto json = js_get(std::string("/hi/start/json"));
  const auto& obj = json.get<picojson::object>();
  this->session = obj.at("session_id").get<std::string>();

  auto jb = obj.at("grid").get<picojson::array>();
  for(int y = 0; y < 4; y++) {
	for(int x = 0; x < 4; x++) {
	  this->b[y][x] = jb[y].get<picojson::array>()[x].get<double>();
	}
  }
}

bool taas::API::move(int d) {
  std::stringstream ss;
  ss << "/hi/state/" << this->session << "/move/" << d << "/json";

  auto json = this->js_get(ss.str());
  const auto& obj = json.get<picojson::object>();


  const auto& jb = obj.at("grid").get<picojson::array>();
  for(int y = 0; y < 4; y++) {
	for(int x = 0; x < 4; x++) {
	  this->b[y][x] = jb[y].get<picojson::array>()[x].get<double>();
	}
  }
  this->score = obj.at("score").get<double>();
  this->moved = obj.at("moved").get<bool>();
  this->over = obj.at("over").get<bool>();
  return this->moved;
}

picojson::value taas::API::js_get(std::string path) {
  int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock < 0){
    throw std::runtime_error(std::strerror(errno));
  }

  struct sockaddr_in addr;
  struct hostent *servhost = gethostbyname(this->serv.c_str());

  memset(static_cast<void *>(&addr), 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  bcopy(servhost->h_addr, &addr.sin_addr, servhost->h_length);
  addr.sin_port = htons(this->port == 0 ? 8080 : this->port);

  if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1 ) {
	throw std::runtime_error(std::strerror(errno));
  }
  
  std::string data;
  std::stringstream ss;

  ss << "GET " << path << " HTTP/1.0" << "\r\n\r\n"; 
  //  ss << "HOST " << serv  << ":" << port << "\r\n\r\n"; 
  std::string msg = ss.str();
  write(sock, msg.c_str(), msg.size()+1);

  while(true) {
	char buf[1024];
	int size = read(sock, buf, 1024);
	if(size <= 0) break;
	data += std::string(buf, buf + size);
  }

  close(sock);

  if(data.find("Location: ") != std::string::npos) {
	auto ia = data.find("Location: ");
	auto ib = data.find("\r\n", ia);
	std::string cont = data.substr(ia + 10, ib - (ia + 10));

	std::cout << "moved: " << cont << std::endl;
	return this->js_get(cont);
  } else{
	picojson::value v;
	std::string err;

	std::string cont = data.substr(data.find("\r\n\r\n")+4);

	std::cout << cont << std::endl;
	picojson::parse(v, cont.begin(), cont.end(), &err);
	if(not err.empty()) throw std::runtime_error(err);
	return v;
  }
}
