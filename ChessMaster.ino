#include <FAB_LED.h>

#define runEvery(t) for (static uint16_t _lasttime;\
		(uint16_t)((uint16_t)millis() - _lasttime) >= (t);\
		_lasttime += (t))

//chess engine init
/***************************************************************************/
/* Ported chess program to Arduino UNO by rom3                             */
/* Based on:         micro-Max,       version 4.8                          */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/* Chessuino by Diego Cueva                                                */
/* Atmel ATMega644 and AVR GCC, by Andre Adrian                            */
/* May 12 2019        Germany                                              */
/***************************************************************************/

#define W while
#define M 0x88
#define S 128
#define I 8000

#define MYRAND_MAX 65535     /* 16bit pseudo random generator */
long  N, T;                  /* N=evaluated positions+S, T=recursion limit */
short Q, O, K, R, k=16;      /* k=moving side */
char *p, c[5], Z;            /* p=pointer to c, c=user input, computer output, Z=recursion counter */

char L,
w[]={0,2,2,7,-1,8,12,23},                             /* relative piece values    */
o[]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0, /* step-vector lists */
		7,-1,11,6,8,3,6,                                 /* 1st dir. in o[] per piece*/
		6,3,5,7,4,5,3,6};                                /* initial piece setup      */
/* board is left part, center-pts table is right part, and dummy */

char b[]={
		22, 19, 21, 23, 20, 21, 19, 22, 28, 21, 16, 13, 12, 13, 16, 21,
		18, 18, 18, 18, 18, 18, 18, 18, 22, 15, 10,  7,  6,  7, 10, 15,
		0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
		0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
		0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
		0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
		9,  9,  9,  9,  9,  9,  9,  9, 22, 15, 10,  7,  6,  7, 10, 15,
		14, 11, 13, 15, 12, 13, 11, 14, 28, 21, 16, 13, 12, 13, 16, 21, 0
};

char bk[16*8+1];

unsigned int seed=0;
uint32_t  byteBoard[8];

char sym[17] = {".?pnkbrq?P?NKBRQ"};
int mn=1;
char lastH[5], lastM[5];
unsigned short ledv=1;

String inputString = "";
bool stringComplete = false;  // whether the string is complete
/***************************************************************************/

//extern int  __bss_end;
//extern int  *__brkval;
//
#define TouchSensorOK 7
#define TouchSensorRIGHT 8
//#define TouchSensorLEFT 9
//#define TouchSensorDOWN 12
#define TouchSensorUP 12 //supposed to be 14 but the touch sensor acts erratic //PIN A0
//
ws2812b<D,2> strip;
//
const uint16_t numPixels = 256;
const uint8_t bitsPerPixel = 2;
const uint8_t numColors = 1<<bitsPerPixel;
// Palette
uint8_t palette[3*numColors] = {1,1,1, 8,8,8, 24,0,0, 0,24,0};
// Pixel array
uint8_t pixels[ARRAY_SIZE(numPixels, bitsPerPixel)];

