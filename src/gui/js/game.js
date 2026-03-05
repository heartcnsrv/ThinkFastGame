//
//  ThinkFast  |  src/gui/js/game.js
//
//  
//  ALL logic lives in the C++ server (ThinkFastServer).
//
//  API calls go to /api/* which Apache proxies via proxy.php
//  to the C++ server at localhost:8080:
//    POST /api/auth        → C++ AuthManager
//    POST /api/validate    → C++ WordValidator::isValid()
//    POST /api/leaderboard → C++ AuthManager::leaderboard()
//    POST /api/room        → C++ RoomManager (all game rules)
//
//  Word validation for LOCAL (vs bots) games still uses
//  the JS local dict + /api/validate for unknown words,
//  so the C++ WordValidator is always the authority.
// 
"use strict";

const BACKEND = '/api';

const _wordCache = {};

// ── Built-in dictionary (same words as WordValidator::buildBuiltinDictionary) ─
// Used for instant local checks and bot AI. API used for unknown words.
const WORD_SET = new Set([
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
  "bill","bind","bird","bite","blow","blue","blur","boat","bold","bolt","bond",
  "bone","book","boom","boot","bore","born","bout","bran","brew","buck","buff",
  "bulk","bull","bump","burn","bush","buzz","cage","cake","call","calm","cane",
  "card","care","cart","case","cash","cast","cave","cell","cent","chat","chef",
  "chin","chip","chop","cite","city","clam","clap","claw","clay","clip","clot",
  "club","clue","coal","coat","code","coil","coin","cold","come","cone","cook",
  "cool","core","corn","cost","cozy","crab","crew","crop","crow","cube","curb",
  "cure","curl","damp","dare","dark","dart","data","date","dawn","dead","deal",
  "dear","deck","deed","deep","deer","deny","desk","dial","dice","dirt","disc",
  "dish","disk","dive","dock","dome","done","doom","door","dose","dove","down",
  "drop","drum","dual","duck","dull","dump","dusk","dust","duty","earl","earn",
  "ease","east","edge","edit","emit","even","ever","evil","exam","exit","face",
  "fact","fade","fail","fake","fall","fame","fare","farm","fast","fate","fear",
  "feat","feed","feel","feet","fell","felt","fend","fern","file","fill","film",
  "find","fine","fire","firm","fish","fist","flag","flat","flaw","flea","fled",
  "flex","flip","flow","foam","fold","folk","fond","font","food","fool","foot",
  "ford","fore","fork","form","fort","foul","four","free","frog","from","fuel",
  "fuse","fuzz","gain","gale","game","gang","gaze","gear","germ","gift","girl",
  "give","glad","glee","glen","glow","glue","goal","goat","gold","golf","gown",
  "grab","gram","gray","grew","grid","grim","grin","grip","grit","grow","gulf",
  "gull","gust","guts","hack","hail","hair","half","hall","halt","hand","hang",
  "hard","harm","harp","hash","haul","have","hawk","haze","head","heal","heap",
  "heat","heel","hell","help","herb","herd","here","hero","hide","high","hill",
  "hint","hire","hive","hold","hole","holy","home","hood","hook","hope","horn",
  "hose","host","hour","huge","hull","hung","hunt","hurt","hymn","idle","into",
  "iron","isle","item","jade","jaws","jerk","jolt","jump","just","keen","keep",
  "kill","kind","king","knee","knob","know","lack","lake","lamb","lamp","land",
  "lane","lash","last","late","lawn","lazy","lead","leaf","leak","lean","leap",
  "left","lend","lens","liar","life","lift","like","lime","line","link","lion",
  "list","live","load","loan","lock","loft","lone","long","look","lord","lore",
  "lose","loss","lost","love","luck","lull","lump","lung","lust","mace","made",
  "mail","main","make","male","mall","mane","mare","mark","mast","math","mate",
  "maze","meal","mean","meat","melt","mere","mesh","mile","milk","mill","mine",
  "mint","mist","mock","mode","mole","moor","more","moss","most","move","much",
  "mule","must","myth","nail","name","neck","need","news","next","nice","nine",
  "node","none","noon","norm","nose","note","noun","nude","obey","once","only",
  "open","oval","oven","over","pace","pack","page","paid","pain","pair","palm",
  "pane","park","part","pass","past","path","peak","pear","peel","peer","pest",
  "pick","pile","pine","pipe","plan","play","plod","plot","plow","plug","plum",
  "plus","poem","poet","pole","poll","pond","pony","pool","poor","pore","pork",
  "port","pose","post","pour","pray","prey","prod","prop","pull","pump","pure",
  "push","quad","race","rack","rage","raid","rail","rain","rake","ramp","rank",
  "rant","rash","rate","read","real","reap","rear","reed","reel","rein","rely",
  "rent","rest","rice","rich","ride","ring","rise","risk","rite","road","roam",
  "roar","robe","rock","role","roll","roof","room","rope","rose","ruin","rule",
  "rush","rust","safe","sage","sail","sake","sale","salt","same","sand","sane",
  "seal","seat","seed","seek","self","sell","send","shed","ship","shoe","shop",
  "shot","show","shut","sick","side","sigh","sign","silk","sill","sing","sink",
  "site","size","skin","slab","slap","slim","slip","slot","slow","slug","smog",
  "snap","snob","snow","soak","soap","sock","soft","soil","sole","some","song",
  "soot","sore","sort","soul","soup","sour","span","spar","spin","spit","spot",
  "stab","stag","star","stay","stem","step","stir","stop","stub","stud","such",
  "suit","sung","sunk","swan","swap","sway","tack","tail","tale","talk","tall",
  "tame","tart","task","team","tear","tell","term","test","text","than","tick",
  "tidy","tier","till","time","tiny","tire","toad","told","toll","tomb","tone",
  "took","tool","torn","tort","toss","tour","town","trim","trio","trip","trod",
  "true","tuck","tune","turf","turn","tusk","twin","type","ugly","undo","unit",
  "upon","used","user","vain","vale","vary","vast","veil","vein","very","vest",
  "veto","view","vile","vine","void","volt","wade","wage","wait","wake","walk",
  "wall","wand","want","ward","warm","warn","warp","wash","wave","weak","wear",
  "weed","week","well","welt","went","were","west","what","when","whim","whip",
  "will","wilt","wind","wine","wing","wink","wire","wise","wish","with","wolf",
  "wood","wool","word","wore","work","worm","wrap","wren","year","yell","your",
  "zero","zone",
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
  // 6+ letter
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
  "hazard","health","heaven","hidden","hollow","hunter","impact","import","income",
  "insect","inside","insist","intact","intend","island","jungle","kettle","killer",
  "kitten","knight","lander","larger","launch","leader","lessen","lesson","letter",
  "market","mirror","modern","monkey","mother","murder","mutual","narrow","nation",
  "native","nature","nearby","needle","nephew","normal","notice","number","object",
  "obtain","occupy","office","orange","origin","output","oyster","palace","parrot",
  "patent","pepper","permit","person","planet","player","plenty","pocket","poetry",
  "poison","police","policy","prefer","pretty","profit","proper","public","punish",
  "puppet","purple","rabbit","rather","reason","recent","record","reduce","reform",
  "refuse","region","relate","remain","remove","render","report","return","reveal",
  "review","reward","riddle","rocket","rotate","rubber","salary","sample","scream",
  "search","season","second","secure","select","settle","shadow","should","signal",
  "silent","silver","simple","single","sister","sketch","slight","slowly","smooth",
  "social","socket","soften","source","spring","square","strain","street","stream",
  "strict","strike","string","strong","summer","supply","switch","symbol","system",
  "talent","target","tennis","theory","throne","ticket","timber","tissue","tomato",
  "toward","travel","triple","tunnel","turtle","twelve","twenty","unique","unless",
  "update","useful","vessel","victim","vision","volume","wallet","warden","wealth",
  "weapon","within","wonder","wooden","writer","yellow","zombie"
]);

