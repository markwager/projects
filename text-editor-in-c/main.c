#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE
//queste sono le feature test macros che permettono al programmatore di migliorare la portabilità del codice
//

#include <termios.h> //fornisce interazioni con terminali
#include <unistd.h> //fornisce interazione con SO
#include <stdlib.h>
#include <ctype.h> //fornisce modifica e classificaz. di char
#include <stdio.h>
#include <errno.h> //fornisce condizioni di errori
#include <sys/ioctl.h> //fornisce controllo I/O dei dispositivi
#include <string.h>
#include <sys/types.h> //fornisce collezioni di typedef e struct
#include <time.h> //fornisce funz di data e ora
#include <stdarg.h> //fornisce la creazione di funzioni che accettano un num var di arg
#include <fcntl.h> //fornisce il controllo file

typedef struct{
    int idx;
    int size;
    int rsize;
    char *chars;
    char *render;
    unsigned char *hl;
    int hl_open_comment;
}erow;//memorizza il contenuto del file

typedef struct{
    char *b;
    int len;
}abuf;//crea buffer per le stringhe da scrivere (non è una buona
//idea avere tanti write nel codice): costruisce cosa stampare sullo schermo
#define ABUF_INIT {NULL, 0} //rappresenta il nostro buffer inizializzato
#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

struct editorSyntax {
  char *filetype;
  char **filematch;
  char **keywords;
  char *singleline_comment_start;
  char *multiline_comment_start;
  char *multiline_comment_end;
  int flags;
};

char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL };
char *C_HL_keywords[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case",
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", NULL
};

