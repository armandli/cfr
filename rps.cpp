#include <cassert>
#include <ctime>
#include <vector>
#include <random>
#include <numeric>
#include <iostream>

namespace s = std;

// demonstration of regret minimization algorithm using rock paper sissors

enum ACTION : unsigned {
  ROCK = 0,
  PAPER,
  SCISSORS,
  ACTION_MAX,
};

s::default_random_engine& get_default_random_engine(){
  static s::default_random_engine eng(time(0));
  return eng;
}

class RPSAgent {
  int regret_sum[ACTION_MAX];

  void random_init(){
    s::default_random_engine& eng = get_default_random_engine();
    s::uniform_int_distribution<int> dist(0, 100);

    for (unsigned i = 0; i < ACTION_MAX; ++i)
      regret_sum[i] = dist(eng);
  }

  void fixed_init(){
    for (unsigned i = 0; i < ACTION_MAX; ++i)
      regret_sum[i] = 1;
  }

  void zero_init(){
    for (unsigned i = 0; i < ACTION_MAX; ++i)
      regret_sum[i] = 0;
  }

  int utility(ACTION a, ACTION b){
    switch (a){
      case ROCK:
        switch (b){
          case ROCK:     return 0;
          case PAPER:    return -1;
          case SCISSORS: return 1;
          default: assert(false);
        }
        break;
      case PAPER:
        switch (b){
          case ROCK:     return 1;
          case PAPER:    return 0;
          case SCISSORS: return -1;
          default: assert(false);
        }
        break;
      case SCISSORS:
        switch (b){
          case ROCK:     return -1;
          case PAPER:    return 1;
          case SCISSORS: return 0;
          default: assert(false);
        }
        break;
      default: assert(false);
    }
  }
public:
  enum TY : unsigned {
    Random,
    Fixed,
    Zero,
  };

  explicit RPSAgent(TY type){
    switch (type){
      case Random: random_init(); break;
      case Fixed:  fixed_init();  break;
      case Zero:   zero_init();   break;
      default: assert(false);
    }
  }

  ACTION get_action(){
    s::vector<double> action_prob;
    action_prob.reserve(ACTION_MAX);
    get_strategy(action_prob);
    double normalizing_prob = s::accumulate(s::begin(action_prob), s::end(action_prob), 0.);

    s::default_random_engine& eng = get_default_random_engine();
    s::uniform_real_distribution<double> dist(0., normalizing_prob);
    double sample = dist(eng);
    for (size_t i = 0; i < action_prob.size(); ++i){
      if (action_prob[i] >= sample)
        return (ACTION)i;
      sample -= action_prob[i];
    }
    return SCISSORS;
  }

  void update(ACTION my_action, ACTION oppo_action){
    int real_utility = utility(my_action, oppo_action);
    for (int i = 0; i < ACTION_MAX; ++i){
      int cfr_utility = utility((ACTION)i, oppo_action);
      regret_sum[i] += cfr_utility - real_utility;
    }
  }

  void get_strategy(s::vector<double>& ret){
    int sum = s::accumulate(regret_sum, regret_sum + ACTION_MAX, 0);
    for (int i = 0; i < ACTION_MAX; ++i)
      if (regret_sum[i] <= 0)
        ret.push_back(1. / (double)ACTION_MAX);
      else
        ret.push_back((double)regret_sum[i] / (double)sum);
  }
};

void train(RPSAgent& a, RPSAgent& b, unsigned iterations){
  for (unsigned i = 0; i < iterations; ++i){
    ACTION action_a = a.get_action();
    ACTION action_b = b.get_action();
    a.update(action_a, action_b);
    b.update(action_b, action_a);
  }
}

int main(int argc, char* argv[]){
  s::srand(s::time(nullptr));
  unsigned iterations = 1000;
  if (argc > 1){
    iterations = atoi(argv[1]);
  }

  RPSAgent a(RPSAgent::Random);
  RPSAgent b(RPSAgent::Random);
  train(a, b, iterations);

  s::vector<double> a_prob, b_prob;
  a.get_strategy(a_prob);
  b.get_strategy(b_prob);

  s::cout << "A Strategy: ";
  for (double p : a_prob)
    s::cout << p << " ";
  s::cout << s::endl;

  s::cout << "B Strategy: ";
  for (double p : b_prob)
    s::cout << p << " ";
  s::cout << s::endl;
}
