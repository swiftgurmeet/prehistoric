#include  <time.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <assert.h>

#define TRUE  1
#define FALSE 0
#define MAXNUMB 12

enum  {	EH , RH , LH };       /* Equal high , right high , left high */
struct node {
        int             value; /* The number in the node */
        int             bf   ; /* balance factor         */
        struct node *   left ; /* left subtree pointer   */
        struct node *   right; /* right subtree pointer  */
};
typedef struct node  *Nodeptr; /* pointer to a node      */

int num_table[MAXNUMB] ; /* Array to store random numbers.             */
int num_comp_ins   = 0;  /* Number of comparisons while insertion.     */
int num_comp_del   = 0;  /* Number of comparisons while deletion.      */
int single_rot_ins = 0;  /* Number of single rotations while insertion.*/
int single_rot_del = 0;  /* Number of single rotations while deletion. */
int double_rot_ins = 0;  /* Number of double rotations while insertion.*/
int double_rot_del = 0;  /* Number of double rotations while deletion. */


void treeprint(Nodeptr); /* Procedure to print a tree. */
void print_num(int number)  /* Prints an int neatly */
{
	static int i = 0;

        if (!(i % 10)) printf("\n");
        printf("%10d ",number);
        i++;
}
void generate_random()
/* Generates MAXNUMB random random numbers and places
                           in the array num_table */
{
        int number;
        int  i,j;
        int dup ;            /* To check if duplicate no. is added to array. */

        printf("\nNumbers before insertion:\n");
/*	randomize();    initialize the random number generator. */
        for (i=0;i<MAXNUMB;){
                number = rand();           /* generate a random integer */
                dup = 0;
                for ( j = 0; j < MAXNUMB ; j++) /* See if already in array.*/
                   if (number == num_table[j])
                   {
                       dup = 1 ;
                       break;
                    }
                if (!dup)
                {
                   num_table[i] = number;    /*Store in array.*/
                   print_num(num_table[i]);  /* Store in file.*/
                   i++;
                 }
        }

} /* of generate random. */
/* swap : swaps two integers */
void swap(int *x,int *y)
{
        int tempint;

        tempint = *x;
        *x = *y;
        *y = tempint;
}

/* shuffle : shuffles the numbers in the num_table in random fashion */
void shuffle(int table[MAXNUMB])
{
        int i;

        for (i = 0; i < MAXNUMB; i++)
		swap(table + i, table + rand() % MAXNUMB);

		 /* Swap the element in each position with an element in a
		    random location. */
}



/* getnode : dynamically allocates a node and returns a pointer to it */
Nodeptr  getnode(void)
{
        return  (Nodeptr) malloc(sizeof(struct node));
}

/* rotateleft : balances the tree by a left rotation */
void rotateleft(Nodeptr *p)   /* Standard! */
{
        Nodeptr temp;

#ifdef DEBUG
        printf("\nInrot_lft");
#endif

	if ((*p) == NULL || (*p)->right == NULL)
        {
                printf("Error : rotate on null\n");
                exit(1);
        }
        temp = (*p)->right;
        (*p)->right = temp->left;
        temp->left = *p;
        *p = temp;
        return;
}
void rotateright(Nodeptr *p)    /* Standard! */
{
        Nodeptr temp;

#ifdef DEBUG
        printf("\nInrot_rgt");
#endif
        if ((*p)==NULL || (*p)->left == NULL)
        {
		printf("Error : rotate on null\n");
                exit(1);
        }
        temp = (*p)->left;
        (*p)->left = temp->right;
        temp->right = *p;
        *p = temp;
        return;
}