uint8_t moves[] = {0, 0, 0, 0};
uint8_t cursor = 0;
uint8_t cursor_X = 0;
uint8_t cursor_Y = 0;
//
boolean currentStateOK = LOW;
boolean lastStateOK = LOW;
boolean currentStateRIGHT = LOW;
boolean lastStateRIGHT = LOW;
//boolean currentStateLEFT = LOW;
//boolean lastStateLEFT = LOW;
//boolean currentStateDOWN = LOW;
//boolean lastStateDOWN = LOW;
boolean currentStateUP = LOW;
boolean lastStateUP = LOW;
//
boolean m_move = 1;
void buttonOK() {
	currentStateOK = digitalRead(TouchSensorOK);
	if (currentStateOK == HIGH && lastStateOK == LOW){
		cursor++;
		if (cursor == 1) {
			moves[0] = cursor_X; moves[1] = cursor_Y;
		}
		else if (cursor == 2) {
			moves[2] = cursor_X; moves[3] = cursor_Y;
		}
		else if (cursor == 3) {
			inputString = move2string(moves[0],moves[1])+move2string(moves[2],moves[3]);
			stringComplete = true;
			moves[0] = moves[1] = moves[2] = moves[3] = 0;
			cursor_X = 0;
			cursor_Y = 0;
			cursor = 0;
			m_move = 1;
		}
	}
	lastStateOK = currentStateOK;
}
//
String move2string (uint8_t x, uint8_t y) {
	char m[2];
	char m1 = char(y/2+97);
	int nr = x/2+1;
	char m2[2]; String str; str = String(nr); str.toCharArray(m2,2);
	m[0]=m1;
	m[1]=m2[0];
	m[2]='\0';
	return String(m);
}
//
void string2move (String move) {
uint8_t nr;
nr = uint8_t(move.charAt(0));
moves[1]= (nr - 97)*2 ;
nr = uint8_t(move.charAt(1));
moves[0]= (nr - 49)*2;
nr = uint8_t(move.charAt(2));
moves[3]= (nr - 97)*2;
nr = uint8_t(move.charAt(3));
moves[2]= (nr - 49)*2;
m_move = 0;
}
void buttonRIGHT() {
	currentStateRIGHT = digitalRead(TouchSensorRIGHT);
	if (currentStateRIGHT == HIGH && lastStateRIGHT == LOW){
		if (cursor_Y < 14) {
			cursor_Y = cursor_Y + 2;
		}
		else cursor_Y = 0;
		delay(1);
	}
	lastStateRIGHT = currentStateRIGHT;
}
//
//void buttonLEFT() {
//	currentStateLEFT = digitalRead(TouchSensorLEFT);
//	if (currentStateLEFT == HIGH && lastStateLEFT == LOW){
//		if (cursor_Y > 0) {
//			cursor_Y = cursor_Y - 2;
//		}
//		else cursor_Y = 14;
//		delay(1);
//	}
//	lastStateLEFT = currentStateLEFT;
//}
//
//void buttonDOWN() {
//	currentStateDOWN = digitalRead(TouchSensorDOWN);
//	if (currentStateDOWN == HIGH && lastStateDOWN == LOW){
//		if (cursor_X > 0) {
//			cursor_X = cursor_X - 2;
//		}
//		else cursor_X = 14;
//		delay(1);
//	}
//	lastStateDOWN = currentStateDOWN;
//}
//
void buttonUP() {
	currentStateUP = digitalRead(TouchSensorUP);
	if (currentStateUP == HIGH && lastStateUP == LOW){
		if (cursor_X < 14) {
			cursor_X = cursor_X + 2;
		}
		else cursor_X = 0;
		delay(1);
	}
	lastStateUP = currentStateUP;
}

//The setup function is called once at startup of the sketch
void setup()
{
	// Add your initialization code here
	//Serial.begin(115200);
	//Serial.println(F("*** MicroMaxChess ***"));
	lastH[0] = 0;
	//serialBoard();
}

// The loop function is called in an endless loop
void loop()
{
	//Add your repeated code here
	buttonOK();
	buttonRIGHT();
//	buttonLEFT();
//	buttonDOWN();
	buttonUP();
	ClearBoard();
	DisplayBoard();
	DisplayChessPieces();
	runEvery(60) {
		if (m_move) ShowCursor(cursor_X, cursor_Y);
	}
	runEvery(20) {
		ShowMove();
	}
	strip.sendPixels<bitsPerPixel>(numPixels, pixels, palette);
	if (stringComplete == true) {
		int r;
		//Take move from human - from serial
			  //while (stringComplete == false) {
			  //  getserialchar();
			  //}
		//Serial.print(mn);
		//Serial.print(". ");
		//Serial.print(inputString.substring(0,4));
		c[0] = inputString.charAt(0);
		c[1] = inputString.charAt(1);
		c[2] = inputString.charAt(2);
		c[3] = inputString.charAt(3);
		c[4] = 0;
		// clear the string:
		inputString = "";
		stringComplete = false;
		//Serial.print(F(" Think "));                       /* Turn for ARDUINO */

		K = *c - 16 * c[1] + 799, L = c[2] - 16 * c[3] + 799; /* parse entered move */
		N = 0;
		T = 0x3F;                                 /* T=Computer Play strength */
		bkp();                                    /* Save the board just in case */
		r = Dx(-I, I, Q, O, 1, 3);                 /* Check & do the human movement */
		if ( !(r > -I + 1) ) {
			//Serial.println(F("Lose "));
			gameOver();
		}
		if (k == 0x10) {                          /* The flag turn must change to 0x08 */
			//Serial.println(F("No valid move"));
			return;
		}

		strcpy(lastH, c);                         /* Valid human movement */

		mn++;                                     /* Next move */

		K = I;
		N = 0;
		T = 0x3F;                                 /* T=Computer Play strength */
		r = Dx(-I, I, Q, O, 1, 3);                 /* Think & do*/
		if ( !(r > -I + 1) ) {
			//Serial.println(F("Lose*"));
			gameOver();
		}

		strcpy(lastM, c);                         /* Valid ARDUINO movement */
		r = Dx(-I, I, Q, O, 1, 3);
		if ( !(r > -I + 1) ) {
			//Serial.println(lastM);
			gameOver();
		}
		//Serial.println(lastM);
		string2move(lastM);
		cursor = 2;
		//Serial.println(String(freemem()));
		//serialBoard();
	}
}