struct editorSyntax HLDB[] = {
  {
    "c",
    C_HL_extensions,
    C_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

void enableRawMode();
void disableRawMode();
void die(char*);
int editorReadKey();
void editorProcessKeypress();
void editorRefreshScreen();
void editorDrawRows(abuf*);
int getWindowSize(int*, int*);
void initEditor();
void abAppend(abuf*, const char*, int);
void abFree(abuf*);
void editorMoveCursor(int);
void editorOpen(char*);
void editorInsertRow(int, char*, size_t);
void editorScroll();
void editorUpdateRow(erow*);
int editorRowCxToRx(erow*, int);
void editorDrawStatusBar(abuf*);
void editorSetStatusMessage(const char*, ...);
void editorDrawMessageBar(abuf *ab);
void editorRowInsertChar(erow*, int, int);
void editorInsertChar(int);
char *editorRowsToString(int*);
void editorSave();
void editorRowDelChar(erow*, int);
void editorDelChar();
void editorFreeRow(erow*);
void editorDelRow(int);
void editorRowAppendString(erow*, char*, size_t);
void editorInsertNewline();
char *editorPrompt(char*, void(*callback)(char*, int));
void editorFind();
int editorRowRxToCx(erow*, int);
void editorFindCallback(char*, int);
void editorUpdateSyntax(erow*);
int editorSyntaxToColor(int);
int is_separator(int);
void editorSelectSyntaxHighlight();
#define CTRL_KEY(k) ((k)&0x1f)
#define KILO_VERSION "0.0.1" //mess benvenuto
#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3

typedef struct{
    int cx, cy; //si riferiscono a dove sta il cursore nel file, non lo schermo
    //cx=colonna, cy=riga
    int rx; //introduciamo questo per far interagire meglio il cursore con i tab (index orizz.)
    int rowoff; //row offset (traccia la riga del file visualizzata)
    int coloff; //col offset (traccia la colonna del file visualizzata)
    int screenrows; //ottiene righe dello schermo
    int screencols; //ottiene colonne dello schermo
    int numrows; //conta quante righe di testo ci sono nel file
    erow *row; //serve per immagazzinare più linee
    int dirty; //tiene conto se file è stato modificato dopo apertura o il salvataggio del file
    char *filename; //per stampare nome file
    char statusmsg[80];
    time_t statusmsg_time; //secondi
    struct editorSyntax *syntax;
    struct termios orig_termios;
}editorConfig; //struttura principale che contiene tutto lo stato globale dell'editor

editorConfig E;

typedef enum{
    BACKSPACE=127, //sua rappr. ASCII (non ha una rappr. di escape seq.)
    ARROW_LEFT=1000, // \x1b[D
    ARROW_RIGHT, // \x1b[C
    ARROW_UP, // \x1b[A
    ARROW_DOWN, // \x1b[B
    DEL_KEY, // \x1b[3~
    HOME_KEY, // \x1b[1~ o \x1b[7~ o \x1b[H o \x1b[OH
    END_KEY, // \x1b[4~ o \x1b[8~ o \x1b[F o \x1b[OF
    PAGE_UP, // \x1b[5~
    PAGE_DOWN // \x1b[6~
    //questi sono vari escape sequence che servono pe rfare diversi compiti di formattazione
}editorKey; //enum è una struttura tipo struct che contiene un gruppo di costanti
//se il primo lo setto a 1000 gi altri so 1001 ecc...

enum editorHighlight{
    HL_NORMAL=0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

void editorSelectSyntaxHighlight() {
  E.syntax = NULL;
  if (E.filename == NULL) return;
  char *ext = strrchr(E.filename, '.');
  for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
    struct editorSyntax *s = &HLDB[j];
    unsigned int i = 0;
    while (s->filematch[i]) {
      int is_ext = (s->filematch[i][0] == '.');
      if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
          (!is_ext && strstr(E.filename, s->filematch[i]))) {
        E.syntax = s;
        int filerow;
        for (filerow = 0; filerow < E.numrows; filerow++) {
          editorUpdateSyntax(&E.row[filerow]);
        }
        return;
      }
      i++;
    }
  }
}

int is_separator(int c) {
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

int editorSyntaxToColor(int hl){
    switch (hl) {
    case HL_COMMENT:
    case HL_MLCOMMENT: return 36;
    case HL_KEYWORD1: return 33;
    case HL_KEYWORD2: return 32;
    case HL_STRING: return 35;
    case HL_NUMBER: return 31;
    case HL_MATCH: return 34;
    default: return 37;
  }
}

void editorUpdateSyntax(erow *row) {
  row->hl = realloc(row->hl, row->rsize);
  memset(row->hl, HL_NORMAL, row->rsize);
  if (E.syntax == NULL) return;
  char **keywords = E.syntax->keywords;
  char *scs = E.syntax->singleline_comment_start;
  char *mcs = E.syntax->multiline_comment_start;
  char *mce = E.syntax->multiline_comment_end;
  int scs_len = scs ? strlen(scs) : 0;
  int mcs_len = mcs ? strlen(mcs) : 0;
  int mce_len = mce ? strlen(mce) : 0;
  int prev_sep = 1;
  int in_string = 0;
  int in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment);
  int i = 0;
  while (i < row->rsize) {
    char c = row->render[i];
    unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;
    if (scs_len && !in_string && !in_comment) {
      if (!strncmp(&row->render[i], scs, scs_len)) {
        memset(&row->hl[i], HL_COMMENT, row->rsize - i);
        break;
      }
    }
    if (mcs_len && mce_len && !in_string) {
      if (in_comment) {
        row->hl[i] = HL_MLCOMMENT;
        if (!strncmp(&row->render[i], mce, mce_len)) {
          memset(&row->hl[i], HL_MLCOMMENT, mce_len);
          i += mce_len;
          in_comment = 0;
          prev_sep = 1;
          continue;
        } else {
          i++;
          continue;
        }
      } else if (!strncmp(&row->render[i], mcs, mcs_len)) {
        memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
        i += mcs_len;
        in_comment = 1;
        continue;
      }
    }
    if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
      if (in_string) {
        row->hl[i] = HL_STRING;
        if (c == '\\' && i + 1 < row->rsize) {
          row->hl[i + 1] = HL_STRING;
          i += 2;
          continue;
        }
        if (c == in_string) in_string = 0;
        i++;
        prev_sep = 1;
        continue;
      } else {
        if (c == '"' || c == '\'') {
          in_string = c;
          row->hl[i] = HL_STRING;
          i++;
          continue;
        }
      }
    }
    if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
      if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
          (c == '.' && prev_hl == HL_NUMBER)) {
        row->hl[i] = HL_NUMBER;
        i++;
        prev_sep = 0;
        continue;
      }
    }
    if (prev_sep) {
      int j;
      for (j = 0; keywords[j]; j++) {
        int klen = strlen(keywords[j]);
        int kw2 = keywords[j][klen - 1] == '|';
        if (kw2) klen--;
        if (!strncmp(&row->render[i], keywords[j], klen) &&
            is_separator(row->render[i + klen])) {
          memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
          i += klen;
          break;
        }
      }
      if (keywords[j] != NULL) {
        prev_sep = 0;
        continue;
      }
    }
    prev_sep = is_separator(c);
    i++;
  }
  int changed = (row->hl_open_comment != in_comment);
  row->hl_open_comment = in_comment;
  if (changed && row->idx + 1 < E.numrows)
    editorUpdateSyntax(&E.row[row->idx + 1]);
}