void rightbalance(Nodeptr *p,int *ptaller)
/* This procedure and leftbalance are straightforward implementation of the
   pascal procedures given in kruse's book. */
{
        Nodeptr x,w;

#ifdef DEBUG
        printf("\nIn rgtbal");
#endif
        x = (*p)->right;
        switch(x->bf){
        case RH :
                (*p)->bf = EH;
                x->bf = EH;
                rotateleft(p);
                single_rot_ins++ ;
                *ptaller = FALSE;
                break;
	case EH :                               /* Impossible case! */
                printf("val=%d\n",x->value);
                printf("Error : rightcase EH\n");
                exit(1);
        case LH :
                {
                        w = x->left;
                        switch(w->bf){
                        case EH :
                                (*p)->bf = EH ;
                                x->bf = EH ;
                                break;
                        case LH :
                                (*p)->bf = EH ;
                                x->bf = RH ;
                                break;
                        case RH :
                                (*p)->bf = LH ;
                                x->bf = EH ;
                                break;
                        }
                        w->bf = EH;
                        rotateright(&x);
                        (*p)->right = x;
                        rotateleft(p);
                        *ptaller = FALSE;
                        double_rot_ins++ ;
                        break;
                }
        }
}
void leftbalance(Nodeptr *p,int *ptaller)
{
        Nodeptr x,w;

#ifdef DEBUG
        printf("\nIn lftbal");
#endif
        x = (*p)->left;
        switch(x->bf){
        case LH :
                (*p)->bf = EH;
                x->bf = EH;
                rotateright(p);
                single_rot_ins++ ;
                *ptaller = FALSE;
                break;
        case EH :
                printf("Error : leftcase EH\n");
                exit(1);
        case RH :
                {
                        w = x->right;
                        switch(w->bf){
                        case EH :
                                (*p)->bf = EH ;
                                x->bf = EH ;
                                break;
                        case RH :
                                (*p)->bf = EH ;
                                x->bf = LH ;
                                break;
                        case LH :
                                (*p)->bf = RH ;
                                x->bf = EH ;
                                break;
                        }
                        w->bf = EH;
			rotateleft(&x);
                        (*p)->left = x;
                        rotateright(p);
                        *ptaller = FALSE;
                        double_rot_ins++ ;
                        break;
                }
        }
}

/* addtree : adds a number to a tree with root *thisptr recursively.If the
   tree becomes unbalanced then it calls procedures to balance it */
void  addtree(int in_num,Nodeptr *thisptr,int *taller)
{
        int     tallersubtree;

        if (*thisptr == NULL)
        {
                *thisptr = getnode();
                (*thisptr)->left = NULL;
                (*thisptr)->right = NULL;
                (*thisptr)->value = in_num;
                (*thisptr)->bf    = EH;
                *taller  = TRUE;
        }
        else if (in_num < (*thisptr)->value)
        {
                num_comp_ins ++;
                addtree(in_num,&((*thisptr)->left),&tallersubtree);
                if (tallersubtree)
                        switch((*thisptr)->bf){
                        case LH :
                                leftbalance(thisptr,taller);
                                break;
                        case EH :
                                (*thisptr)->bf = LH;
                                *taller = TRUE;
                                break;
                        case RH :
                                (*thisptr)->bf = EH;
                                *taller = FALSE;
                                break;
                        }
                else *taller = FALSE;
        }
        else if (in_num >(*thisptr)->value)
        {
                num_comp_ins++ ;
                addtree(in_num,&((*thisptr)->right),&tallersubtree);
                if (tallersubtree)
                        switch((*thisptr)->bf){
                        case RH :
                                rightbalance(thisptr,taller);
                                break;
                        case EH :
                                (*thisptr)->bf = RH;
                                *taller = TRUE;
                                break;
                        case LH :
                                (*thisptr)->bf = EH;
                                *taller = FALSE;
                                break;
                        }
                else *taller = FALSE;
        }
}

/* maketree : reads integers from the file rand100.txt and inserts them in
the tree with variable root as the pointer to the root node of the tree. */
Nodeptr maketree(void)
{
        int   i   ;
        Nodeptr root;
        int   taller;

        root = NULL;
        for ( i = 0; i < MAXNUMB ;i++){
                addtree(num_table[i],&root,&taller);
        }
        printf(" \n");
        return root;
}

/* Prints the tree in inorder traversal in bracketed form :
        (leftsubtree node-label rightsubtree)
*/
void treeprint(Nodeptr root)
{
        if (root != NULL)
        {
                printf("(");
                treeprint(root->left);
		printf("%d",root->value);
                treeprint(root->right);
                printf(")");
        }
}

void leftbalance2(Nodeptr *p,int *pshorter)
/* Analogous to leftbalance used for insertions. */
{
        Nodeptr x,w;

#ifdef DEBUG
        printf("\nIn lftbal2");
#endif
        x = (*p)->right;
	switch(x->bf){
        case RH :
                (*p)->bf = EH;
                x->bf = EH;
                rotateleft(p);
                single_rot_del++ ;
                *pshorter = TRUE;
                break;
        case EH :
		(*p)->bf = RH;
		x->bf = LH;
                rotateleft(p);
                single_rot_del++ ;
		*pshorter = FALSE ;
		break;
        case LH :
                {
			w = x->right;
                        if(w){ 
                                switch(w->bf){
                                case EH :
                                        (*p)->bf = EH ;
                                        x->bf = EH ;
                                        break;
                                case RH :
			        	(*p)->bf = RH ;
                                        x->bf = EH ;
                                        break;
                                case LH :
                                (*p)->bf = EH ;
			        	x->bf = LH ;
                                        break;
                                }
                                w->bf = EH;
		        	rotateright(&x);
                                (*p)->right = x;
	        		rotateleft(p);
	        		*pshorter = TRUE;
                                double_rot_del++ ;
        			break;
                        }
                }
        }
}

