#include <cassert>
#include <ctime>
#include <map>
#include <random>
#include <numeric>
#include <iostream>

namespace s = std;

//demonstration of counterfactual regret minimzation with kuhn poker

enum ACTION : unsigned {
  PASS = 0,
  BET,
  ACTION_MAX,
};

enum PLAYER : unsigned {
  PLAYER1 = 0,
  PLAYER2 = 1,
  PLAYER_UNKNOWN,
};

PLAYER next_player(PLAYER current_player){
  return (PLAYER)(current_player ^ 0x1U);
}

s::default_random_engine& get_default_random_engine(){
  static s::default_random_engine eng(s::time(0));
  return eng;
}

struct Value {
  double regret[ACTION_MAX];
  double probability[ACTION_MAX];

  Value(){
    s::fill(regret, regret + ACTION_MAX, 0.);
    s::fill(probability, probability + ACTION_MAX, .0);
  }

  void update(const double* r){
    assert(r != nullptr);

    for (unsigned a = 0; a < ACTION_MAX; ++a)
      regret[a] += r[a];
  }

  void strategy(double* ret, double w){
    assert(ret != nullptr);

    double sum = 0.;
    for (unsigned a = 0; a < ACTION_MAX; ++a){
      ret[a] = regret[a] > 0. ? regret[a] : 0.;
      sum += ret[a];
    }
    for (unsigned a = 0; a < ACTION_MAX; ++a){
      if (sum > 0.)
        ret[a] /= sum;
      else
        ret[a] = 1. / (double)ACTION_MAX;
      probability[a] += w * ret[a];
    }
  }

  void infoset_strategy(double* ret){
    assert(ret != nullptr);

    double sum = 0.;
    for (unsigned a = 0; a < ACTION_MAX; ++a){
      sum += probability[a];
    }
    for (unsigned a = 0; a < ACTION_MAX; ++a){
      if (sum > 0.)
        ret[a] = probability[a] / sum;
      else
        ret[a] = 1. / (double)ACTION_MAX;
    }
  }
};

constexpr unsigned HISTORY_MAX = 3;

struct InfoKey {
  int card;
  ACTION history[HISTORY_MAX];

  InfoKey(): card(0) {
    s::fill(history, history + HISTORY_MAX, ACTION_MAX);
  }
  InfoKey(int c, const s::vector<ACTION>& h): card(c) {
    s::fill(history, history + HISTORY_MAX, ACTION_MAX);
    for (unsigned i = 0; i < h.size(); ++i){
      history[i] = h[i];
    }
  }
};

bool operator<(const InfoKey& a, const InfoKey& b){
  if (a.card < b.card) return true;
  for (unsigned i = 0; i < HISTORY_MAX; ++i)
    if (a.history[i] < b.history[i])
      return true;
  return false;
}

class KhunGame {
  s::map<InfoKey, Value> info_set;
  int cards[3];

  // all possible action sequences
  // bet bet
  // bet pass
  // pass pass
  // pass bet pass
  // pass bet bet
  bool is_terminal(const s::vector<ACTION>& history){
    if (history.size() < 2) return false;
    size_t len = history.size();
    switch (history[len-1]){
      case BET:
        if (history[len-2] == BET) return true;
        break;
      case PASS:
        return true;
    }
    return false;
  }

  double utility(const s::vector<ACTION>& history, PLAYER player){
    size_t len = history.size();
    switch (history[len-1]){
      case BET:
        return cards[player] > cards[next_player(player)] ? 2. : -2;
      case PASS:
        if (history[len-2] == PASS){
          return cards[player] > cards[next_player(player)] ? 1. : -1.;
        } else {
          return -1.; //TODO: is this 1 or -1?
        }
      default: assert(false);
    }
  }

  double cfr(s::vector<ACTION>& history, PLAYER player, double p0, double p1){
    if (is_terminal(history)){
      return utility(history, player);
    }

    InfoKey info_key(cards[player], history);
    if (info_set.find(info_key) == info_set.end())
      info_set[info_key] = Value();

    Value& value = info_set[info_key];

    double probs[ACTION_MAX];
    value.strategy(probs, player == PLAYER1 ? p0 : p1);
    double utility[ACTION_MAX] = {0.};
    double node_utility = 0.;
    for (unsigned a = 0; a < ACTION_MAX; ++a){
      history.push_back((ACTION)a);
      utility[a] = player == PLAYER1 ?
        -1. * cfr(history, next_player(player), p0 * probs[a], p1):
        -1. * cfr(history, next_player(player), p0, p1 * probs[a]);
      history.pop_back();
      node_utility += probs[a] * utility[a];
    }

    for (unsigned a = 0; a < ACTION_MAX; ++a)
      utility[a] = (utility[a] - node_utility) * (player == PLAYER1 ? p1 : p0);
    value.update(utility);

    return node_utility;
  }
public:
  KhunGame(){
    s::iota(cards, cards + 3, 1);
  }

  void shuffle(){
    unsigned index = rand() % 3;
    cards[0]     ^= cards[index];
    cards[index] ^= cards[0];
    cards[0]     ^= cards[index];
  }

  double play(){
    shuffle();
    s::vector<ACTION> history;
    return cfr(history, PLAYER1, 1., 1.);
  }
};

void train(KhunGame& game, unsigned iterations){
  double total_utility = 0.;
  for (unsigned i = 0; i < iterations; ++i){
    total_utility += game.play();
  }

  s::cout << "Average Utilty: " << (total_utility / (double)iterations) << s::endl;
}

int main(int argc, char* argv[]){
  s::srand(s::time(0));
  unsigned iterations = 1000000;
  if (argc > 1){
    iterations = atoi(argv[1]);
  }
  KhunGame game;
  train(game, iterations);
  //TODO
}