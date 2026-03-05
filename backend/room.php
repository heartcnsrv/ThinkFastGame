<?php

require_once __DIR__ . '/config.php';

$body   = input();
$action = $body['action']   ?? '';
$player = trim($body['player'] ?? '');
$code   = strtoupper(trim($body['code']   ?? ''));

if (!$player && $action !== 'create') {
    jsonErr('Player name required.');
}

switch ($action) {

    case 'create': {
        $mode      = $body['mode']      ?? 'last_letter';
        $timeLimit = (int)($body['time_limit'] ?? 15);
        $maxPlayers= (int)($body['max_players'] ?? 4);

        do {
            $code = generateRoomCode();
        } while (file_exists(roomPath($code)));

        $room = [
            'code'        => $code,
            'mode'        => $mode,
            'time_limit'  => $timeLimit,
            'max_players' => $maxPlayers,
            'status'      => 'waiting',  
            'host'        => $player,
            'players'     => [
                ['name' => $player, 'hearts' => 3, 'eliminated' => false, 'last_seen' => time()],
            ],
            'current_idx' => 0,
            'last_word'   => '',
            'required_letter' => '',
            'used_words'  => [],
            'building'    => '',
            'round'       => 1,
            'turn_started'=> 0,
            'log'         => [],
            'created_at'  => time(),
        ];
        saveRoom($code, $room);
        jsonOk(['code' => $code, 'room' => $room]);
    }

    case 'join': {
        $room = loadRoom($code);
        if (!$room)                         jsonErr('Room not found.', 404);
        if ($room['status'] !== 'waiting')  jsonErr('Game already started.');
        if (count($room['players']) >= $room['max_players']) jsonErr('Room is full.');

        foreach ($room['players'] as $p) {
            if ($p['name'] === $player) {
                jsonOk(['code' => $code, 'room' => $room]);
            }
        }

        $room['players'][] = [
            'name'       => $player,
            'hearts'     => 3,
            'eliminated' => false,
            'last_seen'  => time(),
        ];
        saveRoom($code, $room);
        jsonOk(['code' => $code, 'room' => $room]);
    }

    case 'state': {
        $room = loadRoom($code);
        if (!$room) jsonErr('Room not found.', 404);

        foreach ($room['players'] as &$p) {
            if ($p['name'] === $player) {
                $p['last_seen'] = time();
            }
        }
        unset($p);

        if ($room['status'] === 'playing') {
            foreach ($room['players'] as &$p) {
                if (!$p['eliminated'] && (time() - $p['last_seen']) > 15) {
                    $p['eliminated'] = true;
                    $p['hearts']     = 0;
                    $room['log'][]   = $p['name'] . ' disconnected and was eliminated.';
                }
            }
            unset($p);
            $active = array_filter($room['players'], fn($p) => !$p['eliminated']);
            if (count($active) <= 1) {
                $room['status'] = 'finished';
            }
        }

        saveRoom($code, $room);
        jsonOk(['room' => $room, 'is_your_turn' => isYourTurn($room, $player)]);
    }

    case 'start': {
        $room = loadRoom($code);
        if (!$room)                        jsonErr('Room not found.', 404);
        if ($room['host'] !== $player)     jsonErr('Only the host can start.');
        if ($room['status'] !== 'waiting') jsonErr('Game already started.');
        if (count($room['players']) < 2)   jsonErr('Need at least 2 players.');

        $room['status']       = 'playing';
        $room['current_idx']  = 0;
        $room['turn_started'] = time();

        if ($room['mode'] === 'last_letter') {
            $starters = ['apple','bread','crane','drift','eagle','flame','grant',
                         'hotel','index','jewel','knife','lemon','maple','novel',
                         'ocean','piano','quest','radio','stone','tiger','ultra',
                         'value','water','xenon','yacht','zebra'];
            $start = $starters[array_rand($starters)];
            $room['last_word']       = $start;
            $room['required_letter'] = substr($start, -1);
            $room['used_words'][]    = $start;
            $room['log'][]           = 'Game started. First word: ' . $start;
        } else {
            $room['building'] = '';
            $room['log'][]    = 'Game started. One-by-One mode.';
        }

        saveRoom($code, $room);
        jsonOk(['room' => $room]);
    }

    case 'move': {
        $room = loadRoom($code);
        if (!$room)                        jsonErr('Room not found.', 404);
        if ($room['status'] !== 'playing') jsonErr('Game is not in progress.');
        if (!isYourTurn($room, $player))   jsonErr('Not your turn.');

        $value = strtolower(trim($body['value'] ?? ''));
        if (!$value) jsonErr('No value provided.');

        $log = $room['log'];

        if ($room['mode'] === 'last_letter') {
            $req = $room['required_letter'];
            if ($value[0] !== $req) {
                jsonErr("Word must start with '" . strtoupper($req) . "'.");
            }
            if (!preg_match('/^[a-z]+$/', $value)) {
                jsonErr('Letters only.');
            }
            if (in_array($value, $room['used_words'])) {
                jsonErr('Word already used.');
            }
            $valid = serverValidateWord($value);
            if (!$valid) {
                jsonErr('"' . $value . '" is not a recognised English word.');
            }

            $room['last_word']       = $value;
            $room['required_letter'] = substr($value, -1);
            $room['used_words'][]    = $value;
            $log[] = $player . ' played: ' . $value;
            advanceTurn($room);

        } else {
            if (strlen($value) !== 1 || !ctype_alpha($value)) {
                jsonErr('Submit exactly one letter.');
            }
            $candidate = $room['building'] . $value;
            if (strlen($candidate) >= 3 && serverValidateWord($candidate)) {
                $room['building'] = $candidate;
                $log[] = $player . ' completed the word: ' . $candidate;
                foreach ($room['players'] as &$p) {
                    if ($p['name'] !== $player && !$p['eliminated']) {
                        $p['hearts']--;
                        $log[] = $p['name'] . ' lost a heart.';
                        if ($p['hearts'] <= 0) {
                            $p['eliminated'] = true;
                            $log[] = $p['name'] . ' eliminated.';
                        }
                    }
                }
                unset($p);
                $active = array_filter($room['players'], fn($p) => !$p['eliminated']);
                if (count($active) <= 1) {
                    $room['status'] = 'finished';
                } else {
                    $room['building'] = '';
                    $room['round']++;
                    advanceTurn($room);
                }
            } elseif (!isValidPrefix($candidate)) {
                $log[] = $player . " added '$value' -> '$candidate' (no valid word). -1 heart.";
                foreach ($room['players'] as &$p) {
                    if ($p['name'] === $player) {
                        $p['hearts']--;
                        if ($p['hearts'] <= 0) {
                            $p['eliminated'] = true;
                            $log[] = $player . ' eliminated.';
                        }
                    }
                }
                unset($p);
                $room['building'] = '';
                advanceTurn($room);
            } else {
                $room['building'] = $candidate;
                $log[] = $player . ' added: ' . $value . ' -> ' . $candidate;
                advanceTurn($room);
            }
        }

        $room['log']         = array_slice($log, -50);
        $room['turn_started']= time();
        $active = array_filter($room['players'], fn($p) => !$p['eliminated']);
        if (count($active) <= 1) $room['status'] = 'finished';

        saveRoom($code, $room);
        jsonOk(['room' => $room, 'is_your_turn' => isYourTurn($room, $player)]);
    }

    case 'leave': {
        $room = loadRoom($code);
        if (!$room) jsonOk(['message' => 'Room not found.']);

        foreach ($room['players'] as &$p) {
            if ($p['name'] === $player) {
                $p['eliminated'] = true;
                $p['hearts']     = 0;
            }
        }
        unset($p);
        $room['log'][] = $player . ' left the game.';
        $active = array_filter($room['players'], fn($p) => !$p['eliminated']);
        if (count($active) <= 1) $room['status'] = 'finished';

        saveRoom($code, $room);
        jsonOk(['message' => 'Left room.']);
    }

    default:
        jsonErr('Unknown action.', 400);
}