const BOT_ADJ  = ["Swift","Bold","Quiet","Sharp","Calm","Bright","Sly","Wise","Grim","Keen",
                   "Stern","Witty","Eager","Proud","Deft","Nimble","Stoic","Quick","Rapid",
                   "Clever","Fierce","Steady","Brave","Mellow","Chill","Silent","Blunt","Wary"];
const BOT_NOUN = ["Fox","Wolf","Hawk","Bear","Raven","Sage","Drake","Finch","Crane","Lynx",
                   "Viper","Colt","Storm","Pine","Flint","Marsh","Crest","Dusk","Dawn","Tide",
                   "Peak","Brook","Ridge","Blaze","Grove","Reed","Vale","Stone","Birch","Tern"];
function randomBotName() {
  return BOT_ADJ[Math.floor(Math.random()*BOT_ADJ.length)] +
         BOT_NOUN[Math.floor(Math.random()*BOT_NOUN.length)] +
         (10 + Math.floor(Math.random()*90));
}

function lastChar(w) { return w[w.length-1].toLowerCase(); }

function validWordLocal(w) { return WORD_SET.has(w.toLowerCase().trim()); }

function isValidPrefix(prefix) {
  const p = prefix.toLowerCase();
  if (p.length <= 1) return true;
  for (const w of WORD_SET) if (w.startsWith(p)) return true;
  return false;
}