int editorRowRxToCx(erow *row, int rx) {
  int cur_rx = 0;
  int cx;
  for (cx = 0; cx < row->size; cx++) {
    if (row->chars[cx] == '\t')
      cur_rx += (KILO_TAB_STOP - 1) - (cur_rx % KILO_TAB_STOP);
    cur_rx++;
    if (cur_rx > rx) return cx;
  }
  return cx;
}

void editorFindCallback(char *query, int key) {
  static int last_match = -1;
  static int direction = 1;

  static int saved_hl_line;
  static char *saved_hl = NULL;
  if (saved_hl){
    memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
    free(saved_hl);
    saved_hl = NULL;
  }

  if (key == '\r' || key == '\x1b') {
    last_match = -1;
    direction = 1;
    return;
  } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
    direction = 1;
  } else if (key == ARROW_LEFT || key == ARROW_UP) {
    direction = -1;
  } else {
    last_match = -1;
    direction = 1;
  }
  if (last_match == -1) direction = 1;
  int current = last_match;
  int i;
  for (i = 0; i < E.numrows; i++) {
    current += direction;
    if (current == -1) current = E.numrows - 1;
    else if (current == E.numrows) current = 0;
    erow *row = &E.row[current];
    char *match = strstr(row->render, query);
    if (match) {
      last_match = current;
      E.cy = current;
      E.cx = editorRowRxToCx(row, match - row->render);
      E.rowoff = E.numrows;

      saved_hl_line = current;
      saved_hl = malloc(row->rsize);
      memcpy(saved_hl, row->hl, row->rsize);
      memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
      break;
    }
  }
}

void editorFind() {
  int saved_cx = E.cx;
  int saved_cy = E.cy;
  int saved_coloff = E.coloff;
  int saved_rowoff = E.rowoff;

  char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)",
                             editorFindCallback);  if (query) {
    free(query);
  }
  else {
    E.cx = saved_cx;
    E.cy = saved_cy;
    E.coloff = saved_coloff;
    E.rowoff = saved_rowoff;
  }
}

char *editorPrompt(char *prompt, void(*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  size_t buflen = 0;
  buf[0] = '\0';
  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();
    int c = editorReadKey();
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
      if (buflen != 0) buf[--buflen] = '\0';
    } else if (c == '\x1b') {
      editorSetStatusMessage("");
      if (callback) callback(buf, c);
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buflen != 0) {
        editorSetStatusMessage("");
        if (callback) callback(buf, c);
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
    if (callback) callback(buf, c);
  }
}

