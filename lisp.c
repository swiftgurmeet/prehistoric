



/*--------------------------------------------------------------------------*/

/*
 * The program consists of repeated invocation of three functions. These
 * respectively read the input list ( readlist ) , evaluate the list ( eval )
 * and print the result ( writelist ). The basic data structure used is
 * "Cell" which is a structure with fields for car and cdr pointers and a
 * pointer to char for the literal string in case it is used as part of lists
 * , and has 'next' pointer and binding pointer in case it is used as node on
 * the hash table chain. In addition a field 'node_type' is used to specify
 * whether the node is used for hash table or list  or atom. The 'memory' is
 * simply an array of these 'cells'. The 'cells' are chained in an available
 * list out of which 'cells' are allocated as needed , both for hash table
 * and for list nodes.
 * 
 * The various functions implemented in this lisp interpreter are :
 * 
 * 1) car 2) cdr 3) cons 4) append 5) setq
 * 
 * The eval checks the input list to see what action to take on it and
 * accordingly calls the other functions like car,cdr,cons,setf etc. These
 * functions may again call eval recursively on sublists before taking action
 * on them. All these functions return 0 on error.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define     TRUE 1
#define     FALSE 0
#define     HASHTABSIZE  101	/* Maximum hash table size */
#define     MAXMEM       300	/* Maximum no. of cells possible   */

typedef enum {
	RPARENTOK, 
        LPARENTOK, 
        SYMTOK, 
        QTOK
}               Tokens;
/*
 * |          |           |           | < ')' >     < '(' >   < literal >  <
 * quote >
 */

typedef int     cellcursor;	/* 'Pointer' type */
typedef enum {
	SOURCE, LISTTYPE, ATOM
}               Nodetypes;
typedef struct {
	Nodetypes       node_type;
	cellcursor      binding;/* The defn of atom   */
	cellcursor      next;	/* Next pointer  on hash chain */
	cellcursor      car, cdr;	/* car , cdr pointers   */
	char           *data;	/* The data string of atom */
}               Cell;


char           *makestring(void);	/* Allocate space for strings */

char           *str_global;	/* To store current input token */

char            pchr;		/* Used as input pushback buffer */
Tokens          token;		/* To indicate i/p tokentype    */

Cell            M[MAXMEM];	/* The 'memory' of array of cells */
cellcursor      avail_head;	/* Head of available list of free cells  */
cellcursor      hash_table[HASHTABSIZE];
int             availcell_count;/* No. of available cells   */
cellcursor      in_cur;		/* Input list pointer */
cellcursor      truecell;
/*----------------------------------------------------------------------------*/

char           *
makestring(void)
{

	char           *strg;

	strg = (char *) malloc(20);	/* Literals can be at most of 20 char */
	assert(strg != 0);	/* Check for malloc failing */
	strcpy(strg, "");	/* Make it 0 */
	return strg;
}

void 
init(Cell M[])
/* Initialises the memory  and the hash table */
{
	cellcursor      i;
	int             j;

	str_global = makestring();	/* Used for input by gettok proc */
	pchr = ' ';

	for (i = 0; i <= MAXMEM - 1; i++) {
		M[i].node_type = LISTTYPE;
		M[i].car = 0;
		M[i].cdr = i + 1;
		M[i].node_type = LISTTYPE;
		M[i].data = makestring();
	}			/* chain the cells into available list */
	M[MAXMEM].node_type = LISTTYPE;
	M[MAXMEM].car = 0;
	M[MAXMEM].cdr = 0;
	M[MAXMEM].data = makestring();	/* allocate space for all strings */
	avail_head = 2;		/* initialise head of avail list */

	for (j = 0; j <= HASHTABSIZE; j++) {
		hash_table[j] = 0;
        }

	availcell_count = MAXMEM - 1;	/* Number of cells available */

	return;
}


cellcursor 
hash(char *symbol)
/* Hash function */
{
	int             i, len;
	int             result;

	len = strlen(symbol);
	result = 0;
	for (i = 0; i < len; i++)
		result += (i * symbol[i]) % HASHTABSIZE;
	return result;
}


cellcursor 
in_hashtable(char *symbol)
/* 0 if symbol not in hash_table else cursor to the cell in hash_table. */
{
	cellcursor      tempcursor;

	tempcursor = hash_table[hash(symbol)];
	while ((tempcursor != 0) && ((strcmp(symbol, M[tempcursor].data) != 0)))
		tempcursor = M[tempcursor].next;
	return tempcursor;
}




cellcursor 
new1(void)
/* Storage allocator. */
{
	cellcursor      temp;
	if (avail_head == 0) {
		printf("Error:new1-No more memory.quitting...\n");
		exit(1);
	}
	availcell_count--;	/* Decrement the no. of available cells */
	temp = avail_head;
	avail_head = M[avail_head].cdr;
	return temp;
}

