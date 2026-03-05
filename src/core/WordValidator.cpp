
#include "WordValidator.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <cstdio> 
#include <string>
#include <iostream>

namespace ThinkFast {

WordValidator::WordValidator()
    : rng_(std::random_device{}())
{
    buildBuiltinDictionary();
}

void WordValidator::loadFromCSV(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) return;

    std::string line;
    bool first = true;
    while (std::getline(f, line)) {
        if (first) { first = false; continue; } 
        std::string word = stripQuotes(trim(line));
        if (word.size() < 2) continue;

        bool alpha = true;
        for (unsigned char c : word)
            if (!std::isalpha(c)) { alpha = false; break; }
        if (!alpha) continue;

        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (dict_.insert(word).second)        
            wordList_.push_back(word);
    }
}


bool WordValidator::isValid(const std::string& word) {
    if (word.size() < 2) return false;
    const std::string w = toLower(word);

    if (dict_.count(w)) return true;

    auto it = apiCache_.find(w);
    if (it != apiCache_.end()) return it->second;

    bool result = queryAPI(w);
    apiCache_[w] = result;

    if (result) {
        dict_.insert(w);
        wordList_.push_back(w);
    }
    return result;
}

bool WordValidator::isValidLocal(const std::string& word) const {
    if (word.size() < 2) return false;
    return dict_.count(toLower(word)) > 0;
}

bool WordValidator::isValidPrefix(const std::string& prefix) const {
    const std::string p = toLower(prefix);
    for (const auto& w : wordList_)
        if (w.size() >= p.size() && w.compare(0, p.size(), p) == 0)
            return true;
    return false;
}

char WordValidator::lastChar(const std::string& word) const {
    if (word.empty()) return '\0';
    return static_cast<char>(std::tolower(static_cast<unsigned char>(word.back())));
}

bool WordValidator::startsWithChar(const std::string& word, char ch) const {
    if (word.empty()) return false;
    return std::tolower(static_cast<unsigned char>(word[0])) ==
           std::tolower(static_cast<unsigned char>(ch));
}

std::string WordValidator::cpuPickWord(char startLetter,
                                       const std::unordered_set<std::string>& used,
                                       int difficulty) const {
    const char sl = static_cast<char>(
        std::tolower(static_cast<unsigned char>(startLetter)));

    std::vector<std::string> cands;
    cands.reserve(256);
    for (const auto& w : wordList_) {
        if (!w.empty() &&
            std::tolower(static_cast<unsigned char>(w[0])) == sl &&
            used.find(w) == used.end()) {
            cands.push_back(w);
        }
    }
    if (cands.empty()) return "";

    if (difficulty >= 2) {
        std::sort(cands.begin(), cands.end(),
                  [](const std::string& a, const std::string& b){
                      return a.size() > b.size();
                  });
        const size_t top = std::max(size_t{1}, cands.size() / 5);
        std::uniform_int_distribution<size_t> d(0, top - 1);
        return cands[d(rng_)];
    }

    std::uniform_int_distribution<size_t> d(0, cands.size() - 1);
    return cands[d(rng_)];
}

char WordValidator::cpuNextLetter(const std::string& partial) const {
    const std::string p = toLower(partial);
    std::vector<char> cands;
    for (const auto& w : wordList_)
        if (w.size() > p.size() && w.compare(0, p.size(), p) == 0)
            cands.push_back(w[p.size()]);
    if (cands.empty())
        return static_cast<char>('a' + (rng_() % 26));
    std::uniform_int_distribution<size_t> d(0, cands.size() - 1);
    return cands[d(rng_)];
}

std::string WordValidator::randomStartWord() const {
    std::vector<std::string> pool;
    for (const auto& w : wordList_)
        if (w.size() >= 4 && w.size() <= 6)
            pool.push_back(w);
    if (pool.empty()) return "apple";
    std::uniform_int_distribution<size_t> d(0, pool.size() - 1);
    return pool[d(rng_)];
}

size_t WordValidator::dictionarySize() const {
    return dict_.size();
}

