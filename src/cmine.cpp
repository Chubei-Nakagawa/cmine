#include <cstddef>
#include <cstdlib>
#include <clocale>
#include <ctime>
#include <ncurses.h>

#include <vector>

using namespace std;

class MineApp {
  enum Stat { CLOSE = 0, OPEN, MARK, FLAG };
protected:
  vector< bool > _bomb;
  vector< Stat > _panel; 
  int width;
  int height;
  int bombs;
  int curx;
  int cury;
  Stat wall;
  enum { NONE, WIN, LOSE } stat;
    
  static int dx[8];
  static int dy[8];
  static char dirkeys[8];
  char openkey;
  char markkey;
  char flagkey;
  int dispx;
  int dispy;
  int markrest;
  int openrest;
public:
  MineApp() : stat(NONE) , wall(OPEN), dispy(1), dispx(0) {
    srand(time(NULL));
    markkey = 'z';
    flagkey = 'a';
    openkey = ' ';
  }
  void init(int height, int width, int bombs = 0) {
    this->width = width;
    this->height = height;
    if (bombs == 0) bombs = height * width / 10;
    this->bombs = bombs;
    _bomb.resize((height + 2) * (width * 2));
    _panel.resize(height * width);
    _bomb.assign(_bomb.size(), false);
    _panel.assign(_panel.size(), CLOSE);
    cury = height / 2;
    curx = width / 2;
    setBombs();
    markrest = bombs;
    openrest = width * height - bombs;
    refreshAll();
    printRest();
  }
  void play() {
    while (stat == NONE) {
      int ch = getch();
      char inch = (char)tolower(ch);
      if (inch == openkey) {
	openRecursive(cury, curx);
      } else if (inch == markkey){
	mark(cury, curx);
      } else {
	for (int i = 0; i < 8; ++i ){
	  if (inch == dirkeys[i]){
	    if (inbound(cury + dy[i], curx + dx[i])){
	      int oldx = curx;
	      int oldy = cury;
	      curx += dx[i];
	      cury += dy[i];
	      rewrite(oldy, oldx);
	      rewrite(cury, curx);
	      refresh();
	    }
	    break;
	  }
	}
      }
      if (stat == LOSE){
	mvaddstr(0, 30, "GAMEOVER");
      } else if (stat == WIN){
	mvaddstr(0, 30, "CLEAR");
      }
    }
    refreshAll();
    getch();
  }
protected:
  int scH() { return height + 2; }
  int scW() { return width + 2; }
  void refreshAll(){
    for (int row = 0; row < scH(); ++row){
      for (int col = 0; col < scW(); ++col){
	rewrite(row, col);
      }
    }
    refresh();
  }
  bool inbound(int row, int col){
    return row > 0 && row < height + 1 && col > 0 && col < width + 1;
  }
  void openRecursive(int row, int col){
    if (panel(row,col) != CLOSE) return;
    openOne(row, col);
    if (stat == LOSE) return;
    if (getNumber(row, col) == 0){
      for (int i = 0; i < 8; ++i){
	if (inbound(row + dy[i] , col + dx[i])){
	  openRecursive(row + dy[i] , col + dx[i]);
	}
      }
    }
  }
  void openOne(int row, int col){
    if (panel(row,col) != CLOSE) return;
    panel(row,col) = OPEN;
    openrest--;
    if (bomb(row,col)){
      stat = LOSE;
      return;
    } else if (openrest == 0 ){
      stat = WIN;
    }
    rewrite(row, col);
  }
  void mark(int row, int col){
    if (panel(row, col) == CLOSE){
      markrest--;
      panel(row, col) = FLAG;
    } else if (panel(row, col) == FLAG) {
      markrest++;
      panel(row, col) = CLOSE;
    } else {
      return;
    }
    printRest();
  }
  void printRest() {
    mvprintw(0, 2, "rest: %03d", markrest);
  }
  void setBombs() {
    vector<int> rands(height * width);
    for (int i = 0; i < rands.size() ; ++i){
      rands[i] = i;
    }
    for (int i = 0; i < rands.size() ; ++i){
      swap(rands[i], rands[rand() % rands.size()]);
    }
    for (int i = 0; i < bombs; ++i) {
      _bomb[(rands[i]/width + 1) * (width + 2 ) + rands[i] % width + 1 ] = true;
    }
  }
  int getNumber(int row, int col){
    int ret = 0;
    for (int i = 0; i < 8; ++i){
      if (_bomb[(row + dy[i]) * scW() + col + dx[i]]){
	++ret;
      }
    }
    return ret;
  }
  char getDisplayChar(int row, int col){
    if (row == cury && col == curx){
      if (stat == LOSE && bomb(row, col)) return '*';
      return '@';
    } else if (col == 0 || col == scW() - 1) {
      return '|';
    } else if ( row == 0 || row == scH() - 1){
      return '-';
    }
    switch (panel(row, col)){
    case OPEN:
      {
	int n = getNumber(row, col);
	return n ? '0' + n : ' ';
      }
    case CLOSE:
      if (bomb(row, col)){
	switch (stat){
	case WIN:
	  return '$';
	case LOSE:
	  return '*';
	default:
	  break;
	}
      }
      return '.';
    case FLAG:
      if (stat == LOSE && !bomb(row, col)) return 'X';
      return '$';
    case MARK:
      if (bomb(row, col)){
	switch (stat){
	case WIN:
	  return '$';
	case LOSE:
	  return '*';
	default:
	  break;
	}
      }
      return '?';
    }
  }
  void rewrite(int row, int col) {
    mvaddch(row + dispy, col + dispx, getDisplayChar(row, col));
  }
  bool bomb(int row, int col){
    return _bomb[row * scW() + col];
  }
  Stat& panel(int row, int col){
    if (!inbound(row, col)) return wall;
    return _panel[(row - 1) * width + col - 1];
  }    
};

int MineApp::dx[8] = { -1,0,0,1, -1,1,-1,1};
int MineApp::dy[8] = {0,1,-1,0,-1,-1,1,1};
char MineApp::dirkeys[8] = {'h', 'j', 'k', 'l', 'y', 'u', 'b', 'n'};

int main(int, char**)
{
  initscr();
  start_color();
  cbreak();
  noecho();
  curs_set(0);
  setlocale(LC_ALL, "");
  erase(); 
  timeout(-1);
  MineApp app;
  int w, h;
  getmaxyx(stdscr, h, w);
  app.init(h-3,w-2);
  app.play();
  endwin();
  return 0;
}