void DisplayBoard() {
	int i = 0; int j = 0;
	for (j=0; j<225; j=j+32) {
		for (i=0; i<16; i=i+4) {
			SET_PIXEL(pixels,i+j,bitsPerPixel,0);
			SET_PIXEL(pixels,i+j,bitsPerPixel,0);
			SET_PIXEL(pixels,i+j+2,bitsPerPixel,1);
			SET_PIXEL(pixels,i+j+3,bitsPerPixel,1);
		}
		for (int i=16; i<32; i=i+4) {
			SET_PIXEL(pixels,i+j,bitsPerPixel,1);
			SET_PIXEL(pixels,i+j+1,bitsPerPixel,1);
			SET_PIXEL(pixels,i+j+2,bitsPerPixel,0);
			SET_PIXEL(pixels,i+j+3,bitsPerPixel,0);
		}
	}
}

void ClearBoard() {
	for (uint16_t i=0; i<numPixels; i++) {
		SET_PIXEL(pixels,i,bitsPerPixel,0);
	}
}
//
void ShowCursor (int x, int y) {
	if (cursor < 3) {
		uint8_t c;
		uint8_t index = GET_PIXEL(pixels,XY(x, y),bitsPerPixel);
		if (index == 0) {
			c = 1;
		}
		else {
			c = 0;
		}
		SET_PIXEL(pixels,XY(x, y),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x, y+1),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x+1, y),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x+1, y+1),bitsPerPixel,c);
	}
}

void ShowMove() {
	if (cursor == 1 || cursor == 2) {
		uint8_t c; uint8_t x; uint8_t y;
		x = moves[0]; y = moves[1];
		uint8_t index = GET_PIXEL(pixels,XY(x,y),bitsPerPixel);
		if (index == 0) {
			c = 1;
		}
		else {
			c = 0;
		}
		SET_PIXEL(pixels,XY(x,y),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x,y+1),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x+1,y),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x+1,y+1),bitsPerPixel,c);
	}
	if (cursor == 2) {
		uint8_t c; uint8_t x; uint8_t y;
		x = moves[2]; y = moves[3];
		uint8_t index = GET_PIXEL(pixels,XY(x,y),bitsPerPixel);
		if (index == 0) {
			c = 1;
		}
		else {
			c = 0;
		}
		SET_PIXEL(pixels,XY(x,y),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x,y+1),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x+1,y),bitsPerPixel,c);
		SET_PIXEL(pixels,XY(x+1,y+1),bitsPerPixel,c);
	}
}
//
uint16_t XY( uint8_t x, uint8_t y)
{
	uint16_t i;
	int b = (((x/8)*2) + (y/8))*(64);
	i = (b+((y%8)*8)+(x%8));
	return i;
}
//
//void getserialchar() {
//	while (//Serial.available() > 0) {
//		// get the new byte:
//		char inChar = (char)//Serial.read();
//		// add it to the inputString:
//		inputString += inChar;
//		// if the incoming character is a newline, set a flag so the main loop can
//		// do something about it:
//		if (inChar == '\n') {
//			stringComplete = true;
//		}
//	}
//}

void gameOver() {
	for (;;);
}
void bkp() {
	for (int i = 0; i < 16 * 8 + 1; i++) {
		bk[i] = b[i];
	}
}
//void serialBoard(){
//	//Serial.println(F("  +-----------------+"));
//	for(int i=0; i<8; i++){
//		//Serial.print(' ');
//		//Serial.print(8-i);
//		//Serial.print("| ");
//		for(int j=0; j<8; j++){
//			char c = sym[b[16*i+j]&15];
//			//Serial.print(c);
//			//Serial.print(' ');
//		}
//		//Serial.println('|');
//	}
//	//Serial.println(F("  +-----------------+"));
//	//Serial.println(F("    a b c d e f g h"));
//}