void editorInsertNewline() {
  if (E.cx == 0) {
    editorInsertRow(E.cy, "", 0);
  } else {
    erow *row = &E.row[E.cy];
    editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
    row = &E.row[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
  }
  E.cy++;
  E.cx = 0;
}


void editorDelChar() {
  if (E.cy == E.numrows) return;
  if (E.cx == 0 && E.cy == 0) return;
  erow *row = &E.row[E.cy];
  if (E.cx > 0) {
    editorRowDelChar(row, E.cx - 1);
    E.cx--;
  } else {
    E.cx = E.row[E.cy - 1].size;
    editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
    editorDelRow(E.cy);
    E.cy--;
  }
}

void editorRowAppendString(erow *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  editorUpdateRow(row);
  E.dirty++;
}

void editorDelRow(int at){
if (at < 0 || at >= E.numrows){
    return;
}
  editorFreeRow(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
  for (int j = at; j < E.numrows - 1; j++) E.row[j].idx--;
  E.numrows--;
  E.dirty++;
}

void editorFreeRow(erow *row){
    free(row->render);
    free(row->chars);
    free(row->hl);
}

void editorRowDelChar(erow *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row);
  E.dirty++;
}

void editorSave(){
    if (E.filename == NULL) {
    E.filename = editorPrompt("Save as: %s (ESC to cancel)", NULL);    if (E.filename == NULL) {
      editorSetStatusMessage("Save aborted");
      return;
    }
    editorSelectSyntaxHighlight();
    }
    int len;
    char *buf=editorRowsToString(&len);
    int fd=open(E.filename, O_RDWR|O_CREAT, 0644);
    if(fd!=1){
        if(ftruncate(fd, len)!=-1){
            if(write(fd, buf, len)==len){
                close(fd);
                free(buf);
                E.dirty=0;
                editorSetStatusMessage("%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}//scrive la stringa convertita da editorRowsToString nel disco

char *editorRowsToString(int *buflen){
    int totlen=0;
    int j;
    for(j=0; j<E.numrows; j++){
        totlen+=E.row[j].size+1;
    }//calcola la lunghezza totale della stringa in ouput, sommando la size di ogni riga
    //(+1 per via di nuovalinea)
    *buflen=totlen;
    char *buf=malloc(totlen);
    char *p=buf;
    for(j=0; j<E.numrows; j++){
        memcpy(p, E.row[j].chars, E.row[j].size);
        p+=E.row[j].size;
        *p='\n';
        p++;
    }

    return buf;
}//converte la struct erow in una stringa pronta per essere scritta nel file


void editorInsertChar(int c){
    if(E.cy==E.numrows){
        editorInsertRow(E.numrows,"",0);
    }//se E.cy==E.numrows, allora il cursore è sulla linea tilde dopo la fine del file
    //quindi facciamo un append di una nuova riga al file prima di inserire un char
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
    //incrementiamo il cursore così l'utente può inserire un char nella nuova pos
}//prende un carattere e usa editorRowInsertChar() per inserire quel carattere
//nella posizione del cursore dove sta at

void editorRowInsertChar(erow *row, int at, int c){
    if(at<0 || at>row->size){
        at=row->size;
    }
    //si usa at come indice per il carattere da inserire
    row->chars=realloc(row->chars, row->size+2);
    //poi si fa un realloc di +2 (più il char vuoto)
    memmove(&row->chars[at+1], &row->chars[at], row->size-at+1);
    //è come memcpy ma è sicuro quando ricevitore e trasmettitore si sovrappongono
    row->size++;
    row->chars[at]=c;
    editorUpdateRow(row);
    E.dirty++;
}//funzione che inserisce un singolo carattere in erow

void editorDrawMessageBar(abuf *ab){
    abAppend(ab, "\x1b[K", 3);
    int msglen=strlen(E.statusmsg);
    if(msglen>E.screencols){
        msglen=E.screencols;
    }

    if(msglen && time(NULL)-E.statusmsg_time<5){
        abAppend(ab, E.statusmsg, msglen);
    }
}

void editorSetStatusMessage(const char *fmt, ...){
    va_list ap; //puntatore che tiene traccia degli argomenti passati a una funzione
    va_start(ap, fmt); //inizializza va_list con ap e last
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap); //pulisce e termina uso di va_list
    E.statusmsg_time = time(NULL);
}

void editorDrawStatusBar(abuf *ab) {
  abAppend(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
    E.filename ? E.filename : "[No Name]", E.numrows,
    E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
    E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);
  if (len > E.screencols) len = E.screencols;
  abAppend(ab, status, len);
  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    } else {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}


int editorRowCxToRx(erow *row, int cx){
    int rx=0;
    int j;
    for(j=0;j<cx;j++){
        if(row->chars[j]=='\t'){
            rx+=(KILO_TAB_STOP-1)-(rx%KILO_TAB_STOP);
        }
        rx++;
    }
    return rx;
}//funzione che calcola E.rx

void editorUpdateRow(erow *row){
    int tabs=0;
    int j;
    
    for(j=0; j<row->size; j++){
        if(row->chars[j]=='\t'){
            tabs++;
        }
    }//conta il num di tab

    free(row->render);
    row->render=malloc(row->size+tabs*(KILO_TAB_STOP-1)+1);//max num di char per tab è 8

    int idx=0;
    for(j=0; j<row->size; j++){
        if(row->chars[j]=='\t'){
            row->render[idx++]=' ';
            while(idx%KILO_TAB_STOP!=0){
                row->render[idx++]=' ';
            }
        }
        else{
            row->render[idx++]=row->chars[j];
        }
    }
    row->render[idx]='\0';
    row->rsize=idx;

    editorUpdateSyntax(row);
}//questa funzione usa chars di erow per riempire render string degli stessi caratteri
//in particolare fa un check dei tabs nel file

void editorScroll(){
    E.rx=0;
    if(E.cy<E.numrows){
        E.rx=editorRowCxToRx(&E.row[E.cy],E.cx);
    }//orizzontale
    if(E.cy<E.rowoff){
        E.rowoff=E.cy;
    }//verticale
    if(E.cy>=E.rowoff+E.screenrows){
        E.rowoff=E.cy-E.screenrows+1;
    }
    if(E.rx<E.coloff){
        E.coloff=E.rx;
    }//orizzontale
    if(E.rx>=E.coloff+E.screencols){
        E.coloff=E.rx-E.screencols+1;
    }//verticale

}//questa funzione controlla se il cursore è andato oltre la finestra visibile
//se così fosse aggiustiamo E.rowoff
//se cursore è oltre scrolla su dove sta il cursore

void editorInsertRow(int at, char *s, size_t len) {
  if (at < 0 || at > E.numrows) return;
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
  memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
  for (int j = at + 1; j <= E.numrows; j++) E.row[j].idx++;
  E.row[at].idx = at;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';
  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  E.row[at].hl = NULL;
  E.row[at].hl_open_comment = 0;
  editorUpdateRow(&E.row[at]);
  E.numrows++;
  E.dirty++;
}

void editorOpen(char *filename){
    free(E.filename);
    E.filename=strdup(filename); //fa una copia della stringa allocando la memoria richiesta
    //e assumendo che farai free() di quella memoria

    editorSelectSyntaxHighlight();

    FILE *fp=fopen(filename, "r");
    if(!fp){
        die("fopen");
    }

    char *line=NULL;
    size_t linecap=0;
    ssize_t linelen; //è solo un tipo di dato signed (a differenza di size_t)
    //si eguaglia a getline (legge una riga in file): buffer, dim e file. legge fino a \n o EOF e restituisce
    //quanti caratteri ha letto
    while((linelen=getline(&line, &linecap, fp))!=-1){
        while(linelen>0 && (line[linelen-1]=='\n' || line[linelen-1]=='\r')){
            linelen--;
        }//rimuove i char \r e \r
        editorInsertRow(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty=0;
}//sta funzione legge un file 

void editorMoveCursor(int key){
  erow *row=(E.cy>=E.numrows)? NULL : &E.row[E.cy];
  switch(key) {
    case ARROW_LEFT:
      if(E.cx != 0){
        E.cx--;
      }//muove a sx cursore
      else if(E.cy>0){
        E.cy--;
        E.cx=E.row[E.cy].size;
      }//muove a sx il cursore per andare alla fine della linea corrente
      break;
    case ARROW_RIGHT:
      if(row && E.cx<row->size){
      E.cx++;
    }
    else if(row && E.cx==row->size){
        E.cy++;
        E.cx=0;
    }
      break;
    case ARROW_UP:
      if(E.cy != 0){
        E.cy--;
      }
      break;
    case ARROW_DOWN:
      if(E.cy<E.numrows){
        E.cy++;
      }
      break;
  }

  row=(E.cy>=E.numrows) ? NULL : &E.row[E.cy];
  int rowlen=row ? row->size : 0;
  if(E.cx>rowlen){
    E.cx=rowlen;
  }
  //quest'ultimo blocco serve a nn far andare oltre oltre la fine della
  //riga il cursore quando sei su una riga e quella sotto è + corta
}//diamo dei confini al movimento dei tasti

void abFree(abuf *ab){
    free(ab->b);
}

void abAppend(abuf *ab, const char *s, int len){
    char *new=realloc(ab->b, ab->len+len); //+len0 per far sì che si abbia più spazio per la new string

    if(new==NULL){
        return;
    }

    memcpy(&new[ab->len],s,len); //copia la stringa s alla fine del buffer
    ab->b=new;
    ab->len+=len;
}

void initEditor(){
    E.cx=0;
    E.cy=0;
    E.rx=0;
    E.rowoff=0;
    E.coloff=0;
    E.numrows=0;
    E.row=NULL;
    E.dirty=0;
    E.filename=NULL;
    E.statusmsg[0]='\0';
    E.statusmsg_time=0;
    E.syntax = NULL;
    if(getWindowSize(&E.screenrows, &E.screencols)==-1){
        die("getWindowSize");
    }
    E.screenrows-=2;
}

int getWindowSize(int *rows, int *cols){
    struct winsize ws;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)==-1 || ws.ws_col==0){
        return -1;
    }
    else{
        *cols=ws.ws_col;
        *rows=ws.ws_row;
        return 0;
    }
}//get terminal size con ioctl(). se ha successo, mette il numero di colonne e di righe
//se ha insuccesso allora -1

void editorDrawRows(abuf *ab) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows) {
      if (E.numrows == 0 && y == E.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
          "Kilo editor -- version %s", KILO_VERSION);
        if (welcomelen > E.screencols) welcomelen = E.screencols;
        int padding = (E.screencols - welcomelen) / 2;
        if (padding) {
          abAppend(ab, "~", 1);
          padding--;
        }
        while (padding--) abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
      } else {
        abAppend(ab, "~", 1);
      }
    } else {
      int len = E.row[filerow].rsize - E.coloff;
      if (len < 0) len = 0;
      if (len > E.screencols) len = E.screencols;
      char *c = &E.row[filerow].render[E.coloff];
      unsigned char *hl = &E.row[filerow].hl[E.coloff];
      int current_color = -1;
      int j;
      for (j = 0; j < len; j++) {
        if (iscntrl(c[j])) {
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          abAppend(ab, "\x1b[7m", 4);
          abAppend(ab, &sym, 1);
          abAppend(ab, "\x1b[m", 3);
          if (current_color != -1) {
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
            abAppend(ab, buf, clen);
          }
        } else if (hl[j] == HL_NORMAL) {
          if (current_color != -1) {
            abAppend(ab, "\x1b[39m", 5);
            current_color = -1;
          }
          abAppend(ab, &c[j], 1);
        } else {
          int color = editorSyntaxToColor(hl[j]);
          if (color != current_color) {
            current_color = color;
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
            abAppend(ab, buf, clen);
          }
          abAppend(ab, &c[j], 1);
        }
      }
      abAppend(ab, "\x1b[39m", 5);
    }
    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }
}//scrive la tilde fino al numero di righe ottenute + risoluzione bug

void editorRefreshScreen(){

    editorScroll();
    abuf ab=ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6); //fa sparire il cursore in mezzo al terminale
    //abAppend(&ab, "\x1b[2J", 4);
    abAppend(&ab, "\x1b[H", 3); //riposiziona il cursore in alto a sx (la H serve per il cursor position)
  /*
  la funzione write ammette il file descr., il buff e il num. byte da scrivere
  ritorna il num di byte scritti e in caso di errore -1
  STDOUT_FILENO è l'output, e il primo byte che scrive è \x1b (escape sequence)
  e gli altri 3 sono [2J
  quello che si fa è scrivere una esc-seq sul terminale: comincia sempre con \x1b seguito da [
  noi con J puliamo lo schermo e con 2 lo puliamo tutto
  */
    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx-E.coloff)+1);
    abAppend(&ab, buf, strlen(buf));
    abAppend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