function botPickWord(startLetter, usedWords) {
  const sl = startLetter.toLowerCase();
  const cands = [];
  for (const w of WORD_SET) if (w[0] === sl && !usedWords.has(w)) cands.push(w);
  if (!cands.length) return null;
  return cands[Math.floor(Math.random() * cands.length)];
}

function botNextLetter(partial) {
  const p = partial.toLowerCase();
  const letters = new Set();
  for (const w of WORD_SET) if (w.length > p.length && w.startsWith(p)) letters.add(w[p.length]);
  if (!letters.size) return String.fromCharCode(97 + Math.floor(Math.random()*26));
  const arr = [...letters];
  return arr[Math.floor(Math.random()*arr.length)];
}

function randomStartWord() {
  const pool = [];
  for (const w of WORD_SET) if (w.length >= 4 && w.length <= 6) pool.push(w);
  return pool[Math.floor(Math.random() * pool.length)] || "apple";
}

async function validWordAsync(w) {
  const key = w.toLowerCase().trim();
  if (_wordCache[key] !== undefined) return _wordCache[key];
  if (WORD_SET.has(key)) { _wordCache[key] = true; return true; }
  const r = await api('validate', { word: key });
  const valid = !!(r && r.valid);
  _wordCache[key] = valid;
  if (valid) WORD_SET.add(key);
  return valid;
}


let currentUser = null; 
let gameState   = null;  
let mpState     = null;  
let timerInterval = null;
let timerSeconds  = 0;

function $(id) { return document.getElementById(id); }

function showScreen(name) {
  document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
  const el = $('screen-' + name);
  if (el) el.classList.add('active');
}

function toast(msg, type = '') {
  const area = $('toast-area');
  const el = document.createElement('div');
  el.className = 'toast' + (type ? ' ' + type : '');
  el.textContent = msg;
  area.appendChild(el);
  setTimeout(() => el.remove(), 3200);
}