bool WordValidator::queryAPI(const std::string& word) const {
    for (unsigned char c : word)
        if (!std::isalpha(c)) return false;

    const std::string cmd =
        "curl -s -o /dev/null -w \"%{http_code}\" --max-time 4 "
        "\"https://api.dictionaryapi.dev/api/v2/entries/en/" + word + "\" 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;

    char buf[8] = {};
    if (fgets(buf, sizeof(buf), pipe) == nullptr) {
        pclose(pipe);
        return false;
    }
    pclose(pipe);

    const std::string code(buf);
    return code.find("200") != std::string::npos;
}


std::string WordValidator::toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return r;
}

std::string WordValidator::trim(const std::string& s) {
    const size_t a = s.find_first_not_of(" \t\r\n");
    const size_t b = s.find_last_not_of(" \t\r\n");
    return (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

std::string WordValidator::stripQuotes(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}


void WordValidator::buildBuiltinDictionary() {
    static const char* const WORDS[] = {
        // 3-letter
        "ace","act","add","age","ago","aid","aim","air","all","ant","any","ape","arc",
        "are","ark","arm","art","ash","ask","ate","axe","aye","bad","bag","ban","bar",
        "bat","bay","bed","big","bit","bow","box","boy","bud","bug","bun","bus","but",
        "buy","cab","can","cap","car","cat","cob","cod","cog","cop","cow","cry","cue",
        "cup","cut","dab","dam","day","den","dew","die","dig","dim","dip","dog","dot",
        "dry","dub","dug","duo","dye","ear","eat","egg","ego","elf","elm","end","era",
        "eve","ewe","eye","fad","fan","far","fat","fax","fed","fee","few","fig","fin",
        "fit","fix","flu","fly","foe","fog","fox","fry","fun","fur","gap","gas","gel",
        "gem","get","gin","god","got","gum","gun","gut","guy","gym","had","ham","has",
        "hat","hay","hen","her","hid","him","hip","his","hit","hog","hop","hot","how",
        "hub","hug","hum","hut","ice","ill","imp","inn","ion","ivy","jab","jam","jar",
        "jaw","jet","jig","job","jot","joy","jug","jut","keg","key","kid","kin","kit",
        "lab","lag","lap","law","lay","led","leg","let","lid","lip","lit","log","lot",
        "low","lug","mad","man","map","mat","men","met","mob","mop","mud","mug","nab",
        "nag","nap","net","new","nil","nip","nod","nor","not","now","nun","nut","oak",
        "oar","odd","ode","oil","old","one","opt","orb","ore","our","out","owe","own",
        "pad","pal","pan","paw","pay","peg","pen","pet","pig","pin","pit","pod","pop",
        "pot","pro","pub","pun","pup","put","rag","ram","ran","rap","rat","raw","ray",
        "red","ref","rep","rev","rid","rig","rim","rip","rob","rod","rot","row","rub",
        "rug","rum","run","rut","sad","sag","sap","sat","saw","say","sea","set","sew",
        "shy","sin","sip","sir","six","ski","sky","sly","sob","sod","son","sow","spa",
        "spy","sub","sum","sun","tab","tag","tan","tap","tar","tax","tea","ten","tie",
        "tin","tip","toe","ton","too","top","tow","toy","try","tub","tug","two","urn",
        "use","van","vat","vow","wad","war","was","wax","web","wed","wet","wig","win",
        "wit","woe","wok","won","woo","yak","yam","yap","yew","you","zap","zip","zoo",
        // 4-letter
        "able","ache","acid","acre","also","alto","amid","ante","arch","area","arty",
        "atom","aunt","auto","away","axle","baby","back","bail","bait","bale","ball",
        "balm","band","bang","bank","bare","bark","barn","base","bash","bass","bath",
        "bead","beak","beam","bean","bear","beat","beer","bell","belt","bend","best",
        "bill","bind","bird","bite","blot","blow","blue","blur","boat","bold","bolt",
        "bond","bone","book","boom","boot","bore","born","bout","bran","brew","buck",
        "buff","bulk","bull","bump","burn","bush","buzz","cage","cake","call","calm",
        "cane","card","care","cart","case","cash","cast","cave","cell","cent","chat",
        "chef","chin","chip","chop","cite","city","clam","clap","claw","clay","clip",
        "clot","club","clue","coal","coat","code","coil","coin","cold","come","cone",
        "cook","cool","core","corn","cost","cozy","crab","crew","crop","crow","cube",
        "curb","cure","curl","damp","dare","dark","dart","data","date","dawn","dead",
        "deal","dear","deck","deed","deep","deer","deny","desk","dial","dice","dirt",
        "disc","dish","disk","dive","dock","dome","done","doom","door","dose","dove",
        "down","drop","drum","dual","duck","dull","dump","dusk","dust","duty","earl",
        "earn","ease","east","edge","edit","emit","even","ever","evil","exam","exit",
        "face","fact","fade","fail","fake","fall","fame","fare","farm","fast","fate",
        "fear","feat","feed","feel","feet","fell","felt","fend","fern","file","fill",
        "film","find","fine","fire","firm","fish","fist","flag","flat","flaw","flea",
        "fled","flex","flip","flow","foam","fold","folk","fond","font","food","fool",
        "foot","ford","fore","fork","form","fort","foul","four","free","frog","from",
        "fuel","fuse","fuzz","gain","gale","game","gang","gaze","gear","germ","gift",
        "girl","give","glad","glee","glen","glow","glue","goal","goat","gold","golf",
        "gown","grab","gram","gray","grew","grid","grim","grin","grip","grit","grow",
        "gulf","gull","gust","guts","hack","hail","hair","half","hall","halt","hand",
        "hang","hard","harm","harp","hash","haul","have","hawk","haze","head","heal",
        "heap","heat","heel","hell","help","herb","herd","here","hero","hide","high",
        "hill","hint","hire","hive","hold","hole","holy","home","hood","hook","hope",
        "horn","hose","host","hour","huge","hull","hung","hunt","hurt","hymn","idle",
        "into","iron","isle","item","jade","jaws","jerk","jolt","jump","just","keen",
        "keep","kill","kind","king","knee","knob","know","lack","lake","lamb","lamp",
        "land","lane","lash","last","late","lawn","lazy","lead","leaf","leak","lean",
        "leap","left","lend","lens","liar","life","lift","like","lime","line","link",
        "lion","list","live","load","loan","lock","loft","lone","long","look","lord",
        "lore","lose","loss","lost","love","luck","lull","lump","lung","lust","mace",
        "made","mail","main","make","male","mall","mane","mare","mark","mast","math",
        "mate","maze","meal","mean","meat","melt","mere","mesh","mile","milk","mill",
        "mine","mint","mist","mock","mode","mole","moor","more","moss","most","move",
        "much","mule","must","myth","nail","name","neck","need","news","next","nice",
        "nine","node","none","noon","norm","nose","note","noun","nude","obey","once",
        "only","open","oval","oven","over","pace","pack","page","paid","pain","pair",
        "palm","pane","park","part","pass","past","path","peak","pear","peel","peer",
        "pest","pick","pile","pine","pipe","plan","play","plod","plot","plow","plug",
        "plum","plus","poem","poet","pole","poll","pond","pony","pool","poor","pore",
        "pork","port","pose","post","pour","pray","prey","prod","prop","pull","pump",
        "pure","push","quad","race","rack","rage","raid","rail","rain","rake","ramp",
        "rank","rant","rash","rate","read","real","reap","rear","reed","reel","rein",
        "rely","rent","rest","rice","rich","ride","ring","rise","risk","rite","road",
        "roam","roar","robe","rock","role","roll","roof","room","rope","rose","ruin",
        "rule","rush","rust","safe","sage","sail","sake","sale","salt","same","sand",
        "sane","seal","seat","seed","seek","self","sell","send","shed","ship","shoe",
        "shop","shot","show","shut","sick","side","sigh","sign","silk","sill","sing",
        "sink","site","size","skin","slab","slap","slim","slip","slot","slow","slug",
        "smog","snap","snob","snow","soak","soap","sock","soft","soil","sole","some",
        "song","soot","sore","sort","soul","soup","sour","span","spar","spin","spit",
        "spot","stab","stag","star","stay","stem","step","stir","stop","stub","stud",
        "such","suit","sung","sunk","swan","swap","sway","tack","tail","tale","talk",
        "tall","tame","tart","task","team","tear","tell","term","test","text","than",
        "tick","tidy","tier","till","time","tiny","tire","toad","told","toll","tomb",
        "tone","took","tool","torn","tort","toss","tour","town","trim","trio","trip",
        "trod","true","tuck","tune","turf","turn","tusk","twin","type","ugly","undo",
        "unit","upon","used","user","vain","vale","vary","vast","veil","vein","very",
        "vest","veto","view","vile","vine","void","volt","wade","wage","wait","wake",
        "walk","wall","wand","want","ward","warm","warn","warp","wash","wave","weak",
        "wear","weed","week","well","welt","went","were","west","what","when","whim",
        "whip","will","wilt","wind","wine","wing","wink","wire","wise","wish","with",
        "wolf","wood","wool","word","wore","work","worm","wrap","wren","year","yell",
        "your","zero","zone",
        // 5-letter
        "about","above","abuse","actor","acute","after","again","agent","agree","ahead",
        "alarm","album","alert","alien","alive","allow","alone","angel","anger","angle",
        "ankle","apple","apply","arena","argue","arise","armor","arrow","aside","asset",
        "atlas","audio","avoid","award","aware","awful","beach","beard","beast","begin",
        "below","bench","black","blade","blame","blank","blast","blaze","bleed","blend",
        "blind","blink","block","blood","bloom","board","bonus","boost","bread","break",
        "breed","brick","bride","bring","broad","broke","brook","brown","build","bunch",
        "burst","candy","carry","catch","cause","chain","chair","charm","chase","check",
        "cheer","chest","chief","child","chunk","civil","claim","clash","clean","clear",
        "click","cliff","climb","close","cloud","coach","coast","coral","count","court",
        "cover","crack","craft","crash","cream","creek","crime","cross","crowd","crown",
        "crush","curve","cycle","dance","depth","devil","dizzy","doubt","draft","drain",
        "drama","dream","dress","drift","drill","drink","drive","drown","eagle","early",
        "earth","eight","elite","empty","enemy","enjoy","enter","equal","error","essay",
        "event","exact","exist","extra","fable","faith","fairy","fancy","feast","fence",
        "fever","field","final","flame","flash","fleet","flesh","floor","flush","flute",
        "focus","forge","forum","found","fresh","front","frost","fruit","funny","ghost",
        "giant","glass","globe","gloom","glove","grace","grade","grain","grand","grant",
        "grasp","grass","green","greet","grief","grind","grove","guard","guess","guide",
        "guild","guilt","habit","happy","heart","heavy","hedge","hello","horse","hotel",
        "house","human","humor","hurry","image","issue","jewel","joint","judge","juice",
        "knife","knock","label","lance","laser","laugh","layer","learn","legal","lemon",
        "level","light","limit","local","logic","loose","lucky","lunar","lyric","magic",
        "major","manor","maple","march","match","mayor","media","merit","metal","minor",
        "model","money","month","moral","motor","mount","mouse","mouth","movie","music",
        "nerve","never","night","noble","noise","north","novel","nurse","ocean","offer",
        "olive","opera","orbit","order","otter","outer","owner","ozone","paint","panel",
        "paper","party","pasta","pause","peace","pearl","penny","phase","phone","photo",
        "piano","pilot","pixel","pizza","place","plain","plane","plant","plate","point",
        "polar","power","press","price","pride","prime","print","prize","probe","proof",
        "proud","pulse","query","quest","quick","quiet","quote","racer","radar","radio",
        "ranch","range","rapid","raven","razor","reach","realm","rebel","relay","reply",
        "rider","river","robot","rocky","rouge","round","royal","ruler","saint","salon",
        "sauce","scale","scary","scene","scout","sense","shade","shame","shape","share",
        "sharp","shelf","shell","shift","shine","shirt","shock","short","sight","skill",
        "skull","sleep","slice","slide","slope","smile","smoke","snake","solar","solid",
        "solve","sorry","south","space","spark","speak","speed","spend","spice","spike",
        "spine","split","spoon","sport","spray","squad","staff","stage","stake","stand",
        "stare","start","state","steam","steel","stick","sting","stock","stone","storm",
        "story","stove","straw","study","style","sugar","suite","super","sweep","sweet",
        "swirl","sword","table","taste","teach","teeth","tempo","title","toast","topic",
        "total","touch","tough","tower","toxic","trace","track","trail","train","trait",
        "trash","treat","trend","trial","tribe","trick","troop","truck","trust","truth",
        "twist","ultra","uncle","under","union","unity","upper","urban","usage","usual",
        "valid","value","vapor","vault","video","vigor","viral","virus","visit","vital",
        "vivid","vocal","voice","voter","waltz","water","watch","weave","wedge","wheat",
        "wheel","white","witch","woman","world","worry","worth","wound","yacht","yield",
        "young","youth","zebra","zesty",
        // 6+ letter common words
        "almost","always","answer","appear","arrive","aspect","attach","autumn","battle",
        "beauty","belong","border","borrow","bottle","bottom","bounce","breach","bridge",
        "broken","bronze","budget","bullet","bundle","burden","button","cactus","camera",
        "candle","canvas","castle","casual","cattle","caught","cellar","center","chance",
        "change","charge","cherry","chorus","chosen","chrome","circle","circus","clever",
        "client","closet","cobalt","colony","combat","comedy","common","corner","cotton",
        "coward","create","credit","custom","damage","danger","decide","defend","define",
        "desert","design","detail","differ","direct","divine","double","dragon","drawer",
        "driven","dollar","domain","during","easily","effect","effort","either","emerge",
        "empire","enable","endure","energy","engage","engine","enough","escape","evolve",
        "excess","expect","export","extend","fabric","fallen","family","famous","father",
        "figure","finger","finish","follow","forest","formal","frozen","future","garden",
        "gather","gentle","glance","global","govern","hammer","handle","happen","harbor",
        "harder","hazard","health","heaven","hidden","higher","hollow","hunter","impact",
        "import","income","insect","inside","insist","intact","intend","island","itself",
        "jungle","kernel","kettle","killer","kitten","knight","lander","larger","launch",
        "leader","lessen","lesson","letter","market","mirror","modern","monkey","mother",
        "murder","mutual","narrow","nation","native","nature","nearby","needle","negate",
        "nephew","normal","notice","number","object","obtain","occupy","office","orange",
        "origin","output","oyster","palace","parrot","patent","pepper","permit","person",
        "planet","player","plenty","pocket","poetry","poison","police","policy","prefer",
        "pretty","profit","proper","public","punish","puppet","purple","rabbit","rather",
        "reason","recent","record","reduce","reform","refuse","region","relate","remain",
        "remove","render","report","return","reveal","review","reward","riddle","rocket",
        "rotate","rubber","salary","sample","scream","search","season","second","secure",
        "select","settle","shadow","should","signal","silent","silver","simple","single",
        "sister","sketch","slight","slowly","smooth","social","socket","soften","source",
        "spring","square","strain","street","stream","strict","strike","string","strong",
        "summer","supply","switch","symbol","system","talent","target","tennis","theory",
        "throne","ticket","timber","tissue","tomato","toward","travel","triple","tunnel",
        "turtle","twelve","twenty","typing","unique","unless","update","useful","vessel",
        "victim","vision","volume","wallet","warden","wealth","weapon","within","wonder",
        "wooden","writer","yellow","zombie",
        nullptr 
    };

    for (int i = 0; WORDS[i] != nullptr; ++i) {
        std::string w(WORDS[i]);
        dict_.insert(w);
        wordList_.push_back(w);
    }
}

} 