char 
gtchar()
/*
 * Return a character from input. Skip blanks and newlines. Uses pchr as one
 * character buffer for the last character
 */
{
	char            ch;

	if (pchr != ' ') {
		ch = pchr;
		pchr = ' ';
		return ch;	/* Use the pushbacked character */
	} else {
		while (((ch = getchar()) == '\n') || (ch == ' '));
	}

	return ch;
}


Tokens 
gettok(void)
/*
 * Returns the token type and stores the current token in case it is a string
 * literal in the string str_global
 */
{
	char            ch;
	int             i;

	ch = gtchar();
	switch (ch) {
	case '(':{
			pchr = ' ';
			return LPARENTOK;
		}
	case ')':{
			pchr = ' ';
			return RPARENTOK;
		}
	case '\'':{
			pchr = ' ';
			return QTOK;
		}
	default:{
			str_global[0] = ch;
			i = 0;
			while ((!feof(stdin)) && isalpha(ch)) {
				i++;
				ch = getchar();
				str_global[i] = ch;	/* Add ch to symbol
							 * string */
			}

			if ((ch == ' ') || (ch == '(') || (ch == ')')) {
				str_global[i] = '\0';
				pchr = ch;
			} else
				str_global[i] = '\0';	/* Terminate the string */
			return SYMTOK;
		}
	}
}


cellcursor 
readlist(Tokens token)
/* Reads the input list recursively */
{
	cellcursor      list, save, p;
	int             over;


	if (token == LPARENTOK) {	/* The list starts here */
		token = gettok();
		if (token == RPARENTOK)	/* If list ends immediately return
					 * nil */
			return 0;
		else {
			list = new1();
			save = list;
			M[list].node_type = LISTTYPE;
			M[list].car = readlist(token);	/* Read the sublist
							 * recursively */
			over = FALSE;
			while (!over) {
				token = gettok();
				if (token != RPARENTOK) {
					p = new1();
					M[list].cdr = p;
					list = p;
					M[list].node_type = LISTTYPE;
					M[list].car = readlist(token);
				} else
					over = TRUE;
			}
			M[list].cdr = 0;	/* The list ends */
			return save;
		}
	} else if (token == SYMTOK) {	/* Literal atom string encountered */
		if ((strcmp(str_global, "nil")) == 0)
			return 0;	/* literal nil */
		list = new1();
		M[list].node_type = ATOM;
		M[list].data = strdup(str_global);	/* copy the current
							 * token */
		M[list].cdr = 0;
		return list;
	} else if (token == QTOK) {	/* Take care of quote as special case */
		save = new1();
		M[save].node_type = LISTTYPE;
		list = new1();
		M[list].node_type = ATOM;
		M[list].data = strdup("quote");	/* Convert in the reqd funcn.
						 * form */
		M[list].cdr = 0;
		M[save].car = list;
		list = new1();
		M[list].node_type = LISTTYPE;
		token = gettok();
		M[list].car = readlist(token);
		M[list].cdr = 0;
		M[save].cdr = list;
		return save;
	} else {
		printf("Error : Unmatched \")\" \n");
		return 0;
	}
}


void 
writelist(cellcursor p)
/* Prints the list p recursively */
{
	int             over;

	if (p == 0)
		printf("nil");
	else if (M[p].node_type == LISTTYPE) {
		printf("(");
		while (p != 0) {
			writelist(M[p].car);
			p = M[p].cdr;
			if ((p != 0) && (M[p].node_type == LISTTYPE))
				printf(" ");
			else if (M[p].node_type != LISTTYPE) {
				printf(".");	/* Dotted pair */
				writelist(p);
				break;
			}
		}
		printf(")");
	} else
		printf("%s" , M[p].data);	/* Symbol name */
}
cellcursor 
addtohashtable(char *symbol, cellcursor list)
/* Add the symbol to the hashtable and return pointer to it */
{
	int             hashindex;
	cellcursor      tempcur;
	tempcur = in_hashtable(symbol);
	if (tempcur != 0) {	/* Found! Add there itself */
		M[tempcur].node_type = SOURCE;
		M[tempcur].binding = list;
		return tempcur;
	} else {		/* Not found. Add in the beginning of the
				 * chain */
		hashindex = hash(symbol);
		tempcur = hash_table[hashindex];
		hash_table[hashindex] = new1();
		M[hash_table[hashindex]].node_type = SOURCE;
		M[hash_table[hashindex]].data = symbol;
		M[hash_table[hashindex]].next = tempcur;
		M[hash_table[hashindex]].binding = list;	/* points to defn */
		return hash_table[hashindex];
	}
}
void 
prhash(void)
/* Print the hash table entries for debugging */
{
	int             index;
	int             i;

	for (i = 0; i < HASHTABSIZE; i++) {
		for ((index = hash_table[i]); index != 0; index = M[index].next) {
			if (index != 0)
				printf("-- Hash %d %s--", i, M[index].data);
			writelist(M[index].binding);
		}
	}
}



