#include <cassert>
#include <ctime>
#include <vector>
#include <random>
#include <iostream>

namespace s = std;

// demonstration of regret minimization algorithm using rock paper sissors

enum ACTION : unsigned {
  ROCK = 0,
  PAPER,
  SCISSORS,
  MAX,
};

class RPSAgent {
  int regret_sum[MAX];

  void random_init(){
    s::random_device rd;
    s::mt19937 gen(rd());
    s::uniform_int_distribution<int> dist(0, 100);

    for (unsigned i = 0; i < MAX; ++i)
      regret_sum[i] = dist(gen);
  }

  void fixed_init(){
    for (unsigned i = 0; i < MAX; ++i)
      regret_sum[i] = 1;
  }

  void zero_init(){
    for (unsigned i = 0; i < MAX; ++i)
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
    //TODO
    return ROCK;
  }

  void update(ACTION my_action, ACTION oppo_action){
    int real_utility = utility(my_action, oppo_action);
    for (int i = 0; i < MAX; ++i){
      int cfr_utility = utility((ACTION)i, oppo_action);
      regret_sum[i] += cfr_utility - real_utility;
    }
  }

  void get_strategy(s::vector<double>& ret){
    //TODO
  }
};

void train(unsigned iterations){
}

int main(int argc, char* argv[]){
  s::srand(s::time(nullptr));


}