function isYourTurn(array $room, string $player): bool {
    if ($room['status'] !== 'playing') return false;
    $players = $room['players'];
    $idx     = $room['current_idx'] % count($players);
    return $players[$idx]['name'] === $player && !$players[$idx]['eliminated'];
}

function advanceTurn(array &$room): void {
    $n = count($room['players']);
    for ($i = 0; $i < $n; $i++) {
        $room['current_idx'] = ($room['current_idx'] + 1) % $n;
        if (!$room['players'][$room['current_idx']]['eliminated']) break;
    }
}

function serverValidateWord(string $word): bool {
    static $dict = null;
    if ($dict === null) {
        $core = 'able,about,above,acid,acre,across,act,add,admit,adult,after,again,age,
                 ago,agree,ahead,aim,air,alarm,alive,all,allow,alone,along,also,alter,
                 amid,angel,anger,angle,animal,answer,ant,any,ape,apple,apply,area,argue,
                 arise,arm,armor,arrow,aside,atlas,auto,award,aware,awful,baby,back,ball,
                 band,base,bath,bear,beast,beat,begin,below,bench,best,bind,bird,bite,
                 black,blade,blank,blast,blaze,blend,blind,blink,block,blood,bloom,blow,
                 blue,board,bold,bolt,bone,book,boom,boot,bore,born,both,bread,break,
                 brew,brick,bride,bring,broad,broke,brook,brown,build,bunch,burn,burst,
                 cage,cake,calm,came,cane,card,care,cart,case,cash,cast,cause,cell,
                 chain,chair,charm,chase,chest,chief,child,chunk,city,claim,clash,clean,
                 clear,climb,close,cloud,coat,code,coin,cold,come,cone,cook,cool,coral,
                 corn,cost,count,court,cover,crack,craft,crash,cream,crime,crop,crowd,
                 crown,crush,cube,cure,curl,cycle,dance,dare,dark,dart,data,dawn,dead,
                 deal,dear,deck,deed,deep,deny,desk,dice,dirt,dish,dive,dock,dome,doom,
                 door,down,draft,drain,drama,dream,dress,drift,drill,drink,drive,drove,
                 drum,duck,dust,duty,each,earl,earn,ease,east,edge,edit,eight,empty,
                 enemy,enter,equal,error,essay,event,exact,exist,extra,fable,faith,fall,
                 fame,farm,fast,fate,fear,feat,feed,feel,fell,fence,fever,field,final,
                 fire,firm,fish,flag,flame,flat,flesh,flood,floor,flow,foam,fold,food,
                 foot,ford,fork,form,fort,free,from,fuel,game,gang,gaze,gear,gift,girl,
                 give,glad,glee,glow,goal,goat,gold,good,grab,grain,grand,grant,grass,
                 gray,green,greet,grief,grind,grove,guard,guess,guide,guilt,hack,hand,
                 hard,harm,heart,heavy,heel,held,hello,help,herb,hero,hide,high,hill,
                 hint,hire,hold,hole,holy,home,hook,hope,horn,host,huge,hunt,hurt,icon,
                 idle,into,iron,jack,jade,jump,just,keen,keep,kill,kind,king,knew,know,
                 lack,lake,lamb,lamp,land,lane,lark,last,late,lawn,lead,lean,leap,left,
                 lend,lens,life,lift,like,lime,line,link,lion,live,load,loan,lock,lone,
                 long,look,lord,love,luck,lung,lust,mail,main,make,male,mare,mark,mate,
                 maze,meal,mean,meat,melt,mile,milk,mind,mint,mist,mock,mole,moor,more,
                 moss,move,much,must,myth,nail,name,neck,need,news,next,nice,nine,node,
                 nose,note,noun,oath,once,open,oven,over,pace,pack,page,pain,pair,palm,
                 park,part,pass,past,path,peak,pest,pile,pine,pipe,plan,play,plot,plow,
                 poem,pole,pond,poor,pore,pork,port,pour,pray,prey,pull,pump,pure,push,
                 race,rack,rage,rail,rain,rake,rank,rant,read,real,rear,reed,reel,rely,
                 rent,rest,rice,ride,ring,rise,risk,road,roam,roar,robe,rock,role,roll,
                 roof,room,rope,rose,ruin,rule,rush,rust,safe,sail,salt,same,sand,sane,
                 seal,seat,seed,seek,sell,send,shed,ship,shoe,shop,shot,show,shut,side,
                 sign,silk,sing,sink,skin,slow,snap,snow,soap,sock,soft,soil,some,song,
                 sore,sort,soul,soup,sour,span,spin,spot,star,stay,stem,step,stir,stop,
                 suit,swap,tack,tail,tale,talk,tall,tame,task,team,tear,tell,term,test,
                 text,tick,tier,till,time,tire,told,toll,tone,tool,town,trim,trio,trip,
                 true,tuck,tune,turn,tusk,twin,type,ugly,unit,used,vain,vary,vast,veil,
                 very,vest,view,vine,void,wade,wait,wake,walk,wall,want,ward,warm,warn,
                 wave,weak,weed,week,well,went,west,will,wind,wine,wing,wire,wise,wish,
                 wolf,wood,wool,word,work,wrap,year,yell,your,zero,zone,
                 about,above,abuse,actor,acute,after,again,agent,agree,ahead,alarm,album,
                 alert,alien,align,alive,alley,allow,aloft,alone,along,aloud,alter,angel,
                 anger,angle,ankle,annex,apart,apple,apply,arena,argue,arise,armor,aroma,
                 arrow,aside,asset,atlas,audio,audit,avoid,award,aware,awful,azure,badly,
                 basis,beach,beard,beast,begin,being,below,bench,black,blade,blame,blank,
                 blast,blaze,bleed,blend,bless,blind,blink,block,blood,bloom,blown,board,
                 bonus,boost,booth,bread,break,breed,brick,bride,brief,bring,broad,broke,
                 brook,brown,brush,build,built,bunch,burst,cabin,candy,carry,catch,cause,
                 chain,chair,chalk,charm,chase,cheap,check,cheer,chess,chest,chief,child,
                 chord,chunk,civil,claim,clash,class,clean,clear,clerk,click,cliff,climb,
                 clone,close,cloud,coach,coast,cobra,coral,count,court,cover,crack,craft,
                 crash,crazy,cream,creek,crime,crisp,cross,crowd,crown,crush,curve,cycle,
                 daily,dance,dealt,delay,delta,depot,depth,devil,dizzy,donor,doubt,draft,
                 drain,drama,dream,dress,drift,drill,drink,drive,drown,dying,eagle,early,
                 earth,eight,elite,empty,enemy,enjoy,enter,equal,error,essay,event,exact,
                 exist,extra,fable,faith,fairy,fancy,feast,fence,fever,field,final,flame,
                 flare,flash,fleet,flesh,floor,flush,flute,focus,forge,forum,found,frank,
                 freak,fresh,front,frost,fruit,funny,ghost,giant,glass,gleam,glide,globe,
                 gloom,glove,grace,grade,grain,grand,grant,grasp,grass,graze,greed,green,
                 greet,grief,grind,groan,grove,guard,guess,guide,guild,guilt,habit,happy,
                 harsh,havoc,heart,heavy,hedge,hello,hence,horse,hotel,hound,house,human,
                 humor,hurry,image,issue,jewel,joint,judge,juice,juicy,kayak,knife,knock,
                 label,lance,laser,laugh,layer,learn,legal,lemon,level,light,limit,local,
                 logic,loose,lucky,lunar,lyric,magic,major,manor,maple,march,match,mayor,
                 media,merit,metal,minor,model,money,month,moral,motor,mount,mouse,mouth,
                 movie,music,nerve,never,night,noble,noise,north,novel,nurse,ocean,offer,
                 olive,opera,orbit,order,otter,outer,owner,ozone,paint,panel,paper,party,
                 pasta,pause,peace,pearl,penny,phase,phone,photo,piano,pilot,pixel,pizza,
                 place,plain,plane,plant,plate,plaza,point,polar,power,press,price,pride,
                 prime,print,prize,probe,proof,proud,pulse,query,quest,quick,quiet,quote,
                 racer,radar,radio,ranch,range,rapid,raven,razor,reach,realm,rebel,relay,
                 remix,reply,rider,river,robot,rocky,rouge,round,royal,ruler,saint,salon,
                 sauce,scale,scary,scene,scout,sense,shade,shame,shape,share,sharp,shelf,
                 shell,shift,shine,shirt,shock,short,sight,skill,skull,sleep,slice,slide,
                 slope,smile,smoke,snake,solar,solid,solve,sorry,south,space,spark,speak,
                 speed,spend,spice,spike,spine,split,spoon,sport,spray,squad,staff,stage,
                 stake,stand,stare,start,state,steam,steel,stick,sting,stock,stone,storm,
                 story,stove,straw,study,style,sugar,suite,super,sweep,sweet,swirl,sword,
                 table,taste,teach,teeth,tempo,title,toast,topic,total,touch,tough,tower,
                 toxic,trace,track,trail,train,trait,trash,treat,trend,trial,tribe,trick,
                 troop,truck,trust,truth,twist,ultra,uncle,under,union,unity,upper,urban,
                 usage,usual,valid,value,vapor,vault,video,vigor,viral,virus,visit,vital,
                 vivid,vocal,voice,voter,waltz,water,watch,weave,wedge,wheat,wheel,white,
                 witch,woman,world,worry,worth,wound,yacht,yield,young,youth,zebra,zesty';
        $dict = array_flip(array_map('trim', explode(',', preg_replace('/\s+/', '', $core))));
    }
    return isset($dict[$word]);
}

function isValidPrefix(string $prefix): bool {
    if (strlen($prefix) <= 1) return true;
    if (strlen($prefix) <= 2) return true;
    return strlen($prefix) < 6 || serverValidateWord($prefix);
}