//char sym[17] = {".?pnkbrq?P?NKBRQ"};
void DisplayChessPieces() {
	for (uint8_t i=0; i<8; i++) {
		for(uint8_t j=0; j<8; j++){
			uint8_t X0 = (7-i)*2;
			uint8_t Y0 = j*2;
			char piece = sym[b[16*i+j]&15];
			switch (piece) {
			case 'p':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,2);
				break;
			case 'P':
				SET_PIXEL(pixels,XY(X0+1, Y0+1),bitsPerPixel,3);
				break;
			case 'r':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0, Y0+1),bitsPerPixel,2);
				break;
			case 'R':
				SET_PIXEL(pixels,XY(X0+1, Y0),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0+1, Y0+1),bitsPerPixel,3);
				break;
			case 'n':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0, Y0+1),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0+1, Y0+1),bitsPerPixel,2);
				break;
			case 'N':
				SET_PIXEL(pixels,XY(X0, Y0+1),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0+1, Y0),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0+1, Y0+1),bitsPerPixel,3);
				break;
			case 'b':
				SET_PIXEL(pixels,XY(X0, Y0+1),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0+1, Y0),bitsPerPixel,2);
				break;
			case 'B':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0+1, Y0+1),bitsPerPixel,3);
				break;
			case 'q':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0+1, Y0),bitsPerPixel,2);
				break;
			case 'Q':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0+1, Y0),bitsPerPixel,3);
				break;
			case 'k':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0+1, Y0),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0, Y0+1),bitsPerPixel,2);
				SET_PIXEL(pixels,XY(X0+1, Y0+1),bitsPerPixel,2);
				break;
			case 'K':
				SET_PIXEL(pixels,XY(X0, Y0),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0+1, Y0),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0, Y0+1),bitsPerPixel,3);
				SET_PIXEL(pixels,XY(X0+1, Y0+1),bitsPerPixel,3);
				break;
			}
		}
	}
}

