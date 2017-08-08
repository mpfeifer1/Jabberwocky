#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

// For the purposes of this program, Ace is low

int DEAL_NUM = 2;
int PLAYERS = 2;

enum suit {
    hearts,
    spades,
    diamonds,
    clubs
};

struct card {
    suit s;
    int n;
};

#define hand vector<card>
#define used vector<bool>

int nextplayer(int player);
void start(card topcard, vector<pair<hand, used>> hands);
void lead(int player, card topcard, vector<pair<hand, used>> hands, bool broken, used played, vector<card> path, vector<int> ledpath, vector<int> wonpath);
void cont(int player, card topcard, vector<pair<hand, used>> hands, bool broken, used played, card led, card winningcard, int winningplayer, vector<card> path, vector<int> ledpath, vector<int> wonpath);
string print(card c);
bool beats(card topcard, card led, card played, card winning);


int main() {
    // Create and fill the deck
    vector<card> deck;
    for(int s = hearts; s <= clubs; s++) {
        for(int n = 1; n <= 13; n++) {
            card c;
            c.s = static_cast<suit>(s);
            c.n = n;
            deck.push_back(c);
        }
    }

    // Shuffle the deck
    srand(time(0));
    random_shuffle(deck.begin(), deck.end());

    // Get player's hands and mark all cards unused
    vector<pair<hand, used>> hands;
    for(int i = 0; i < PLAYERS; i++) {
        hand h;
        used u;
        hands.push_back({h, u});
    }

    // Deal out cards one-by-one
    for(int i = 0; i < DEAL_NUM; i++) {
        for(auto& h : hands) {
            h.first.push_back(deck[0]);
            h.second.push_back(false);
            deck.erase(deck.begin());
        }
    }

    // Flip up top card
    card topcard = deck[0];
    deck.erase(deck.begin());

    // Tell what everyone was dealt
    cout << "Top card: " << print(topcard) << endl;
    for(int i = 0; i < PLAYERS; i++) {
        cout << "Player " << i+1 << " was dealt:" << endl;
        for(auto cd : hands[i].first) {
            cout << print(cd) << endl;
        }
    }
    cout << endl;

    // Start playing
    start(topcard, hands);
}



int nextplayer(int player) {
    player++;
    if(player == PLAYERS) {
        player = 0;
    }
    return player;
}



void start(card topcard, vector<pair<hand, used>> hands) {
    used played;
    vector<card> path;
    vector<int> wonpath;
    vector<int> ledpath;
    played.resize(hands.size(), false);
    lead(0, topcard, hands, false, played, path, ledpath, wonpath);
}



void lead(int player, card topcard, vector<pair<hand, used>> hands, bool broken, used played, vector<card> path, vector<int> ledpath, vector<int> wonpath) {
    // Mark this player played
    played[player] = true;
    ledpath.push_back(player);

    // Check if only trump left
    bool onlytrump = true;
    for(int i = 0; i < hands[player].first.size(); i++) {
        // If not used, and not trump, set onlytrump
        if(!hands[player].second[i] && hands[player].first[i].s != topcard.s) {
            onlytrump = false;
            break;
        }
    }

    // For each card
    for(int i = 0; i < hands[player].first.size(); i++) {
        // If the card isn't used
        if(!hands[player].second[i]) {
            // If broken, not trump, or only trump left, play it
            if(broken || hands[player].first[i].s != topcard.s || onlytrump) {
                // Use card
                hands[player].second[i] = true;
                path.push_back(hands[player].first[i]);

                // Make card the winning card
                card led = hands[player].first[i];
                card winningcard = hands[player].first[i];
                int winningplayer = player;

                // Play rest of game
                cont(nextplayer(player), topcard, hands, broken, played, led, winningcard, winningplayer, path, ledpath, wonpath);

                // Un-use card
                hands[player].second[i] = false;
                path.pop_back();
            }
        }
    }

    // Mark player didn't play
    played[player] = false;
    ledpath.pop_back();
}



