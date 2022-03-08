import numpy as np

# kuhn poker with vanilla CFR

# total number of possible actions, check c or bet b
N_ACTIONS = 2
# deck size, only contains 3 cards: J Q K
N_CARDS = 3

class InfoSet:
    def __init__(self, key):
        self.key = key

        self.regret_sum = np.zeros(N_ACTIONS)   # sum of counterfactual regret for each action over all visits
        self.strategy = np.repeat(1 / N_ACTIONS, N_ACTIONS) # strategy (pi) for current iteration
        self.reach_pr = 0     # accumulate probability in reaching this information set

        # following are only used for computing average strategy after reaching epsilon-equilibrium
        self.strategy_sum = np.zeros(N_ACTIONS) # sum of each visit's strategy multiplied by the information set player's reach probability, for average_strategy
        # initial strategy is uniform distribution for all actions
        self.reach_pr_sum = 0 # for average strategy
        self.iterations = 0

    def next_strategy(self):
        self.strategy_sum += self.reach_pr * self.strategy
        self.strategy = self.calc_strategy()
        self.reach_pr_sum += self.reach_pr
        self.reach_pr = 0
        self.iterations += 1

    def calc_strategy(self):
        strategy = self.make_positive(self.regret_sum)
        total = sum(strategy)
        if total > 0:
            strategy = strategy / total
        else:
            n = N_ACTIONS
            strategy = np.repeat(1/n, n)
        return strategy

    def get_average_strategy(self):
        strategy = self.strategy_sum / self.reach_pr_sum
        # purify to remove actions that are likely a mistake ?
        strategy = np.where(strategy < 0.001, 0, strategy)
        total = sum(strategy)
        strategy /= total
        return strategy

    def make_positive(self, x):
        return np.where(x > 0, x, 0)

    def __str__(self):
        strategies = ['{:03.2f}'.format(x) for x in self.get_average_strategy()]
        reach_pr = '{:03.2f}'.format(self.reach_pr_sum / self.iterations)
        return '{} {} {}'.format(self.key.ljust(6), reach_pr, strategies)

def is_chance_node(history):
    return history == ""

def chance_util(i_map):
    expected_value = 0
    n_possibilities = 6
    for i in range(N_CARDS):
        for j in range(N_CARDS):
            if i != j:
                expected_value += cfr(i_map, "rr", i, j, pr_c=1 / n_possibilities)
    return expected_value / n_possibilities

def is_terminal(history):
    possibilities = {'rrcc' : True, 'rrcbc' : True, 'rrcbb' : True, 'rrbc' : True, 'rrbb' : True}
    return history in possibilities

def terminal_util(history, card_1, card_2):
    n = len(history)
    card_player = card_1 if n % 2 == 0 else card_2
    card_oppo   = card_2 if n % 2 == 0 else card_1
    # fold
    if history == 'rrcbc' or history == 'rrbc':
        return 1
    elif history == 'rrcc':
        return 1 if card_player > card_oppo else -1
    assert(history == 'rrcbb' or history == 'rrbb')
    return 2 if card_player > card_oppo else -2

def card_str(card):
    if card == 0:
        return 'J'
    elif card == 1:
        return 'Q'
    else:
        return 'K'

def get_info_set(i_map, card, history):
    # infoset key is the player's card and history
    key = card_str(card) + ' ' + history
    info_set = None
    if key not in i_map:
        info_set = InfoSet(key)
        i_map[key] = info_set
        return info_set
    return i_map[key]

# pr_1 is probability player 1 reach this node
# pr_2 is probability player 2 reach this node
# pr_c is probability for chance events to reach this node
def cfr(i_map, history="", card_1=-1, card_2=-1, pr_1=1, pr_2=1, pr_c=1):
    if is_chance_node(history):
        return chance_util(i_map)
    if is_terminal(history):
        return terminal_util(history, card_1, card_2)
    n = len(history)
    is_player_1 = n % 2 == 0
    print("History: {} player {} pr1 {} pr2 {} prc {}".format(history, is_player_1, pr_1, pr_2, pr_c))
    info_set = get_info_set(i_map, card_1 if is_player_1 else card_2, history)
    strategy = info_set.strategy
    if is_player_1:
        info_set.reach_pr += pr_1
    else:
        info_set.reach_pr += pr_2

    action_utils = np.zeros(N_ACTIONS)
    for i, action in enumerate(['c', 'b']):
        next_history = history + action
        if is_player_1:
            action_utils[i] = -1 * cfr(i_map, next_history, card_1, card_2, pr_1 * strategy[i], pr_2, pr_c)
        else:
            action_utils[i] = -1 * cfr(i_map, next_history, card_1, card_2, pr_1, pr_2 * strategy[i], pr_c)
    util = sum(action_utils * strategy)
    regrets = action_utils - util
    if is_player_1:
        info_set.regret_sum += pr_2 * pr_c * regrets
    else:
        info_set.regret_sum += pr_1 * pr_c * regrets
    return util

def display_results(ev, i_map):
    print('player 1 expected value: {}'.format(ev))
    print('player 2 expected value: {}'.format(-1 * ev))

    print()
    print('player 1 strategies:')
    sorted_items = sorted(i_map.items(), key=lambda x: x[0])
    for _, v in filter(lambda x: len(x[0]) % 2 == 0, sorted_items):
        print(v)
    print()
    print('player 2 strategies:')
    for _, v in filter(lambda x: len(x[0]) % 2 == 1, sorted_items):
        print(v)

def main():
    i_map = {}
    n_iterations = 4
    expected_game_value = 0
    for i in range(n_iterations):
        print("Iteration {}".format(i))
        expected_game_value += cfr(i_map)
        for _, v in i_map.items():
            v.next_strategy()

    expected_game_value /= n_iterations

    display_results(expected_game_value, i_map)

if __name__ == '__main__':
    main();
