// ============================================================
//  ThinkFast  |  src/utils/BotNames.cpp
// ============================================================

#include "BotNames.h"
#include <vector>
#include <random>
#include <string>

namespace ThinkFast {

std::string BotNames::random() {
    static std::mt19937 rng(std::random_device{}());

    static const std::vector<std::string> ADJ = {
        "Swift","Bold","Quiet","Sharp","Calm","Bright","Sly","Wise","Grim","Keen",
        "Stern","Witty","Eager","Proud","Deft","Nimble","Stoic","Quick","Rapid",
        "Clever","Fierce","Steady","Brave","Mellow","Chill","Silent","Blunt","Wary"
    };

    static const std::vector<std::string> NOUN = {
        "Fox","Wolf","Hawk","Bear","Raven","Sage","Drake","Finch","Crane","Lynx",
        "Viper","Colt","Storm","Pine","Flint","Marsh","Crest","Dusk","Dawn","Tide",
        "Peak","Brook","Ridge","Blaze","Grove","Reed","Vale","Stone","Birch","Tern"
    };

    std::uniform_int_distribution<size_t> da(0, ADJ.size()  - 1);
    std::uniform_int_distribution<size_t> dn(0, NOUN.size() - 1);
    std::uniform_int_distribution<int>    dd(10, 99);

    return ADJ[da(rng)] + NOUN[dn(rng)] + std::to_string(dd(rng));
}

} // namespace ThinkFast
