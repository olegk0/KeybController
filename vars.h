#define DEBOUNCE 10


#define FNKEY 0xff
#define EXTKEY 0xe0
#define TPKEY 0xf1
#define PAUSEKEY 0x81
#define PrScrKEY 0x82

const unsigned char PauseCode[] = {0xe1,0x14,0x77,0xe1,0xf0,0x14,0xf0,0x77,0x0};
const unsigned char PrScrCode[] = {0xe0,0x12,0xe0,0x7c,0x0};

const unsigned char scodes[18][8] =  {
//				 0      1      2    3    4    5    6     7
//				  v    t      f    g    r   6   5    F6
				{0x2a,0x2c,0x2b,0x34,0x2d,0x36,0x2e,0x0b},//0
//				  b    u    n    h    y    8    7     F7   
				{0x32,0x3c,0x31,0x33,0x35,0x3e,0x3d,0x83},//1
//			    EDown ERight EHOME EUp  		        Fn
				{0x72,0x74 , 0x6c,0x75,   0,   0,   0,FNKEY},//2
//				       1    a   CpsL  TAB   F1   `   ESC  
				{   0,0x16,0x1c,0x58,0x0D,0x05,0x0e,0x76},//3
//				                 RSh       LSh
				{   0,   0,   0,0x59,   0,0x12,   0,   0},//4
//				       RCtr                   LCtr     EEND
				{   0,EXTKEY+6,   0,  0,0,0, 0x14,   0x69},//5
//				                      Win        
				{0x1f,   0,   0,   0,EXTKEY,   0,  0,  0},//6
//				 z    q     x    s    w    F2    2    F3   
				{0x1a,0x15,0x22,0x1b,0x1d,0x06,0x1e,0x04},//7
//				       3    c    d     e    F4   4    F5
				{   0,0x26,0x21,0x23,0x24,0x0c,0x25,0x03},//8
//				    RAl         LAl
				{EXTKEY+2,   0,0x11,   0,   0,   0,   0,   0},//9
//				 SPS   j    ,    m    k    F8   i    F9
				{0x29,0x3b,0x41,0x3a,0x42,0x0a,0x43,0x01},//10
//				 CMenu     INS      
				{EXTKEY+3,EXTKEY+2,0x70,0x2f,   0,   0,   0, 0},//11
//				  '    p    .    EUR?  l    9    o    F10
				{0x52,0x4d,0x49,   0,0x4b,0x46,0x44,0x09},//12
//				 USD?  -     /         [   F12   0    F11
				{   0,0x4e,0x4a,   0,0x54,0x07,0x45,0x78},//13
//				 Left        ]     HOME       Up      ELeft PrScr     =  
				{EXTKEY+4,0x5b,EXTKEY+8+2,EXTKEY+8+2,0x6b,PrScrKEY,0x55, 0},//14
//				 Down        Right      ;    \    ENT   PAUSE   BcSps END  
				{EXTKEY+8+2,EXTKEY+8+2,0x4c,0x5d,0x5a,PAUSEKEY,0x66,EXTKEY+8+5},//15
//				 DEL                 PgUp      
				{EXTKEY+1,0x71,0x7d,EXTKEY+2,0,0,   0,  0},//16
//				            PgDn           
				{0x7a,   0,EXTKEY,   0,   0,0,0,0},	//17
};

const unsigned char fncodes[18][8] =  {
//				                  BL-off
				{0,0,0,0,0,0,0,EXTKEY+8+6},//0
//				    u-4        8-8  7-7 TP-off
				{0,0x6B,0,0,0,0x75,0x6c,TPKEY},//1
//			     ELgt-     EPlay EVolUp		EMute E/   Fn
				{0x63,0  ,0x34 , 0x32  , 0,0x23,0x4a,FNKEY},//2
//				            F1-?
				{0,0,0,0,0,0x4a,0,0},//3
//				 EVolDn ELgt+      RSh      LSh       ENext
				{ 0x21, 0x64,   0,0   , 0 ,0   ,   0,0x4d},//4
//				       RCtr                  EPower    EWake
				{    0,   0,   0,   0,   0,  0x37,0, 0x5e},//5
//				                      Win  ESleep      EBL-off
				{  0,   0,   0,   0,    0,  0x3f,   0,0x62},//6
//				             F2-Power     F3- Wake
				{0,0,0,0,0,EXTKEY+8+5,0,EXTKEY+8+5},//7
//				ESwDsp          F4-Sleep    F5-SwDsp
				{0x5f,0,0,0,0,EXTKEY+8+6,0,EXTKEY},//8
//				  RAl      LAl
				{   0,   0,  0,   0,   0,   0,   0,   0},//9
//				    j-1    m-0  k-2   F8-Mute   i-5 
				{0,0x69,0,0x70,0x72,EXTKEY+8+2,0x73,0},//10
//				 CMenu INS 
				{  0,     0,0,0,   0,   0,   0,   0},//11
//				    p-*  .-. EUR? l-3  9-9 o-6  F10
				{0,0x7c,0x71,  0,0x7a,0x7d,0x74, 0},//12
//				 USD?    /-+         F12-SCRL  0-/       F11-Nl
				{   0,0,0x79,   0,0,  0x7e  ,EXTKEY+8+2,0x77},//13
//				  Left-Lgt-    HOME-Play   Up-VolUp
				{EXTKEY+8+2,0,EXTKEY+8+2,EXTKEY+8+2,0,0,0,0},//14
//				 Down-VolDn  Right-Lgt+ ;--           END-Next
				{EXTKEY+8+4,EXTKEY+8+4,0x7b,0,0,0,0,EXTKEY+8+4},//15
//				  DEL      	     PgUp-Stop             
				{   0,   0x3b,  0,EXTKEY+1 ,0,0,   0,0},//16
//				            PgDn-Prev            
				{  0x15,   0,EXTKEY,   0,   0, 0,   0,  0 },	//17
};