unsigned short myrand(void) {
	unsigned short r = (unsigned short)(seed % MYRAND_MAX);
	return r = ((r << 11) + (r << 7) + r) >> 1;
}
/* recursive minimax search */
/* (q,l)=window, e=current eval. score, */
/* E=e.p. sqr.z=prev.dest, n=depth; return score */
short Dx(short q, short l, short e, unsigned char E, unsigned char z, unsigned char n) {
	short m, v, i, P, V, s;
	unsigned char t, p, u, x, y, X, Y, H, B, j, d, h, F, G, C;
	signed char r;
	if (++Z > 30) {                                   /* stack underrun check */
		--Z; return e;
	}
	q--;                                          /* adj. window: delay bonus */
	k ^= 24;                                      /* change sides             */
	d = Y = 0;                                    /* start iter. from scratch */
	X = myrand() & ~M;                            /* start at random field    */
	W(d++ < n || d < 3 ||                         /* iterative deepening loop */
			z & K == I && (N < T & d < 98 ||            /* root: deepen upto time   */
					(K = X, L = Y & ~M, d = 3)))                /* time's up: go do best    */
	{ x = B = X;                                   /* start scan at prev. best */
	h = Y & S;                                   /* request try noncastl. 1st*/
	P = d < 3 ? I : Dx(-l, 1 - l, -e, S, 0, d - 3); /* Search null move         */
	m = -P < l | R > 35 ? d > 2 ? -I : e : -P;   /* Prune or stand-pat       */
	++N;                                         /* node count (for timing)  */
	do {
		u = b[x];                                   /* scan board looking for   */
		if (u & k) {                                /*  own piece (inefficient!)*/
			r = p = u & 7;                             /* p = piece type (set r>0) */
			j = o[p + 16];                             /* first step vector f.piece*/
			W(r = p > 2 & r < 0 ? -r : -o[++j])        /* loop over directions o[] */
			{ A:                                        /* resume normal after best */
				y = x; F = G = S;                         /* (x,y)=move, (F,G)=castl.R*/
				do {                                      /* y traverses ray, or:     */
					H = y = h ? Y ^ h : y + r;               /* sneak in prev. best move */
					if (y & M)break;                         /* board edge hit           */
					m = E - S & b[E] && y - E < 2 & E - y < 2 ? I : m; /* bad castling             */
					if (p < 3 & y == E)H ^= 16;              /* shift capt.sqr. H if e.p.*/
					t = b[H]; if (t & k | p < 3 & !(y - x & 7) - !t)break; /* capt. own, bad pawn mode */
					i = 37 * w[t & 7] + (t & 192);           /* value of capt. piece t   */
					m = i < 0 ? I : m;                       /* K capture                */
					if (m >= l & d > 1)goto C;               /* abort on fail high       */
					v = d - 1 ? e : i - p;                   /* MVV/LVA scoring          */
					if (d - !t > 1)                          /* remaining depth          */
					{ v = p < 6 ? b[x + 8] - b[y + 8] : 0;    /* center positional pts.   */
					b[G] = b[H] = b[x] = 0; b[y] = u | 32;  /* do move, set non-virgin  */
					if (!(G & M))b[F] = k + 6, v += 50;     /* castling: put R & score  */
					v -= p - 4 | R > 29 ? 0 : 20;           /* penalize mid-game K move */
					if (p < 3)                              /* pawns:                   */
					{ v -= 9 * ((x - 2 & M || b[x - 2] - u) + /* structure, undefended    */
							(x + 2 & M || b[x + 2] - u) - 1  /*        squares plus bias */
							+ (b[x ^ 16] == k + 36))          /* kling to non-virgin King */
						- (R >> 2);                       /* end-game Pawn-push bonus */
					V = y + r + 1 & S ? 647 - p : 2 * (u & y + 16 & 32); /* promotion or 6/7th bonus */
					b[y] += V; i += V;                     /* change piece, add score  */
					}
					v += e + i; V = m > q ? m : q;          /* new eval and alpha       */
					C = d - 1 - (d > 5 & p > 2 & !t & !h);
					C = R > 29 | d < 3 | P - I ? C : d;     /* extend 1 ply if in check */
					do
						s = C > 2 | v > V ? -Dx(-l, -V, -v,     /* recursive eval. of reply */
								F, 0, C) : v;    /* or fail low if futile    */
					W(s > q&++C < d); v = s;
					if (z && K - I && v + I && x == K & y == L) /* move pending & in root:  */
					{ Q = -e - i; O = F;                     /*   exit if legal & found  */
					R += i >> 7; --Z; return l;            /* captured non-P material  */
					}
					b[G] = k + 6; b[F] = b[y] = 0; b[x] = u; b[H] = t; /* undo move,G can be dummy */
					}
					if (v > m)                               /* new best, update max,best*/
						m = v, X = x, Y = y | S & F;            /* mark double move with S  */
					if (h) {
						h = 0;  /* redo after doing old best*/
						goto A;
					}
					if (x + r - y | u & 32 |                 /* not 1st step,moved before*/
							p > 2 & (p - 4 | j - 7 ||             /* no P & no lateral K move,*/
									b[G = x + 3 ^ r >> 1 & 7] - k - 6     /* no virgin R in corner G, */
									|| b[G ^ 1] | b[G ^ 2])               /* no 2 empty sq. next to R */
					)t += p < 5;                           /* fake capt. for nonsliding*/
					else F = y;                              /* enable e.p.              */
				} W(!t);                                  /* if not capt. continue ray*/
			}
		}
	} W((x = x + 9 & ~M) - B);                 /* next sqr. of board, wrap */
	C: if (m > I - M | m < M - I)d = 98;           /* mate holds to any depth  */
	m = m + I | P == I ? m : 0;                  /* best loses K: (stale)mate*/
	if (z && d > 2)
	{ *c = 'a' + (X & 7); c[1] = '8' - (X >> 4); c[2] = 'a' + (Y & 7); c[3] = '8' - (Y >> 4 & 7); c[4] = 0;
	char buff[150];
	}
	}                                             /*    encoded in X S,8 bits */
	k ^= 24;                                      /* change sides back        */
	--Z; return m += m < e;                       /* delayed-loss bonus       */
}

//int freemem()
//{
//	int free_memory;
//	if((int)__brkval == 0)
//		free_memory = ((int)&free_memory) - ((int)&__bss_end);
//	else
//		free_memory = ((int)&free_memory) - ((int)__brkval);
//	return free_memory;
//}
