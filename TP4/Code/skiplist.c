#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "skiplist.h"

#include "rng.h"

/*<====================>*Macros*<====================>-------*/

#define tabSize __uint8_t

// #define DEBUG


/*<====================>*Structs*<====================>------*/

struct s_SkipList{
	unsigned int size;
	struct s_Node* sentinel;

	RNG rng;
} ;

typedef struct s_DoubleLink{
	struct s_Node * next;
	struct s_Node * previous;
} DoubleLink;

typedef struct s_Node{
	int val;
	tabSize level; //DoubleLink tab size
	struct s_DoubleLink *dl_tab;  //DoubleLink tab

}  Node;

/*<====================*Utility funcs*====================>*/
DoubleLink create_db(Node *next,Node *previous);
void print_debug(const char* s){
	#ifdef DEBUG
	printf("%s",s);
	#endif
	(void)s;

}

void * unwrapMalloc(size_t size){
	void * ptr = malloc(size);
	if(!ptr){
		fprintf(stderr,"error : could not initialize a list; alloc failed");
		exit(1);
	}
	else return ptr;
}

Node* create_node(int val,tabSize nb_level){

	print_debug("creating node\n");

	Node * node = unwrapMalloc(sizeof(Node));
	node->dl_tab = malloc(sizeof(DoubleLink)*nb_level);
	node->val = val;
	node->level = nb_level;
	#ifdef DEBUG
	for(int i =0; i<nb_level;i++){
		node->dl_tab[i] = create_db(NULL,NULL);
	}
	#endif

	print_debug("end creating node\n");


	return node;
}

void delete_node(Node* node){
	print_debug("deleting node\n");

	assert(node!=NULL);
	
	free(node);
	node =NULL;
}

Node * node_next(Node *node){
	return node->dl_tab[0].next;
}

Node * node_get_nth_next_node(Node *node,unsigned int n){
	assert(n<node->level);
	return node->dl_tab[n].next;
}

DoubleLink create_db(Node *next,Node *previous){
	DoubleLink db;
	db.next = next;
	db.previous = previous;
	return db;
}

Node* skiplist_node_at(const SkipList * d, unsigned int pos){
	assert(pos<=skiplist_size(d));

	Node * curs_pos = d->sentinel;
	for(unsigned int i = 0; i<pos;i++){
		curs_pos = node_next(curs_pos);
	}
	return curs_pos;
}


/*<====================*Publics funcs*====================>*/

/* >-------Create and delete funcs-------< */
SkipList* skiplist_create(int nblevels) {
	
	print_debug("creating list\n");

	//ensure continuity
	SkipList *list = unwrapMalloc(sizeof(SkipList) );//+ sizeof(struct s_Node) + sizeof(struct s_DoubleLink));
	list->size = 0;
	list->sentinel = unwrapMalloc(sizeof(struct s_Node));//(Node*)list+1;
	list->sentinel->level = nblevels;
	list->sentinel->dl_tab = unwrapMalloc( sizeof(struct s_DoubleLink)*nblevels) ;//(DoubleLink*)(list->sentinel+1);
	list->rng = rng_initialize(0x4B1DE,nblevels);

	for(int i=0;i<nblevels;i++){
		
		list->sentinel->dl_tab[i].next = list->sentinel;
		list->sentinel->dl_tab[i].previous = list->sentinel;
	}
	print_debug("end creating list\n");

	return (SkipList*)list;
}

void skiplist_delete(SkipList** d) {
	print_debug("deleting list\n");

	Node * curent = node_next((*d)->sentinel);
	while (curent != (*d)->sentinel){
		Node * to_delete = curent;
		curent = node_next(curent);
		
		delete_node(to_delete);
	}

	free(*d);
	*d =NULL;
}

SkipList* skiplist_insert(SkipList* d, int value) {//TODO
	print_debug("inserting\n");

	Node *to_insert_after [d->sentinel->level];
	Node * new_node = create_node(value,rng_get_value(&d->rng)+1);
	Node * cur_pos = d->sentinel;
	Node * next ;

	int level_pos = d->sentinel->level - 1;
	while (level_pos>=0){
		next = cur_pos->dl_tab[level_pos].next;
		if (next == d->sentinel || next->val>value){
			to_insert_after[level_pos]=cur_pos;
			level_pos--;
		}
		else if(next->val < value)
			cur_pos = next;
		else{
			print_debug("clé dupliqué\n");
			return d;
		}

	}

	for (unsigned int i=0; i<new_node->level;++i){

		to_insert_after[i]->dl_tab[i].next->dl_tab[i].previous = new_node;
		new_node->dl_tab[i].next = to_insert_after[i]->dl_tab[i].next;

		to_insert_after[i] ->dl_tab[i].next = new_node;
		new_node->dl_tab[i].previous = to_insert_after[i];
		

	}
	d->size++;
	print_debug("end inserting\n");

	return d;
}

/* >----------Infos funcs----------< */

unsigned int skiplist_size(const SkipList *d){
	return d->size;
}

int skiplist_at(const SkipList* d, unsigned int i){
	return skiplist_node_at(d,i)->val;
}

/* >----------Util funcs----------< */

void skiplist_map(const SkipList* d, ScanOperator f, void *user_data){

	Node * cur_pos = node_next(d->sentinel);
	while (cur_pos != d->sentinel){
		f(cur_pos->val,user_data);
		cur_pos = node_next(cur_pos);
	}
}