async function api(endpoint, body) {
  try {
    const res  = await fetch(`${BACKEND}/${endpoint}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body),
    });
    const text = await res.text();
    if (text.trimStart().startsWith('<')) {
      console.error('[ThinkFast] Got HTML instead of JSON from', endpoint,
        '\nCheck: 1) ThinkFastServer.exe is running  2) Apache vhost config  3) PHP curl enabled');
      return { ok: false, error: 'C++ backend unreachable. Is ThinkFastServer running?' };
    }
    return JSON.parse(text);
  } catch (e) {
    return { ok: false, error: 'Network error: ' + e.message };
  }
}

function switchLoginTab(tab) {
  ['login','register','guest'].forEach(t => {
    $('form-' + t).style.display = (t === tab) ? 'block' : 'none';
    document.querySelector(`.tab[data-tab="${t}"]`).classList.toggle('active', t === tab);
  });
}

async function doLogin() {
  const user = $('login-user').value.trim();
  const pass = $('login-pass').value;
  const alert = $('login-alert');
  alert.style.display = 'none';

  if (!user || !pass) { alert.textContent = 'Username and password required.'; alert.style.display = 'block'; return; }

  const r = await api('auth', { action: 'login', username: user, password: pass });
  if (!r.ok) { alert.textContent = r.error || 'Login failed.'; alert.style.display = 'block'; return; }

  currentUser = r;
  showScreen('menu');
}

async function doRegister() {
  const user  = $('reg-user').value.trim();
  const pass  = $('reg-pass').value;
  const pass2 = $('reg-pass2').value;
  const alert = $('reg-alert');
  alert.style.display = 'none';

  if (user.length < 2)  { alert.textContent = 'Username must be at least 2 characters.'; alert.style.display='block'; return; }
  if (pass.length < 3)  { alert.textContent = 'Password must be at least 3 characters.'; alert.style.display='block'; return; }
  if (pass !== pass2)   { alert.textContent = 'Passwords do not match.'; alert.style.display='block'; return; }

  const r = await api('auth', { action: 'register', username: user, password: pass });
  if (!r.ok) { alert.textContent = r.error || 'Registration failed.'; alert.style.display='block'; return; }

  currentUser = r;
  showScreen('menu');
}

function doGuest() {
  const name = $('guest-name').value.trim() || 'Guest';
  currentUser = { username: name, guest: true, wins: 0, losses: 0, games_played: 0 };
  showScreen('menu');
}

function doLogout() {
  stopTimer();
  currentUser = null;
  gameState   = null;
  mpState     = null;
  showScreen('login');
}

async function saveStats() {
  if (!currentUser || currentUser.guest) return;
  await api('auth', {
    action: 'stats',
    username: currentUser.username,
    wins:    currentUser.wins,
    losses:  currentUser.losses,
    games:   currentUser.games_played,
  });
}


function goToLobby()       { showScreen('lobby'); }
function goToMultiplayer() { showScreen('multiplayer'); }

async function goToLeaderboard() {
  showScreen('leaderboard');
  const r = await api('leaderboard', {});
  const tbody = $('lb-body');
  if (!r.ok || !r.players || !r.players.length) {
    tbody.innerHTML = '<tr><td colspan="5" class="tmuted" style="padding:16px">No players yet.</td></tr>';
    return;
  }
  tbody.innerHTML = r.players.map((p, i) => {
    const cls = i===0 ? 'gold' : i===1 ? 'silver' : i===2 ? 'bronze' : '';
    return `<tr>
      <td><span class="rank-num ${cls}">${i+1}</span></td>
      <td class="bold">${esc(p.username)}</td>
      <td>${p.wins}</td><td>${p.losses}</td><td>${p.games_played}</td>
    </tr>`;
  }).join('');
}

let selectedMode = 'last_letter';
function selectMode(mode) {
  selectedMode = mode;
  document.querySelectorAll('.mode-tile').forEach(t => {
    t.classList.toggle('selected', t.dataset.mode === mode);
  });
  $('time-option').style.display = (mode === 'last_letter') ? 'flex' : 'none';
}


function startLocalGame() {
  const botCount  = parseInt($('bot-count').value);
  const timeLimit = parseInt($('time-limit').value);

  const bots = Array.from({ length: botCount }, () => ({
    name: randomBotName(),
    hearts: 3, 
    eliminated: false,
    isBot: true,
  }));

  const human = {
    name: currentUser.username,
    hearts: 3,
    eliminated: false,
    isBot: false,
  };

  gameState = {
    mode:           selectedMode,
    players:        [human, ...bots],
    currentIdx:     0,
    lastWord:       '',
    requiredLetter: '',
    usedWords:      new Set(),
    building:       '',
    round:          1,
    timeLimitSecs:  timeLimit,
    running:        true,
    log:            [],
  };

  if (selectedMode === 'last_letter') {
    const start = randomStartWord();
    gameState.lastWord       = start;
    gameState.requiredLetter = lastChar(start);
    gameState.usedWords.add(start);
    gameState.log.push({ text: `Game started. First word: ${start}`, type: 'info' });
    $('game-mode-label').textContent = 'Last Letter';
    $('word-label').textContent = 'Last word';
    $('req-letter-wrap').style.display = 'flex';
    $('word-input').placeholder = 'Type a word...';
  } else {
    gameState.building = '';
    gameState.log.push({ text: 'Game started. One-by-One mode.', type: 'info' });
    $('game-mode-label').textContent = 'One-by-One';
    $('word-label').textContent = 'Building';
    $('req-letter-wrap').style.display = 'none';
    $('word-input').placeholder = 'Type one letter...';
  }

  $('game-round-badge').textContent = 'Round 1';
  showScreen('game');
  renderGame();
  advanceToNextPlayer();
}

function renderGame() {
  if (!gameState) return;

  $('game-round-badge').textContent = 'Round ' + gameState.round;

  if (gameState.mode === 'last_letter') {
    $('word-value').textContent = gameState.lastWord || '-';
    if (gameState.lastWord) {
      $('word-value').classList.add('flash');
      setTimeout(() => $('word-value').classList.remove('flash'), 500);
    }
    $('required-letter-display').textContent = gameState.requiredLetter.toUpperCase();
  } else {
    $('word-value').textContent = gameState.building || '_';
    if (gameState.building) {
      $('word-value').classList.add('flash');
      setTimeout(() => $('word-value').classList.remove('flash'), 500);
    }
  }

  const panel = $('game-players');
  panel.innerHTML = gameState.players.map((p, i) => {
    const isActive = (i === gameState.currentIdx);
    const hearts   = Array.from({ length: 3 }, (_, j) =>
      `<span class="heart-dot${j < p.hearts ? '' : ' empty'}"></span>`
    ).join('');
    return `<div class="player-card${isActive ? ' active-turn' : ''}${p.eliminated ? ' out' : ''}">
      <div class="player-card-name">
        ${esc(p.name)}${p.isBot ? ' <span class="bot-tag">BOT</span>' : ''}
      </div>
      <div class="hearts-row">${hearts}</div>
    </div>`;
  }).join('');

  const logEl = $('game-log');
  logEl.innerHTML = gameState.log.slice(-12).map(e =>
    `<div class="log-line ${e.type || ''}">${esc(e.text)}</div>`
  ).join('');
  logEl.scrollTop = logEl.scrollHeight;
}

function advanceToNextPlayer() {
  if (!gameState || !gameState.running) return;

  let safety = 0;
  while (gameState.players[gameState.currentIdx].eliminated && safety++ < gameState.players.length)
    gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;

  const cur = gameState.players[gameState.currentIdx];
  renderGame();

  if (cur.isBot) {
    setBannerWaiting(`${cur.name} is thinking...`);
    setInputEnabled(false);
    stopTimer();
    const delay = 700 + Math.floor(Math.random() * 700);
    setTimeout(() => doBotTurn(), delay);
  } else {
    const notice = $('turn-notice');
    notice.textContent = "Your turn!";
    notice.className = 'turn-banner your-turn';
    setInputEnabled(true);
    $('word-input').value = '';
    $('word-input').focus();
    startTimer(gameState.timeLimitSecs);
  }
}

function setBannerWaiting(msg) {
  const notice = $('turn-notice');
  notice.textContent = msg;
  notice.className = 'turn-banner';
}

function setInputEnabled(enabled) {
  $('word-input').disabled = !enabled;
  $('word-input-wrap').querySelector('.btn').disabled = !enabled;
}

function startTimer(seconds) {
  stopTimer();
  timerSeconds = seconds;
  const bar    = $('timer-bar');
  bar.style.width     = '100%';
  bar.className       = 'timer-fill';

  timerInterval = setInterval(() => {
    timerSeconds--;
    const pct = Math.max(0, (timerSeconds / seconds) * 100);
    bar.style.width = pct + '%';
    if (pct < 25)      bar.className = 'timer-fill danger';
    else if (pct < 50) bar.className = 'timer-fill warn';
    else               bar.className = 'timer-fill';

    if (timerSeconds <= 0) {
      stopTimer();
      onWrongWord("Time's up! -1 heart.");
    }
  }, 1000);
}

function stopTimer() {
  if (timerInterval) { clearInterval(timerInterval); timerInterval = null; }
  const bar = $('timer-bar');
  if (bar) { bar.style.width = '100%'; bar.className = 'timer-fill'; }
}

function submitWord() {
  if (!gameState) return;
  const raw  = $('word-input').value.trim().toLowerCase();
  if (!raw) return;
  $('word-input').value = '';

  if (gameState.mode === 'last_letter') {
    submitLastLetterWord(raw);
  } else {
    submitOneByOneLetter(raw[0]);
  }
}

async function submitLastLetterWord(word) {
  if (!gameState || !gameState.running) return;

  if (word[0] !== gameState.requiredLetter) {
    toast(`Must start with "${gameState.requiredLetter.toUpperCase()}".`, 'bad');
    return;
  }
  if (gameState.usedWords.has(word)) {
    toast(`"${word}" was already used.`, 'bad');
    return;
  }

  stopTimer();
  setInputEnabled(false);
  setBannerWaiting(`Checking "${word}"...`);

  const valid = await validWordAsync(word);

  if (!valid) {
    toast(`"${word}" is not a recognised English word.`, 'bad');
    onWrongWord(`"${word}" is not a real word. -1 heart.`);
    return;
  }

  acceptWord(word);
}

async function submitOneByOneLetter(letter) {
  if (!gameState || !gameState.running) return;
  if (!/^[a-z]$/.test(letter)) { toast('Type a single letter.', 'bad'); return; }

  stopTimer();
  setInputEnabled(false);

  const candidate = gameState.building + letter;

  if (candidate.length >= 3 && validWordLocal(candidate)) {
    gameState.building = candidate;
    gameState.log.push({ text: `${currentUser.username} completed: "${candidate}"`, type: 'good' });
    renderGame();

    gameState.players.forEach((p, i) => {
      if (i !== gameState.currentIdx && !p.eliminated) {
        p.hearts--;
        gameState.log.push({ text: `${p.name} -1 heart`, type: 'bad' });
        if (p.hearts <= 0) {
          p.eliminated = true;
          gameState.log.push({ text: `${p.name} eliminated!`, type: 'bad' });
        }
      }
    });

    renderGame();
    if (checkEndGame()) return;

    setTimeout(() => {
      gameState.building = '';
      gameState.round++;
      gameState.log.push({ text: `Round ${gameState.round} begins`, type: 'info' });
      advanceToNextPlayerOBO();
    }, 1400);

  } else if (!isValidPrefix(candidate)) {
    gameState.log.push({ text: `"${candidate}" has no valid continuation. -1 heart.`, type: 'bad' });
    onWrongWordOBO(candidate);
  } else {
    gameState.building = candidate;
    gameState.log.push({ text: `${currentUser.username} added: ${letter} → ${candidate}`, type: 'info' });
    renderGame();

    gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
    
    let safety = 0;
    while (gameState.players[gameState.currentIdx].eliminated && safety++ < gameState.players.length)
      gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;

    renderGame();
    setTimeout(() => advanceToNextPlayer(), 200);
  }
}

function doBotTurn() {
  if (!gameState || !gameState.running) return;
  const bot = gameState.players[gameState.currentIdx];

  if (gameState.mode === 'last_letter') {
    const word = botPickWord(gameState.requiredLetter, gameState.usedWords);
    if (!word) {
      gameState.log.push({ text: `${bot.name} [bot] couldn't find a word. -1 heart.`, type: 'bad' });
      bot.hearts--;
      if (bot.hearts <= 0) {
        bot.eliminated = true;
        gameState.log.push({ text: `${bot.name} eliminated!`, type: 'bad' });
      }
      renderGame();
      if (checkEndGame()) return;
      gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
      setTimeout(() => advanceToNextPlayer(), 600);
      return;
    }

    gameState.log.push({ text: `${bot.name} [bot]: ${word}`, type: 'info' });
    gameState.lastWord       = word;
    gameState.requiredLetter = lastChar(word);
    gameState.usedWords.add(word);
    renderGame();

  } else {
    const letter    = botNextLetter(gameState.building);
    const candidate = gameState.building + letter;
    gameState.log.push({ text: `${bot.name} [bot] added: ${letter} → ${candidate}`, type: 'info' });

    if (candidate.length >= 3 && validWordLocal(candidate)) {
      gameState.building = candidate;
      gameState.log.push({ text: `${bot.name} [bot] completed: "${candidate}"`, type: 'good' });
      renderGame();
      gameState.players.forEach((p, i) => {
        if (i !== gameState.currentIdx && !p.eliminated) {
          p.hearts--;
          gameState.log.push({ text: `${p.name} -1 heart`, type: 'bad' });
          if (p.hearts <= 0) { p.eliminated = true; gameState.log.push({ text: `${p.name} eliminated!`, type: 'bad' }); }
        }
      });
      renderGame();
      if (checkEndGame()) return;
      setTimeout(() => {
        gameState.building = '';
        gameState.round++;
        gameState.log.push({ text: `Round ${gameState.round} begins`, type: 'info' });
        advanceToNextPlayerOBO();
      }, 1000);
      return;
    } else if (!isValidPrefix(candidate)) {
      gameState.building = '';
      gameState.log.push({ text: `${bot.name}'s "${candidate}" had no continuation. -1 heart.`, type: 'bad' });
      bot.hearts--;
      if (bot.hearts <= 0) { bot.eliminated = true; gameState.log.push({ text: `${bot.name} eliminated!`, type: 'bad' }); }
      renderGame();
      if (checkEndGame()) return;
    } else {
      gameState.building = candidate;
    }
    renderGame();
  }

  gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  let safety = 0;
  while (gameState.players[gameState.currentIdx].eliminated && safety++ < gameState.players.length)
    gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  renderGame();
  setTimeout(() => advanceToNextPlayer(), 400);
}