cellcursor      eval(cellcursor list);


/*----------0--------------*/
cellcursor 
nullp(cellcursor p)
{
	if (eval(M[M[p].cdr].car) == 0)
		return (truecell);
	else
		return (0);
}

/*----------NOT--------------*/
cellcursor 
not(cellcursor p)
{
	p = eval(M[p].car);
	if (p == 0)
		return (truecell);
	else
		return (0);
}

/*----------ATOM--------------*/
cellcursor 
atom(cellcursor p)
{
	if (M[eval(M[p].car)].node_type == ATOM)
		return (truecell);
	else
		return (0);
}

/*----------LISTP--------------*/
cellcursor 
listp(cellcursor p)
{
	if (M[eval(M[p].car)].node_type == LISTTYPE)
		return (truecell);
	else
		return (0);
}

cellcursor 
equal1(cellcursor p)
{
	cellcursor      x, y;

	x = eval(M[p].car);
	p = M[p].cdr;
	y = eval(M[p].car);
	if (M[x].node_type == ATOM && M[y].node_type == ATOM) {
		if (!strcmp(M[x].data, M[y].data))
			return (truecell);
		else
			return (0);
	} else if (M[x].node_type == LISTTYPE && M[y].node_type == LISTTYPE) {
		if (x == y)
			return (truecell);
		else
			return (0);
	} else
		return (0);
}
/*----------AND--------------*/
cellcursor 
and(cellcursor p)
{
	cellcursor      save = 0;
	int             flag = TRUE;

	while (p != 0 && flag) {
		save = eval(M[p].car);
		if (save == 0)
			flag = FALSE;
		p = M[p].cdr;
	}
	return (save);
}

/*----------OR--------------*/
cellcursor 
or(cellcursor p)
{
	cellcursor      save = 0;
	int             flag = TRUE;

	while (p != 0 && flag) {
		save = eval(M[p].car);
		if (save != 0)
			flag = FALSE;
		p = M[p].cdr;
	}
	return (save);
}
/*----------COND--------------*/

cellcursor 
cond(cellcursor p)
{
	cellcursor      save = 0;
	cellcursor      head;
	int             flag = TRUE;

	while (p != 0 && flag) {
		head = M[p].car;
		save = eval(M[head].car);
		head = M[head].cdr;
		if (save != 0) {
			flag = FALSE;
			while (head != 0) {
				save = eval(M[head].car);
				head = M[head].cdr;
			}
		}
		p = M[p].cdr;
	}
	return (save);
}

cellcursor 
quote(cellcursor list)
{
	if (list == 0) {
		printf("Error:quote on nil.\n");
		return 0;
	}
	return M[list].car;
}

cellcursor 
car(cellcursor list)
{
	cellcursor      tempcur;
	if (list == 0) {
		printf("error:car on nil.\n");
		return 0;
	} else {
		tempcur = eval(M[list].car);
		return M[tempcur].car;
	}
}
cellcursor 
cdr(cellcursor list)
{
	cellcursor      tempcur;
	if (list == 0) {
		printf("Error:cdr on nil.\n");
		return 0;
	} else {
		tempcur = eval(M[list].car);
		return M[tempcur].cdr;
	}
}


cellcursor 
setq(cellcursor list)
/* Put the symbol in the hash table and bind it to its defn. */
{
	cellcursor      tempcur;
	char           *symbol = makestring();

	tempcur = M[list].car;
	symbol = M[tempcur].data;
	tempcur = M[list].cdr;
	tempcur = M[tempcur].car;
	if (tempcur == 0) {

		printf("Error:definition missing in setq.\n");
		return 0;
	}
	tempcur = addtohashtable(symbol, eval(tempcur));
	return M[tempcur].binding;
}

