/**
 * OS id2206
 * Lab 3 malloc.c
 *
 * Written by: Jan Nordberg & Felix Rios
 */
#include "malloc.h"
#include <stdio.h>
#define NALLOC 1024
#define QUICKLISTELEMENTS 8 /* Antal element en ny quicklist */
#define SMALLESTQUICKLIST 8 /* Units */
/* Om NRQUICKLISTS == 0 så får vi STRATEGY 1-3 */ 
typedef long Align; 
union header{
    struct{
        union header *ptr;
        size_t size; 
    } s;
    Align x;
};

typedef union header Header;

static Header base; /* Första addressen i free-listan */
static Header *freep = NULL; /* Nästa lediga plats i free-listan */ 

#ifndef NRQUICKLISTS
#define NRQUICKLISTS 0
#endif
static Header* quicklist[NRQUICKLISTS]={0}; /* En lista med QUICKLISTELEMENTS headers */

static Header *quickfit_morecore(size_t nu){
    char *cp;
    char *it;
    int i=0;
    cp = sbrk(nu*sizeof(Header)*QUICKLISTELEMENTS);
    if(cp == (char *) -1 ){/* Gör koden portabel */
        return NULL; /* Om det inte fanns disk */
    }
    for(it=cp;i<QUICKLISTELEMENTS;it+=nu*sizeof(Header), i++){
        ((Header*) it)->s.size = nu; /* Antal units */
        ((Header*) it)->s.ptr = it+(nu*sizeof(Header));
    }
    it -= nu*sizeof(Header);
    ((Header*) it)->s.ptr = NULL; /* Sista pekar på NULL */    
    return (Header *)cp;
}

static Header *morecore(size_t nu){    
    char *cp;
    Header *up;
  
    if(nu < NALLOC && STRATEGY != 4){ /* Även om man frågar efter lite så blir den minst NALLOC */
        nu = NALLOC;
    }
    cp = sbrk(nu * sizeof(Header)); /* Fråga efer mer utrymme */
    if(cp == (char *) -1 ){/* Gör koden portabel */
        return NULL; /* Om det inte fanns disk */
    }
    up = (Header *) cp; /* Castar det nya utrymmer till en Header*/
    up->s.size = nu; /* Sätter storleken på det nya utrymmet till det man frågade efter*/
    /* Kör free på den ny vunna platsen, men inte på headern för att 
     * lägga till den i freelistan 
     */
    free((void *)(up +1)); 
    return freep;
}
/* Hitta index i quicklistan */
int get_ql_index(int bytes){
    int index=0;
    int f_list_size=0;
    for (f_list_size=SMALLESTQUICKLIST;
         bytes > f_list_size ;     
         ++index, f_list_size *=2) {
    }    
    return index;
}