function onWrongWord(msg) {
  const cur = gameState.players[gameState.currentIdx];
  cur.hearts--;
  gameState.log.push({ text: msg, type: 'bad' });
  if (cur.hearts <= 0) {
    cur.eliminated = true;
    gameState.log.push({ text: `${cur.name} eliminated!`, type: 'bad' });
  }
  setInputEnabled(false);
  renderGame();
  if (checkEndGame()) return;

  setTimeout(() => {
    gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
    let s = 0;
    while (gameState.players[gameState.currentIdx].eliminated && s++ < gameState.players.length)
      gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
    advanceToNextPlayer();
  }, 1200);
}

function onWrongWordOBO(candidate) {
  const cur = gameState.players[gameState.currentIdx];
  cur.hearts--;
  if (cur.hearts <= 0) {
    cur.eliminated = true;
    gameState.log.push({ text: `${cur.name} eliminated!`, type: 'bad' });
  }
  gameState.building = '';
  renderGame();
  if (checkEndGame()) return;
  gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  let s = 0;
  while (gameState.players[gameState.currentIdx].eliminated && s++ < gameState.players.length)
    gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  setTimeout(() => advanceToNextPlayer(), 1200);
}

function advanceToNextPlayerOBO() {
  gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  let s = 0;
  while (gameState.players[gameState.currentIdx].eliminated && s++ < gameState.players.length)
    gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  advanceToNextPlayer();
}