int editorReadKey(){
    char c;
    int nread;
    while((nread= read(STDIN_FILENO, &c, 1)) != 1){
        if(nread == -1 && errno != EAGAIN){
            die("read");
    } 
  }

    if(c=='\x1b'){
        char seq[3];

        if(read(STDIN_FILENO, &seq[0], 1)!=1){
            return '\x1b';
        }

        if(read(STDIN_FILENO, &seq[1], 1)!=1){
            return '\x1b';
        }

        if(seq[0]=='['){
            if(seq[1]>='0' && seq[1]<='9'){
                if (read(STDIN_FILENO, &seq[2], 1) != 1){
                    return '\x1b';
                }

                if(seq[2]=='~'){
                    switch(seq[1]){
                    case '1': return HOME_KEY;
                    case '3': return DEL_KEY;
                    case '4': return END_KEY;
                    case '5': return PAGE_UP;
                    case '6': return PAGE_DOWN;
                    case '7': return HOME_KEY;
                    case '8': return END_KEY;
                }
            }
        }

        else{
            switch(seq[1]){
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }
        }
        else if(seq[0]=='O'){
        switch (seq[1]) {
            case 'H': return HOME_KEY;
            case 'F': return END_KEY;
      }
        }
        return '\x1b';
    }
    else{
        return c;
    }
}//legge i caratteri

void editorProcessKeypress(){
    static int quit_times=KILO_QUIT_TIMES;
    int c=editorReadKey();
    switch(c){
        case '\r': //enter
            editorInsertNewline();
            break;
        case CTRL_KEY('q'):
        if (E.dirty && quit_times > 0){
            editorSetStatusMessage("WARNING!!! File has unsaved changes. " "Press Ctrl-Q %d more times to quit.", quit_times);
            quit_times--;
            return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
        case CTRL_KEY('s'):
            editorSave();
            break;
        case HOME_KEY:
            E.cx=0;
            break;
        case END_KEY:
            if(E.cy<E.numrows){
                E.cx=E.row[E.cy].size;
            }
            break;
        case CTRL_KEY('f'):
            editorFind();
            break;
        case BACKSPACE:
        case CTRL_KEY('h'): //manda il control code 8 (ciò che mandava originariamente backspace)
        case DEL_KEY:       //ma ora backspace è mappato con 127
            if(c==DEL_KEY){
                editorMoveCursor(ARROW_RIGHT);
            }
            editorDelChar();
            break;
        case PAGE_UP:
        case PAGE_DOWN:
        {
            if(c==PAGE_UP){
                E.cy=E.rowoff;
            }
            else if(c==PAGE_DOWN){
                E.cy=E.rowoff+E.screenrows-1;
                if(E.cy>E.numrows){
                    E.cy=E.numrows;
                }
            }
            int times=E.screenrows;
            while(times--){
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }    //vuol dire-> condizione ? val se vero : val se falso

        }
        break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c); 
            break;
        
        case CTRL_KEY('l'): //tradizionalmente usato per refreshare lo schermo
        case '\x1b':
            break;
        
        default:
            editorInsertChar(c);
            break;
    //di default fai questo se non ci sono i case di prima:
    //inserisce qualsiasi char non mappato nel file
    quit_times=KILO_QUIT_TIMES;
  }
  
}//mappa le combinazioni ctrl e processa i tasti premuti come frecce, home ecc...

void die(char *s){

    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s); //printa errore
    exit(1); //stoppa il programma con un exit status '1'
}