enum{
	RELEASE = 0,
	PRESSED = 1,
	REPEAT = 2,
};

#define MINKEYDELAY 250
#define MAXKEYDELAY 1000
#define DEFKEYDELAY 500

#define MINKEYPER 33
#define MAXKEYPER 500
#define DEFKEYPER 100

unsigned char LedFl = 0;
unsigned char TouchPadOn = 1;
unsigned char FnKey=0;

uint16_t KeyDelay = DEFKEYDELAY;//ms
uint16_t KeyPeriod = DEFKEYPER;//ms

#define RXCMDTIMEOUT 100

#define PS2_CMD 0xF5 //data from/to ps2
#define KEY_CMD 0xF4 //data from keys

//На каждую принятую от компьютера команду, или проще сказать на каждый принятый байт,
// клавиатура/мышь должны обязательно ответить одним из следующих байтов:
#define ACKNOWLEDGE 0xFA
//– подтверждение об успешном приеме;
#define RESEND 0xFE
//  - команда принята с ошибкой (вероятно ошибка CRC);
#define FAILURE 0xFC
// – произошла ошибка (не знаю, что это такое, может внутренняя ошибка устройства?).

//Если компьютер примет от клавиатуры или мыши не 0xFA, а 0xFE или не дай бог 0xFC,
// то скорее всего будет пытаться переповторить посылку команды или последнего посланного байта.

//Для клавиатуры компьютер может послать следующие команды:
#define SET_KEYBOARD_INDICATORS 0xED
// – зажечь или потушить светодиоды CAPS/NUM/SCROLL.
// Если клавиатура принимает эту команду, то больше она не пошлет ничего, до тех пор,
// пока компьютер не пришлет следующий байт-параметр.
// Этот параметр определяет битовую маску – один бит – это один светодиод.
// Битовая маска для светодиолов клавиатуры определена вот так:
#define KEYBOARD_KANA_LOCK_ON     8 // Japanese keyboard
#define KEYBOARD_CAPS_LOCK_ON     4
#define KEYBOARD_NUM_LOCK_ON      2
#define KEYBOARD_SCROLL_LOCK_ON   1

#define SELECT_SCAN_CODE_SET 0xF0
//– установить текущую таблицу кодов клавиш. Следом будет байт-параметр, номер выбираемой таблицы;
#define READ_KEYBOARD_ID 0xF2
// – не знаю, что это такое. Драйвер из WinDDK похоже не использует эту команду.
#define SET_KEYBOARD_TYPEMATIC 0xF3
//– это тоже двухбайтовая команда. После этой команды следует параметр определяющий частоту
// повтора кодов при нажатой клавише и интервал времени между нажатием и началом повторов.
// Параметр байт typematic выглядит следующим образом:
// 0  	 d6  	 d5  	 t4  	 t3  	 t2  	 t1  	 t0 
//Бит 7  - всегда ноль.
//Биты d5 and d6 определяют задержку (Delay) от времени нажатия, когда посылается первый код,
// до момента когда начинаются повторы кодов, если удерживать клавишу. 
//Задержку можно вычислить по формуле (1+typematic[6:5])*250 миллисекунд.(250-1000ms)
//Биты t4, t3, t2, t1, t0 определяют частоту повторов кодов при удерживании клавиши нажатой.
// Период повторов можно вычислить по формуле:
//Period = (8 + typematic[2:0]) * (2^typematic[4:3]) * 0.00417 секунд.(33-500ms)

#define KEYBOARD_RESET 0xFF
// – получая эту команду клавиатура отвечает, как обычно, 0xFA, а затем, сбрасывается и посылает
// в ответ байт
#define KEYBOARD_COMPLETE_SUCCESS 0xAA
//После подачи напряжения питания клавиатура посылает компьютеру код KEYBOARD_COMPLETE_SUCCESS
 //и немедленно готова к работе.
//По умолчанию клавиатура посылает на нажатие один байт-код, а на отжатие клавиши два байта.
// Первый байт в кодах "отпускания клавиши" – это префикс отжатия
#define KEYBOARD_BREAK_CODE 0xF0