function acceptWord(word) {
  const cur = gameState.players[gameState.currentIdx];
  gameState.log.push({ text: `${cur.name}: ${word}`, type: 'good' });
  gameState.lastWord       = word;
  gameState.requiredLetter = lastChar(word);
  gameState.usedWords.add(word);
  setInputEnabled(false);
  renderGame();

  gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  let s = 0;
  while (gameState.players[gameState.currentIdx].eliminated && s++ < gameState.players.length)
    gameState.currentIdx = (gameState.currentIdx + 1) % gameState.players.length;
  setTimeout(() => advanceToNextPlayer(), 300);
}

function checkEndGame() {
  if (!gameState) return false;
  const alive = gameState.players.filter(p => !p.eliminated);
  if (alive.length > 1) return false;

  stopTimer();
  gameState.running = false;

  const winner = alive[0] || null;

  gameState.players.forEach(p => {
    if (currentUser && p.name === currentUser.username) {
      if (!p.eliminated) {
        currentUser.wins = (currentUser.wins || 0) + 1;
      } else {
        currentUser.losses = (currentUser.losses || 0) + 1;
      }
      currentUser.games_played = (currentUser.games_played || 0) + 1;
    }
  });
  saveStats();

  const isWin = winner && winner.name === currentUser.username;
  $('end-result').textContent  = isWin ? 'You Win!' : 'Game Over';
  $('end-result').className    = 'end-result ' + (isWin ? 'win' : 'loss');
  $('end-detail').textContent  = winner
    ? `${winner.name} wins the game!`
    : 'All players eliminated.';
  $('end-overlay').classList.add('show');
  return true;
}