void *malloc(size_t nbytes){
    if(nbytes <= 0) return NULL;
    Header *p; /* Pekare till nästa lediga minnesarea */
    Header *prevp; /* Pekare till förra lediga minnesarean */
    Header *morecore(size_t); /* Funktion som allocerar mer minne */
    size_t nunits; /* Antal block som efterfrågas */
    Header * bestp = NULL; /* hålla reda på bäst hittills i bestfit */
    Header * best_prevp = NULL; /* För att kunna plocka ur freelistan i bestfit */
    int list_place = 0;
    
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header)+1;
    if((prevp = freep) == NULL){ /* Ingen free-list ännu*/
        base.s.ptr = freep = prevp = &base; /* Alla pekar på &base */
        base.s.size = 0;
    }
    
    /* Vilken freelista */
    if(4==STRATEGY && nbytes > SMALLESTQUICKLIST * (1<<(NRQUICKLISTS-1))){
        list_place=NRQUICKLISTS;
    }else{         
        /* Hitta lista */
        list_place=get_ql_index(nbytes);
    }
    
    if(STRATEGY == 4 &&        
       list_place < NRQUICKLISTS &&
       quicklist[list_place]==NULL){        
        /* Gör ny lista */
        /* Ej sista listan, som implementeras med vanlig first-fit */
        quicklist[list_place] = quickfit_morecore(nunits);
        if (quicklist[list_place] == NULL){
            return NULL;
        }
    }
    /* Fanns plats, eller i sista listan */
    if (STRATEGY == 4 && list_place < NRQUICKLISTS) {
        Header * ptr = quicklist[list_place]; 
        quicklist[list_place] = (quicklist[list_place]->s.ptr);
        return ptr+1;
    } else {/* stora firstfit listan */        
        for(p = prevp->s.ptr; ; prevp = p , p=p->s.ptr){ 
            if(p->s.size >= nunits){
                if(p->s.size == nunits){
                    /* ta bort den ur listan */
                    prevp->s.ptr = p->s.ptr;
                    freep = prevp; /* peka om nästa lediga plats till den föregående. */
                    return (void *) (p+1); /* returnera +1 för att få adress till datat, inte headern...*/
                }else{ /* Det blev plats över */
                    if (STRATEGY == 1 || STRATEGY == 4) {
                        p->s.size -= nunits; /* minska antalet platser som finns kvar */
                        p += p->s.size; /* Pekar-aritmetik. Flyttar fram den lediga positionen. */
                        p->s.size = nunits; 
                        freep = prevp; /* peka om nästa lediga plats till den föregående */  
                        return (void *)(p+1);/* Returnera +1 för att få adressen till platsen och inte headern */
                    }
                    if (STRATEGY == 2) {
                        /* är det en fin plats? */
                        if (NULL == bestp || (p->s.size - nunits) < (bestp->s.size - nunits)) { /* Inte hittat nån eller hittat en bättre! */
                            bestp = p;
                            best_prevp = prevp;            
                        }
                    }
                    if (STRATEGY == 3) {
                        /* är det en fin plats? */
                        if (NULL == bestp || (p->s.size - nunits) > (bestp->s.size - nunits)) { /* Inte hittat nån eller hittat en "sämre"! */
                            bestp = p;
                            best_prevp = prevp;            
                        }
                    }                    
                }
            }
            if(p==freep){ /* Gått igenom hela listan */
                if (NULL == bestp) { /* hittade ingen plats som duger */
                    /* Fanns ingen plats */
                    if((p = morecore(nunits)) == NULL){ /* Om all disk är slut */
                        return NULL;
                    } 
                } else { /* Vi har hittat en trevlig plats med lite mer än krävd plats */
                    bestp->s.size -= nunits; /* minska antalet platser som finns kvar */
                    bestp += bestp->s.size; /* Pekar-aritmetik. Flyttar fram den lediga positionen. */
                    bestp->s.size = nunits;
                    freep = best_prevp; /* peka om nästa lediga plats till den föregående */
                    return (void *)(bestp+1);/* Returnera +1 för att få adressen till platsen och inte headern */
                }
            }             
        }
    }
}
/* Debugfunction för att skriva ut alla freelistor */
void print_free_lists(){
    Header * ap;
    int i;
    for(i=0;i<NRQUICKLISTS;i++){
        if(quicklist[i]==NULL){
            continue;
        }
        ap=quicklist[i];
        fprintf(stderr,"QL[%d] address: 0x%x\n",i,quicklist[i]);
        while(ap!=NULL){
            fprintf(stderr,"QL[%d] - Element 0x%x pekar på: 0x%x. Size: %d\n",i,ap,ap->s.ptr,ap->s.size);
            ap = ap->s.ptr;
        }
    }
    
    fprintf(stderr,"\nVanliga freelistan\n"); 
    ap = freep;
    fprintf(stderr,"&base: 0x%x\n",&base);
    while(ap!=NULL){
        fprintf(stderr,"L - Element 0x%x pekar på: 0x%x. Size: %d\n",ap,ap->s.ptr,ap->s.size);
        ap = ap->s.ptr;
        if(ap == &base){
            break;
        }
    }    
}

void free(void *ap){    
    if(ap == NULL) return;
    Header *bp;
    Header *p;
    bp = (Header *)ap-1; /* För att peka på headern och inte det tomma utrymmet */
    int list_place = 0;
    int f_list_size;
    
    if(bp->s.size < 0){
        fprintf(stderr,"försöker freea en header som har size negativt: 0x%x\n",bp->s.size);
        return;
    }
    if(bp->s.size == 0){
        fprintf(stderr,"försöker freea en header som har size 0! 0x%x\n",bp->s.size);
        return;
    }

    /* Om det är någon av quicklistorna. OBS size antas vara antal Headers.*/        
    if(STRATEGY==4){
        if((bp->s.size - 1)*SMALLESTQUICKLIST < (SMALLESTQUICKLIST * (1<<(NRQUICKLISTS-1)))) {
            /* hitta lista */ 
            list_place = get_ql_index((bp->s.size-1)*sizeof(Header));
            bp->s.ptr = quicklist[list_place];
            quicklist[list_place] = bp;
            return;
        }
    }
    /* Hitta position är den nya platsen skall sättas in */
    for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr){
        if(p >= p->s.ptr && (bp > p || bp < p->s.ptr)){
            break;
        }
    }
    
    if(bp + bp->s.size == p->s.ptr){ /* Om granne med en övre ledig, Joina! */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    }else{
        bp->s.ptr = p->s.ptr;
    }
    if(p + p->s.size == bp){ /*Om den är granne med en undre ledig, Joina!*/
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    }else{
        p->s.ptr = bp;
    }
    freep = p;  
}

void *realloc(void *ptr, size_t  size){        
    if(ptr==NULL) return malloc(size); 
    if(ptr!=NULL && 0 == size) {    
        free(ptr);    
        return NULL;  
    }
    void * p = malloc(size);
    if (NULL==p) return NULL;
    if(size < ((((Header *)ptr)-1)->s.size-1)*sizeof(Header)){
        memcpy(p, ptr,size);
    }else{
        memcpy(p, ptr, (((Header *)(ptr)-1)->s.size-1)*sizeof(Header));
    }
    free(ptr); 
    return p;
}