void disableRawMode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios)==-1){
        die("tcsetattr");
    }
}//funzione che riporta la termios struct nel suo stato originale e lo applica
//quando si esce da programma

void enableRawMode(){
    //infatti racchiude molti membri di default nell'header termios.h
    if(tcgetattr(STDIN_FILENO, &E.orig_termios)==-1){
        die("tcgetattr");
    } //recupera tutte le impostazioni correnti del temrinale

    struct termios raw=E.orig_termios;//dichiarazione di variabile (vale solo quando hai termios.h)

    atexit(disableRawMode); //in caso di chiusura del programma salva le impostazioni     
    raw.c_iflag &= ~(BRKINT|INPCK|ISTRIP|ICRNL|IXON); //disabilita ctrl-s e ctrl-q perchè servirebbero altrimenti come flow control
    //e questa volta la I è un input flag a differenza degli altri. Poi fixa l'ascii di ctrl-m

    raw.c_oflag &= ~(OPOST); //disabilita tutti i processi di output
    raw.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG); //modifica il membro c_cflag (il membro è per local flags)
    raw.c_cflag |= (CS8);
    raw.c_cc[VMIN]=0; //setta il minimo numero di byte di input necessari prima che ritorni read
    raw.c_cc[VTIME]=1; //setta il max num di tempo per aspettare prima che read ritorni
    //metto timeout con c_cc che stanno per control char

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1){
        die("tcsetattr");
    } //e passa la modifica a tcsetattr
    //tcsetattr e tcgetattr ritornano 0 se hanno successo
    //TCSAFLUSH specifica quando applicare la modifica: aspetta che l'output sia finito (ma cancella input in coda)
}//questa funzione permette di abilitare la raw mode: disabilita echo per non far vedere ciò che scrivo e poi se premo q esce subito
//e disabilita pure l'uso "improprio" di ctrl-c, ctrl-z e ctrl-v

int main(int argc, char *argv[]){

    enableRawMode();
    initEditor();
    if(argc>=2){ //se numero di argomenti>2
        editorOpen(argv[1]);
    }
    editorSetStatusMessage("HELP: Ctrl-S=save | Ctrl-Q = quit | Ctrl-F = find");
    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
   /*read è una funzione che legge
   argomenti: 
   file descriptor (intero che identifica un file aperto del processo
   dentro il file descriptor table), buffer, lunghezza buffer
   noi abbiamo il primo parametro che è di tipo input
    read ritorna il num di byte letti se ha successo
    e ritornerà 0 quando raggiunge la fine del file
    */
}