function closeEndOverlay() {
  $('end-overlay').classList.remove('show');
  showScreen('menu');
}


let mpPollTimer = null;

async function createRoom() {
  const mode       = $('mp-mode').value;
  const timeLimit  = parseInt($('mp-time').value);
  const maxPlayers = parseInt($('mp-maxp').value);
  const r = await api('room', {
    action: 'create',
    player: currentUser.username,
    mode, time_limit: timeLimit, max_players: maxPlayers,
  });
  if (!r.ok) { toast(r.error, 'bad'); return; }
  enterMpLobby(r.room);
}

async function joinRoom() {
  const code  = $('join-code').value.trim().toUpperCase();
  const alert = $('join-alert');
  alert.style.display = 'none';
  if (code.length !== 6) { alert.textContent = 'Room code must be 6 characters.'; alert.style.display='block'; return; }
  const r = await api('room', { action: 'join', player: currentUser.username, code });
  if (!r.ok) { alert.textContent = r.error; alert.style.display='block'; return; }
  enterMpLobby(r.room);
}

function enterMpLobby(room) {
  mpState = { code: room.code, isHost: room.host === currentUser.username };
  $('room-code-display').textContent = room.code;
  $('mp-lobby-mode').textContent     = room.mode === 'last_letter' ? 'Last Letter' : 'One-by-One';
  $('start-room-btn').style.display  = mpState.isHost ? 'block' : 'none';
  showScreen('mp-lobby');
  renderWaitingList(room);
  startMpPoll();
}

function renderWaitingList(room) {
  $('waiting-list').innerHTML = room.players.map(p =>
    `<li>
      <span class="online-dot"></span>
      ${esc(p.name)}
      ${p.name === room.host ? '<span class="host-badge">HOST</span>' : ''}
    </li>`
  ).join('');
}

async function startRoom() {
  const r = await api('room', { action: 'start', player: currentUser.username, code: mpState.code });
  if (!r.ok) { toast(r.error, 'bad'); return; }
  enterMpGame(r.room);
}

function enterMpGame(room) {
  showScreen('mp-game');
  renderMpGame(room, false);
}