cellcursor 
cons(cellcursor list)
{
	cellcursor      tempcur1, tempcur2, newcell;
	tempcur1 = eval(M[list].car);
	tempcur2 = eval(M[M[list].cdr].car);
	if ((tempcur1 == 0) || (tempcur2 == 0)) {
		printf("Error:cons - 0 pointers.\n");
		return 0;
	}
	newcell = new1();	/* Get new cell */
	M[newcell].node_type = LISTTYPE;
	M[newcell].car = tempcur1;	/* Bind car and cdr pointers */
	M[newcell].cdr = tempcur2;
	return newcell;
}
cellcursor 
copy(cellcursor list)
/*
 * Makes a copy of the list without copying the atoms. The original ones are
 * used.
 */
{
	cellcursor      tempcur;
	if (list == 0)
		return 0;
	else if (M[list].node_type == LISTTYPE) {
		tempcur = new1();
		M[tempcur].node_type = LISTTYPE;
		M[tempcur].car = copy(M[list].car);
		M[tempcur].cdr = copy(M[list].cdr);
		return tempcur;
	} else if (M[list].node_type == ATOM)
		return list;
	return 0;
}

cellcursor 
append(cellcursor list)
/* appends two lists after evaluating them. */
{
	cellcursor      tempcur1, tempcur2, newcell;
	tempcur1 = M[list].car;
	tempcur2 = M[list].cdr;
	if (tempcur1 == 0) {
		printf("Error:append- 1st argument missing.\n");
		return 0;
	} else if (tempcur2 == 0) {
		printf("Error:append-2nd list missing.\n");
		return 0;
	} else if (M[tempcur2].car == 0) {
		printf("Error:append-2nd argument missing.\n");
		return 0;
	} else {
		tempcur1 = copy(eval(tempcur1));
		newcell = tempcur1;
		tempcur2 = eval(M[tempcur2].car);
		while (M[tempcur1].cdr != 0)
			tempcur1 = M[tempcur1].cdr;
		M[tempcur1].cdr = tempcur2;
		return newcell;
	}
}

cellcursor 
eval(cellcursor list)
/*
 * The main eval function. It calls all the subsidiary functions like car ,
 * cdr etc which again call eval as required. If an atom is encountered a
 * search is done in the hash table and if found the binding of the variable
 * is returned. Else check is made for all the standard functions and those
 * are invoked accordingly. The calls are indirect recursion with many
 * functions calling eval and vice versa.
 */
{
	char           *func;
	cellcursor      tempcur, temp_res;
	if (list == 0) {
		return 0;
	} else if (M[list].node_type == ATOM) {
		tempcur = in_hashtable(M[list].data);
		if (tempcur != 0)
			temp_res = M[tempcur].binding;
		else {
			printf("Error:eval:-- %s -- not defined", M[list].data);
			return 0;
		}
	} else {
		tempcur = M[list].car;
		func = M[tempcur].data;
		if (!strcmp(func, "car"))
			temp_res = car(M[list].cdr);
		else if (!strcmp(func, "cdr"))
			temp_res = cdr(M[list].cdr);
		else if (!strcmp(func, "cons"))
			temp_res = cons(M[list].cdr);
		else if (!strcmp(func, "append"))
			temp_res = append(M[list].cdr);
		else if (!strcmp(func, "setq"))
			temp_res = setq(M[list].cdr);

		else if (!strcmp(func, "quote"))
			temp_res = quote(M[list].cdr);
		else if (!strcmp(func, "not"))
			temp_res = not(M[list].cdr);
		else if (!strcmp(func, "cond"))
			temp_res = cond(M[list].cdr);
		else if (!strcmp(func, "and"))
			temp_res = and(M[list].cdr);
		else if (!strcmp(func, "listp"))
			temp_res = listp(M[list].cdr);
		else if (!strcmp(func, "equal"))
			temp_res = equal1(M[list].cdr);
		else if (!strcmp(func, "nullp"))
			temp_res = nullp(M[list].cdr);
		else if (!strcmp(func, "atom"))
			temp_res = atom(M[list].cdr);
		else if (!strcmp(func, "or"))
			temp_res = or(M[list].cdr);
		else if (!strcmp(func, "exit"))
			exit(0);
		else {
			printf("Error:eval-undefined function:-- %s --  \n", func);
			return 0;
		}
	}
	return temp_res;
}



void 
maketruecell()
{
	cellcursor      i;
	int             j;

	i = new1();
	M[i].data = makestring();
	M[i].car = 0;
	M[i].cdr = 0;
	M[i].node_type = ATOM;
	strcpy(M[i].data, "T");

	j = addtohashtable(M[i].data, i);
	M[j].node_type = SOURCE;
	truecell = i;
}


main()
{
	cellcursor      tempcur;

	printf("\n<Lisp-Interpreter>\n");
	init(M);		/* Initialise the memory and other things.  */
	maketruecell();
	while (TRUE) {		/* Endless loop */
		fprintf(stdout, "\nLisp>");
		tempcur = readlist(gettok());	/* Read the input list */
		tempcur = eval(tempcur);	/* Evaluate it */
		writelist(tempcur);	/* Print the result */
	}
}