void rightbalance2(Nodeptr *p,int *pshorter)
/* Analogous to rightbalance used for insertion. */
{
        Nodeptr x,w;

#ifdef DEBUG
        printf("\nIn rgtbal2");
#endif
	x = (*p)->left;
        switch(x->bf){
        case LH :
                (*p)->bf = EH;
                x->bf = EH;
                rotateright(p);
                single_rot_del++ ;
                *pshorter = TRUE;
                break;
        case EH :
                (*p)->bf = LH;
                x->bf = RH;
                rotateright(p);
                single_rot_del++ ;
                *pshorter = FALSE ;
                break;
        case RH :
                {
			w = x-> left;
                        switch(w->bf){
                        case EH :
                                (*p)->bf = EH ;
                                x->bf = EH ;
                                break;
                        case RH :
                                (*p)->bf = LH ;
                                x->bf = EH ;
                                break;
                        case LH :
                                (*p)->bf = EH ;
                                x->bf = RH ;
                                break;
                        }
                        w->bf = EH;
			rotateleft(&x);
                        (*p)->left = x;
			rotateright(p);
                        double_rot_del++ ;
                        *pshorter = TRUE;
                        break;
                }
        }
}

void  deletetree(int in_num,Nodeptr *thisptr,int *pshorter)
{
        int     shortersubtree;
        Nodeptr  p,
                 q ;
        int      saveint;

#ifdef DEBUG
    printf("\nIndeltree");
#endif
    if (*thisptr == NULL)
    {
         printf("Error:delete on non existent number\n");
         exit(1);
    }
    if (in_num == (*thisptr)->value)
    {
#ifdef DEBUG
	printf("found : %d ",(*thisptr)->value );
#endif
        if ( (*thisptr)->left == NULL )
        {
           *thisptr = (*thisptr)->right;
           *pshorter = TRUE;
           return;
        }
        else if ( (*thisptr)->right == NULL )
        {
           *thisptr = (*thisptr)->left;
           *pshorter = TRUE;
           return;
        }
        else
        {
           q = *thisptr;
           p = (*thisptr)->left;
           while (p->right != NULL)
           {
               q = p ;
               p = p->right;
           }
	   swap(&p->value , &( (*thisptr)->value) );
	   treeprint(*thisptr);
           deletetree(p->value,&(*thisptr)->left,&shortersubtree);
           if (shortersubtree)
                        switch((*thisptr)->bf){
                        case LH :
                                (*thisptr)->bf = EH;
                                *pshorter = TRUE;
                                break;
                        case EH :
                                (*thisptr)->bf = RH;
                                *pshorter = FALSE;
                                break;
                        case RH :
                                leftbalance2(thisptr,pshorter);
                                break;
                        }
                else *pshorter = FALSE;
           return ;
        }
    }
    else
    {
            if (in_num < (*thisptr)->value)
            {
	       num_comp_del++ ;
#ifdef DEBUG
		printf("going lft ");
#endif
                deletetree(in_num,&((*thisptr)->left),&shortersubtree);
                if (shortersubtree)
                        switch((*thisptr)->bf){
                        case LH :
                                (*thisptr)->bf = EH;
                                *pshorter = TRUE;
                                break;
                        case EH :
                                (*thisptr)->bf = RH;
                                *pshorter = FALSE;
                                break;
                        case RH :
                                leftbalance2(thisptr,pshorter);
                                break;
                        }
                else *pshorter = FALSE;
             }
            else if (in_num > (*thisptr)->value)
            {
		  num_comp_del++ ;
#ifdef DEBUG
		  printf("Going rgt ");
#endif
                  deletetree(in_num,&((*thisptr)->right),&shortersubtree);
                  if (shortersubtree)
                          switch((*thisptr)->bf){
                          case LH :
                                  rightbalance2(thisptr,pshorter);
                                  break;
                          case EH :
                                  (*thisptr)->bf = LH;
                                  *pshorter = FALSE;
                                  break;
                          case RH :
                                  (*thisptr)->bf = EH;
                                  *pshorter = TRUE;
                                  break;
                          }
                  else *pshorter = FALSE;
              }
    }
}
void removetree(Nodeptr root)
{
     int      i ;
     int  shorter;


     shuffle(num_table);
     for ( i = 0; i< MAXNUMB; i++){
	printf("\n");
	treeprint(root);
	printf("\nDeleting %d ",num_table[i]);
        deletetree(num_table[i] , &root ,&shorter);
     }
}

main()
{
        Nodeptr root;
        FILE    *outfp;

/*        outfp = fopen("avlout.dat","w"); */

        generate_random();
        root = maketree();
        removetree(root);
        printf(" \n");
        return (0);
}