function renderMpGame(room, isYourTurn) {
  const modeLabel = room.mode === 'last_letter' ? 'Last Letter' : 'One-by-One';
  $('mp-round-badge').textContent = 'Round ' + room.round;

  if (room.mode === 'last_letter') {
    $('mp-word-label').textContent   = 'Last word';
    $('mp-word-value').textContent   = room.last_word || '-';
    $('mp-req-wrap').style.display   = 'flex';
    $('mp-req-letter').textContent   = (room.required_letter || '?').toUpperCase();
  } else {
    $('mp-word-label').textContent   = 'Building';
    $('mp-word-value').textContent   = room.building || '_';
    $('mp-req-wrap').style.display   = 'none';
  }

  const curPlayer = room.players[room.current_idx % room.players.length];
  $('mp-game-players').innerHTML = room.players.map(p => {
    const isActive = (p.name === curPlayer.name);
    const hearts   = Array.from({length:3},(_,j)=>
      `<span class="heart-dot${j<p.hearts?'':' empty'}"></span>`).join('');
    return `<div class="player-card${isActive?' active-turn':''}${p.eliminated?' out':''}">
      <div class="player-card-name">${esc(p.name)}</div>
      <div class="hearts-row">${hearts}</div>
    </div>`;
  }).join('');

  const notice   = $('mp-turn-notice');
  const inputWrap= $('mp-input-wrap');
  if (room.status === 'finished') {
    const alive = room.players.filter(p => !p.eliminated);
    const winner = alive[0];
    notice.textContent  = winner ? `Game over — ${winner.name} wins!` : 'Game over!';
    notice.className    = 'turn-banner';
    inputWrap.style.display = 'none';
    stopMpPoll();

    room.players.forEach(p => {
      if (currentUser && p.name === currentUser.username) {
        if (!p.eliminated) currentUser.wins = (currentUser.wins||0)+1;
        else               currentUser.losses = (currentUser.losses||0)+1;
        currentUser.games_played = (currentUser.games_played||0)+1;
      }
    });
    saveStats();

    const isWin = room.players.some(p => p.name===currentUser.username && !p.eliminated);
    $('end-result').textContent = isWin ? 'You Win!' : 'Game Over';
    $('end-result').className   = 'end-result '+(isWin?'win':'loss');
    $('end-detail').textContent = winner ? `${winner.name} wins!` : 'All players out.';
    setTimeout(() => $('end-overlay').classList.add('show'), 1500);
    return;
  }

  if (isYourTurn) {
    notice.textContent = 'Your turn!';
    notice.className   = 'turn-banner your-turn';
    inputWrap.style.display = 'flex';
    $('mp-word-input').placeholder = room.mode==='last_letter' ? 'Type a word...' : 'Type one letter...';
    $('mp-word-input').focus();
  } else {
    notice.textContent  = `${curPlayer.name}'s turn...`;
    notice.className    = 'turn-banner';
    inputWrap.style.display = 'none';
  }

  const logEl = $('mp-game-log');
  logEl.innerHTML = (room.log || []).slice(-12).map(t =>
    `<div class="log-line">${esc(t)}</div>`).join('');
  logEl.scrollTop = logEl.scrollHeight;
}

function startMpPoll() {
  stopMpPoll();
  mpPollTimer = setInterval(pollRoom, 2000);
  pollRoom();
}

function stopMpPoll() {
  if (mpPollTimer) { clearInterval(mpPollTimer); mpPollTimer = null; }
}

async function pollRoom() {
  if (!mpState) return;
  const r = await api('room', { action: 'state', player: currentUser.username, code: mpState.code });
  if (!r.ok) return;

  const room = r.room;
  if (room.status === 'waiting') {
    renderWaitingList(room);
    return;
  }
  if (room.status === 'playing' && document.getElementById('screen-mp-lobby').classList.contains('active')) {
    enterMpGame(room);
  }
  renderMpGame(room, r.is_your_turn);
}

async function submitMpMove() {
  if (!mpState) return;
  const val = $('mp-word-input').value.trim().toLowerCase();
  if (!val) return;
  $('mp-word-input').value = '';

  const r = await api('room', { action: 'move', player: currentUser.username, code: mpState.code, value: val });
  if (!r.ok) { toast(r.error, 'bad'); return; }
  renderMpGame(r.room, r.is_your_turn);
}

async function leaveRoom() {
  if (mpState) {
    stopMpPoll();
    await api('room', { action: 'leave', player: currentUser.username, code: mpState.code });
    mpState = null;
  }
  showScreen('menu');
}


function esc(s) {
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}

document.addEventListener('keydown', e => {
  if (e.key === 'Enter') {
    const s = document.querySelector('.screen.active');
    if (!s) return;
    const id = s.id;
    if (id === 'screen-login') {
      const active = document.querySelector('.tab.active');
      if (active) { if(active.dataset.tab==='login') doLogin(); else if(active.dataset.tab==='register') doRegister(); else doGuest(); }
    }
  }
});