void cont(int player, card topcard, vector<pair<hand, used>> hands, bool broken, used played, card led, card winningcard, int winningplayer, vector<card> path, vector<int> ledpath, vector<int> wonpath) {
    // Mark this player played
    played[player] = true;

    // Check if player still has that suit
    bool canfollow = false;
    for(int i = 0; i < hands[player].first.size(); i++) {
        // If not used, and follows suit, set canfolllow
        bool cardused = hands[player].second[i];
        bool followssuit = hands[player].first[i].s == led.s;
        if(!cardused && followssuit) {
            canfollow = true;
            break;
        }
    }

    // For each card
    for(int i = 0; i < hands[player].first.size(); i++) {
        // If the card isn't used
        if(!hands[player].second[i]) {
            // Figure out if the card can be played
            bool playable = false;

            // If it can follow and follows, it can be played
            if(canfollow && hands[player].first[i].s == led.s) {
                playable = true;
            }

            // If it can't follow, it can be played
            if(!canfollow) {
                playable = true;
            }

            // Play it
            if(playable) {
                // Use card
                hands[player].second[i] = true;
                path.push_back(hands[player].first[i]);

                // Save old winning card
                card oldwinningcard = winningcard;
                int  oldwinningplayer = winningplayer;
                bool oldbroken = broken;

                // If it wins, make card the winning card, maybe set broken
                if(beats(topcard, led, hands[player].first[i], winningcard)) {
                    winningcard = hands[player].first[i];
                    winningplayer = player;

                    if(winningcard.s == led.s) {
                        broken = true;
                    }
                }

                // Determine if another player has to play
                bool finished = true;
                for(auto i : played) {
                    finished &= i;
                }

                // If there's another player, continue
                if(!finished) {
                    // Play rest of game
                    cont(nextplayer(winningplayer), topcard, hands, broken, played, led, winningcard, winningplayer, path, ledpath, wonpath);
                }

                // If there aren't more plyaers
                else {
                    // Find out if there are more cards
                    bool more = false;
                    for(auto i : hands[player].second) {
                        if(!i) {
                            more = true;
                        }
                    }

                    // If there are more cards, mark winner and lead again
                    if(more) {
                        vector<bool> newplayed;
                        newplayed.resize(played.size(), false);
                        wonpath.push_back(winningplayer);
                        lead(winningplayer, topcard, hands, broken, newplayed, path, ledpath, wonpath);
                        wonpath.pop_back();
                    }
                    // If there aren't any more cards, add score
                    else {
                        cout << "GAME" << endl;
                        for(int i = 0; i < ledpath.size(); i++) {
                            cout << "Player " << ledpath[i]+1 << " led. ";
                            for(int j = 0; j < PLAYERS; j++) {
                                cout << print(path[i*PLAYERS+j]) << " ";
                            }
                            cout << "Player " << wonpath[i]+1 << " won." << endl;
                        }
                        cout << endl;
                        //TODO Add to global score array
                    }
                }


                // Un-use card, set to old winning card
                hands[player].second[i] = false;
                path.pop_back();
                winningcard = oldwinningcard;
                winningplayer = oldwinningplayer;
                broken = oldbroken;
            }
        }
    }

    // Mark player didn't play
    played[player] = false;
}



bool beats(card topcard, card led, card played, card winning) {
    // If not trump or led suit, it loses
    if(played.s != topcard.s && played.s != led.s) {
        return false;
    }

    // If trump
    if(played.s == topcard.s) {
        // If other card is trump
        if(winning.s == topcard.s) {
            // Determine by number
            return played.n > winning.n;
        }
        // Other card not
        else {
            return true;
        }
    }

    // If not trump
    else {
        // If other card is trump
        if(winning.s == topcard.s) {
            return false;
        }
        // Other card not
        else {
            // Determine by number
            return played.n > winning.n;
        }
    }

    cout << "SHOULDNT BE HERE" << endl;
    return false;
}



string print(card c) {
    string ret = to_string(c.n) + " of ";
    if(c.s == hearts) {
        ret += "hearts";
    }
    if(c.s == spades) {
        ret += "spades";
    }
    if(c.s == diamonds) {
        ret += "diamonds";
    }
    if(c.s == clubs) {
        ret += "clubs";
    }
    return ret;
}
